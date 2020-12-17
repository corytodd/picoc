/**
 * @file platform_msvc.h
 * @details
 * Sane MSVC defaults
 */
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <math.h>
#include <stdbool.h>

#define PICOC_FN_MALLOC malloc
#define PICOC_FN_CALLOC calloc
#define PICOC_FN_REALLOC realloc
#define PICOC_FN_FREE free
#define PICOC_FN_SNPRINTF _snprintf
#define PICOC_FN_FILENO _fileno

#define PICOC_SIZEOF_FILE sizeof(FILE)
#define PICOC_NO_UNISTD

//! Disable compiler warnings about unsafe functions
#define _CRT_SECURE_NO_WARNINGS
