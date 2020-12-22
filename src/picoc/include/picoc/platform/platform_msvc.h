/**
 * @file platform_msvc.h
 * @details
 * Sane MSVC defaults
 */
#pragma once

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PICOC_FN_MALLOC malloc
#define PICOC_FN_CALLOC calloc
#define PICOC_FN_REALLOC realloc
#define PICOC_FN_FREE free
#define PICOC_FN_SNPRINTF _snprintf
#define PICOC_FN_FILENO _fileno

#define PICOC_SIZEOF_FILE sizeof(FILE)
#define PICOC_NO_UNISTD
