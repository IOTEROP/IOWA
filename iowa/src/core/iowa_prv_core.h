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

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <limits.h>

#include "iowa_config.h"

#include "iowa_platform.h"
#include "iowa_prv_core_backward_compatibility.h"
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

// Internal only data types
#define INTERNAL_LWM2M_TYPE_BLOCK  (uint8_t)100    // used as a flag
#define IOWA_LWM2M_TYPE_URI_ONLY   10
#define IOWA_LWM2M_TYPE_NULL       11

#define CORE_FREE_AND_CLEAR(P) iowa_system_free((P)); (P) = NULL

#define CORE_STR_EVENT_TYPE(S)                                                                      \
((S) == IOWA_EVENT_UNDEFINED ? "IOWA_EVENT_UNDEFINED" :                                             \
((S) == IOWA_EVENT_REG_UNREGISTERED ? "IOWA_EVENT_REG_UNREGISTERED" :                               \
((S) == IOWA_EVENT_REG_REGISTERING ? "IOWA_EVENT_REG_REGISTERING" :                                 \
((S) == IOWA_EVENT_REG_REGISTERED ? "IOWA_EVENT_REG_REGISTERED" :                                   \
((S) == IOWA_EVENT_REG_UPDATING ? "IOWA_EVENT_REG_UPDATING" :                                       \
((S) == IOWA_EVENT_REG_FAILED ? "IOWA_EVENT_REG_FAILED" :                                           \
((S) == IOWA_EVENT_REG_UPDATE_FAILED ? "IOWA_EVENT_REG_UPDATE_FAILED" :                             \
((S) == IOWA_EVENT_BS_PENDING ? "IOWA_EVENT_BS_PENDING" :                                           \
((S) == IOWA_EVENT_BS_FINISHED ? "IOWA_EVENT_BS_FINISHED" :                                         \
((S) == IOWA_EVENT_BS_FAILED ? "IOWA_EVENT_BS_FAILED" :                                             \
((S) == IOWA_EVENT_OBSERVATION_STARTED ? "IOWA_EVENT_OBSERVATION_STARTED" :                         \
((S) == IOWA_EVENT_OBSERVATION_NOTIFICATION ? "IOWA_EVENT_OBSERVATION_NOTIFICATION" :               \
((S) == IOWA_EVENT_OBSERVATION_NOTIFICATION_ACKED ? "IOWA_EVENT_OBSERVATION_NOTIFICATION_ACKED" :   \
((S) == IOWA_EVENT_OBSERVATION_NOTIFICATION_FAILED ? "IOWA_EVENT_OBSERVATION_NOTIFICATION_FAILED" : \
((S) == IOWA_EVENT_OBSERVATION_CANCELED ? "IOWA_EVENT_OBSERVATION_CANCELED" :                       \
((S) == IOWA_EVENT_OBJECT_INSTANCE_CREATED ? "IOWA_EVENT_OBJECT_INSTANCE_CREATED" :                 \
((S) == IOWA_EVENT_OBJECT_INSTANCE_DELETED ? "IOWA_EVENT_OBJECT_INSTANCE_DELETED" :                 \
((S) == IOWA_EVENT_EVALUATION_PERIOD ? "IOWA_EVENT_EVALUATION_PERIOD" :                             \
"Unknown"))))))))))))))))))


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
    volatile uint16_t             action;
    void                          *userData;
};

/************************************************
* Functions
*/

// Implemented in iowa_client.c
void clientNotificationLock(iowa_context_t contextP, bool enter);

// Call event callback set by iowa_client_configure for Register and Bootstrap Events from server connection.
// Returned value: none.
// Parameters:
// - contextP: the IOWA context on which iowa_client_configure was called.
// - serverP: server information.
// - eventType: event identification.
// - code: for Registration events, the error code
// Note:
// - serverP can not be nil.
// - This function is called in a critical section.
void coreServerEventCallback(iowa_context_t contextP, lwm2m_server_t *serverP, iowa_event_type_t eventType, bool isInternal, uint8_t code);

// Implemented in iowa_buffer.c

// Initialize an iowa_buffer_t.
// Returned value: the iowa_buffer_t in case of success or IOWA_BUFFER_EMPTY if dynamical allocation failed.
// Parameters:
// - dataP: data to copy and save in the iowa_buffer_t.
// - length: dataP's length.
// Note:
// - if data is NULL, saved information is considered with length and value of 0.
// - for any arguments, saved information are allocated
iowa_buffer_t coreBufferNew(const uint8_t *dataP, size_t length);

// Set an iowa_buffer_t.
// Returned value: none.
// Parameters:
// - bufferP: iowa_buffer_t to set.
// - dataP: data to set in the iowa_buffer_t.
// - length: dataP's length.
// Note:
// - bufferP can not be nil.
void coreBufferSet(iowa_buffer_t *bufferP, uint8_t *dataP, size_t length);

// Extend an iowa_buffer_t.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - bufferP: iowa_buffer_t to extend. Can not be nil.
// - dataP: data to copy and save in the iowa_buffer_t. Can not be nil.
// - length: dataP's length. Can not be 0.
iowa_status_t coreBufferExtend(iowa_buffer_t *bufferP, const uint8_t *dataP, size_t length);

// Merge several iowa_buffer_t into one.
// Returned value: the iowa_buffer_t in case of success or IOWA_BUFFER_EMPTY if dynamical allocation failed.
// Parameters:
// - bufferList: the several iowa_buffer_t to merge.
// Note:
// - bufferList can not be nil.
// - overflow is not handle.
iowa_buffer_t coreBufferMerge(iowa_linked_buffer_t *bufferList);

// Clear iowa_buffer_t, free data allocated in the iowa_buffer_t.
// Returned value: none.
// Parameters:
// - bufferP: iowa_buffer_t to clear.
void coreBufferClear(iowa_buffer_t *bufferP);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_PRV_CORE_INCLUDE_
