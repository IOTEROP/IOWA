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

#ifndef _CORE_INTERNALS_
#define _CORE_INTERNALS_

#include "iowa_config.h"

#include "iowa_prv_core.h"

#include "iowa_prv_core_check_config.h" // This include checks the IOWA config

#define PAUSE_TIME_BUFFER 5

#define CORE_STR_EVENT_TYPE(S)                                                      \
((S) == IOWA_EVENT_UNDEFINED ? "IOWA_EVENT_UNDEFINED" :                             \
((S) == IOWA_EVENT_REG_UNREGISTERED ? "IOWA_EVENT_REG_UNREGISTERED" :               \
((S) == IOWA_EVENT_REG_REGISTERING ? "IOWA_EVENT_REG_REGISTERING" :                 \
((S) == IOWA_EVENT_REG_REGISTERED ? "IOWA_EVENT_REG_REGISTERED" :                   \
((S) == IOWA_EVENT_REG_UPDATING ? "IOWA_EVENT_REG_UPDATING" :                       \
((S) == IOWA_EVENT_REG_FAILED ? "IOWA_EVENT_REG_FAILED" :                           \
((S) == IOWA_EVENT_REG_UPDATE_FAILED ? "IOWA_EVENT_REG_UPDATE_FAILED" :             \
((S) == IOWA_EVENT_BS_PENDING ? "IOWA_EVENT_BS_PENDING" :                           \
((S) == IOWA_EVENT_BS_FINISHED ? "IOWA_EVENT_BS_FINISHED" :                         \
((S) == IOWA_EVENT_BS_FAILED ? "IOWA_EVENT_BS_FAILED" :                             \
((S) == IOWA_EVENT_OBSERVATION_STARTED ? "IOWA_EVENT_OBSERVATION_STARTED" :         \
((S) == IOWA_EVENT_OBSERVATION_CANCELED ? "IOWA_EVENT_OBSERVATION_CANCELED" :       \
((S) == IOWA_EVENT_OBJECT_INSTANCE_CREATED ? "IOWA_EVENT_OBJECT_INSTANCE_CREATED" : \
((S) == IOWA_EVENT_OBJECT_INSTANCE_DELETED ? "IOWA_EVENT_OBJECT_INSTANCE_DELETED" : \
((S) == IOWA_EVENT_EVALUATION_PERIOD ? "IOWA_EVENT_EVALUATION_PERIOD" :             \
"Unknown")))))))))))))))

typedef enum
{
    BOOTSTRAP_CMD_CALL = 0,
    BOOTSTRAP_CMD_CALL_BIS,
    BOOTSTRAP_CMD_USER_CALLBACK
} bootstrap_high_level_cmd_state_t;

typedef struct
{
    void *userData;
} operation_data_t;

uint32_t clientGetMaxDelayOperation(iowa_context_t contextP);
iowa_status_t clientAddServer(iowa_context_t contextP, lwm2m_server_t *serverP);
iowa_status_t clientRemoveServer(iowa_context_t contextP, lwm2m_server_t *serverP);

// defined in iowa_context.c

// Save the current IOWA context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - isSnapshot: boolean to save the runtime information.
iowa_status_t saveContext(iowa_context_t contextP, bool isSnapshot);

// Load a saved IOWA context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - bufferP: pointer to the buffer to load.
// - bufferLength: length of the buffer.
iowa_status_t loadContext(iowa_context_t contextP, uint8_t * bufferP, size_t bufferLength);

#endif
