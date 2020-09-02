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
* Copyright (c) 2019-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
*
**********************************************/

#include "iowa_prv_core_internals.h"
#include "iowa_prv_lwm2m_internals.h"

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_timer_t *coreTimerNew(iowa_context_t contextP,
                           int32_t delay,
                           timer_callback_t callback,
                           void *userData)
{

    iowa_timer_t *timerP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Entering with delay: %ds, callback: %p, userData: %p, currentTime: %ds.", delay, callback, userData, contextP->currentTime);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (delay <= 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_BASE, "Invalid delay: %ds.", delay);
        return NULL;
    }
    if (callback == NULL)
    {
        IOWA_LOG_WARNING(IOWA_PART_BASE, "Missing callback.");
        return NULL;
    }
#endif

    timerP = (iowa_timer_t *)iowa_system_malloc(sizeof(iowa_timer_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (timerP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _iowa_timer_t));
        return NULL;
    }
#endif
    memset(timerP, 0, sizeof(struct _iowa_timer_t));

    timerP->callback = callback;
    timerP->userData = userData;
    timerP->executionTime = contextP->currentTime + delay;
    if (timerP->executionTime < contextP->currentTime)
    {
        IOWA_LOG_WARNING(IOWA_PART_BASE, "Integer overflow.");
        iowa_system_free(timerP);
        return NULL;
    }

    contextP->timerList = (iowa_timer_t *)IOWA_UTILS_LIST_ADD(contextP->timerList, timerP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Exiting with iowa_timer_t: %p, execution time: %ds.", timerP, timerP->executionTime);

    return timerP;
}

void coreTimerDelete(iowa_context_t contextP,
                     iowa_timer_t *timerP)
{


    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Entering with iowa_timer_t %p.", timerP);

    contextP->timerList = (iowa_timer_t *)IOWA_UTILS_LIST_REMOVE(contextP->timerList, timerP);

    iowa_system_free(timerP);

    IOWA_LOG_TRACE(IOWA_PART_BASE, "Exiting.");
}

iowa_status_t coreTimerReset(iowa_context_t contextP,
                             iowa_timer_t *timerP,
                             int32_t delay)
{


    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Entering with timerP: %p, delay: %ds, currentTime: %d.", timerP, delay, contextP->currentTime);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (delay <= 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_BASE, "Invalid delay: %ds.", delay);
        return IOWA_COAP_400_BAD_REQUEST;
    }
#endif

    timerP->executionTime = contextP->currentTime + delay;
    if (timerP->executionTime < contextP->currentTime)
    {
        IOWA_LOG_WARNING(IOWA_PART_BASE, "Integer overflow.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Exiting with execution time: %ds.", timerP->executionTime);

    return IOWA_COAP_NO_ERROR;
}

void coreTimerStep(iowa_context_t contextP)
{

    iowa_timer_t *timerP;
    iowa_timer_t *parentTimerP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Entering currentTime: %ds, timeoutP: %ds.", contextP->currentTime, contextP->timeout);

    timerP = contextP->timerList;
    parentTimerP = NULL;
    while (timerP != NULL)
    {
        iowa_timer_t *nextTimerP;

        nextTimerP = timerP->nextP;

        IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Execution time: %ds.", timerP->executionTime);

        if (timerP->executionTime <= contextP->currentTime)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Calling callback for iowa_timer_t %p.", timerP);
            timerP->callback(contextP, timerP->userData);
            IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Callback for iowa_timer_t %p returned.", timerP);

            if (parentTimerP == NULL)
            {
                contextP->timerList = timerP->nextP;
            }
            else
            {
                parentTimerP->nextP = timerP->nextP;
            }

            iowa_system_free(timerP);
        }
        else
        {
            int32_t delay;

            delay = timerP->executionTime - contextP->currentTime;
            IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Execution delay for iowa_timer_t %p is %ds.", timerP, delay);

            if (delay < contextP->timeout)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Updating global timeout from %ds to %ds.", contextP->timeout, delay);
                contextP->timeout = delay;
            }

            parentTimerP = timerP;
        }
        timerP = nextTimerP;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Exiting with final timeoutP: %ds.", contextP->timeout);
}

void coreTimerClose(iowa_context_t contextP)
{

    IOWA_LOG_TRACE(IOWA_PART_BASE, "Entering.");

    IOWA_UTILS_LIST_FREE(contextP->timerList, iowa_system_free);
}
