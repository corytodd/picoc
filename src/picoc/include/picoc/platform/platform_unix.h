/**
 * @file platform_unix.h
 * @details
 * Sane Unix defaults
 */
#pragma once

#include <stdint.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//! Yes, we want strptime
#undef PICOC_NO_STRPTIME