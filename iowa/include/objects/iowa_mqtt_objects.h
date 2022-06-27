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
* MQTT Broker & Publication
*
** Description
*** These LwM2M Objects provide a range of MQTT Broker and publications related information which allows the possibility to connect IOWA client with an MQTT Broker.
*
** Object Definition
*** Broker Object Id: 18830
*** Publication Object Id: 18831
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_MQTT_OBJECT_INCLUDE_
#define _IOWA_MQTT_OBJECT_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

// IOWA headers
#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Defines used when declaring a new MQTT Publication or MQTT Broker object.
#define IOWA_MQTT_PUBLICATION_RSC_ACTIVE         (1<<0)
#define IOWA_MQTT_PUBLICATION_RSC_ENCODING       (1<<1)
#define IOWA_MQTT_BROKER_CERTIFICATE_USAGE       (1<<0)

// Structure to store the information of an MQTT broker
typedef struct
{
    char     *uri;
    char     *clientId;
    bool      cleanSession;
    uint16_t  keepAlive;
    char     *userName;
    uint8_t  *password;
    size_t    passwordLength;

    iowa_security_mode_t    securityMode;
    iowa_cert_usage_mode_t  certificateUsage;
    uint8_t                *identity;
    size_t                  identityLength;
    uint8_t                *brokerIdentity;
    size_t                  brokerIdentityLength;
    uint8_t                *privateKey;
    size_t                  privateKeyLength;
} iowa_mqtt_broker_t;

// Structure containing the details of an MQTT publication
typedef struct
{
    iowa_sensor_t           brokerId;
    char                   *source;
    char                   *topic;
    uint8_t                 qos;
    bool                    retain;
    bool                    active;
    iowa_content_format_t   encoding;
} iowa_mqtt_publication_t;

// The callback called when a LwM2M Server modifies the MQTT Broker Object.
// Returned value: None.
// Parameters:
// - operation: the type of the operation among IOWA_DM_READ, IOWA_DM_WRITE and IOWA_DM_EXECUTE.
// - brokerId: the ID of the modified MQTT broker.
// - brokerDetailsP: the details of the modified MQTT broker.
// - userData: Past as argument to the callbacks. This can be nil.
// - contextP: the IOWA context on which iowa_client_enable_mqtt_broker() was called.
typedef void (*iowa_mqtt_broker_update_callback_t) (iowa_dm_operation_t operation,
                                                    iowa_sensor_t brokerId,
                                                    iowa_mqtt_broker_t *brokerDetailsP,
                                                    void *userData,
                                                    iowa_context_t contextP);

// The callback called when a LwM2M Server modifies the MQTT Publication Object.
// Returned value: None.
// Parameters:
// - operation: the type of the operation among IOWA_DM_READ, IOWA_DM_WRITE and IOWA_DM_EXECUTE.
// - publicationId: the ID of the modified MQTT Publication.
// - publicationDetailsP: the details of the modified MQTT Publication.
// - userData: Past as argument to the callbacks. This can be nil.
// - contextP: the IOWA context on which iowa_client_enable_mqtt_publication() was called.
typedef void (*iowa_mqtt_publication_update_callback_t) (iowa_dm_operation_t operation,
                                                         iowa_sensor_t publicationId,
                                                         iowa_mqtt_publication_t *publicationDetailsP,
                                                         void *userData,
                                                         iowa_context_t contextP);

/**************************************************
*
* MQTT Broker LwM2M Object APIs.
*
*/

// Enable the MQTT brokers management.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - brokerCb: callback used when a LwM2M Server modify the MQTT brokers.
// - userData: user data past to callback.
iowa_status_t iowa_client_enable_mqtt_broker(iowa_context_t contextP,
                                             iowa_mqtt_broker_update_callback_t brokerCb,
                                             void *userData);

// Disable the MQTT brokers management.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_client_disable_mqtt_broker(iowa_context_t contextP);

// Add an MQTT broker.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - brokerDetailsP: details of the MQTT Broker. Copied internally by IOWA.
// - brokerIdP: OUT. The ID assigned to the MQTT Broker by IOWA.
iowa_status_t iowa_client_add_mqtt_broker(iowa_context_t contextP,
                                          uint16_t optFlags,
                                          const iowa_mqtt_broker_t *brokerDetailsP,
                                          iowa_sensor_t *brokerIdP);

// Remove an MQTT broker.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - brokerId: The ID assigned to the MQTT Broker by IOWA.
iowa_status_t iowa_client_remove_mqtt_broker(iowa_context_t contextP,
                                             iowa_sensor_t brokerId);

// Retrieve the details of an MQTT broker.
// Returned value: a pointer to a iowa_mqtt_broker_t or NULL in case of error.
// Parameters:
// - contextP: returned by iowa_init().
// - brokerId: The ID assigned to the MQTT Broker by IOWA.
iowa_mqtt_broker_t *iowa_client_get_mqtt_broker(iowa_context_t contextP,
                                                iowa_sensor_t brokerId);

/**************************************************
*
* MQTT Publication LwM2M Object APIs.
*
*/

// Enable the MQTT Publication management.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - publicationCB: callback used when a LwM2M Server modify the MQTT Publications.
// - userData: user data past to callback.
iowa_status_t iowa_client_enable_mqtt_publication(iowa_context_t contextP,
                                                  iowa_mqtt_publication_update_callback_t publicationCB,
                                                  void *userData);

// Disable the MQTT Publication management.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_client_disable_mqtt_publication(iowa_context_t contextP);

// Add an MQTT Publication.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional ressources.
// - publicationDetailsP: details of the MQTT Publication. Copied internally by IOWA.
// - publicationIdP: OUT. The ID assigned to the MQTT Publication by IOWA.
iowa_status_t iowa_client_add_mqtt_publication(iowa_context_t contextP,
                                               uint16_t optFlags,
                                               const iowa_mqtt_publication_t *publicationDetailsP,
                                               iowa_sensor_t *publicationIdP);

// Remove an MQTT Publication.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - publicationId: The ID assigned to the MQTT Publication by IOWA.
iowa_status_t iowa_client_remove_mqtt_publication(iowa_context_t contextP,
                                                  iowa_sensor_t publicationId);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_MQTT_OBJECT_INCLUDE_*/
