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
* IPSO Gyrometer
*
** Description
*** This IPSO Object is used to report the current reading of a gyrometer sensor in 3 axes.
*** It provides tracking of the minimum and maximum angular rate in all 3 axes.
*** An example unit of measure is radians per second (ucum:rad/s).
*
** Object Definition
*** Object Id: 3334
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_GYROMETER_INCLUDE_
#define _IOWA_GYROMETER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
// Defines used when declaring a new gyrometer object.
#define IOWA_GYROMETER_RSC_Y_VALUE              (1<<0)
#define IOWA_GYROMETER_RSC_Z_VALUE              (1<<1)
#define IOWA_GYROMETER_RSC_MIN_X_VALUE          (1<<2)
#define IOWA_GYROMETER_RSC_MAX_X_VALUE          (1<<3)
#define IOWA_GYROMETER_RSC_MIN_Y_VALUE          (1<<4)
#define IOWA_GYROMETER_RSC_MAX_Y_VALUE          (1<<5)
#define IOWA_GYROMETER_RSC_MIN_Z_VALUE          (1<<6)
#define IOWA_GYROMETER_RSC_MAX_Z_VALUE          (1<<7)
#define IOWA_GYROMETER_RSC_RESET_MIN_MAX_VALUES (1<<8)
#define IOWA_GYROMETER_RSC_MIN_RANGE_VALUE      (1<<9)
#define IOWA_GYROMETER_RSC_MAX_RANGE_VALUE      (1<<10)

// Defines used when declaring a new gyrometer object.
// These defines are more general than the ones above, multiple ressources are supported by each define.
#define IOWA_GYROMETER_3_AXIS         (IOWA_GYROMETER_RSC_Y_VALUE                   \
                                       | IOWA_GYROMETER_RSC_Z_VALUE)
#define IOWA_GYROMETER_MIN_MAX_VALUES (IOWA_GYROMETER_RSC_MIN_X_VALUE               \
                                       | IOWA_GYROMETER_RSC_MAX_X_VALUE             \
                                       | IOWA_GYROMETER_RSC_MIN_Y_VALUE             \
                                       | IOWA_GYROMETER_RSC_MAX_Y_VALUE             \
                                       | IOWA_GYROMETER_RSC_MIN_Z_VALUE             \
                                       | IOWA_GYROMETER_RSC_MAX_Z_VALUE             \
                                       | IOWA_GYROMETER_RSC_RESET_MIN_MAX_VALUES)
#define IOWA_GYROMETER_RANGE_VALUE    (IOWA_GYROMETER_RSC_MIN_RANGE_VALUE           \
                                       | IOWA_GYROMETER_RSC_MAX_RANGE_VALUE)

/**************************************************************
 * API
 **************************************************************/

// Add an gyrometer object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - minRangeValue: the minimum value that can be measured by the sensor.
// - maxRangeValue: the maximum value that can be measured by the sensor.
// - sensorUnits: measurement units definition.
// - applicationType: the application type as a string.
// - idP: OUT. ID of the created instance of the gyrometer object.
iowa_status_t iowa_client_add_gyrometer_object(iowa_context_t contextP,
                                               uint16_t optFlags,
                                               float minRangeValue,
                                               float maxRangeValue,
                                               const char * sensorUnits,
                                               const char * applicationType,
                                               iowa_sensor_t * idP);

// Remove an gyrometer object created with iowa_client_add_gyrometer_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the gyrometer object.
iowa_status_t iowa_client_remove_gyrometer_object(iowa_context_t contextP,
                                                  iowa_sensor_t id);

// Update the axis values of the gyrometer object.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the gyrometer object.
// - xValue: X value.
// - yValue: Y value.
// - zValue: Z value.
iowa_status_t iowa_client_gyrometer_update_axis(iowa_context_t contextP,
                                                iowa_sensor_t id,
                                                float xValue,
                                                float yValue,
                                                float zValue);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_GYROMETER_INCLUDE_*/
