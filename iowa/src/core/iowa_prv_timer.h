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

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa.h"

/**************************************************************
* Typedef Timer API
**************************************************************/

// The timer callback called when the linked timer's delay has expired.
// contextP: the IOWA context on which iowa_client_configure was called.
// userData: the data passed to coreTimerNew.
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

// Initialize an iowa_timer_t and add it to the iowa context.
// Returned value: the iowa_timer_t in case of success or NULL if dynamical allocation failed.
// Parameters:
// - contextP: as returned by iowa_init().
// - delay: timer's delay.
// - callback: callback to called when delay has expired.
// - userData: userData passed through the callback.
iowa_timer_t *coreTimerNew(iowa_context_t contextP, int32_t delay, timer_callback_t callback, void *userData);

// Delete an iowa_timer_t and remove it from the iowa context.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - timerP: iowa_timer_t to delete.
void coreTimerDelete(iowa_context_t contextP, iowa_timer_t *timerP);

// Reset an iowa_timer_t.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: as returned by iowa_init().
// - timerP: iowa_timer_t to reset.
// - delay: new timer's delay.
iowa_status_t coreTimerReset(iowa_context_t contextP, iowa_timer_t *timerP, int32_t delay);

// State Machine of iowa timers. Check all timers in the iowa context and call the corresponding callback when timer's delay has expired.
// Parameters:
// - contextP: as returned by iowa_init().
void coreTimerStep(iowa_context_t contextP);

// Delete all timers in the iowa context.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
void coreTimerClose(iowa_context_t contextP);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_PRV_TIMER_INCLUDE_
