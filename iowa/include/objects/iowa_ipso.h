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
* IPSO Objects
*
** Description
*** Allow the possibility to use several IPSO Objects.
*
** Object Definition
*** Object Id: iowa_IPSO_ID_t
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_IPSO_INCLUDE_
#define _IOWA_IPSO_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"
#include "iowa_IPSO_ID.h"

/**************************************************************
 * API
 **************************************************************/

// Add an IPSO Common Template Object sensor to the client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - type: type of the IPSO Object.
// - value: the initial value measured by the sensor.
// - unit: informative. The unit of the measured value. Can be nil.
// - appType: informative. The application type description for this value. Can be nil.
// - rangeMin, rangeMax: informative. The range that can be measured. If both are equal to zero, the matching resources are omitted.
// - idP: OUT. ID of the created sensor.
iowa_status_t iowa_client_IPSO_add_sensor(iowa_context_t contextP,
                                          iowa_IPSO_ID_t type,
                                          float value,
                                          const char * unit,
                                          const char * appType,
                                          float rangeMin,
                                          float rangeMax,
                                          iowa_sensor_t * idP);

// Remove an IPSO Common Template Object sensor from the client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - type: type of the IPSO Object.
// - id: ID of the sensor.
iowa_status_t iowa_client_IPSO_remove_sensor(iowa_context_t contextP,
                                             iowa_sensor_t id);

// Update the value of an IPSO Object sensor.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the sensor.
// - value: the new value of the sensor.
iowa_status_t iowa_client_IPSO_update_value(iowa_context_t contextP,
                                            iowa_sensor_t id,
                                            float value);

typedef struct
{
    float value;
    int32_t timestamp;
} iowa_ipso_timed_value_t;

// Update multiple times the value of an IPSO Object sensor.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the sensor.
// - valueCount: the size of the list.
// - valueArray: the list of new values.
iowa_status_t iowa_client_IPSO_update_values(iowa_context_t contextP,
                                             iowa_sensor_t id,
                                             size_t valueCount,
                                             iowa_ipso_timed_value_t *valueArray);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_IPSO_INCLUDE_*/
