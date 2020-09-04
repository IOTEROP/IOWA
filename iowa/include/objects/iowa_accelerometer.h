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
* IPSO Accelerometer
*
** Description
*** This IPSO object can be used to represent a 1-3 axis accelerometer.
*
** Object Definition
*** Object Id: 3313
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_ACCELEROMETER_INCLUDE_
#define _IOWA_ACCELEROMETER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Defines used when declaring a new accelerometer object.
#define IOWA_ACCELEROMETER_RSC_Y_VALUE         (1<<0)
#define IOWA_ACCELEROMETER_RSC_Z_VALUE         (1<<1)
#define IOWA_ACCELEROMETER_RSC_MIN_RANGE_VALUE (1<<2)
#define IOWA_ACCELEROMETER_RSC_MAX_RANGE_VALUE (1<<3)

// Defines used when declaring a new accelerometer object.
// These defines are more general than the ones above, multiple ressources are supported by each define.
#define IOWA_ACCELEROMETER_3_AXIS      (IOWA_ACCELEROMETER_RSC_Y_VALUE          \
                                       | IOWA_ACCELEROMETER_RSC_Z_VALUE)
#define IOWA_ACCELEROMETER_RANGE_VALUE (IOWA_ACCELEROMETER_RSC_MIN_RANGE_VALUE  \
                                       | IOWA_ACCELEROMETER_RSC_MAX_RANGE_VALUE)


/**************************************************************
 * API
 **************************************************************/

// Add an accelerometer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - minRangeValue: the minimum value that can be measured by the sensor.
// - maxRangeValue: the maximum value that can be measured by the sensor.
// - sensorUnits: measurement units definition.
// - idP: OUT. ID of the created instance of the accelerometer object.
iowa_status_t iowa_client_add_accelerometer_object(iowa_context_t contextP,
                                                   uint16_t optFlags,
                                                   float minRangeValue,
                                                   float maxRangeValue,
                                                   const char *sensorUnits,
                                                   iowa_sensor_t *idP);

// Remove an accelerometer object created with iowa_client_add_accelerometer_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the accelerometer object.
iowa_status_t iowa_client_remove_accelerometer_object(iowa_context_t contextP,
                                                      iowa_sensor_t id);

// Update the axis values of the accelerometer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the accelerometer object.
// - xValue: X value.
// - yValue: Y value.
// - zValue: Z value.
iowa_status_t iowa_client_accelerometer_update_axis(iowa_context_t contextP,
                                                    iowa_sensor_t id,
                                                    float xValue,
                                                    float yValue,
                                                    float zValue);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_ACCELEROMETER_INCLUDE_*/
