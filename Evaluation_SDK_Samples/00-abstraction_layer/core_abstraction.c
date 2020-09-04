/**********************************************
 *
 * Copyright (c) 2016-2020 IoTerop.
 * All rights reserved.
 *
 * This program and the accompanying materials
 * are made available under the terms of
 * IoTerop’s IOWA License (LICENSE.TXT) which
 * accompany this distribution.
 *
 **********************************************/

/**********************************************
 *
 * This file implements simple IOWA system
 * abstraction functions for Linux and Windows.
 *
 **********************************************/

// IOWA header
#include "iowa_platform.h"

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <time.h>
#endif

// We bind this function directly to malloc().
void * iowa_system_malloc(size_t size)
{
    return malloc(size);
}

// We bind this function directly to free().
void iowa_system_free(void *pointer)
{
    free(pointer);
}

// We return the number of seconds since Epoch.
int32_t iowa_system_gettime(void)
{
#ifdef _WIN32
    return (int32_t)(GetTickCount() / 1000);
#else
    return (int32_t)time(NULL);
#endif
}

// We fake a reboot by exiting the application.
void iowa_system_reboot(void *userData)
{
    (void)userData;

    fprintf(stdout, "\n\tFaking a reboot.\r\n\n");
    exit(0);
}

// Traces are output on stderr.
void iowa_system_trace(const char *format,
                       va_list varArgs)
{
    vfprintf(stderr, format, varArgs);
}
