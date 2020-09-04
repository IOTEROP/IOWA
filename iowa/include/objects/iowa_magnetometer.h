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
* IPSO Magnetometer
*
** Description
*** This IPSO object can be used to represent a 1-3 axis magnetometer with optional compass direction.
*
** Object Definition
*** Object Id: 3314
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_MAGNETOMETER_INCLUDE_
#define _IOWA_MAGNETOMETER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new magnetometer object.
#define IOWA_MAGNETOMETER_RSC_Y_VALUE           (1<<0)
#define IOWA_MAGNETOMETER_RSC_Z_VALUE           (1<<1)
#define IOWA_MAGNETOMETER_RSC_COMPASS_DIRECTION (1<<2)

// Defines used when declaring a new magnetometer object.
// These defines are more general than the ones above, multiple ressources are supported by each define.
#define IOWA_MAGNETOMETER_3_AXIS (IOWA_MAGNETOMETER_RSC_Y_VALUE     \
                                  | IOWA_MAGNETOMETER_RSC_Z_VALUE)

/**************************************************************
 * API
 **************************************************************/
// Add an magnetometer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - sensorUnits: measurement units definition.
// - idP: OUT. ID of the created instance of the magnetometer object.
iowa_status_t iowa_client_add_magnetometer_object(iowa_context_t contextP,
                                                  uint16_t optFlags,
                                                  const char * sensorUnits,
                                                  iowa_sensor_t * idP);

// Remove an magnetometer object created with iowa_client_add_magnetometer_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the magnetometer object.
iowa_status_t iowa_client_remove_magnetometer_object(iowa_context_t contextP,
                                                     iowa_sensor_t id);

// Update the axis values of the magnetometer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the magnetometer object.
// - xValue: X value.
// - yValue: Y value.
// - zValue: Z value.
// - compassDirection: compass direction.
iowa_status_t iowa_client_magnetometer_update_values(iowa_context_t contextP,
                                                     iowa_sensor_t id,
                                                     float xValue,
                                                     float yValue,
                                                     float zValue,
                                                     float compassDirection);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_MAGNETOMETER_INCLUDE_*/
