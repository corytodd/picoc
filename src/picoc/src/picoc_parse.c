/* picoc parser - parses source and executes statements */
#include "picoc/picoc_interpreter.h"
#include "picoc/picoc_picoc.h"

static enum ParseResult ParseStatementMaybeRun(struct ParseState* Parser, int Condition, int CheckTrailingSemicolon);
static int ParseCountParams(struct ParseState* Parser);
static int ParseArrayInitializer(struct ParseState* Parser, struct Value* NewVariable, int DoAssignment);
static void ParseDeclarationAssignment(struct ParseState* Parser, struct Value* NewVariable, int DoAssignment);
static int ParseDeclaration(struct ParseState* Parser, enum LexToken Token);
static void ParseMacroDefinition(struct ParseState* Parser);
static void ParseFor(struct ParseState* Parser);
static enum RunMode ParseBlock(struct ParseState* Parser, int AbsorbOpenBrace, int Condition);
static void ParseTypedef(struct ParseState* Parser);

#ifdef PICOC_DEBUGGER_ENABLE
static int gEnableDebugger = true;
#else
static int gEnableDebugger = false;
#endif

/* deallocate any memory */
void ParseCleanup(Picoc* pc) {
    while(pc->CleanupTokenList != NULL) {
        struct CleanupTokenNode* next = pc->CleanupTokenList->Next;

        HeapFreeMem(pc, pc->CleanupTokenList->Tokens);
        if(pc->CleanupTokenList->SourceText != NULL) {
            HeapFreeMem(pc, (void*)pc->CleanupTokenList->SourceText);
        }

        HeapFreeMem(pc, pc->CleanupTokenList);
        pc->CleanupTokenList = next;
    }
}

/* parse a statement, but only run it if Condition is true */
enum ParseResult ParseStatementMaybeRun(struct ParseState* Parser, int Condition, int CheckTrailingSemicolon) {
    if(Parser->Mode != RunModeSkip && !Condition) {
        enum RunMode oldMode = Parser->Mode;
        int result = 0;
        Parser->Mode = RunModeSkip;
        result = ParseStatement(Parser, CheckTrailingSemicolon);
        Parser->Mode = oldMode;
        return (enum ParseResult)result;
    }

    return ParseStatement(Parser, CheckTrailingSemicolon);
}

/* count the number of parameters to a function or macro */
int ParseCountParams(struct ParseState* Parser) {
    int paramCount = 0;

    enum LexToken token = LexGetToken(Parser, NULL, true);
    if(token != TokenCloseBracket && token != TokenEOF) {
        /* count the number of parameters */
        paramCount++;
        while((token = LexGetToken(Parser, NULL, true)) != TokenCloseBracket && token != TokenEOF) {
            if(token == TokenComma) {
                paramCount++;
            }
        }
    }

    return paramCount;
}

/* parse a function definition and store it for later */
struct Value* ParseFunctionDefinition(struct ParseState* Parser, struct ValueType* ReturnType, char* Identifier) {
    int paramCount = 0;
    char* paramIdentifier = NULL;
    enum LexToken token = TokenNone;
    struct ValueType* paramType = NULL;
    struct ParseState paramParser;
    struct Value* funcValue = NULL;
    struct Value* oldFuncValue = NULL;
    struct ParseState funcBody;
    Picoc* pc = Parser->pc;

    if(pc->TopStackFrame != NULL) {
        ProgramFail(Parser, "nested function definitions are not allowed");
    }

    LexGetToken(Parser, NULL, true); /* open bracket */
    ParserCopy(&paramParser, Parser);
    paramCount = ParseCountParams(Parser);
    if(paramCount > PICOC_CONFIG_PARAMETER_MAX) {
        ProgramFail(Parser, "too many parameters (%d allowed)", PICOC_CONFIG_PARAMETER_MAX);
    }

    funcValue = VariableAllocValueAndData(
        pc, Parser, sizeof(struct FuncDef) + sizeof(struct ValueType*) * paramCount + sizeof(const char*) * paramCount,
        false, NULL, true);
    funcValue->Typ = &pc->FunctionType;
    funcValue->Val->FuncDef.ReturnType = ReturnType;
    funcValue->Val->FuncDef.NumParams = paramCount;
    funcValue->Val->FuncDef.VarArgs = false;
    funcValue->Val->FuncDef.ParamType = (struct ValueType**)((char*)funcValue->Val + sizeof(struct FuncDef));
    funcValue->Val->FuncDef.ParamName =
        (char**)((char*)funcValue->Val->FuncDef.ParamType + sizeof(struct ValueType*) * paramCount);

    for(paramCount = 0; paramCount < funcValue->Val->FuncDef.NumParams; paramCount++) {
        /* harvest the parameters into the function definition */
        if(paramCount == funcValue->Val->FuncDef.NumParams - 1 &&
           LexGetToken(&paramParser, NULL, false) == TokenEllipsis) {
            /* ellipsis at end */
            funcValue->Val->FuncDef.NumParams--;
            funcValue->Val->FuncDef.VarArgs = true;
            break;
        }

        /* add a parameter */
        TypeParse(&paramParser, &paramType, &paramIdentifier, NULL);
        if(paramType->Base == TypeVoid) {
            /* this isn't a real parameter at all - delete it */
            // ParamCount--;
            funcValue->Val->FuncDef.NumParams--;
        } else {
            funcValue->Val->FuncDef.ParamType[paramCount] = paramType;
            funcValue->Val->FuncDef.ParamName[paramCount] = paramIdentifier;
        }

        token = LexGetToken(&paramParser, NULL, true);
        if(token != TokenComma && paramCount < funcValue->Val->FuncDef.NumParams - 1) {
            ProgramFail(&paramParser, "comma expected");
        }
    }

    if(funcValue->Val->FuncDef.NumParams != 0 && token != TokenCloseBracket && token != TokenComma &&
       token != TokenEllipsis) {
        ProgramFail(&paramParser, "bad parameter");
    }

    if(strcmp(Identifier, "main") == 0) {
        /* make sure it's int main() */
        if(funcValue->Val->FuncDef.ReturnType != &pc->IntType && funcValue->Val->FuncDef.ReturnType != &pc->VoidType) {
            ProgramFail(Parser, "main() should return an int or void");
        }

        if(funcValue->Val->FuncDef.NumParams != 0 &&
           (funcValue->Val->FuncDef.NumParams != 2 || funcValue->Val->FuncDef.ParamType[0] != &pc->IntType)) {
            { ProgramFail(Parser, "bad parameters to main()"); }
        }
    }

    /* look for a function body */
    token = LexGetToken(Parser, NULL, false);
    if(token == TokenSemicolon) {
        LexGetToken(Parser, NULL, true); /* it's a prototype, absorb
                                           the trailing semicolon */
    } else {
        /* it's a full function definition with a body */
        if(token != TokenLeftBrace) {
            ProgramFail(Parser, "bad function definition");
        }

        ParserCopy(&funcBody, Parser);
        if(ParseStatementMaybeRun(Parser, false, true) != ParseResultOk) {
            ProgramFail(Parser, "function definition expected");
        }

        funcValue->Val->FuncDef.Body = funcBody;
        funcValue->Val->FuncDef.Body.Pos = LexCopyTokens(&funcBody, Parser);

        /* is this function already in the global table? */
        if(TableGet(&pc->GlobalTable, Identifier, &oldFuncValue, NULL, NULL, NULL)) {
            if(oldFuncValue->Val->FuncDef.Body.Pos == NULL) {
                /* override an old function prototype */
                VariableFree(pc, TableDelete(pc, &pc->GlobalTable, Identifier));
            } else {
                ProgramFail(Parser, "'%s' is already defined", Identifier);
            }
        }
    }

    if(!TableSet(pc, &pc->GlobalTable, Identifier, funcValue, (char*)Parser->FileName, Parser->Line,
                 Parser->CharacterPos)) {
        ProgramFail(Parser, "'%s' is already defined", Identifier);
    }

    return funcValue;
}

/* parse an array initializer and assign to a variable */
int ParseArrayInitializer(struct ParseState* Parser, struct Value* NewVariable, int DoAssignment) {
    int arrayIndex = 0;
    enum LexToken token = 0;
    struct Value* cValue = NULL;

    /* count the number of elements in the array */
    if(DoAssignment && Parser->Mode == RunModeRun) {
        struct ParseState countParser;
        int numElements = 0;

        ParserCopy(&countParser, Parser);
        numElements = ParseArrayInitializer(&countParser, NewVariable, false);

        if(NewVariable->Typ->Base != TypeArray) {
            AssignFail(Parser, "%t from array initializer", NewVariable->Typ, NULL, 0, 0, NULL, 0);
        }

        if(NewVariable->Typ->ArraySize == 0) {
            NewVariable->Typ = TypeGetMatching(Parser->pc, Parser, NewVariable->Typ->FromType, NewVariable->Typ->Base,
                                               numElements, NewVariable->Typ->Identifier, true);
            VariableRealloc(Parser, NewVariable, TypeSizeValue(NewVariable, false));
        }
#ifdef PICOC_DEBUG_ARRAY_INITIALIZER
        PRINT_SOURCE_POS();
        printf("array size: %d \n", NewVariable->Typ->ArraySize);
#endif
    }

    /* parse the array initializer */
    token = LexGetToken(Parser, NULL, false);
    while(token != TokenRightBrace) {
        if(LexGetToken(Parser, NULL, false) == TokenLeftBrace) {
            /* this is a sub-array initializer */
            int subArraySize = 0;
            struct Value* subArray = NewVariable;
            if(Parser->Mode == RunModeRun && DoAssignment) {
                subArraySize = TypeSize(NewVariable->Typ->FromType, NewVariable->Typ->FromType->ArraySize, true);
                subArray = VariableAllocValueFromExistingData(
                    Parser, NewVariable->Typ->FromType,
                    (union AnyValue*)(&NewVariable->Val->ArrayMem[0] + subArraySize * arrayIndex), true, NewVariable);
#ifdef PICOC_DEBUG_ARRAY_INITIALIZER
                int FullArraySize = TypeSize(NewVariable->Typ, NewVariable->Typ->ArraySize, true);
                PRINT_SOURCE_POS();
                PRINT_TYPE(NewVariable->Typ)
                printf("[%d] subarray size: %d (full: %d,%d) \n", ArrayIndex, SubArraySize, FullArraySize,
                       NewVariable->Typ->ArraySize);
#endif
                if(arrayIndex >= NewVariable->Typ->ArraySize) {
                    ProgramFail(Parser, "too many array elements");
                }
            }
            LexGetToken(Parser, NULL, true);
            ParseArrayInitializer(Parser, subArray, DoAssignment);
        } else {
            struct Value* arrayElement = NULL;

            if(Parser->Mode == RunModeRun && DoAssignment) {
                struct ValueType* elementType = NewVariable->Typ;
                int totalSize = 1;
                int elementSize = 0;

                /* int x[3][3] = {1,2,3,4} => handle it
                    just like int x[9] = {1,2,3,4} */
                while(elementType->Base == TypeArray) {
                    totalSize *= elementType->ArraySize;
                    elementType = elementType->FromType;

                    /* char x[10][10] = {"abc", "def"} => assign "abc" to
                        x[0], "def" to x[1] etc */
                    if(LexGetToken(Parser, NULL, false) == TokenStringConstant &&
                       elementType->FromType->Base == TypeChar) {
                        break;
                    }
                }
                elementSize = TypeSize(elementType, elementType->ArraySize, true);
#ifdef PICOC_DEBUG_ARRAY_INITIALIZER
                PRINT_SOURCE_POS();
                printf("[%d/%d] element size: %d (x%d) \n", ArrayIndex, TotalSize, ElementSize, ElementType->ArraySize);
#endif
                if(arrayIndex >= totalSize) {
                    ProgramFail(Parser, "too many array elements");
                }
                arrayElement = VariableAllocValueFromExistingData(
                    Parser, elementType, (union AnyValue*)(&NewVariable->Val->ArrayMem[0] + elementSize * arrayIndex),
                    true, NewVariable);
            }

            /* this is a normal expression initializer */
            if(!ExpressionParse(Parser, &cValue)) {
                ProgramFail(Parser, "expression expected");
            }

            if(Parser->Mode == RunModeRun && DoAssignment) {
                ExpressionAssign(Parser, arrayElement, cValue, false, NULL, 0, false);
                VariableStackPop(Parser, cValue);
                VariableStackPop(Parser, arrayElement);
            }
        }

        arrayIndex++;

        token = LexGetToken(Parser, NULL, false);
        if(token == TokenComma) {
            LexGetToken(Parser, NULL, true);
            token = LexGetToken(Parser, NULL, false);
        } else if(token != TokenRightBrace) {
            ProgramFail(Parser, "comma expected");
        }
    }

    if(token == TokenRightBrace) {
        LexGetToken(Parser, NULL, true);
    } else {
        ProgramFail(Parser, "'}' expected");
    }

    return arrayIndex;
}

/* assign an initial value to a variable */
void ParseDeclarationAssignment(struct ParseState* Parser, struct Value* NewVariable, int DoAssignment) {
    struct Value* cValue = NULL;

    if(LexGetToken(Parser, NULL, false) == TokenLeftBrace) {
        /* this is an array initializer */
        LexGetToken(Parser, NULL, true);
        ParseArrayInitializer(Parser, NewVariable, DoAssignment);
    } else {
        /* this is a normal expression initializer */
        if(!ExpressionParse(Parser, &cValue)) {
            ProgramFail(Parser, "expression expected");
        }

        if(Parser->Mode == RunModeRun && DoAssignment) {
            ExpressionAssign(Parser, NewVariable, cValue, false, NULL, 0, false);
            VariableStackPop(Parser, cValue);
        }
    }
}

/* declare a variable or function */
int ParseDeclaration(struct ParseState* Parser, enum LexToken Token) {
    int isStatic = false;
    int firstVisit = false;
    char* identifier;
    struct ValueType* basicType;
    struct ValueType* typ;
    struct Value* newVariable = NULL;
    Picoc* pc = Parser->pc;

    TypeParseFront(Parser, &basicType, &isStatic);
    do {
        TypeParseIdentPart(Parser, basicType, &typ, &identifier);
        if((Token != TokenVoidType && Token != TokenStructType && Token != TokenUnionType && Token != TokenEnumType) &&
           identifier == pc->StrEmpty) {
            ProgramFail(Parser, "identifier expected");
        }

        if(identifier != pc->StrEmpty) {
            /* handle function definitions */
            if(LexGetToken(Parser, NULL, false) == TokenOpenBracket) {
                ParseFunctionDefinition(Parser, typ, identifier);
                return false;
            }

            if(typ == &pc->VoidType && identifier != pc->StrEmpty) {
                ProgramFail(Parser, "can't define a void variable");
            }

            if(Parser->Mode == RunModeRun || Parser->Mode == RunModeGoto) {
                newVariable = VariableDefineButIgnoreIdentical(Parser, identifier, typ, isStatic, &firstVisit);
            }

            if(LexGetToken(Parser, NULL, false) == TokenAssign) {
                /* we're assigning an initial value */
                LexGetToken(Parser, NULL, true);
                ParseDeclarationAssignment(Parser, newVariable, !isStatic || firstVisit);
            }
        }

        Token = LexGetToken(Parser, NULL, false);
        if(Token == TokenComma) {
            LexGetToken(Parser, NULL, true);
        }
    } while(Token == TokenComma);

    return true;
}

/* parse a #define macro definition and store it for later */
void ParseMacroDefinition(struct ParseState* Parser) {
    char* macroNameStr = NULL;
    struct Value* macroName = NULL;
    struct Value* paramName = NULL;
    struct Value* macroValue = NULL;

    if(LexGetToken(Parser, &macroName, true) != TokenIdentifier) {
        ProgramFail(Parser, "identifier expected");
    }

    macroNameStr = macroName->Val->Identifier;

    if(LexRawPeekToken(Parser) == TokenOpenMacroBracket) {
        /* it's a parameterized macro, read the parameters */
        enum LexToken token = LexGetToken(Parser, NULL, true);

        struct ParseState paramParser;
        int numParams = 0;
        int paramCount = 0;

        ParserCopy(&paramParser, Parser);
        numParams = ParseCountParams(&paramParser);
        macroValue = VariableAllocValueAndData(
            Parser->pc, Parser, sizeof(struct MacroDef) + sizeof(const char*) * numParams, false, NULL, true);
        macroValue->Val->MacroDef.NumParams = numParams;
        macroValue->Val->MacroDef.ParamName = (char**)((char*)macroValue->Val + sizeof(struct MacroDef));

        token = LexGetToken(Parser, &paramName, true);

        while(token == TokenIdentifier) {
            /* store a parameter name */
            macroValue->Val->MacroDef.ParamName[paramCount++] = paramName->Val->Identifier;

            /* get the trailing comma */
            token = LexGetToken(Parser, NULL, true);
            if(token == TokenComma) {
                token = LexGetToken(Parser, &paramName, true);

            } else if(token != TokenCloseBracket) {
                ProgramFail(Parser, "comma expected");
            }
        }

        if(token != TokenCloseBracket) {
            ProgramFail(Parser, "close bracket expected");
        }
    } else {
        /* allocate a simple unparameterized macro */
        macroValue = VariableAllocValueAndData(Parser->pc, Parser, sizeof(struct MacroDef), false, NULL, true);
        macroValue->Val->MacroDef.NumParams = 0;
    }

    /* copy the body of the macro to execute later */
    ParserCopy(&macroValue->Val->MacroDef.Body, Parser);
    macroValue->Typ = &Parser->pc->MacroType;
    LexToEndOfMacro(Parser);
    macroValue->Val->MacroDef.Body.Pos = LexCopyTokens(&macroValue->Val->MacroDef.Body, Parser);

    if(!TableSet(Parser->pc, &Parser->pc->GlobalTable, macroNameStr, macroValue, (char*)Parser->FileName, Parser->Line,
                 Parser->CharacterPos)) {
        ProgramFail(Parser, "'%s' is already defined", macroNameStr);
    }
}

/* copy the entire parser state */
void ParserCopy(struct ParseState* To, struct ParseState* From) { memcpy((void*)To, (void*)From, sizeof(*To)); }

/* copy where we're at in the parsing */
void ParserCopyPos(struct ParseState* To, struct ParseState* From) {
    To->Pos = From->Pos;
    To->Line = From->Line;
    To->HashIfLevel = From->HashIfLevel;
    To->HashIfEvaluateToLevel = From->HashIfEvaluateToLevel;
    To->CharacterPos = From->CharacterPos;
}

/* parse a "for" statement */
void ParseFor(struct ParseState* Parser) {
    int condition = 0;
    struct ParseState preConditional;
    struct ParseState preIncrement;
    struct ParseState preStatement;
    struct ParseState after;

    enum RunMode oldMode = Parser->Mode;

    int prevScopeId = 0;
    int scopeId = VariableScopeBegin(Parser, &prevScopeId);

    if(LexGetToken(Parser, NULL, true) != TokenOpenBracket) {
        ProgramFail(Parser, "'(' expected");
    }

    if(ParseStatement(Parser, true) != ParseResultOk) {
        ProgramFail(Parser, "statement expected");
    }

    ParserCopyPos(&preConditional, Parser);
    if(LexGetToken(Parser, NULL, false) == TokenSemicolon) {
        condition = true;
    } else {
        condition = ExpressionParseInt(Parser);
    }

    if(LexGetToken(Parser, NULL, true) != TokenSemicolon) {
        ProgramFail(Parser, "';' expected");
    }

    ParserCopyPos(&preIncrement, Parser);
    ParseStatementMaybeRun(Parser, false, false);

    if(LexGetToken(Parser, NULL, true) != TokenCloseBracket) {
        ProgramFail(Parser, "')' expected");
    }

    ParserCopyPos(&preStatement, Parser);
    if(ParseStatementMaybeRun(Parser, condition, true) != ParseResultOk) {
        ProgramFail(Parser, "statement expected");
    }

    if(Parser->Mode == RunModeContinue && oldMode == RunModeRun) {
        Parser->Mode = RunModeRun;
    }

    ParserCopyPos(&after, Parser);

    while(condition && Parser->Mode == RunModeRun) {
        ParserCopyPos(Parser, &preIncrement);
        ParseStatement(Parser, false);

        ParserCopyPos(Parser, &preConditional);
        if(LexGetToken(Parser, NULL, false) == TokenSemicolon) {
            condition = true;
        } else {
            condition = ExpressionParseInt(Parser);
        }

        if(condition) {
            ParserCopyPos(Parser, &preStatement);
            ParseStatement(Parser, true);

            if(Parser->Mode == RunModeContinue) {
                Parser->Mode = RunModeRun;
            }
        }
    }

    if(Parser->Mode == RunModeBreak && oldMode == RunModeRun) {
        Parser->Mode = RunModeRun;
    }

    VariableScopeEnd(Parser, scopeId, prevScopeId);

    ParserCopyPos(Parser, &after);
}

/* parse a block of code and return what mode it returned in */
enum RunMode ParseBlock(struct ParseState* Parser, int AbsorbOpenBrace, int Condition) {
    int prevScopeId = 0;
    int scopeId = VariableScopeBegin(Parser, &prevScopeId);

    if(AbsorbOpenBrace && LexGetToken(Parser, NULL, true) != TokenLeftBrace) {
        ProgramFail(Parser, "'{' expected");
    }

    if(Parser->Mode == RunModeSkip || !Condition) {
        /* condition failed - skip this block instead */
        enum RunMode oldMode = Parser->Mode;
        Parser->Mode = RunModeSkip;
        while(ParseStatement(Parser, true) == ParseResultOk) {
        }
        Parser->Mode = oldMode;
    } else {
        /* just run it in its current mode */
        while(ParseStatement(Parser, true) == ParseResultOk) {
        }
    }

    if(LexGetToken(Parser, NULL, true) != TokenRightBrace) {
        ProgramFail(Parser, "'}' expected");
    }

    VariableScopeEnd(Parser, scopeId, prevScopeId);

    return Parser->Mode;
}

/* parse a typedef declaration */
void ParseTypedef(struct ParseState* Parser) {
    char* typeName;
    struct ValueType* typ;
    struct ValueType** typPtr;
    struct Value initValue;

    TypeParse(Parser, &typ, &typeName, NULL);

    if(Parser->Mode == RunModeRun) {
        typPtr = &typ;
        initValue.Typ = &Parser->pc->TypeType;
        initValue.Val = (union AnyValue*)typPtr;
        VariableDefine(Parser->pc, Parser, typeName, &initValue, NULL, false);
    }
}

/* parse a statement */
enum ParseResult ParseStatement(struct ParseState* Parser, int CheckTrailingSemicolon) {
    int condition = 0;
    enum LexToken token = 0;
    struct Value* cValue = NULL;
    struct Value* lexerValue = NULL;
    struct Value* varValue = NULL;
    struct ParseState preState;

#ifdef PICOC_DEBUGGER_ENABLE
    /* if we're debugging, check for a breakpoint */
    if(Parser->DebugMode && Parser->Mode == RunModeRun)
        DebugCheckStatement(Parser);
#endif

    /* take note of where we are and then grab a token to see what
        statement we have */
    ParserCopy(&preState, Parser);
    token = LexGetToken(Parser, &lexerValue, true);

    switch(token) {
        case TokenEOF:
            return ParseResultEOF;
        case TokenIdentifier:
            /* might be a typedef-typed variable declaration or it might
                be an expression */
            if(VariableDefined(Parser->pc, lexerValue->Val->Identifier)) {
                VariableGet(Parser->pc, Parser, lexerValue->Val->Identifier, &varValue);
                if(varValue->Typ->Base == Type_Type) {
                    *Parser = preState;
                    ParseDeclaration(Parser, token);
                    CheckTrailingSemicolon = false;
                    break;
                }
            } else {
                /* it might be a goto label */
                enum LexToken nextToken = LexGetToken(Parser, NULL, false);
                if(nextToken == TokenColon) {
                    /* declare the identifier as a goto label */
                    LexGetToken(Parser, NULL, true);
                    if(Parser->Mode == RunModeGoto && lexerValue->Val->Identifier == Parser->SearchGotoLabel) {
                        Parser->Mode = RunModeRun;
                    }
                    CheckTrailingSemicolon = false;
                    break;
                }
            }
            /* else fallthrough to expression */
            /* no break */
        case TokenAsterisk:
        case TokenAmpersand:
        case TokenIncrement:
        case TokenDecrement:
        case TokenOpenBracket:
            *Parser = preState;
            ExpressionParse(Parser, &cValue);
            if(Parser->Mode == RunModeRun) {
                VariableStackPop(Parser, cValue);
            }
            break;
        case TokenLeftBrace:
            ParseBlock(Parser, false, true);
            CheckTrailingSemicolon = false;
            break;
        case TokenIf:
            if(LexGetToken(Parser, NULL, true) != TokenOpenBracket) {
                ProgramFail(Parser, "'(' expected");
            }
            condition = ExpressionParseInt(Parser);
            if(LexGetToken(Parser, NULL, true) != TokenCloseBracket) {
                ProgramFail(Parser, "')' expected");
            }
            if(ParseStatementMaybeRun(Parser, condition, true) != ParseResultOk) {
                ProgramFail(Parser, "statement expected");
            }
            if(LexGetToken(Parser, NULL, false) == TokenElse) {
                LexGetToken(Parser, NULL, true);
                if(ParseStatementMaybeRun(Parser, !condition, true) != ParseResultOk) {
                    ProgramFail(Parser, "statement expected");
                }
            }
            CheckTrailingSemicolon = false;
            break;
        case TokenWhile: {
            struct ParseState preConditional;
            enum RunMode preMode = Parser->Mode;
            if(LexGetToken(Parser, NULL, true) != TokenOpenBracket) {
                ProgramFail(Parser, "'(' expected");
            }
            ParserCopyPos(&preConditional, Parser);
            do {
                ParserCopyPos(Parser, &preConditional);
                condition = ExpressionParseInt(Parser);
                if(LexGetToken(Parser, NULL, true) != TokenCloseBracket) {
                    ProgramFail(Parser, "')' expected");
                }
                if(ParseStatementMaybeRun(Parser, condition, true) != ParseResultOk) {
                    ProgramFail(Parser, "statement expected");
                }
                if(Parser->Mode == RunModeContinue) {
                    Parser->Mode = preMode;
                }
            } while(Parser->Mode == RunModeRun && condition);
            if(Parser->Mode == RunModeBreak) {
                Parser->Mode = preMode;
            }
            CheckTrailingSemicolon = false;
        } break;
        case TokenDo: {
            struct ParseState preStatement;
            enum RunMode preMode = Parser->Mode;
            ParserCopyPos(&preStatement, Parser);
            do {
                ParserCopyPos(Parser, &preStatement);
                if(ParseStatement(Parser, true) != ParseResultOk) {
                    ProgramFail(Parser, "statement expected");
                }
                if(Parser->Mode == RunModeContinue) {
                    Parser->Mode = preMode;
                }
                if(LexGetToken(Parser, NULL, true) != TokenWhile) {
                    ProgramFail(Parser, "'while' expected");
                }
                if(LexGetToken(Parser, NULL, true) != TokenOpenBracket) {
                    ProgramFail(Parser, "'(' expected");
                }
                condition = ExpressionParseInt(Parser);
                if(LexGetToken(Parser, NULL, true) != TokenCloseBracket) {
                    ProgramFail(Parser, "')' expected");
                }
            } while(condition && Parser->Mode == RunModeRun);
            if(Parser->Mode == RunModeBreak) {
                Parser->Mode = preMode;
            }
        } break;
        case TokenFor:
            ParseFor(Parser);
            CheckTrailingSemicolon = false;
            break;
        case TokenSemicolon:
            CheckTrailingSemicolon = false;
            break;
        case TokenIntType:
        case TokenShortType:
        case TokenCharType:
        case TokenLongType:
        case TokenFloatType:
        case TokenDoubleType:
        case TokenVoidType:
        case TokenStructType:
        case TokenUnionType:
        case TokenEnumType:
        case TokenSignedType:
        case TokenUnsignedType:
        case TokenStaticType:
        case TokenAutoType:
        case TokenRegisterType:
        case TokenExternType:
            *Parser = preState;
            CheckTrailingSemicolon = ParseDeclaration(Parser, token);
            break;
        case TokenHashDefine:
            ParseMacroDefinition(Parser);
            CheckTrailingSemicolon = false;
            break;
        case TokenHashInclude:
            if(LexGetToken(Parser, &lexerValue, true) != TokenStringConstant) {
                ProgramFail(Parser, "\"filename.h\" expected");
            }
            IncludeFile(Parser->pc, (char*)lexerValue->Val->Pointer);
            CheckTrailingSemicolon = false;
            break;
        case TokenSwitch:
            if(LexGetToken(Parser, NULL, true) != TokenOpenBracket) {
                ProgramFail(Parser, "'(' expected");
            }
            condition = ExpressionParseInt(Parser);
            if(LexGetToken(Parser, NULL, true) != TokenCloseBracket) {
                ProgramFail(Parser, "')' expected");
            }
            if(LexGetToken(Parser, NULL, false) != TokenLeftBrace) {
                ProgramFail(Parser, "'{' expected");
            }
            {
                /* new block so we can store parser state */
                enum RunMode oldMode = Parser->Mode;
                int oldSearchLabel = Parser->SearchLabel;
                Parser->Mode = RunModeCaseSearch;
                Parser->SearchLabel = condition;
                ParseBlock(Parser, true, (oldMode != RunModeSkip) && (oldMode != RunModeReturn));
                if(Parser->Mode != RunModeReturn) {
                    Parser->Mode = oldMode;
                }
                Parser->SearchLabel = oldSearchLabel;
            }
            CheckTrailingSemicolon = false;
            break;
        case TokenCase:
            if(Parser->Mode == RunModeCaseSearch) {
                Parser->Mode = RunModeRun;
                condition = ExpressionParseInt(Parser);
                Parser->Mode = RunModeCaseSearch;
            } else {
                condition = ExpressionParseInt(Parser);
            }
            if(LexGetToken(Parser, NULL, true) != TokenColon) {
                ProgramFail(Parser, "':' expected");
            }
            if(Parser->Mode == RunModeCaseSearch && condition == Parser->SearchLabel) {
                Parser->Mode = RunModeRun;
            }
            CheckTrailingSemicolon = false;
            break;
        case TokenDefault:
            if(LexGetToken(Parser, NULL, true) != TokenColon) {
                ProgramFail(Parser, "':' expected");
            }
            if(Parser->Mode == RunModeCaseSearch) {
                Parser->Mode = RunModeRun;
            }
            CheckTrailingSemicolon = false;
            break;
        case TokenBreak:
            if(Parser->Mode == RunModeRun) {
                Parser->Mode = RunModeBreak;
            }
            break;
        case TokenContinue:
            if(Parser->Mode == RunModeRun) {
                Parser->Mode = RunModeContinue;
            }
            break;
        case TokenReturn:
            if(Parser->Mode == RunModeRun) {
                if(!Parser->pc->TopStackFrame || Parser->pc->TopStackFrame->ReturnValue->Typ->Base != TypeVoid) {
                    if(!ExpressionParse(Parser, &cValue)) {
                        ProgramFail(Parser, "value required in return");
                    }
                    if(!Parser->pc->TopStackFrame) { /* return from top-level program? */
                        PlatformExit(Parser->pc, ExpressionCoerceInteger(cValue));
                    } else {
                        ExpressionAssign(Parser, Parser->pc->TopStackFrame->ReturnValue, cValue, true, NULL, 0, false);
                    }
                    VariableStackPop(Parser, cValue);
                } else {
                    if(ExpressionParse(Parser, &cValue)) {
                        ProgramFail(Parser, "value in return from a void function");
                    }
                }
                Parser->Mode = RunModeReturn;
            } else {
                ExpressionParse(Parser, &cValue);
            }
            break;
        case TokenTypedef:
            ParseTypedef(Parser);
            break;
        case TokenGoto:
            if(LexGetToken(Parser, &lexerValue, true) != TokenIdentifier) {
                ProgramFail(Parser, "identifier expected");
            }
            if(Parser->Mode == RunModeRun) {
                /* start scanning for the goto label */
                Parser->SearchGotoLabel = lexerValue->Val->Identifier;
                Parser->Mode = RunModeGoto;
            }
            break;
        case TokenDelete: {
            /* try it as a function or variable name to delete */
            if(LexGetToken(Parser, &lexerValue, true) != TokenIdentifier) {
                ProgramFail(Parser, "identifier expected");
            }
            if(Parser->Mode == RunModeRun) {
                /* delete this variable or function */
                cValue = TableDelete(Parser->pc, &Parser->pc->GlobalTable, lexerValue->Val->Identifier);
                if(cValue == NULL) {
                    ProgramFail(Parser, "'%s' is not defined", lexerValue->Val->Identifier);
                }

                VariableFree(Parser->pc, cValue);
            }
            break;
        }
        default:
            *Parser = preState;
            return ParseResultError;
    }

    if(CheckTrailingSemicolon) {
        if(LexGetToken(Parser, NULL, true) != TokenSemicolon) {
            ProgramFail(Parser, "';' expected");
        }
    }

    return ParseResultOk;
}

/* quick scan a source file for definitions */
void PicocParse(Picoc* pc, const char* FileName, const char* Source, int SourceLen, int RunIt, int CleanupNow,
                int CleanupSource, int EnableDebugger) {
    char* regFileName = TableStrRegister(pc, FileName);
    enum ParseResult ok = 0;
    struct ParseState parser;
    struct CleanupTokenNode* newCleanupNode = NULL;

    void* tokens = LexAnalyse(pc, regFileName, Source, SourceLen, NULL);

    /* allocate a cleanup node so we can clean up the tokens later */
    if(!CleanupNow) {
        newCleanupNode = HeapAllocMem(pc, sizeof(struct CleanupTokenNode));
        if(newCleanupNode == NULL) {
            ProgramFailNoParser(pc, "(PicocParse) out of memory");
        }

        newCleanupNode->Tokens = tokens;
        if(CleanupSource) {
            newCleanupNode->SourceText = Source;
        } else {
            newCleanupNode->SourceText = NULL;
        }

        newCleanupNode->Next = pc->CleanupTokenList;
        pc->CleanupTokenList = newCleanupNode;
    }

    /* do the parsing */
    LexInitParser(&parser, pc, Source, tokens, regFileName, RunIt, EnableDebugger);

    do {
        ok = ParseStatement(&parser, true);
    } while(ok == ParseResultOk);

    if(ok == ParseResultError) {
        ProgramFail(&parser, "parse error");
    }

    /* clean up */
    if(CleanupNow) {
        HeapFreeMem(pc, tokens);
    }
}

/* parse interactively */
void PicocParseInteractiveNoStartPrompt(Picoc* pc, int EnableDebugger) {
    enum ParseResult ok = 0;
    struct ParseState parser;

    LexInitParser(&parser, pc, NULL, NULL, pc->StrEmpty, true, EnableDebugger);
    PicocPlatformSetExitPoint(pc);
    LexInteractiveClear(pc, &parser);

    do {
        LexInteractiveStatementPrompt(pc);
        ok = ParseStatement(&parser, true);
        LexInteractiveCompleted(pc, &parser);

    } while(ok == ParseResultOk);

    if(ok == ParseResultError) {
        ProgramFail(&parser, "parse error");
    }

    PlatformPrintf(pc->pCStdOut->pStdout, "\n");
}

/* parse interactively, showing a startup message */
void PicocParseInteractive(Picoc* pc) {
    PlatformPrintf(pc->pCStdOut->pStdout, PICOC_INTERACTIVE_PROMPT_START);
    PicocParseInteractiveNoStartPrompt(pc, gEnableDebugger);
}
