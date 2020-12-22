/* picoc include system - can emulate system includes from built-in libraries
 * or it can include and parse files if the system has files */

#include "picoc/picoc_interpreter.h"
#include "picoc/picoc_picoc.h"

/* initialize the built-in include libraries */
void IncludeInit(Picoc* pc) {
    IncludeRegister(pc, "ctype.h", NULL, &StdCtypeFunctions[0], NULL);
    IncludeRegister(pc, "errno.h", &StdErrnoSetupFunc, NULL, NULL);
#ifndef PICOC_CONFIG_NO_FP
    IncludeRegister(pc, "math.h", &MathSetupFunc, &MathFunctions[0], NULL);
#endif
    IncludeRegister(pc, "stdbool.h", &StdboolSetupFunc, NULL, StdboolDefs);
    IncludeRegister(pc, "stdio.h", &StdioSetupFunc, &StdioFunctions[0], StdioDefs);
    IncludeRegister(pc, "stdlib.h", &StdlibSetupFunc, &StdlibFunctions[0], NULL);
    IncludeRegister(pc, "string.h", &StringSetupFunc, &StringFunctions[0], NULL);
    IncludeRegister(pc, "time.h", &StdTimeSetupFunc, &StdTimeFunctions[0], StdTimeDefs);
#ifndef PICOC_NO_UNISTD
    IncludeRegister(pc, "unistd.h", &UnistdSetupFunc, &UnistdFunctions[0], UnistdDefs);
#endif
}

/* clean up space used by the include system */
void IncludeCleanup(Picoc* pc) {
    struct IncludeLibrary* thisInclude = pc->IncludeLibList;
    struct IncludeLibrary* nextInclude = NULL;

    while(thisInclude != NULL) {
        nextInclude = thisInclude->NextLib;
        HeapFreeMem(pc, thisInclude);
        thisInclude = nextInclude;
    }

    pc->IncludeLibList = NULL;
}

/* register a new build-in include file */
void IncludeRegister(Picoc* pc, const char* IncludeName, void (*SetupFunction)(Picoc* pc),
                     struct LibraryFunction* FuncList, const char* SetupCSource) {
    struct IncludeLibrary* newLib = HeapAllocMem(pc, sizeof(struct IncludeLibrary));
    newLib->IncludeName = TableStrRegister(pc, IncludeName);
    newLib->SetupFunction = SetupFunction;
    newLib->FuncList = FuncList;
    newLib->SetupCSource = SetupCSource;
    newLib->NextLib = pc->IncludeLibList;
    pc->IncludeLibList = newLib;
}

/* include all of the system headers */
void PicocIncludeAllSystemHeaders(Picoc* pc) {
    struct IncludeLibrary* thisInclude = pc->IncludeLibList;

    for(; thisInclude != NULL; thisInclude = thisInclude->NextLib) {
        IncludeFile(pc, thisInclude->IncludeName);
    }
}

/* include one of a number of predefined libraries, or perhaps an actual file */
void IncludeFile(Picoc* pc, char* FileName) {
    struct IncludeLibrary* lInclude = NULL;

    /* scan for the include file name to see if it's in our list
        of predefined includes */
    for(lInclude = pc->IncludeLibList; lInclude != NULL; lInclude = lInclude->NextLib) {
        if(strcmp(lInclude->IncludeName, FileName) == 0) {
            /* found it - protect against multiple inclusion */
            if(!VariableDefined(pc, FileName)) {
                VariableDefine(pc, NULL, FileName, NULL, &pc->VoidType, false);

                /* run an extra startup function if there is one */
                if(lInclude->SetupFunction != NULL) {
                    (*lInclude->SetupFunction)(pc);
                }

                /* parse the setup C source code - may define types etc. */
                if(lInclude->SetupCSource != NULL) {
                    PicocParse(pc, FileName, lInclude->SetupCSource, strlen(lInclude->SetupCSource), true, true, false,
                               false);
                }

                /* set up the library functions */
                if(lInclude->FuncList != NULL) {
                    LibraryAdd(pc, lInclude->FuncList);
                }
            }

            return;
        }
    }

    /* not a predefined file, read a real file */
    PicocPlatformScanFile(pc, FileName);
}
