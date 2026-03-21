#pragma once
#include <stdlib.h>
#include <stdio.h>

#define testErrFatal(expected, call) do { \
    int _rc = (call); \
    if (_rc != (expected)) { \
        fprintf(stderr, "%s:%d: %s failed: got %d, expected %d\n", \
                __FILE__, __LINE__, #call, (int)_rc, (int)(expected)); \
        exit(1); \
    } \
} while(0)
