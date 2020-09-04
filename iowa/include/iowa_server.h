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
* Copyright (c) 2017-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_SERVER_INCLUDE_
#define _IOWA_SERVER_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa.h"
#include "iowa_security.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

typedef uint8_t iowa_supported_format_t;

#define IOWA_SUPPORTED_FORMAT_UNKNOWN    0x00
#define IOWA_SUPPORTED_FORMAT_TLV        0x01
#define IOWA_SUPPORTED_FORMAT_JSON       0x02
#define IOWA_SUPPORTED_FORMAT_OLD_TLV    0x04
#define IOWA_SUPPORTED_FORMAT_OLD_JSON   0x08
#define IOWA_SUPPORTED_FORMAT_CBOR       0x10
#define IOWA_SUPPORTED_FORMAT_SENML_JSON 0x20
#define IOWA_SUPPORTED_FORMAT_SENML_CBOR 0x40

typedef enum
{
    IOWA_STATE_UNDEFINED = 0,
    IOWA_STATE_INITIAL,
    IOWA_STATE_BOOTSTRAP_REQUIRED,
    IOWA_STATE_BOOTSTRAPPING,
    IOWA_STATE_BOOTSTRAP_FAILED,
    IOWA_STATE_BOOTSTRAP_FINISHED,
    IOWA_STATE_UNREGISTERED,
    IOWA_STATE_REGISTER_REQUIRED,
    IOWA_STATE_REGISTERING,
    IOWA_STATE_REGISTER_FAILED,
    IOWA_STATE_REGISTERED,
    IOWA_STATE_UPDATING,
    IOWA_STATE_READY
} iowa_state_t;

typedef enum
{
    IOWA_LWM2M_VERSION_UNDEFINED = 0,
    IOWA_LWM2M_VERSION_1_0,
    IOWA_LWM2M_VERSION_1_1
} iowa_lwm2m_protocol_version_t;

typedef struct
{
    const char                    *name;
    uint16_t                       id;
    bool                           queueMode;
    iowa_supported_format_t        supportedFormats;
    const char                    *msisdn;
    size_t                         objectLinkCount;
    iowa_lwm2m_object_link_t      *objectLinkArray;
    uint32_t                       lifetime;
    iowa_connection_type_t         connectionType;
    bool                           secureConnection;
    iowa_lwm2m_protocol_version_t  lwm2mVersion;
} iowa_client_t;

/**************************************************************
* Server Role APIs
**************************************************************/

/****************************
 * Configuration APIs
 */

// The client state monitoring callback.
// clientP: the client updating its state.
// state: the state of the client among
//      - IOWA_STATE_REGISTERED: when a new or returning client registers.
//      - IOWA_STATE_UPDATING: when a client updates its registration.
//      - IOWA_STATE_UNREGISTERED: when a client ends its registration or when the registration expires.
// callbackUserData: the iowa_server_configure() parameter.
// contextP: the IOWA context on which server_configure was called.
typedef void (*iowa_monitor_callback_t)(const iowa_client_t *clientP,
                                        iowa_state_t state,
                                        void *callbackUserData,
                                        iowa_context_t contextP);

// The resource type callback.
// Returned value: the type of the resource or IOWA_LWM2M_TYPE_UNDEFINED.
// objectID: the object the resource belongs to.
// resourceID: the resource.
// callbackUserData: the iowa_server_configure() parameter.
typedef iowa_lwm2m_data_type_t(*iowa_resource_type_callback_t) (uint16_t objectID,
                                                                uint16_t resourceID,
                                                                void *callbackUserData);

// Configure the server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - monitorCb: callback called by IOWA when a client updates its status.
// - resTypeCb: callback to retrieve data type of non-standard resources.
// - callbackUserData: past as argument to monitorCb and resTypeCb.
iowa_status_t iowa_server_configure(iowa_context_t contextP,
                                    iowa_monitor_callback_t monitorCb,
                                    iowa_resource_type_callback_t resTypeCb,
                                    void *callbackUserData);

// Inform the stack of a new incoming connection.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - type: the type of the connection.
// - connP: the connection of the same opaque type as returned by iowa_system_connection_open().
// - isSecure: set to true if the security must be enabled on this connection.
iowa_status_t iowa_server_new_incoming_connection(iowa_context_t contextP,
                                                  iowa_connection_type_t type,
                                                  void * connP,
                                                  bool isSecure);

// Create a Registration Update Trigger message.
// Returned value: The length of the serialized buffer. If zero, an error occurred.
// Parameters:
// - serverInstanceId: the server instance ID.
// - bufferP: OUT. the message.
size_t iowa_server_create_registration_update_trigger_message(uint16_t serverInstanceId,
                                                              uint8_t **bufferP);

/****************************
 * Device Management APIs
 */

// The operation result callback.
// Returned value: None.
// Parameters:
// - operation: the type of the operation.
// - clientId: the ID of the client targeted by the operation.
// - objectId, instanceId, resourceId: the URI targeted by the operation.
// - status: the status of the operation.
// - dataCount, dataArray: the data returned by the client.
// - resultUserData: the iowa_server_dm_...() parameter.
// - contextP: the IOWA context on which the device management API iowa_server_dm_exec() was called.
typedef void(*iowa_result_callback_t) (iowa_dm_operation_t operation,
                                       uint16_t clientId,
                                       uint16_t objectId, uint16_t instanceId, uint16_t resourceId,
                                       iowa_status_t status,
                                       size_t dataCount,
                                       iowa_lwm2m_data_t *dataArray,
                                       void *resultUserData,
                                       iowa_context_t contextP);

// Perform an Execute operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientID: the ID of the client to do an execute on.
// - objectID, instanceID, resourceID: the URI targeted by the operation.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_server_dm_exec(iowa_context_t contextP,
                                  uint32_t clientID,
                                  uint16_t objectID, uint16_t instanceID, uint16_t resourceID,
                                  iowa_result_callback_t resultCb,
                                  void * resultUserData);

// Perform a Create operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to do a delete on.
// - objectId, instanceId: the URI targeted by the operation.
// - dataCount, dataArrayP: the data to write.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_server_dm_create(iowa_context_t contextP,
                                    uint32_t clientId,
                                    uint16_t objectId, uint16_t instanceId,
                                    size_t dataCount,
                                    iowa_lwm2m_data_t *dataArrayP,
                                    iowa_result_callback_t resultCb,
                                    void *resultUserData);

// Perform a Delete operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to do a delete on.
// - objectId, instanceId: the URI targeted by the operation.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_server_dm_delete(iowa_context_t contextP,
                                    uint32_t clientId,
                                    uint16_t objectId, uint16_t instanceId,
                                    iowa_result_callback_t resultCb,
                                    void *resultUserData);

// Perform a Discover operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientID: the ID of the client to do a discover on.
// - objectID, instanceID, resourceID: the URI targeted by the operation.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_server_dm_discover(iowa_context_t contextP,
                                      uint32_t clientID,
                                      uint16_t objectID, uint16_t instanceID, uint16_t resourceID,
                                      iowa_result_callback_t resultCb,
                                      void * resultUserData);

/****************************
 * Device Management APIs for LwM2M 1.1
 */

// Perform a Read operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to read.
// - uriCount, uriP: the URIs targeted by the operation.
// - responseCb: a callback to receive the result of the operation.
// - userDataP: past as argument to resultCb.
iowa_status_t iowa_server_read(iowa_context_t contextP,
                               uint32_t clientId,
                               size_t uriCount,
                               iowa_lwm2m_uri_t *uriP,
                               iowa_response_callback_t responseCb,
                               void *userDataP);

// Start an Observation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientID: the ID of the client to observe.
// - uriCount, uriP: the URIs targeted by the operation.
// - responseCb: a callback to receive the notification.
// - userDataP: past as argument to resultCb.
// - observeIDP: OUT. ID of the observation.
iowa_status_t iowa_server_observe(iowa_context_t contextP,
                                  uint32_t clientId,
                                  size_t uriCount,
                                  iowa_lwm2m_uri_t *uriP,
                                  iowa_response_callback_t responseCb,
                                  void *userDataP,
                                  uint16_t *observeIdP);

// Cancel an Observation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client under observation.
// - observeId: ID of the observation as returned by iowa_server_observe().
iowa_status_t iowa_server_observe_cancel(iowa_context_t contextP,
                                         uint32_t clientId,
                                         uint16_t observeId);

// Perform a Write operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to write to.
// - dataCount, dataArrayP: the data to write.
// - responseCb: a callback to receive the result of the operation. This can be nil.
// - userDataP: past as argument to resultCb.
iowa_status_t iowa_server_write(iowa_context_t contextP,
                                uint32_t clientId,
                                size_t dataCount,
                                iowa_lwm2m_data_t *dataArrayP,
                                iowa_response_callback_t responseCb,
                                void *userDataP);

// Perform a Write-Attributes operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to write to.
// - uriP: the URI targeted by the operation.
// - attributesStr: the attributes to set.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - userDataP: past as argument to resultCb.
iowa_status_t iowa_server_write_attributes_string(iowa_context_t contextP,
                                                  uint32_t clientId,
                                                  iowa_lwm2m_uri_t *uriP,
                                                  const char *attributesStr,
                                                  iowa_response_callback_t responseCb,
                                                  void *userDataP);

/****************************
 * Data Push APIs
 */

// Enables/disables the data push operation for all client.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
// - responseCb: a callback to receive the result of the operation. This can be nil.
// - userDataP: past as argument to resultCb.
void iowa_server_configure_data_push(iowa_context_t contextP,
                                     iowa_response_callback_t responseCb,
                                     void *userDataP);


/**************************************************************
* Bootstrap Server Role APIs
**************************************************************/

 // The bootstrap operation result callback.
 // operation: the type of the operation.
 // clientId: the ID of the client targeted by the operation.
 // objectId, instanceId, resourceId: the URI targeted by the operation.
 // status: the status of the operation.
 // length, buffer: the data returned by the client if any.
 // resultUserData: the iowa_bootstrap_server_...() parameter.
 // contextP: the IOWA context on which the bootstrap API iowa_bootstrap_server_write(), iowa_bootstrap_server_delete() or iowa_bootstrap_server_finish() was called.
typedef void(*iowa_bootstrap_result_callback_t) (iowa_bootstrap_operation_t operation,
                                                 uint16_t clientId,
                                                 uint16_t objectId, uint16_t instanceId, uint16_t resourceId,
                                                 iowa_status_t status,
                                                 size_t length, uint8_t *buffer,
                                                 void *resultUserData,
                                                 iowa_context_t contextP);

// Inform the stack of a new incoming connection.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - type: the type of the connection.
// - connP: the connection of the same opaque type as returned by iowa_system_connection_open().
// - isSecure: set to true if the security must be enabled on this connection.
iowa_status_t iowa_bootstrap_server_new_incoming_connection(iowa_context_t contextP,
                                                            iowa_connection_type_t type,
                                                            void * connP,
                                                            bool isSecure);

/****************************
 * Configuration APIs
 */

// Configure the bootstrap server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - monitorCb: callback called by IOWA when a client updates its status.
// - callbackUserData: past as argument to monitorCb.
iowa_status_t iowa_bootstrap_server_configure(iowa_context_t contextP,
                                              iowa_monitor_callback_t monitorCb,
                                              void *callbackUserData);

/****************************
 * Bootstrap low level APIs
 */

// Perform a Read operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to write to.
// - uriP: the URI targeted by the operation.
// - responseCb: a callback to receive the result of the operation.
// - userDataP: past as argument to responseCb.
iowa_status_t iowa_bootstrap_server_read(iowa_context_t contextP,
                                         uint32_t clientId,
                                         iowa_lwm2m_uri_t *uriP,
                                         iowa_response_callback_t responseCb,
                                         void *userDataP);

// Perform a Write operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client to write to.
// - objectId, instanceId: the Object Instance targeted by the operation.
// - dataCount, dataArray: the data to write.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_write(iowa_context_t contextP,
                                          uint32_t clientId,
                                          uint16_t objectId, uint16_t instanceId,
                                          size_t dataCount, iowa_lwm2m_data_t *dataArray,
                                          iowa_bootstrap_result_callback_t resultCb,
                                          void *resultUserData);

// Send a Delete on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
// - objectId, instanceId, resourceId: the URI to delete.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_delete(iowa_context_t contextP,
                                           uint32_t clientId,
                                           uint16_t objectId, uint16_t instanceId,
                                           iowa_bootstrap_result_callback_t resultCb,
                                           void *resultUserData);

// Perform a Discover operation on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientID: the ID of the client to do a discover on.
// - objectID: the ID of the Object targeted by the operation.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_discover(iowa_context_t contextP,
                                             uint32_t clientId,
                                             uint16_t objectId,
                                             iowa_bootstrap_result_callback_t resultCb,
                                             void * resultUserData);

// Send a Finish on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_finish(iowa_context_t contextP,
                                           uint32_t clientId,
                                           iowa_bootstrap_result_callback_t resultCb,
                                           void *resultUserData);

/****************************
 * Bootstrap high level APIs
 */

// Add a server on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
// - shortServerId: the short ID assigned to this Server.
// - uri: the URI to reach this Server e.g. "coap://127.0.0.1:5683", "coaps://[::1]:5684", "sms://+33102030405"
// - lifetime: the lifetime in seconds of the registration.
// - securityDataP: the security data to use when connecting to this Server.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_add_server(iowa_context_t contextP,
                                               uint32_t clientId,
                                               uint16_t shortServerId,
                                               const char *uri,
                                               uint32_t lifetime,
                                               iowa_security_data_t *securityDataP,
                                               iowa_bootstrap_result_callback_t resultCb,
                                               void *resultUserData);

// Remove a server from a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
// - shortServerId: the short ID assigned to this Server.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_remove_server(iowa_context_t contextP,
                                                  uint32_t clientId,
                                                  uint16_t shortServerId,
                                                  iowa_bootstrap_result_callback_t resultCb,
                                                  void *resultUserData);

// Add a bootstrap server on a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
// - uri: the URI to reach this Server e.g. "coap://127.0.0.1:5683", "coaps://[::1]:5684", "sms://+33102030405"
// - clientHoldOff: the number of seconds to wait before initiating a Client Initiated Bootstrap.
// - bootstrapAccountTimeout: time to wait by the client before to purge the LwM2M Bootstrap-Server Account.
// - securityDataP: the security data to use when connecting to this Server.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_add_bootstrap_server(iowa_context_t contextP,
                                                         uint32_t clientId,
                                                         const char *uri,
                                                         int32_t clientHoldOff,
                                                         uint32_t bootstrapAccountTimeout,
                                                         iowa_security_data_t *securityDataP,
                                                         iowa_bootstrap_result_callback_t resultCb,
                                                         void *resultUserData);

// Remove a bootstrap server from a client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
// - resultCb: a callback to receive the result of the operation. This can be nil.
// - resultUserData: past as argument to resultCb.
iowa_status_t iowa_bootstrap_server_remove_bootstrap_server(iowa_context_t contextP,
                                                            uint32_t clientId,
                                                            iowa_bootstrap_result_callback_t resultCb,
                                                            void *resultUserData);

/**************************************************************
* Bootstrap Server / Server Role APIs
**************************************************************/

// The verify client callback.
// Returned value: IOWA_COAP_NO_ERROR if the client is known, else otherwise.
// clientP: the client to verify.
// state: the state of the client among
//      - IOWA_STATE_REGISTERING: when a new or returning client try to register.
//      - IOWA_STATE_UPDATING: when a client updates its registration.
//      - IOWA_STATE_BOOTSTRAP_REQUIRED: when a new or returning client connects to the bootstrap server.
// callbackUserData: the iowa_server_configure() parameter.
// contextP: the IOWA context on which server_configure was called.
typedef iowa_status_t (*iowa_verify_client_callback_t) (const iowa_client_t *clientP,
                                                        iowa_state_t state,
                                                        void *callbackUserData,
                                                        iowa_context_t contextP);

// Configure the verify client callback.
// Returned value: None.
// Parameters:
// - contextP: returned by iowa_init().
// - verifyClientCb: callback called by IOWA when a client registers to the Server.
// - callbackUserData: past as argument to verifyClientCb.
void iowa_server_set_verify_client_callback(iowa_context_t contextP,
                                            iowa_verify_client_callback_t verifyClientCb,
                                            void *callbackUserData);

// Set the content format to use when requesting data from a Client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientID: the ID of the client.
// - multiResourcesFormat: format to use with multiples resources.
// - singleResourceFormat: format to use with single resource.
iowa_status_t iowa_server_set_response_format(iowa_context_t contextP,
                                              uint32_t clientID,
                                              iowa_content_format_t multiResourcesFormat,
                                              iowa_content_format_t singleResourceFormat);

// Set the content format to use when sending data to a Client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientID: the ID of the client.
// - multiResourcesFormat: format to use with multiples resources.
// - singleResourceFormat: format to use with single resource.
iowa_status_t iowa_server_set_payload_format(iowa_context_t contextP,
                                             uint32_t clientID,
                                             iowa_content_format_t multiResourcesFormat,
                                             iowa_content_format_t singleResourceFormat);

// Close the current connection with a Client.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the ID of the client.
iowa_status_t iowa_server_close_client_connection(iowa_context_t contextP,
                                                  uint32_t clientId);

#ifdef __cplusplus
}
#endif

#endif
