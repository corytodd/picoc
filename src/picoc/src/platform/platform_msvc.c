#include "picoc/picoc_interpreter.h"
#include "picoc/picoc_picoc.h"

#ifdef PICOC_DEBUGGER_ENABLE
static int gEnableDebugger = true;
#else
static int gEnableDebugger = false;
#endif

/* mark where to end the program for platforms which require this */
jmp_buf PicocExitBuf;

static picoc_io_t* l_PicocIO;

void PlatformInit(Picoc* pc) { l_PicocIO = pc->pCStdOut; }

void PlatformCleanup(Picoc* pc) {}

/* get a line of interactive input */
char* PlatformGetLine(char* Buf, int MaxLen, const char* Prompt) {
    if(Prompt != NULL) {
        printf("%s", Prompt);
    }

    fflush(l_PicocIO->pStdout);
    return fgets(Buf, MaxLen, stdin);
}

/* get a character of interactive input */
int PlatformGetCharacter() {
    fflush(l_PicocIO->pStdout);
    return getchar();
}

/* write a character to the console */
void PlatformPutc(unsigned char OutCh, union OutputStreamInfo* Stream) { putchar(OutCh); }

/* read a file into memory */
char* PlatformReadFile(Picoc* pc, const char* FileName) {
    struct stat fileInfo;
    char* readText;
    FILE* inFile;
    int bytesRead;
    char* p = NULL;

    if(stat(FileName, &fileInfo)) {
        ProgramFailNoParser(pc, "can't read file %s\n", FileName);
    }

    readText = malloc(fileInfo.st_size + 1);
    if(readText == NULL) {
        ProgramFailNoParser(pc, "out of memory\n");
    }

    inFile = fopen(FileName, "r");
    if(inFile == NULL) {
        ProgramFailNoParser(pc, "can't read file %s\n", FileName);
    }

    bytesRead = fread(readText, 1, fileInfo.st_size, inFile);
    if(bytesRead == 0) {
        ProgramFailNoParser(pc, "can't read file %s\n", FileName);
    }

    readText[bytesRead] = '\0';
    fclose(inFile);

    if((readText[0] == '#') && (readText[1] == '!')) {
        for(p = readText; (*p != '\r') && (*p != '\n'); ++p) {
            *p = ' ';
        }
    }

    return readText;
}

/* read and scan a file for definitions */
void PicocPlatformScanFile(Picoc* pc, const char* FileName) {
    char* sourceStr = PlatformReadFile(pc, FileName);
    PicocParse(pc, FileName, sourceStr, strlen(sourceStr), true, false, true, gEnableDebugger);
}

/* exit the program */
void PlatformExit(Picoc* pc, int RetVal) {
    pc->PicocExitValue = RetVal;
    longjmp(pc->PicocExitBuf, 1);
}
