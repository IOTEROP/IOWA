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
* IPSO Location
*
** Description
*** This IPSO object represents GPS coordinates.
*** This object is compatible with the LWM2M management object for location, but uses reusable resources
*
** Object Definition
*** Object Id: 3336
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_GPS_INCLUDE_
#define _IOWA_GPS_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Defines used when declaring a new GPS object.
#define IOWA_GPS_RSC_UNCERTAINTY       (1<<0)
#define IOWA_GPS_RSC_COMPASS_DIRECTION (1<<1)
#define IOWA_GPS_RSC_VELOCITY          (1<<2)
#define IOWA_GPS_RSC_TIMESTAMP         (1<<3)

/**************************************************************
 * API
 **************************************************************/

// Add a GPS object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - applicationType: The application type as a string.
// - idP: OUT. ID of the created instance of the GPS object.
iowa_status_t iowa_client_add_gps_object(iowa_context_t contextP,
                                         uint16_t optFlags,
                                         const char * applicationType,
                                         iowa_sensor_t * idP);

// Remove a GPS object created with iowa_client_add_gps_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the GPS object.
iowa_status_t iowa_client_remove_gps_object(iowa_context_t contextP,
                                            iowa_sensor_t id);

// Update the mandatory ressources of the GPS object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the GPS object.
// - latitude: new latitude.
// - longitude: new longitude.
iowa_status_t iowa_client_gps_update_location(iowa_context_t contextP,
                                              iowa_sensor_t id,
                                              const char * latitude,
                                              const char * longitude);

// Update the optional ressources defined during the creation of the GPS object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the GPS object.
// - latitude: new latitude.
// - longitude: new longitude.
// - uncertainty: new uncertainty.
// - compassDirection: new compass direction.
// - velocityLength: length of the velocity opaque ressource.
// - velocity: new velocity.
iowa_status_t iowa_client_gps_update_location_full(iowa_context_t contextP,
                                                   iowa_sensor_t id,
                                                   const char * latitude,
                                                   const char * longitude,
                                                   const char * uncertainty,
                                                   float compassDirection,
                                                   size_t velocityLength,
                                                   uint8_t * velocity);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_GPS_INCLUDE_*/
