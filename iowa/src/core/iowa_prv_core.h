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
* Copyright (c) 2016-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_CORE_INCLUDE_
#define _IOWA_PRV_CORE_INCLUDE_

#include <string.h>
#include <limits.h>

#include "iowa_config.h"

#include "iowa_platform.h"
#include "iowa_prv_data.h"
#include "iowa_utils.h"
#include "iowa_prv_comm.h"
#include "iowa_prv_coap.h"
#include "iowa_prv_lwm2m.h"
#include "iowa_prv_logger.h"
#include "iowa_prv_misc.h"
#if defined(LWM2M_CLIENT_MODE)
#include "iowa_prv_objects.h"
#endif
#include "iowa_prv_timer.h"
#include "iowa_prv_security.h"


/************************************************
 * Macros
 */

#define CRIT_SECTION_ENTER(C)
#define CRIT_SECTION_LEAVE(C)
#define INTERRUPT_SELECT(C)

#define ACTION_REBOOT           (1<<0)
#define ACTION_EXIT             (1<<1)
#define ACTION_FW_DOWNLOAD      (1<<2)
#define ACTION_FW_UPDATE        (1<<3)
#define ACTION_SAVE_CONTEXT     (1<<4)
#define ACTION_SW_MGMT_DOWNLOAD (1<<5)
#define ACTION_SW_MGMT_INSTALL  (1<<6)
#define ACTION_SW_MGMT_ACTIVATE (1<<7)
#define ACTION_SW_CMP_ACTIVATE  (1<<8)
#define ACTION_FACTORY_RESET    (1<<9)

#define PMAX_UNSET_VALUE 0

#define MSISDN_MAX_LENGTH 15


#define INTERNAL_LWM2M_TYPE_BLOCK  100
#define IOWA_LWM2M_TYPE_URI_ONLY   10
#define IOWA_LWM2M_TYPE_NULL       11

#define CORE_BUFFER_EMPTY (iowa_buffer_t){NULL, 0}

#define CORE_FREE_AND_CLEAR(P) iowa_system_free((P)); (P) = NULL

/************************************************
 * Datatypes
 */

typedef struct _iowa_context_callback_t
{
    struct _iowa_context_callback_t *nextP;
    uint16_t                        callbackId;
    iowa_load_callback_t            loadCallback;
    iowa_save_callback_t            saveCallback;
    void                            *userData;
} iowa_context_callback_t;

struct _iowa_context_t
{
    lwm2m_context_t               *lwm2mContextP;
    coap_context_t                 coapContextP;
    comm_context_t                 commContextP;
    iowa_security_context_t        securityContextP;
    int32_t                        currentTime;
    int32_t                        timeout;
    iowa_timer_t                  *timerList;
#ifdef LWM2M_CLIENT_MODE
    iowa_event_callback_t          eventCb;
#endif
    uint16_t                       action;
    void                          *userData;
};

/************************************************
* Functions
*/


void clientNotificationLock(iowa_context_t contextP, bool enter);


void coreObservationEventCallback(iowa_context_t contextP, lwm2m_observed_t *targetP, iowa_event_type_t eventType);
void coreServerEventCallback(iowa_context_t contextP, lwm2m_server_t *serverP, iowa_event_type_t eventType);


iowa_buffer_t coreBufferNew(const uint8_t *dataP, size_t length);

#endif
