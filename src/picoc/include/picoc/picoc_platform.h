/**
 * @file picoc_platform.h
 * @details
 * Loads defaults and platform specific configurations
 */

#pragma once

/***********************************************************
 * Configuration
 *
 * Required features
 **********************************************************/
#undef PICOC_SIZEOF_FILE

//! void* malloc(size_t size)
#undef PICOC_FN_MALLOC

//! void* calloc(size_t num, size_t size)
#undef PICOC_FN_CALLOC

//! void* realloc(void* ptr, size_t new_size)
#undef PICOC_FN_REALLOC

//! void free(void* ptr)
#undef PICOC_FN_FREE

//! int snprintf(char* const buff, size_t const size, char const* const format)
#undef PICOC_FN_SNPRINTF

//! int fileno(FILE* stream)
#undef PICOC_FN_FILENO

/***********************************************************
 * Configuration
 *
 * Optional features
 **********************************************************/

//! Global variable table
#define PICOC_CONFIG_GLOBAL_TABLE_SIZE (97)

//! Shared string table size
#define PICOC_CONFIG_STRING_TABLE_SIZE (97)

//! string literal table size
#define PICOC_CONFIG_STRING_LITERAL_TABLE_SIZE (97)

//! reserved word table size
#define PICOC_CONFIG_RESERVED_WORD_TABLE_SIZE (97)

//! maximum number of parameters to a function
#define PICOC_CONFIG_PARAMETER_MAX (16)

//! maximum number of characters on a line
#define PICOC_CONFIG_LINEBUFFER_MAX (256)

//! size of local variable table (can expand)
#define PICOC_CONFIG_LOCAL_TABLE_SIZE (11)

//! size of struct/union member table (can expand)
#define PICOC_CONFIG_STRUCT_TABLE_SIZE (11)

//! Do not use floating point. This includes blocking all builtins that use fp.
#undef PICOC_CONFIG_NO_FP

//! Disable strptime function unless your platform wants it
#define PICOC_NO_STRPTIME

//! Disable unistd
#undef PICOC_NO_UNISTD

//! Store constants in RAM where possible
#define PICOC_CONSTANTS_IN_RAM

#define PICOC_INTERACTIVE_PROMPT_START "starting picoc " PICOC_VERSION " (Ctrl+D to exit)\n"
#define PICOC_INTERACTIVE_PROMPT_STATEMENT "picoc> "
#define PICOC_INTERACTIVE_PROMPT_LINE "     > "

#if defined(__hppa__) || defined(__sparc__)
//! the default data type to use for alignment
#define PICOC_ALIGN_TYPE double
#else
//! the default data type to use for alignment
#define PICOC_ALIGN_TYPE void*
#endif

/***********************************************************
 * Configuration
 *
 * Platform Features
 **********************************************************/
#if defined(__unix) || defined(__unix__)
#include "picoc/platform/platform_unix.h"
#elif defined(_WIN32) /*(predefined on MSVC)*/
#include "picoc/platform/platform_msvc.h"
#else
#include "picoc/platform_port.h"
#endif

#ifndef PICOC_SIZEOF_FILE
#error "PICO_SIZEOF_FILE not defined in platform_xxx.h"
#endif

#ifndef PICOC_FN_MALLOC
#error "PICOC_FN_MALLOC not defined in platform_xxx.h"
#endif

#ifndef PICOC_FN_CALLOC
#error "PICOC_FN_CALLOC not defined in platform_xxx.h"
#endif

#ifndef PICOC_FN_REALLOC
#error "PICOC_FN_REALLOC not defined in platform_xxx.h"
#endif

#ifndef PICOC_FN_FREE
#error "PICOC_FN_FREE not defined in platform_xxx.h"
#endif

#ifndef PICOC_FN_SNPRINTF
#error "PICOC_FN_SNPRINTF not defined in platform_xxx.h"
#endif

#ifndef PICOC_FN_FILENO
#error "PICOC_FN_FILENO not defined in platform_xxx.h"
#endif

#ifndef PICOC_PATH_MAX
#define PICOC_PATH_MAX 255
#endif

/***********************************************************
 * Debug Features
 **********************************************************/

//! Enable embedded debugger (experimental)
#undef PICOC_DEBUGGER_ENABLE

//! Debug heap allocator
#undef PICOC_DEBUG_HEAP

//! Debug expression parser
#undef PICOC_DEBUG_EXPRESSIONS

//! Show extra-detailed error messages
#undef PICOC_FANCY_ERROR_MESSAGES

//! Debug array initialization
#undef PICOC_DEBUG_ARRAY_INITIALIZER

//! Debug source lexer
#undef PICOC_DEBUG_LEXER

// !Debug variable scoping
#undef PICOC_DEBUG_VAR_SCOPE

extern jmp_buf ExitBuf;
