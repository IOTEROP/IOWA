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
* IPSO Digital output
*
** Description
*** Generic digital output for non-specific actuators.
*
** Object Definition
*** Object Id: 3201
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_DIGITAL_OUTPUT_INCLUDE_
#define _IOWA_DIGITAL_OUTPUT_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new digital output object.
#define IOWA_DIGITAL_OUTPUT_STATS_RSC_POLARITY (1<<0)

// Defines used by the state type resource.
#define IOWA_DIGITAL_OUTPUT_POLARITY_NORMAL   false
#define IOWA_DIGITAL_OUTPUT_POLARITY_REVERSED true

// Callback called when the state of the digital output changed.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: instance of the digital output.
// - state: state of the digital output.
// - polarity: polarity of the digital output.
// - userDataCallback: user data callback.
typedef iowa_status_t (*iowa_digital_output_state_callback_t)(iowa_sensor_t id,
                                                              bool state,
                                                              bool polarity,
                                                              void *userDataCallback,
                                                              iowa_context_t contextP);


/**************************************************************
 * API
 **************************************************************/
// Add a digital output object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - updateStateCallback: callback used when the state of the digital output changed.
// - userDataCallback: user data passed to the callback.
// - applicationType: the application type as a string.
// - idP: OUT. ID of the created instance.
iowa_status_t iowa_client_add_digital_output_object(iowa_context_t contextP,
                                                    uint16_t optFlags,
                                                    iowa_digital_output_state_callback_t updateStateCallback,
                                                    void *userDataCallback,
                                                    const char *applicationType,
                                                    iowa_sensor_t *idP);

// Remove a digital output object created with iowa_client_add_digital_output_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the digital output object.
iowa_status_t iowa_client_remove_digital_output_object(iowa_context_t contextP,
                                                       iowa_sensor_t id);

// Update the mandatories ressources of the digital output object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the digital output object.
// - state: new state.
// - polarity: new polarity.
iowa_status_t iowa_client_digital_output_update_state(iowa_context_t contextP,
                                                      iowa_sensor_t id,
                                                      bool state,
                                                      bool polarity);


#ifdef __cplusplus
}
#endif

#endif /*_IOWA_DIGITAL_OUTPUT_INCLUDE_*/
