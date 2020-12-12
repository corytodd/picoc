/**
 * @file picoc_picoc.h
 * @details
 * picoc external interface. This should be the only header you need to use if
 * you're using picoc as a library. Internal details are in interpreter.h
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PROJECT_VER
#define PICOC_VERSION PROJECT_VER
#else
#define PICOC_VERSION "UNKNOWN"
#endif

#define PICOC_EXPORT_CALL PICOCAPI_EXPORT PICOCAPI_CALL /**< API export and call macro*/

#include "picoc/picoc_interpreter.h"

/* this has to be a macro, otherwise errors will occur due to
	the stack being corrupt */
#define PicocPlatformSetExitPoint(pc) setjmp((pc)->PicocExitBuf)

extern void PicocVersion(char* pVersion, int maxLen);

/* parse.c */
extern void PicocParse(Picoc *pc, const char *FileName, const char *Source,
	int SourceLen, int RunIt, int CleanupNow, int CleanupSource, int EnableDebugger);
extern void PicocParseInteractive(Picoc *pc);

/* platform.c */
extern void PicocCallMain(Picoc *pc, int argc, char **argv);
extern void PicocInitialize(Picoc *pc, int StackSize);
extern void PicocCleanup(Picoc *pc);
extern void PicocPlatformScanFile(Picoc *pc, const char *FileName);

/* include.c */
extern void PicocIncludeAllSystemHeaders(Picoc *pc);

#ifdef __cplusplus
}
#endif