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
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

void iowa_system_mutex_lock(void *userData)
{
#ifdef _WIN32
    HANDLE *mutexP;

    mutexP = (HANDLE *)userData;

    WaitForSingleObject(*mutexP, INFINITE);
#else
    pthread_mutex_t *mutexP;

    mutexP = (pthread_mutex_t *)userData;

    pthread_mutex_lock(mutexP);
#endif
}

void iowa_system_mutex_unlock(void *userData)
{
#ifdef _WIN32
    HANDLE *mutexP;

    mutexP = (HANDLE *)userData;

    ReleaseMutex(*mutexP);
#else
    pthread_mutex_t *mutexP;

    mutexP = (pthread_mutex_t *)userData;

    pthread_mutex_unlock(mutexP);
#endif
}
