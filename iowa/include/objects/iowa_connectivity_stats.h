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
* Connectivity Statistics
*
** Description
*** This LwM2M Object enables client to collect statistical information and enables the LwM2M Server to retrieve these information, set the collection duration and reset the statistical parameters.
*
** Object Definition
*** Object Id: 7
*** Instances: Single
*** Optional
*************************************************************************************/

#ifndef _IOWA_CONNECTIVITY_STATS_INCLUDE_
#define _IOWA_CONNECTIVITY_STATS_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Defines used when specifying the RX/TX for updating the statistics.
#define IOWA_CONNECTIVITY_STATS_TX 0
#define IOWA_CONNECTIVITY_STATS_RX 1

// Defines used when declaring a new connectivity statistics object.
#define IOWA_CONNECTIVITY_STATS_RSC_SMS_TX_COUNTER       (1<<0)
#define IOWA_CONNECTIVITY_STATS_RSC_SMS_RX_COUNTER       (1<<1)
#define IOWA_CONNECTIVITY_STATS_RSC_TX_DATA              (1<<2)
#define IOWA_CONNECTIVITY_STATS_RSC_RX_DATA              (1<<3)
#define IOWA_CONNECTIVITY_STATS_RSC_MAX_MESSAGE_SIZE     (1<<4)
#define IOWA_CONNECTIVITY_STATS_RSC_AVERAGE_MESSAGE_SIZE (1<<5)
#define IOWA_CONNECTIVITY_STATS_RSC_COLLECTION_PERIOD    (1<<6)

// Defines used when declaring a new connectivity statistics object.
// These defines are more general than the ones above, multiple ressources are supported by each define.
#define IOWA_CONNECTIVITY_STATS_SMS     (IOWA_CONNECTIVITY_STATS_RSC_SMS_TX_COUNTER         \
                                         | IOWA_CONNECTIVITY_STATS_RSC_SMS_RX_COUNTER)
#define IOWA_CONNECTIVITY_STATS_IP_DATA (IOWA_CONNECTIVITY_STATS_RSC_TX_DATA                \
                                         | IOWA_CONNECTIVITY_STATS_RSC_RX_DATA              \
                                         | IOWA_CONNECTIVITY_STATS_RSC_MAX_MESSAGE_SIZE     \
                                         | IOWA_CONNECTIVITY_STATS_RSC_AVERAGE_MESSAGE_SIZE)

/**************************************************************
 * API
 **************************************************************/

// Add a connectivity statistics object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - idP: OUT. ID of the created instance of the connectivity statistics object.
iowa_status_t iowa_client_add_connectivity_stats_object(iowa_context_t contextP,
                                                        uint16_t optFlags,
                                                        iowa_sensor_t *idP);

// Remove a connectivity statistics object created with iowa_client_add_connectivity_stats_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the connectivity statistics object.
iowa_status_t iowa_client_remove_connectivity_stats_object(iowa_context_t contextP,
                                                           iowa_sensor_t id);

// Update the SMS TX or RX statistics.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the connectivity statistics object.
// - direction: specify if this is a reception or a transmission trigger.
iowa_status_t iowa_client_connectivity_stats_update_sms(iowa_context_t contextP,
                                                        iowa_sensor_t id,
                                                        uint8_t direction);

// Update the IP data statistics.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the connectivity statistics object.
// - direction: specify if this is a reception or a transmission trigger.
// - length: length in bytes of the transmitted or received data.
iowa_status_t iowa_client_connectivity_stats_update_ip_data(iowa_context_t contextP,
                                                            iowa_sensor_t id,
                                                            uint8_t direction,
                                                            size_t length);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_CONNECTIVITY_STATS_INCLUDE_*/
