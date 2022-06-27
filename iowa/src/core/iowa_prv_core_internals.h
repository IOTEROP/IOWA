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

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_config.h"

#include "iowa_prv_core.h"

#include "iowa_prv_core_check_config.h" // This include checks the IOWA config

#define PAUSE_TIME_BUFFER 5

typedef enum
{
    BOOTSTRAP_CMD_CALL = 0,
    BOOTSTRAP_CMD_CALL_BIS,
    BOOTSTRAP_CMD_USER_CALLBACK
} bootstrap_high_level_cmd_state_t;

typedef struct
{
    uint8_t *requestBufferP;
    void    *userData;
} operation_data_t;


// defined in iowa_client.c

// Get maximal delay before next planned operation.
// Returned value: maximal delay wanted.
// Parameters:
// - contextP: returned by iowa_init().
uint32_t clientGetMaxDelayOperation(iowa_context_t contextP);

// This function will create an instance in the object Security and the linked instance in the object Server if the server is not a bootstrap server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
// - serverP: server information.
iowa_status_t clientAddServer(iowa_context_t contextP, lwm2m_server_t *serverP);

// This function will remove an instance in the object Security and if the server is not a bootstrap server, will remove the linked instance in the object Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
// - serverP: server information.
iowa_status_t clientRemoveServer(iowa_context_t contextP, lwm2m_server_t *serverP);

// defined in iowa_context.c

// Save the current IOWA context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - isSnapshot: boolean to save the runtime information.
iowa_status_t core_saveContext(iowa_context_t contextP, bool isSnapshot);

// Load a saved IOWA context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - bufferP: pointer to the buffer to load.
// - bufferLength: length of the buffer.
iowa_status_t core_loadContext(iowa_context_t contextP, uint8_t * bufferP, size_t bufferLength);

#ifdef __cplusplus
}
#endif

#endif
