/**
 * @file main.c
 * @details
 * Program entry point.
 */

#include "picoc/picoc_license.h"
#include "picoc/picoc_picoc.h"

/* Override via STACKSIZE environment variable */
#define PICOC_STACK_SIZE (128000 * 4)

int main(int argc, char** argv) {
    char version[20] = {0};
    PicocVersion(version, sizeof(version));

    int ParamCount = 1;
    int DontRunMain = false;
    int StackSize = getenv("STACKSIZE") ? atoi(getenv("STACKSIZE")) : PICOC_STACK_SIZE;
    char* outfile = getenv("OUTFILE");
    Picoc pc;

    if(argc < 2 || strcmp(argv[ParamCount], "-h") == 0) {
        printf("%s \n"
               "Format:\n\n"
               "> picoc <file1.c>... [- <arg1>...]    : run a program, calls main() as the entry point\n"
               "> picoc -s <file1.c>... [- <arg1>...] : run a script, runs the program without calling main()\n"
               "> picoc -i                            : interactive mode, Ctrl+d to exit\n"
               "> picoc -c                            : copyright info\n"
               "> picoc -h                            : this help message\n",
               version);
        return 0;
    }

    if(strcmp(argv[ParamCount], "-c") == 0) {
        printf("%s\n", (char*)&__LICENSE);
        return 0;
    }

    IOFILE* pStdout = NULL;
    bool closeFile = false;
    if(outfile != NULL) {
        pStdout = fopen(outfile, "w");
        if(pStdout != NULL) {
            closeFile = true;
        }
    }
    if(pStdout == NULL) {
        pStdout = stdout;
    }

    picoc_io_t io = {.pStdout = pStdout, .pStdin = stdin, .pStderr = stderr};

    PicocInitialize(&pc, StackSize, &io);

    if(strcmp(argv[ParamCount], "-s") == 0) {
        DontRunMain = true;
        PicocIncludeAllSystemHeaders(&pc);
        ParamCount++;
    }

    if(argc > ParamCount && strcmp(argv[ParamCount], "-i") == 0) {
        PicocIncludeAllSystemHeaders(&pc);
        PicocParseInteractive(&pc);
    } else {
        if(PicocPlatformSetExitPoint(&pc)) {
            PicocCleanup(&pc);
            if(closeFile) {
                fclose(pStdout);
            }
            return pc.PicocExitValue;
        }

        for(; ParamCount < argc && strcmp(argv[ParamCount], "-") != 0; ParamCount++) {
            PicocPlatformScanFile(&pc, argv[ParamCount]);
        }

        if(!DontRunMain) {
            PicocCallMain(&pc, argc - ParamCount, &argv[ParamCount]);
        }
    }

    PicocCleanup(&pc);
    if(closeFile) {
        fclose(pStdout);
    }
    return pc.PicocExitValue;
}
