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
**********************************************/

/*************************************************************************************
* IPSO Light Control
*
** Description
*** This Object is used to control a light source, such as a LED or other light.
*** It allows a light to be turned on or off and its dimmer setting to be control as a % between 0 and 100.
*** An optional colour setting enables a string to be used to indicate the desired colour.
*
** Object Definition
*** Object Id: 3311
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_LIGHT_CONTROL_INCLUDE_
#define _IOWA_LIGHT_CONTROL_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new light control object
#define IOWA_LIGHT_CONTROL_RSC_DIMMER                  (1<<0)
#define IOWA_LIGHT_CONTROL_RSC_ON_TIME                 (1<<1)
#define IOWA_LIGHT_CONTROL_RSC_CUMULATIVE_ACTIVE_POWER (1<<2)
#define IOWA_LIGHT_CONTROL_RSC_POWER_FACTOR            (1<<3)

// Defines used when declaring a new light control object.
// These defines are more general than the ones above, multiple ressources are supported by each define.
#define IOWA_LIGHT_CONTROL_POWER  (IOWA_LIGHT_CONTROL_RSC_CUMULATIVE_ACTIVE_POWER \
                                   | IOWA_LIGHT_CONTROL_RSC_POWER_FACTOR)

// The callback called to change the state of the light.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: instance of the light.
// - powerOn: state of the light.
// - dimmer: light dim value.
// - colour: light colour.
// - userDataCallback: user data callback.
typedef iowa_status_t (*iowa_light_control_update_state_callback_t)(iowa_sensor_t id,
                                                                    bool powerOn,
                                                                    int dimmer,
                                                                    char * colour,
                                                                    void * userDataCallback,
                                                                    iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Add a light control object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - powerFactor: the power factor of the light.
// - colorSpace: color space of the light.
// - updateStateCallback: callback called to change the state of the light (can be nil).
// - userDataCallback: user data passed to the callback.
// - idP: OUT. ID of the created instance.
iowa_status_t iowa_client_add_light_control_object(iowa_context_t contextP,
                                                   uint16_t optFlags,
                                                   float powerFactor,
                                                   const char *colorSpace,
                                                   iowa_light_control_update_state_callback_t updateStateCallback,
                                                   void *userDataCallback,
                                                   iowa_sensor_t *idP);

// Remove a light control object created with iowa_client_add_light_control_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the light control object.
iowa_status_t iowa_client_remove_light_control_object(iowa_context_t contextP,
                                                      iowa_sensor_t id);

// Set the current state of the light.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the light control object.
// - powerOn: state of the light.
// - dimmer: dimmer value.
// - colour: colour of the light.
iowa_status_t iowa_client_light_control_set_state(iowa_context_t contextP,
                                                  iowa_sensor_t id,
                                                  bool powerOn,
                                                  int dimmer,
                                                  const char * colour);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_LIGHT_CONTROL_INCLUDE_*/
