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

/*************************************************************************************
* AT command
*
** Description
*** Used to execute an AT command on a cellular modem.
*
** Object Definition
*** Object Id: 10251
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_AT_COMMAND_INCLUDE_
#define _IOWA_AT_COMMAND_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new AT Command object.
#define IOWA_AT_COMMAND_RSC_TIMEOUT         (1<<0)

// The callback called to execute the resource Run.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: instance of the AT Command.
// - command: the AT command to run.
// - timeout: amount of time in seconds allowed for the modem to respond to the command.
// - userDataCallback: user data callback.
// - contextP: the IOWA context.
typedef iowa_status_t (*iowa_at_command_run_t)(iowa_sensor_t id,
                                               char *command,
                                               int timeout,
                                               void *userDataCallback,
                                               iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Add an AT Command object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional resources.
// - run: run callback.
// - userDataCallback: user data callback.
// - idP: OUT. ID of the created instance of the accelerometer object.
iowa_status_t iowa_client_add_at_command_object(iowa_context_t contextP,
                                                uint16_t optFlags,
                                                iowa_at_command_run_t run,
                                                void *userDataCallback,
                                                iowa_sensor_t *idP);

// Remove an AT Command object created with iowa_client_add_at_command_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the AT Command object.
iowa_status_t iowa_client_remove_at_command_object(iowa_context_t contextP,
                                                   iowa_sensor_t id);

// Update the AT Command.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the AT Command object.
// - command: the AT command to run.
// - response: response to the AT command.
// - status: status of the command execution as returned by the modem.
iowa_status_t iowa_client_at_command_set_response(iowa_context_t contextP,
                                                  iowa_sensor_t id,
                                                  const char *command,
                                                  const char *response,
                                                  const char *status);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_AT_COMMAND_INCLUDE_*/
