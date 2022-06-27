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
* Copyright (c) 2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

/*************************************************************************************
* IPSO Dimmer
*
** Description
*** This IPSO object should be used with a dimmer or level control to report the state of the control.
*
** Object Definition
*** Object Id: 3343
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_DIMMER_INCLUDE_
#define _IOWA_DIMMER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"
#include "iowa_ipso.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new dimmer object.
#define IOWA_DIMMER_RSC_ON_TIME            (1<<0)
#define IOWA_DIMMER_RSC_OFF_TIME           (1<<1)

// Callback called when the state of the dimmer changed.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: instance of the dimmer.
// - level: level value.
// - userDataCallback: user data callback.
// - contextP: returned by iowa_init().
typedef iowa_status_t (*iowa_dimmer_state_callback_t)(iowa_sensor_t id,
                                                      float level,
                                                      void *userDataCallback,
                                                      iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/
// Add a dimmer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - level: The initial level of the dimmer object
// - updateStateCallback: callback called to change the state of the dimmer.
// - userDataCallback: user data passed to the callback (can be nil).
// - applicationType: The application type as a string
// - idP: OUT. ID of the created instance of the dimmer object.
iowa_status_t iowa_client_add_dimmer_object(iowa_context_t contextP,
                                            uint16_t optFlags,
                                            float level,
                                            iowa_dimmer_state_callback_t updateStateCallback,
                                            void *userDataCallback,
                                            const char * applicationType,
                                            iowa_sensor_t * idP);

// Remove a dimmer object created with iowa_client_add_dimmer_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the dimmer object.
iowa_status_t iowa_client_remove_dimmer_object(iowa_context_t contextP,
                                               iowa_sensor_t id);

// Update the dimmer object's level value.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the dimmer object.
// - level: level value.
iowa_status_t iowa_client_dimmer_update_value(iowa_context_t contextP,
                                              iowa_sensor_t id,
                                              float level);

// Update multiple times the value of a dimmer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the sensor.
// - valueCount: the size of the list.
// - valueArray: the list of new values.
iowa_status_t iowa_client_dimmer_update_values(iowa_context_t contextP,
                                               iowa_sensor_t id,
                                               size_t valueCount,
                                               iowa_ipso_timed_value_t *valueArray);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_DIMMER_INCLUDE_*/
