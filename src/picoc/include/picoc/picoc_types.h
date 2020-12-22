//
// Created by Cory Todd on 12/16/2020.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#else
struct FILE;
#endif

typedef FILE IOFILE;

/**
 * @brief Input and Output buffers
 */
typedef struct {
    IOFILE* pStdout; ///< Receives output
    IOFILE* pStdin;  ///< Receives input
    IOFILE* pStderr; ///< Receives error outpout
} picoc_io_t;

#ifdef __cplusplus
}
#endif