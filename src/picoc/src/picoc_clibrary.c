/*  */
#include "picoc/picoc_interpreter.h"
#include "picoc/picoc_picoc.h"

/* endian-ness checking */
static const int __ENDIAN_CHECK__ = 1;
// TODO make endianess a field in PC to avoid storing state
static int BigEndian;
static int LittleEndian;

/* global initialisation for libraries */
void LibraryInit(Picoc* pc) {

    /* define the version number macro */
    pc->VersionString = TableStrRegister(pc, PICOC_VERSION);
    VariableDefinePlatformVar(pc, NULL, "PICOC_VERSION", pc->CharPtrType, (union AnyValue*)&pc->VersionString, false);

    /* define endian-ness macros */
    BigEndian = ((*(char*)&__ENDIAN_CHECK__) == 0);
    LittleEndian = ((*(char*)&__ENDIAN_CHECK__) == 1);

    VariableDefinePlatformVar(pc, NULL, "BIG_ENDIAN", &pc->IntType, (union AnyValue*)&BigEndian, false);
    VariableDefinePlatformVar(pc, NULL, "LITTLE_ENDIAN", &pc->IntType, (union AnyValue*)&LittleEndian, false);
}

/* add a library */
void LibraryAdd(Picoc* pc, struct LibraryFunction* FuncList) {
    struct ParseState parser;
    char* identifier = NULL;
    struct ValueType* returnType = NULL;
    struct Value* newValue = NULL;
    void* tokens = NULL;
    char* intrinsicName = TableStrRegister(pc, "c library");

    /* read all the library definitions */
    for(int count = 0; FuncList[count].Prototype != NULL; count++) {
        tokens = LexAnalyse(pc, (const char*)intrinsicName, FuncList[count].Prototype,
                            strlen((char*)FuncList[count].Prototype), NULL);
        LexInitParser(&parser, pc, FuncList[count].Prototype, tokens, intrinsicName, true, false);
        TypeParse(&parser, &returnType, &identifier, NULL);
        newValue = ParseFunctionDefinition(&parser, returnType, identifier);
        newValue->Val->FuncDef.Intrinsic = FuncList[count].Func;
        HeapFreeMem(pc, tokens);
    }
}

/* print a type to a stream without using printf/sprintf */
void PrintType(struct ValueType* Typ, IOFILE* Stream) {
    switch(Typ->Base) {
        case TypeVoid:
            PrintStr("void", Stream);
            break;
        case TypeInt:
            PrintStr("int", Stream);
            break;
        case TypeShort:
            PrintStr("short", Stream);
            break;
        case TypeChar:
            PrintStr("char", Stream);
            break;
        case TypeLong:
            PrintStr("long", Stream);
            break;
        case TypeUnsignedInt:
            PrintStr("unsigned int", Stream);
            break;
        case TypeUnsignedShort:
            PrintStr("unsigned short", Stream);
            break;
        case TypeUnsignedLong:
            PrintStr("unsigned long", Stream);
            break;
        case TypeUnsignedChar:
            PrintStr("unsigned char", Stream);
            break;
        case TypeFP:
            PrintStr("double", Stream);
            break;
        case TypeFunction:
            PrintStr("function", Stream);
            break;
        case TypeMacro:
            PrintStr("macro", Stream);
            break;
        case TypePointer:
            if(Typ->FromType) {
                PrintType(Typ->FromType, Stream);
            }
            PrintCh('*', Stream);
            break;
        case TypeArray:
            PrintType(Typ->FromType, Stream);
            PrintCh('[', Stream);
            if(Typ->ArraySize != 0) {
                PrintSimpleInt(Typ->ArraySize, Stream);
            }
            PrintCh(']', Stream);
            break;
        case TypeStruct:
            PrintStr("struct ", Stream);
            PrintStr(Typ->Identifier, Stream);
            break;
        case TypeUnion:
            PrintStr("union ", Stream);
            PrintStr(Typ->Identifier, Stream);
            break;
        case TypeEnum:
            PrintStr("enum ", Stream);
            PrintStr(Typ->Identifier, Stream);
            break;
        case TypeGotoLabel:
            PrintStr("goto label ", Stream);
            break;
        case Type_Type:
            PrintStr("type ", Stream);
            break;
    }
}
