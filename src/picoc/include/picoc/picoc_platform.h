/* all platform-specific includes and defines go in this file */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <math.h>
#include <stdbool.h>

#define PICOC_NO_STRPTIME

/* host platform includes */
#ifdef UNIX_HOST
#include "picoc/platform/platform_unix.h"
#elif defined(WIN32) /*(predefined on MSVC)*/
#include "picoc/platform/platform_msvc.h"
#else
#include "picoc/platform_port.h"
#endif

#ifndef PICO_SIZEOF_FILE
#define PICO_SIZEOF_FILE sizeof(FILE)
#endif

#ifndef PICOC_MALLOC
#define PICOC_MALLOC malloc
#endif

#ifndef PICOC_CALLOC
#define PICOC_CALLOC calloc
#endif

#ifndef PICOC_REALLOC
#define PICOC_REALLOC realloc
#endif

#ifndef PICOC_FREE
#define PICOC_FREE free
#endif

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

/* undocumented, but probably useful */
#undef DEBUG_HEAP
#undef DEBUG_EXPRESSIONS
#undef FANCY_ERROR_MESSAGES
#undef DEBUG_ARRAY_INITIALIZER
#undef DEBUG_LEXER
#undef DEBUG_VAR_SCOPE


#if defined(__hppa__) || defined(__sparc__)
/* the default data type to use for alignment */
#define ALIGN_TYPE double
#else
/* the default data type to use for alignment */
#define ALIGN_TYPE void*
#endif

#define GLOBAL_TABLE_SIZE (97)                /* global variable table */
#define STRING_TABLE_SIZE (97)                /* shared string table size */
#define STRING_LITERAL_TABLE_SIZE (97)        /* string literal table size */
#define RESERVED_WORD_TABLE_SIZE (97)         /* reserved word table size */
#define PARAMETER_MAX (16)                    /* maximum number of parameters to a function */
#define LINEBUFFER_MAX (256)                  /* maximum number of characters on a line */
#define LOCAL_TABLE_SIZE (11)                 /* size of local variable table (can expand) */
#define STRUCT_TABLE_SIZE (11)                /* size of struct/union member table (can expand) */

#define INTERACTIVE_PROMPT_START "starting picoc " PICOC_VERSION " (Ctrl+D to exit)\n"
#define INTERACTIVE_PROMPT_STATEMENT "picoc> "
#define INTERACTIVE_PROMPT_LINE "     > "

extern jmp_buf ExitBuf;

#endif /* PLATFORM_H */
