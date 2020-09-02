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
* Location
*
** Description
*** This LwM2M Objects provide a range of device related information which can be queried by the LwM2M Server, and a device reboot and factory reset function
*
** Object Definition
*** Object Id: 6
*** Instances: Single
*** Optional
*************************************************************************************/

#ifndef _IOWA_LOCATION_INCLUDE_
#define _IOWA_LOCATION_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new location object.
#define IOWA_LOCATION_RSC_ALTITUDE (1<<0)
#define IOWA_LOCATION_RSC_RADIUS   (1<<1)
#define IOWA_LOCATION_RSC_VELOCITY (1<<2)
#define IOWA_LOCATION_RSC_SPEED    (1<<3)

/**************************************************************
 * API
 **************************************************************/
// Add a location object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - idP: OUT. ID of the created instance of the location object.
iowa_status_t iowa_client_add_location_object(iowa_context_t contextP,
                                              uint16_t optFlags,
                                              iowa_sensor_t *idP);

// Remove a location object created with iowa_client_add_location_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the location object.
iowa_status_t iowa_client_remove_location_object(iowa_context_t contextP,
                                                 iowa_sensor_t id);

// Update the mandatory ressources of the location object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the location object.
// - latitude: new latitude.
// - longitude: new longitude.
iowa_status_t iowa_client_location_update(iowa_context_t contextP,
                                          iowa_sensor_t id,
                                          float latitude,
                                          float longitude);

// Update the optional ressources defined during the creation of the location object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the location object.
// - latitude: new latitude.
// - longitude: new longitude.
// - altitude: new altitude.
// - radius: new radius.
// - velocityLength: length of the velocity opaque ressource.
// - velocity: new velocity.
// - speed: new speed.
iowa_status_t iowa_client_location_update_full(iowa_context_t contextP,
                                               iowa_sensor_t id,
                                               float latitude,
                                               float longitude,
                                               float altitude,
                                               float radius,
                                               size_t velocityLength,
                                               uint8_t * velocity,
                                               float speed);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_LOCATION_INCLUDE_*/
