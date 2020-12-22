/* picoc data type module. This manages a tree of data types and has facilities
 * for parsing data types. */

#include "picoc/picoc_interpreter.h"

static struct ValueType* TypeAdd(Picoc* pc, struct ParseState* Parser, struct ValueType* ParentType, enum BaseType Base,
                                 int ArraySize, const char* Identifier, int Sizeof, int AlignBytes);
static void TypeAddBaseType(Picoc* pc, struct ValueType* TypeNode, enum BaseType Base, int Sizeof, int AlignBytes);
static void TypeCleanupNode(Picoc* pc, struct ValueType* Typ);
static void TypeParseStruct(struct ParseState* Parser, struct ValueType** Typ, int IsStruct);
static void TypeParseEnum(struct ParseState* Parser, struct ValueType** Typ);
static struct ValueType* TypeParseBack(struct ParseState* Parser, struct ValueType* FromType);

/* some basic types */
static int PointerAlignBytes;
static int IntAlignBytes;

/* add a new type to the set of types we know about */
struct ValueType* TypeAdd(Picoc* pc, struct ParseState* Parser, struct ValueType* ParentType, enum BaseType Base,
                          int ArraySize, const char* Identifier, int Sizeof, int AlignBytes) {
    struct ValueType* newType = VariableAlloc(pc, Parser, sizeof(struct ValueType), true);
    newType->Base = Base;
    newType->ArraySize = ArraySize;
    newType->Sizeof = Sizeof;
    newType->AlignBytes = AlignBytes;
    newType->Identifier = Identifier;
    newType->Members = NULL;
    newType->FromType = ParentType;
    newType->DerivedTypeList = NULL;
    newType->OnHeap = true;
    newType->Next = ParentType->DerivedTypeList;
    ParentType->DerivedTypeList = newType;

    return newType;
}

/* given a parent type, get a matching derived type and make one if necessary.
 * Identifier should be registered with the shared string table. */
struct ValueType* TypeGetMatching(Picoc* pc, struct ParseState* Parser, struct ValueType* ParentType,
                                  enum BaseType Base, int ArraySize, const char* Identifier, int AllowDuplicates) {
    int Sizeof = 0;
    int alignBytes = 0;
    struct ValueType* thisType = ParentType->DerivedTypeList;
    while(thisType != NULL &&
          (thisType->Base != Base || thisType->ArraySize != ArraySize || thisType->Identifier != Identifier)) {
        thisType = thisType->Next;
    }

    if(thisType != NULL) {
        if(AllowDuplicates) {
            return thisType;
        }
        ProgramFail(Parser, "data type '%s' is already defined", Identifier);
    }

    switch(Base) {
        case TypePointer:
            Sizeof = sizeof(void*);
            alignBytes = PointerAlignBytes;
            break;
        case TypeArray:
            Sizeof = ArraySize * ParentType->Sizeof;
            alignBytes = ParentType->AlignBytes;
            break;
        case TypeEnum:
            Sizeof = sizeof(int);
            alignBytes = IntAlignBytes;
            break;
        default:
            Sizeof = 0;
            alignBytes = 0;
            break; /* structs and unions will get bigger
                       when we add members to them */
    }

    return TypeAdd(pc, Parser, ParentType, Base, ArraySize, Identifier, Sizeof, alignBytes);
}

/* stack space used by a value */
int TypeStackSizeValue(struct Value* Val) {
    if(Val != NULL && Val->ValOnStack) {
        return TypeSizeValue(Val, false);
    }
    return 0;
}

/* memory used by a value */
int TypeSizeValue(struct Value* Val, int Compact) {
    if(IS_INTEGER_NUMERIC(Val) && !Compact) {
        return sizeof(PICOC_ALIGN_TYPE); /* allow some extra room for type extension */
    }
    if(Val->Typ->Base != TypeArray) {
        return Val->Typ->Sizeof;
    }
    return Val->Typ->FromType->Sizeof * Val->Typ->ArraySize;
}

/* memory used by a variable given its type and array size */
int TypeSize(struct ValueType* Typ, int ArraySize, int Compact) {
    if(IS_INTEGER_NUMERIC_TYPE(Typ) && !Compact) {
        return sizeof(PICOC_ALIGN_TYPE); /* allow some extra room for type extension */
    }
    if(Typ->Base != TypeArray) {
        return Typ->Sizeof;
    }
    return Typ->FromType->Sizeof * ArraySize;
}

/* add a base type */
void TypeAddBaseType(Picoc* pc, struct ValueType* TypeNode, enum BaseType Base, int Sizeof, int AlignBytes) {
    TypeNode->Base = Base;
    TypeNode->ArraySize = 0;
    TypeNode->Sizeof = Sizeof;
    TypeNode->AlignBytes = AlignBytes;
    TypeNode->Identifier = pc->StrEmpty;
    TypeNode->Members = NULL;
    TypeNode->FromType = NULL;
    TypeNode->DerivedTypeList = NULL;
    TypeNode->OnHeap = false;
    TypeNode->Next = pc->UberType.DerivedTypeList;
    pc->UberType.DerivedTypeList = TypeNode;
}

/* initialize the type system */
void TypeInit(Picoc* pc) {
    struct IntAlign {
        char x;
        int y;
    } ia;
    struct ShortAlign {
        char x;
        short y;
    } sa;
    struct CharAlign {
        char x;
        char y;
    } ca;
    struct LongAlign {
        char x;
        long y;
    } la;
    struct DoubleAlign {
        char x;
        double y;
    } da;
    struct PointerAlign {
        char x;
        void* y;
    } pa;

    IntAlignBytes = (char*)&ia.y - &ia.x;
    PointerAlignBytes = (char*)&pa.y - &pa.x;

    pc->UberType.DerivedTypeList = NULL;
    TypeAddBaseType(pc, &pc->IntType, TypeInt, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->ShortType, TypeShort, sizeof(short), (char*)&sa.y - &sa.x);
    TypeAddBaseType(pc, &pc->CharType, TypeChar, sizeof(char), (char*)&ca.y - &ca.x);
    TypeAddBaseType(pc, &pc->LongType, TypeLong, sizeof(long), (char*)&la.y - &la.x);
    TypeAddBaseType(pc, &pc->UnsignedIntType, TypeUnsignedInt, sizeof(unsigned int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->UnsignedShortType, TypeUnsignedShort, sizeof(unsigned short), (char*)&sa.y - &sa.x);
    TypeAddBaseType(pc, &pc->UnsignedLongType, TypeUnsignedLong, sizeof(unsigned long), (char*)&la.y - &la.x);
    TypeAddBaseType(pc, &pc->UnsignedCharType, TypeUnsignedChar, sizeof(unsigned char), (char*)&ca.y - &ca.x);
    TypeAddBaseType(pc, &pc->VoidType, TypeVoid, 0, 1);
    TypeAddBaseType(pc, &pc->FunctionType, TypeFunction, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->MacroType, TypeMacro, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->GotoLabelType, TypeGotoLabel, 0, 1);
    TypeAddBaseType(pc, &pc->FPType, TypeFP, sizeof(double), (char*)&da.y - &da.x);
    TypeAddBaseType(pc, &pc->TypeType, Type_Type, sizeof(double),
                    (char*)&da.y - &da.x); /* must be large enough to cast to a double */
    pc->CharArrayType =
        TypeAdd(pc, NULL, &pc->CharType, TypeArray, 0, pc->StrEmpty, sizeof(char), (char*)&ca.y - &ca.x);
    pc->CharPtrType = TypeAdd(pc, NULL, &pc->CharType, TypePointer, 0, pc->StrEmpty, sizeof(void*), PointerAlignBytes);
    pc->CharPtrPtrType =
        TypeAdd(pc, NULL, pc->CharPtrType, TypePointer, 0, pc->StrEmpty, sizeof(void*), PointerAlignBytes);
    pc->VoidPtrType = TypeAdd(pc, NULL, &pc->VoidType, TypePointer, 0, pc->StrEmpty, sizeof(void*), PointerAlignBytes);
}

/* deallocate heap-allocated types */
void TypeCleanupNode(Picoc* pc, struct ValueType* Typ) {
    struct ValueType* subType;
    struct ValueType* nextSubType;

    /* clean up and free all the sub-nodes */
    for(subType = Typ->DerivedTypeList; subType != NULL; subType = nextSubType) {
        nextSubType = subType->Next;
        TypeCleanupNode(pc, subType);
        if(subType->OnHeap) {
            /* if it's a struct or union deallocate all the member values */
            if(subType->Members != NULL) {
                VariableTableCleanup(pc, subType->Members);
                HeapFreeMem(pc, subType->Members);
            }

            /* free this node */
            HeapFreeMem(pc, subType);
        }
    }
}

void TypeCleanup(Picoc* pc) { TypeCleanupNode(pc, &pc->UberType); }

/* parse a struct or union declaration */
void TypeParseStruct(struct ParseState* Parser, struct ValueType** Typ, int IsStruct) {
    char* memberIdentifier = NULL;
    char* structIdentifier = NULL;
    enum LexToken token = 0;
    int alignBoundary = 0;
    struct Value* memberValue = NULL;
    Picoc* pc = Parser->pc;
    struct Value* lexValue = NULL;
    struct ValueType* memberType = NULL;

    token = LexGetToken(Parser, &lexValue, false);
    if(token == TokenIdentifier) {
        LexGetToken(Parser, &lexValue, true);
        structIdentifier = lexValue->Val->Identifier;
        token = LexGetToken(Parser, NULL, false);
    } else {
        static char tempNameBuf[7] = "^s0000";
        structIdentifier = PlatformMakeTempName(pc, tempNameBuf);
    }

    *Typ = TypeGetMatching(pc, Parser, &Parser->pc->UberType, IsStruct ? TypeStruct : TypeUnion, 0, structIdentifier,
                           true);
    if(token == TokenLeftBrace && (*Typ)->Members != NULL) {
        ProgramFail(Parser, "data type '%t' is already defined", *Typ);
    }

    token = LexGetToken(Parser, NULL, false);
    if(token != TokenLeftBrace) {
        /* use the already defined structure */
#if 0
        if ((*Typ)->Members == NULL)
            ProgramFail(Parser, "structure '%s' isn't defined",
                LexValue->Val->Identifier);
#endif
        return;
    }

    if(pc->TopStackFrame != NULL) {
        ProgramFail(Parser, "struct/union definitions can only be globals");
    }

    LexGetToken(Parser, NULL, true);
    (*Typ)->Members = VariableAlloc(
        pc, Parser, sizeof(struct Table) + PICOC_CONFIG_STRUCT_TABLE_SIZE * sizeof(struct TableEntry), true);
    (*Typ)->Members->HashTable = (struct TableEntry**)((char*)(*Typ)->Members + sizeof(struct Table));
    TableInitTable((*Typ)->Members, (struct TableEntry**)((char*)(*Typ)->Members + sizeof(struct Table)),
                   PICOC_CONFIG_STRUCT_TABLE_SIZE, true);

    do {
        TypeParse(Parser, &memberType, &memberIdentifier, NULL);
        if(memberType == NULL || memberIdentifier == NULL) {
            ProgramFail(Parser, "invalid type in struct");
        }

        memberValue = VariableAllocValueAndData(pc, Parser, sizeof(int), false, NULL, true);
        memberValue->Typ = memberType;
        if(IsStruct) {
            /* allocate this member's location in the struct */
            alignBoundary = memberValue->Typ->AlignBytes;
            if(((*Typ)->Sizeof & (alignBoundary - 1)) != 0) {
                (*Typ)->Sizeof += alignBoundary - ((*Typ)->Sizeof & (alignBoundary - 1));
            }

            memberValue->Val->Integer = (*Typ)->Sizeof;
            (*Typ)->Sizeof += TypeSizeValue(memberValue, true);
        } else {
            /* union members always start at 0, make sure it's big enough
                to hold the largest member */
            memberValue->Val->Integer = 0;
            if(memberValue->Typ->Sizeof > (*Typ)->Sizeof) {
                (*Typ)->Sizeof = TypeSizeValue(memberValue, true);
            }
        }

        /* make sure to align to the size of the largest member's alignment */
        if((*Typ)->AlignBytes < memberValue->Typ->AlignBytes) {
            (*Typ)->AlignBytes = memberValue->Typ->AlignBytes;
        }

        /* define it */
        if(!TableSet(pc, (*Typ)->Members, memberIdentifier, memberValue, Parser->FileName, Parser->Line,
                     Parser->CharacterPos)) {
            ProgramFail(Parser, "member '%s' already defined", &memberIdentifier);
        }

        if(LexGetToken(Parser, NULL, true) != TokenSemicolon) {
            ProgramFail(Parser, "semicolon expected");
        }

    } while(LexGetToken(Parser, NULL, false) != TokenRightBrace);

    /* now align the structure to the size of its largest member's alignment */
    alignBoundary = (*Typ)->AlignBytes;
    if(((*Typ)->Sizeof & (alignBoundary - 1)) != 0) {
        (*Typ)->Sizeof += alignBoundary - ((*Typ)->Sizeof & (alignBoundary - 1));
    }

    LexGetToken(Parser, NULL, true);
}

/* create a system struct which has no user-visible members */
struct ValueType* TypeCreateOpaqueStruct(Picoc* pc, struct ParseState* Parser, const char* StructName, int Size) {
    struct ValueType* typ = TypeGetMatching(pc, Parser, &pc->UberType, TypeStruct, 0, StructName, false);

    /* create the (empty) table */
    typ->Members = VariableAlloc(
        pc, Parser, sizeof(struct Table) + PICOC_CONFIG_STRUCT_TABLE_SIZE * sizeof(struct TableEntry), true);
    typ->Members->HashTable = (struct TableEntry**)((char*)typ->Members + sizeof(struct Table));
    TableInitTable(typ->Members, (struct TableEntry**)((char*)typ->Members + sizeof(struct Table)),
                   PICOC_CONFIG_STRUCT_TABLE_SIZE, true);
    typ->Sizeof = Size;

    return typ;
}

/* parse an enum declaration */
void TypeParseEnum(struct ParseState* Parser, struct ValueType** Typ) {
    int enumValue = 0;
    char* enumIdentifier = NULL;
    enum LexToken token = 0;
    struct Value* lexValue = NULL;
    struct Value initValue;
    Picoc* pc = Parser->pc;

    token = LexGetToken(Parser, &lexValue, false);
    if(token == TokenIdentifier) {
        LexGetToken(Parser, &lexValue, true);
        enumIdentifier = lexValue->Val->Identifier;
        token = LexGetToken(Parser, NULL, false);
    } else {
        static char tempNameBuf[7] = "^e0000";
        enumIdentifier = PlatformMakeTempName(pc, tempNameBuf);
    }

    TypeGetMatching(pc, Parser, &pc->UberType, TypeEnum, 0, enumIdentifier, token != TokenLeftBrace);
    *Typ = &pc->IntType;
    if(token != TokenLeftBrace) {
        /* use the already defined enum */
        if((*Typ)->Members == NULL) {
            ProgramFail(Parser, "enum '%s' isn't defined", enumIdentifier);
        }

        return;
    }

    if(pc->TopStackFrame != NULL) {
        ProgramFail(Parser, "enum definitions can only be globals");
    }

    LexGetToken(Parser, NULL, true);
    (*Typ)->Members = &pc->GlobalTable;
    memset((void*)&initValue, '\0', sizeof(struct Value));
    initValue.Typ = &pc->IntType;
    initValue.Val = (union AnyValue*)&enumValue;
    do {
        if(LexGetToken(Parser, &lexValue, true) != TokenIdentifier) {
            ProgramFail(Parser, "identifier expected");
        }

        enumIdentifier = lexValue->Val->Identifier;
        if(LexGetToken(Parser, NULL, false) == TokenAssign) {
            LexGetToken(Parser, NULL, true);
            enumValue = ExpressionParseInt(Parser);
        }

        VariableDefine(pc, Parser, enumIdentifier, &initValue, NULL, false);

        token = LexGetToken(Parser, NULL, true);
        if(token != TokenComma && token != TokenRightBrace) {
            ProgramFail(Parser, "comma expected");
        }

        enumValue++;
    } while(token == TokenComma);
}

/* parse a type - just the basic type */
int TypeParseFront(struct ParseState* Parser, struct ValueType** Typ, int* IsStatic) {
    int isUnsigned = false;
    int isStaticQualifier = false;
    enum LexToken token = 0;
    struct ParseState before;
    struct Value* lexerValue = NULL;
    struct Value* varValue = NULL;
    Picoc* pc = Parser->pc;
    *Typ = NULL;

    /* ignore leading type qualifiers */
    ParserCopy(&before, Parser);
    token = LexGetToken(Parser, &lexerValue, true);
    while(token == TokenStaticType || token == TokenAutoType || token == TokenRegisterType ||
          token == TokenExternType) {
        if(token == TokenStaticType) {
            isStaticQualifier = true;
        }

        token = LexGetToken(Parser, &lexerValue, true);
    }

    if(IsStatic != NULL) {
        *IsStatic = isStaticQualifier;
    }

    /* handle signed/unsigned with no trailing type */
    if(token == TokenSignedType || token == TokenUnsignedType) {
        enum LexToken followToken = LexGetToken(Parser, &lexerValue, false);
        isUnsigned = (token == TokenUnsignedType);

        if(followToken != TokenIntType && followToken != TokenLongType && followToken != TokenShortType &&
           followToken != TokenCharType) {
            if(token == TokenUnsignedType) {
                *Typ = &pc->UnsignedIntType;
            } else {
                *Typ = &pc->IntType;
            }

            return true;
        }

        token = LexGetToken(Parser, &lexerValue, true);
    }

    switch(token) {
        case TokenIntType:
            *Typ = isUnsigned ? &pc->UnsignedIntType : &pc->IntType;
            break;
        case TokenShortType:
            *Typ = isUnsigned ? &pc->UnsignedShortType : &pc->ShortType;
            break;
        case TokenCharType:
            *Typ = isUnsigned ? &pc->UnsignedCharType : &pc->CharType;
            break;
        case TokenLongType:
            *Typ = isUnsigned ? &pc->UnsignedLongType : &pc->LongType;
            break;
        case TokenFloatType:
        case TokenDoubleType:
            *Typ = &pc->FPType;
            break;
        case TokenVoidType:
            *Typ = &pc->VoidType;
            break;
        case TokenStructType:
        case TokenUnionType:
            if(*Typ != NULL) {
                ProgramFail(Parser, "bad type declaration");
            }
            TypeParseStruct(Parser, Typ, token == TokenStructType);
            break;
        case TokenEnumType:
            if(*Typ != NULL) {
                ProgramFail(Parser, "bad type declaration");
            }

            TypeParseEnum(Parser, Typ);
            break;
        case TokenIdentifier:
            /* we already know it's a typedef-defined type because we got here */
            VariableGet(pc, Parser, lexerValue->Val->Identifier, &varValue);
            *Typ = varValue->Val->Typ;
            break;

        default:
            ParserCopy(Parser, &before);
            return false;
    }

    return true;
}

/* parse a type - the part at the end after the identifier. eg.
    array specifications etc. */
struct ValueType* TypeParseBack(struct ParseState* Parser, struct ValueType* FromType) {
    enum LexToken token = 0;
    struct ParseState before;

    ParserCopy(&before, Parser);
    token = LexGetToken(Parser, NULL, true);
    if(token == TokenLeftSquareBracket) {
        /* add another array bound */
        if(LexGetToken(Parser, NULL, false) == TokenRightSquareBracket) {
            /* an unsized array */
            LexGetToken(Parser, NULL, true);
            return TypeGetMatching(Parser->pc, Parser, TypeParseBack(Parser, FromType), TypeArray, 0,
                                   Parser->pc->StrEmpty, true);
        }

        /* get a numeric array size */
        enum RunMode oldMode = Parser->Mode;
        int arraySize = 0;
        Parser->Mode = RunModeRun;
        arraySize = ExpressionParseInt(Parser);
        Parser->Mode = oldMode;

        if(LexGetToken(Parser, NULL, true) != TokenRightSquareBracket) {
            ProgramFail(Parser, "']' expected");
        }

        return TypeGetMatching(Parser->pc, Parser, TypeParseBack(Parser, FromType), TypeArray, arraySize,
                               Parser->pc->StrEmpty, true);
    }

    /* the type specification has finished */
    ParserCopy(Parser, &before);
    return FromType;
}

/* parse a type - the part which is repeated with each
    identifier in a declaration list */
void TypeParseIdentPart(struct ParseState* Parser, struct ValueType* BasicTyp, struct ValueType** Typ,
                        char** Identifier) {
    int done = false;
    enum LexToken token = 0;
    struct Value* lexValue = NULL;
    struct ParseState before;
    *Typ = BasicTyp;
    *Identifier = Parser->pc->StrEmpty;

    while(!done) {
        ParserCopy(&before, Parser);
        token = LexGetToken(Parser, &lexValue, true);
        switch(token) {
            case TokenOpenBracket:
                if(*Typ != NULL) {
                    ProgramFail(Parser, "bad type declaration");
                }

                TypeParse(Parser, Typ, Identifier, NULL);
                if(LexGetToken(Parser, NULL, true) != TokenCloseBracket) {
                    ProgramFail(Parser, "')' expected");
                }
                break;

            case TokenAsterisk:
                if(*Typ == NULL) {
                    ProgramFail(Parser, "bad type declaration");
                }

                *Typ = TypeGetMatching(Parser->pc, Parser, *Typ, TypePointer, 0, Parser->pc->StrEmpty, true);
                break;

            case TokenIdentifier:
                if(*Typ == NULL || *Identifier != Parser->pc->StrEmpty) {
                    ProgramFail(Parser, "bad type declaration");
                }

                *Identifier = lexValue->Val->Identifier;
                done = true;
                break;

            default:
                ParserCopy(Parser, &before);
                done = true;
                break;
        }
    }

    if(*Typ == NULL) {
        ProgramFail(Parser, "bad type declaration");
    }

    if(*Identifier != Parser->pc->StrEmpty) {
        /* parse stuff after the identifier */
        *Typ = TypeParseBack(Parser, *Typ);
    }
}

/* parse a type - a complete declaration including identifier */
void TypeParse(struct ParseState* Parser, struct ValueType** Typ, char** Identifier, int* IsStatic) {
    struct ValueType* basicType = NULL;

    TypeParseFront(Parser, &basicType, IsStatic);
    TypeParseIdentPart(Parser, basicType, Typ, Identifier);
}

/* check if a type has been fully defined - otherwise it's
    just a forward declaration */
int TypeIsForwardDeclared(struct ParseState* Parser, struct ValueType* Typ) {
    if(Typ->Base == TypeArray) {
        return TypeIsForwardDeclared(Parser, Typ->FromType);
    }

    if((Typ->Base == TypeStruct || Typ->Base == TypeUnion) && Typ->Members == NULL) {
        return true;
    }

    return false;
}
