/* picoc's interface to the underlying platform. most platform-specific code
 * is in platform/platform_XX.c and platform/library_XX.c */

#include "picoc/config.h"
#include "picoc/picoc_interpreter.h"
#include "picoc/picoc_picoc.h"

static void PrintSourceTextErrorLine(IOFILE* Stream, const char* FileName, const char* SourceText, int Line,
                                     int CharacterPos);

#ifdef PICOC_DEBUGGER_ENABLE
static int gEnableDebugger = true;
#else
static int gEnableDebugger = false;
#endif

/* initialize everything */
void PicocInitialize(Picoc* pc, int stackSize, picoc_io_t* pIO) {
    memset(pc, '\0', sizeof(*pc));
    PlatformInit(pc);
    BasicIOInit(pc, pIO);
    HeapInit(pc, stackSize);
    TableInit(pc);
    VariableInit(pc);
    LexInit(pc);
    TypeInit(pc);
    IncludeInit(pc);
    LibraryInit(pc);
    PlatformLibraryInit(pc);
#ifdef PICOC_DEBUGGER_ENABLE
    DebugInit(pc);
#endif
}

/* free memory */
void PicocCleanup(Picoc* pc) {
#ifdef PICOC_DEBUGGER_ENABLE
    DebugCleanup(pc);
#endif
    IncludeCleanup(pc);
    ParseCleanup(pc);
    LexCleanup(pc);
    VariableCleanup(pc);
    TypeCleanup(pc);
    TableStrFree(pc);
    HeapCleanup(pc);
    PlatformCleanup(pc);
}

/* platform-dependent code for running programs */
#if defined(PICOC_HOST_UNIX) || defined(WIN32)

#define CALL_MAIN_NO_ARGS_RETURN_VOID "main();"
#define CALL_MAIN_WITH_ARGS_RETURN_VOID "main(__argc,__argv);"
#define CALL_MAIN_NO_ARGS_RETURN_INT "__exit_value = main();"
#define CALL_MAIN_WITH_ARGS_RETURN_INT "__exit_value = main(__argc,__argv);"

void PicocCallMain(Picoc* pc, int argc, char** argv) {
    /* check if the program wants arguments */
    struct Value* funcValue = NULL;

    if(!VariableDefined(pc, TableStrRegister(pc, "main"))) {
        ProgramFailNoParser(pc, "main() is not defined");
    }

    VariableGet(pc, NULL, TableStrRegister(pc, "main"), &funcValue);
    if(funcValue->Typ->Base != TypeFunction) {
        ProgramFailNoParser(pc, "main is not a function - can't call it");
    }

    if(funcValue->Val->FuncDef.NumParams != 0) {
        /* define the arguments */
        VariableDefinePlatformVar(pc, NULL, "__argc", &pc->IntType, (union AnyValue*)&argc, false);
        VariableDefinePlatformVar(pc, NULL, "__argv", pc->CharPtrPtrType, (union AnyValue*)&argv, false);
    }

    if(funcValue->Val->FuncDef.ReturnType == &pc->VoidType) {
        if(funcValue->Val->FuncDef.NumParams == 0) {
            PicocParse(pc, "startup", CALL_MAIN_NO_ARGS_RETURN_VOID, strlen(CALL_MAIN_NO_ARGS_RETURN_VOID), true, true,
                       false, gEnableDebugger);
        } else {
            PicocParse(pc, "startup", CALL_MAIN_WITH_ARGS_RETURN_VOID, strlen(CALL_MAIN_WITH_ARGS_RETURN_VOID), true,
                       true, false, gEnableDebugger);
        }
    } else {
        VariableDefinePlatformVar(pc, NULL, "__exit_value", &pc->IntType, (union AnyValue*)&pc->PicocExitValue, true);

        if(funcValue->Val->FuncDef.NumParams == 0) {
            PicocParse(pc, "startup", CALL_MAIN_NO_ARGS_RETURN_INT, strlen(CALL_MAIN_NO_ARGS_RETURN_INT), true, true,
                       false, gEnableDebugger);
        } else {
            PicocParse(pc, "startup", CALL_MAIN_WITH_ARGS_RETURN_INT, strlen(CALL_MAIN_WITH_ARGS_RETURN_INT), true,
                       true, false, gEnableDebugger);
        }
    }
}
#endif

void PrintSourceTextErrorLine(IOFILE* Stream, const char* FileName, const char* SourceText, int Line,
                              int CharacterPos) {
    int lineCount;
    int charCount;
    const char* linePos = NULL;
    const char* cPos = NULL;

    if(SourceText != NULL) {
        /* find the source line */
        for(linePos = SourceText, lineCount = 1; *linePos != '\0' && lineCount < Line; linePos++) {
            if(*linePos == '\n') {
                lineCount++;
            }
        }

        /* display the line */
        for(cPos = linePos; *cPos != '\n' && *cPos != '\0'; cPos++) {
            PrintCh(*cPos, Stream);
        }
        PrintCh('\n', Stream);

        /* display the error position */
        for(cPos = linePos, charCount = 0; *cPos != '\n' && *cPos != '\0' && (charCount < CharacterPos || *cPos == ' ');
            cPos++, charCount++) {
            if(*cPos == '\t') {
                PrintCh('\t', Stream);
            } else {
                PrintCh(' ', Stream);
            }
        }
    } else {
        /* assume we're in interactive mode - try to make the arrow match
            up with the input text */
        for(charCount = 0; charCount < CharacterPos + (int)strlen(PICOC_INTERACTIVE_PROMPT_STATEMENT); charCount++) {
            PrintCh(' ', Stream);
        }
    }
    PlatformPrintf(Stream, "^\n%s:%d:%d ", FileName, Line, CharacterPos);
}

/* exit with a message */
void ProgramFail(struct ParseState* Parser, const char* Message, ...) {
    va_list args = NULL;

    FILE* stream = Parser->pc->pCStdOut->pStdout;
    PrintSourceTextErrorLine(stream, Parser->FileName, Parser->SourceText, Parser->Line, Parser->CharacterPos);
    va_start(args, Message);
    PlatformVPrintf(stream, Message, args);
    va_end(args);
    PlatformPrintf(stream, "\n");
    PlatformExit(Parser->pc, 1);
}

/* exit with a message, when we're not parsing a program */
void ProgramFailNoParser(Picoc* pc, const char* Message, ...) {
    va_list args = NULL;

    FILE* stream = pc->pCStdOut->pStdout;
    va_start(args, Message);
    PlatformVPrintf(stream, Message, args);
    va_end(args);
    PlatformPrintf(stream, "\n");
    PlatformExit(pc, 1);
}

/* like ProgramFail() but gives descriptive error messages for assignment */
void AssignFail(struct ParseState* Parser, const char* Format, struct ValueType* Type1, struct ValueType* Type2,
                int Num1, int Num2, const char* FuncName, int ParamNo) {
    FILE* stream = Parser->pc->pCStdOut->pStdout;

    PrintSourceTextErrorLine(stream, Parser->FileName, Parser->SourceText, Parser->Line, Parser->CharacterPos);
    PlatformPrintf(stream, "can't %s ", (FuncName == NULL) ? "assign" : "set");

    if(Type1 != NULL) {
        PlatformPrintf(stream, Format, Type1, Type2);
    } else {
        PlatformPrintf(stream, Format, Num1, Num2);
    }

    if(FuncName != NULL) {
        PlatformPrintf(stream, " in argument %d of call to %s()", ParamNo, FuncName);
    }

    PlatformPrintf(stream, "\n");
    PlatformExit(Parser->pc, 1);
}

/* exit lexing with a message */
void LexFail(Picoc* pc, struct LexState* Lexer, const char* Message, ...) {
    va_list args = NULL;

    FILE* stream = pc->pCStdOut->pStdout;

    PrintSourceTextErrorLine(stream, Lexer->FileName, Lexer->SourceText, Lexer->Line, Lexer->CharacterPos);
    va_start(args, Message);
    PlatformVPrintf(stream, Message, args);
    va_end(args);
    PlatformPrintf(stream, "\n");
    PlatformExit(pc, 1);
}

/* printf for compiler error reporting */
void PlatformPrintf(IOFILE* Stream, const char* Format, ...) {
    va_list args = NULL;

    va_start(args, Format);
    PlatformVPrintf(Stream, Format, args);
    va_end(args);
}

void PlatformVPrintf(IOFILE* Stream, const char* Format, va_list Args) {
    const char* fPos = NULL;

    for(fPos = Format; *fPos != '\0'; fPos++) {
        if(*fPos == '%') {
            fPos++;
            switch(*fPos) {
                case 's':
                    PrintStr(va_arg(Args, char*), Stream);
                    break;
                case 'd':
                    PrintSimpleInt(va_arg(Args, int), Stream);
                    break;
                case 'c':
                    PrintCh(va_arg(Args, int), Stream);
                    break;
                case 't':
                    PrintType(va_arg(Args, struct ValueType*), Stream);
                    break;
                case 'f':
                    PrintFP(va_arg(Args, double), Stream);
                    break;
                case '%':
                    PrintCh('%', Stream);
                    break;
                case '\0':
                    fPos--;
                    break;
                default:
                    break;
            }
        } else {
            PrintCh(*fPos, Stream);
        }
    }
}

/* make a new temporary name. takes a static buffer of char [7] as a parameter.
 * should be initialized to "XX0000"
 * where XX can be any characters */
char* PlatformMakeTempName(Picoc* pc, char* TempNameBuffer) {
    int charPos = 5;

    while(charPos > 1) {
        if(TempNameBuffer[charPos] < '9') {
            TempNameBuffer[charPos]++;
            return TableStrRegister(pc, TempNameBuffer);
        }
        TempNameBuffer[charPos] = '0';
        charPos--;
    }

    return TableStrRegister(pc, TempNameBuffer);
}

void PicocVersion(char* pVersion, int maxLen) {
    size_t len = PICOC_MIN(strlen(pVersion), strlen(PROJECT_VER));
    memcpy(pVersion, PROJECT_VER, len);
    pVersion[maxLen - 1] = '\0';
}