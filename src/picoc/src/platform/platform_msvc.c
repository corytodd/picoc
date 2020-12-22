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
    if(Prompt != NULL)
        printf("%s", Prompt);

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
    struct stat FileInfo;
    char* ReadText;
    FILE* InFile;
    int BytesRead;
    char* p;

    if(stat(FileName, &FileInfo))
        ProgramFailNoParser(pc, "can't read file %s\n", FileName);

    ReadText = malloc(FileInfo.st_size + 1);
    if(ReadText == NULL)
        ProgramFailNoParser(pc, "out of memory\n");

    InFile = fopen(FileName, "r");
    if(InFile == NULL)
        ProgramFailNoParser(pc, "can't read file %s\n", FileName);

    BytesRead = fread(ReadText, 1, FileInfo.st_size, InFile);
    if(BytesRead == 0)
        ProgramFailNoParser(pc, "can't read file %s\n", FileName);

    ReadText[BytesRead] = '\0';
    fclose(InFile);

    if((ReadText[0] == '#') && (ReadText[1] == '!')) {
        for(p = ReadText; (*p != '\r') && (*p != '\n'); ++p) {
            *p = ' ';
        }
    }

    return ReadText;
}

/* read and scan a file for definitions */
void PicocPlatformScanFile(Picoc* pc, const char* FileName) {
    char* SourceStr = PlatformReadFile(pc, FileName);
    PicocParse(pc, FileName, SourceStr, strlen(SourceStr), true, false, true, gEnableDebugger);
}

/* exit the program */
void PlatformExit(Picoc* pc, int RetVal) {
    pc->PicocExitValue = RetVal;
    longjmp(pc->PicocExitBuf, 1);
}
