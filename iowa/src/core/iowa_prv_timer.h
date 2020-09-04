/**********************************************
*
*  _________ _________ ___________ _________
* |         |         |   |   |   |         |
* |_________|         |   |   |   |    _    |
* |         |    |    |   |   |   |         |
* |         |    |    |           |         |
* |         |    |    |           |    |    |
* |         |         |           |    |    |
* |_________|_________|___________|____|____|
*
* Copyright (c) 2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_TIMER_INCLUDE_
#define _IOWA_PRV_TIMER_INCLUDE_

#include "iowa.h"

/**************************************************************
* Typedef Timer API
**************************************************************/

typedef void (*timer_callback_t)(iowa_context_t contextP, void *userData);

typedef struct _iowa_timer_t
{
    struct _iowa_timer_t *nextP;
    int32_t               executionTime;
    timer_callback_t      callback;
    void                 *userData;
} iowa_timer_t;

/**************************************************************
* Timer API
**************************************************************/

iowa_timer_t *coreTimerNew(iowa_context_t contextP, int32_t delay, timer_callback_t callback, void *userData);
void coreTimerDelete(iowa_context_t contextP, iowa_timer_t *timerP);
iowa_status_t coreTimerReset(iowa_context_t contextP, iowa_timer_t *timerP, int32_t delay);
void coreTimerStep(iowa_context_t contextP);
void coreTimerClose(iowa_context_t contextP);

#endif
