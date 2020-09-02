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
* Copyright (c) 2018-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Fabien Fleutot - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Scott Bertin - Please refer to git log
 *
 *******************************************************************************/
/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>

*/

#ifndef _IOWA_PRV_LWM2M_INTERNALS_INCLUDE_
#define _IOWA_PRV_LWM2M_INTERNALS_INCLUDE_

#include "iowa_config.h"

#include "iowa_prv_core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <inttypes.h>

#define LWM2M_SERVER_STR_STATUS(S)                                \
((S) == STATE_DISCONNECTED ? "STATE_DISCONNECTED" :               \
((S) == STATE_WAITING_CONNECTION ? "STATE_WAITING_CONNECTION" :   \
((S) == STATE_REG_REGISTERING ? "STATE_REG_REGISTERING" :         \
((S) == STATE_REG_REGISTERED ? "STATE_REG_REGISTERED" :           \
((S) == STATE_REG_FAILED ? "STATE_REG_FAILED" :                   \
((S) == STATE_REG_UPDATE_PENDING ? "STATE_REG_UPDATE_PENDING" :   \
((S) == STATE_BS_HOLD_OFF ? "STATE_BS_HOLD_OFF" :                 \
((S) == STATE_BS_REQUEST_PENDING ? "STATE_BS_REQUEST_PENDING" :   \
((S) == STATE_BS_BOOTSTRAPPING ? "STATE_BS_BOOTSTRAPPING" :       \
((S) == STATE_BS_FINISHED ? "STATE_BS_FINISHED" :                 \
((S) == STATE_BS_FAILED ? "STATE_BS_FAILED" :                     \
"Unknown")))))))))))

#define STR_MEDIA_TYPE(M)                                                   \
((M) == IOWA_CONTENT_FORMAT_TEXT ? "IOWA_CONTENT_FORMAT_TEXT" :             \
((M) == LWM2M_CONTENT_FORMAT_CORE_LINK ? "LWM2M_CONTENT_FORMAT_CORE_LINK" : \
((M) == IOWA_CONTENT_FORMAT_OPAQUE ? "IOWA_CONTENT_FORMAT_OPAQUE" :         \
((M) == IOWA_CONTENT_FORMAT_CBOR ? "IOWA_CONTENT_FORMAT_CBOR" :             \
((M) == IOWA_CONTENT_FORMAT_SENML_JSON ? "IOWA_CONTENT_FORMAT_SENML_JSON" : \
((M) == IOWA_CONTENT_FORMAT_SENML_CBOR ? "IOWA_CONTENT_FORMAT_SENML_CBOR" : \
((M) == IOWA_CONTENT_FORMAT_TLV_OLD ? "IOWA_CONTENT_FORMAT_TLV_OLD" :       \
((M) == IOWA_CONTENT_FORMAT_JSON_OLD ? "IOWA_CONTENT_FORMAT_JSON_OLD" :     \
((M) == IOWA_CONTENT_FORMAT_TLV ? "IOWA_CONTENT_FORMAT_TLV" :               \
((M) == IOWA_CONTENT_FORMAT_JSON ? "IOWA_CONTENT_FORMAT_JSON" :             \
"Unknown"))))))))))

#define LWM2M_STR_STATE(S)                                    \
((S) == STATE_INITIAL ? "STATE_INITIAL" :                     \
((S) == STATE_BOOTSTRAP ? "STATE_BOOTSTRAP" :                 \
((S) == STATE_DEVICE_MANAGEMENT ? "STATE_DEVICE_MANAGEMENT" : \
"Unknown")))

#define STR_DATA_TYPE(M)                                                \
((M) == IOWA_LWM2M_TYPE_UNDEFINED ? "IOWA_LWM2M_TYPE_UNDEFINED" :       \
((M) == IOWA_LWM2M_TYPE_STRING ? "IOWA_LWM2M_TYPE_STRING" :             \
((M) == IOWA_LWM2M_TYPE_OPAQUE ? "IOWA_LWM2M_TYPE_OPAQUE" :             \
((M) == IOWA_LWM2M_TYPE_INTEGER ? "IOWA_LWM2M_TYPE_INTEGER" :           \
((M) == IOWA_LWM2M_TYPE_FLOAT ? "IOWA_LWM2M_TYPE_FLOAT" :               \
((M) == IOWA_LWM2M_TYPE_BOOLEAN ? "IOWA_LWM2M_TYPE_BOOLEAN" :           \
((M) == IOWA_LWM2M_TYPE_CORE_LINK ? "IOWA_LWM2M_TYPE_CORE_LINK" :       \
((M) == IOWA_LWM2M_TYPE_OBJECT_LINK ? "IOWA_LWM2M_TYPE_OBJECT_LINK" :   \
((M) == IOWA_LWM2M_TYPE_TIME ? "IOWA_LWM2M_TYPE_TIME" :                 \
((M) == IOWA_LWM2M_TYPE_URI_ONLY ? "IOWA_LWM2M_TYPE_URI_ONLY" :         \
"Unknown"))))))))))

#define STR_DM_OPERATION(M)                                     \
((M) == IOWA_DM_UNDEFINED ? "IOWA_DM_UNDEFINED" :               \
((M) == IOWA_DM_READ ? "IOWA_DM_READ" :                         \
((M) == IOWA_DM_FREE ? "IOWA_DM_FREE" :                         \
((M) == IOWA_DM_WRITE ? "IOWA_DM_WRITE" :                       \
((M) == IOWA_DM_EXECUTE ? "IOWA_DM_EXECUTE" :                   \
((M) == IOWA_DM_CREATE ? "IOWA_DM_CREATE" :                     \
((M) == IOWA_DM_DELETE ? "IOWA_DM_DELETE" :                     \
((M) == IOWA_DM_DISCOVER ? "IOWA_DM_DISCOVER" :                 \
((M) == IOWA_DM_WRITE_ATTRIBUTES ? "IOWA_DM_WRITE_ATTRIBUTES" : \
((M) == IOWA_DM_NOTIFY ? "IOWA_DM_NOTIFY" :                     \
((M) == IOWA_DM_CANCEL ? "IOWA_DM_CANCEL" :                     \
"Unknown")))))))))))

#define STR_BOOTSTRAP_OPERATION(M)                                                          \
((M) == IOWA_BOOTSTRAP_UNDEFINED ? "IOWA_BOOTSTRAP_UNDEFINED" :                             \
((M) == IOWA_BOOTSTRAP_READ ? "IOWA_BOOTSTRAP_READ" :                                       \
((M) == IOWA_BOOTSTRAP_WRITE ? "IOWA_BOOTSTRAP_WRITE" :                                     \
((M) == IOWA_BOOTSTRAP_DELETE ? "IOWA_BOOTSTRAP_DELETE" :                                   \
((M) == IOWA_BOOTSTRAP_DISCOVER ? "IOWA_BOOTSTRAP_DISCOVER" :                               \
((M) == IOWA_BOOTSTRAP_FINISH ? "IOWA_BOOTSTRAP_FINISH" :                                   \
((M) == IOWA_BOOTSTRAP_ADD_SERVER ? "IOWA_BOOTSTRAP_ADD_SERVER" :                           \
((M) == IOWA_BOOTSTRAP_REMOVE_SERVER ? "IOWA_BOOTSTRAP_REMOVE_SERVER" :                     \
((M) == IOWA_BOOTSTRAP_ADD_BOOTSTRAP_SERVER ? "IOWA_BOOTSTRAP_ADD_BOOTSTRAP_SERVER" :       \
((M) == IOWA_BOOTSTRAP_REMOVE_BOOTSTRAP_SERVER ? "IOWA_BOOTSTRAP_REMOVE_BOOTSTRAP_SERVER" : \
"Unknown"))))))))))

#define REG_RESOURCE_TYPE     "\"oma.lwm2m\""
#define REG_RESOURCE_TYPE_LEN (size_t)11

#define ID_MAX_STRING_LEN (size_t)5   // 65535
#define REG_PATH_DELIMITER  '/'

#define URI_REGISTRATION_SEGMENT        "rd"
#define URI_REGISTRATION_SEGMENT_LEN    (size_t)2
#define URI_BOOTSTRAP_SEGMENT           "bs"
#define URI_BOOTSTRAP_SEGMENT_LEN       (size_t)2
#define URI_SEND_SEGMENT                "dp"
#define URI_SEND_SEGMENT_LEN            (size_t)2

#define QUERY_STARTER        "?"
#define QUERY_STARTER_LEN    (size_t)1
#define QUERY_NAME           "ep="
#define QUERY_NAME_LEN       (size_t)3       // strlen("ep=")
#define QUERY_SMS            "sms="
#define QUERY_SMS_LEN        (size_t)4
#define QUERY_LIFETIME       "lt="
#define QUERY_LIFETIME_LEN   (size_t)3
#define QUERY_VERSION        "lwm2m="
#define QUERY_VERSION_LEN    (size_t)6
#define QUERY_BINDING        "b="
#define QUERY_BINDING_LEN    (size_t)2
#define QUERY_QUEUE_MODE     "Q"
#define QUERY_QUEUE_MODE_LEN (size_t)1
#define QUERY_DELIMITER      "&"
#define QUERY_DELIMITER_LEN  (size_t)1

#define QUERY_BINDING_UDP        'U'
#define QUERY_BINDING_TCP        'T'
#define QUERY_BINDING_SMS        'S'
#define QUERY_BINDING_NON_IP     'N'
#define QUERY_BINDING_QUEUE_MODE 'Q'

#define QUERY_BINDING_MAX_LEN  (size_t)1
#define QUERY_LIFETIME_MAX_LEN (size_t)10    // strlen("4294967295") 2^32-1
#define QUERY_SEPARATOR        '&'

#define LWM2M_VERSION_1_0      "1.0"
#define LWM2M_VERSION_1_1      "1.1"
#define LWM2M_VERSION_LEN      (size_t)3

#define LWM2M_DEFAULT_VERSION LWM2M_VERSION_1_0

#define ATTR_MIN_PERIOD_STR      "pmin="
#define ATTR_MIN_PERIOD_LEN      (size_t)5
#define ATTR_MAX_PERIOD_STR      "pmax="
#define ATTR_MAX_PERIOD_LEN      (size_t)5
#define ATTR_GREATER_THAN_STR    "gt="
#define ATTR_GREATER_THAN_LEN    (size_t)3
#define ATTR_LESS_THAN_STR       "lt="
#define ATTR_LESS_THAN_LEN       (size_t)3
#define ATTR_STEP_STR            "st="
#define ATTR_STEP_LEN            (size_t)3
#define ATTR_MIN_EVAL_PERIOD_STR "epmin="
#define ATTR_MIN_EVAL_PERIOD_LEN (size_t)6
#define ATTR_MAX_EVAL_PERIOD_STR "epmax="
#define ATTR_MAX_EVAL_PERIOD_LEN (size_t)6

#define URI_MAX_STRING_LEN      (size_t)24 // /65535/65535/65535/65535
#define LOCATION_MAX_STRING_LEN (size_t)12 // /65535/65535

#define ATTR_FLAG_NUMERIC (uint8_t)(LWM2M_ATTR_FLAG_LESS_THAN | LWM2M_ATTR_FLAG_GREATER_THAN | LWM2M_ATTR_FLAG_STEP)

typedef enum
{
    LWM2M_URI_TYPE_UNKNOWN,
    LWM2M_URI_TYPE_DM,
    LWM2M_URI_TYPE_REGISTRATION,
    LWM2M_URI_TYPE_BOOTSTRAP
} lwm2m_uri_type_t;

#define CONTEXT_FLAG_INSIDE_CALLBACK (uint8_t)0x01
#define CONTEXT_FLAG_NOTIFY_REQUIRED (uint8_t)0x02

#define LWM2M_OBSERVE_FLAG_UPDATE     (uint8_t)0x01 // indicates if observe's value has been updated, used in lwm2m_observed_t and lwm2m_observed_uri_info_t
#define LWM2M_OBSERVE_FLAG_INTEGER    (uint8_t)0x02 // indicates if observe's value is an integer, used in lwm2m_observed_uri_info_t
#define LWM2M_OBSERVE_FLAG_FLOAT      (uint8_t)0x04 // indicates if observe's value is a float, used in lwm2m_observed_uri_info_t
#define LWM2M_OBSERVE_FLAG_URI_UNSET  (uint8_t)0x08 // indicates if observe's uri is unset due to instance deletion, used in lwm2m_observed_uri_info_t

// Macro to check if observe's value is numeric
// Returned value: true if observe's value is numeric, else false.
// Parameters:
// - ObsInfo: observe's information, lwm2m_observed_uri_info_t.
#define LWM2M_OBSERVE_IS_NUMERIC(ObsInfo) (((ObsInfo)->flags & (LWM2M_OBSERVE_FLAG_INTEGER|LWM2M_OBSERVE_FLAG_FLOAT)) != 0)

#define LWM2M_OBSERVE_REQUEST_NEW     0 // coap option's value for new observation
#define LWM2M_OBSERVE_REQUEST_CANCEL  1 // coap option's value for cancellation

#define PRV_INT32_MAX_STR_LEN (size_t)11
#define PRV_FLOAT_MAX_STR_LEN (size_t)(PRV_INT32_MAX_STR_LEN + 1 + 17)  // Arbitrary: integer + 1 for dot + 17 for double IEEE-754 decimal digits
#define PRV_URI_BUFFER_SIZE (size_t)15

typedef struct
{
    uint32_t                 clientID;
    iowa_lwm2m_uri_t         uri;
    char                    *altPath;
    lwm2m_result_callback_t  callback;
    void                    *userData;
} dm_data_t;

typedef struct
{
    iowa_lwm2m_uri_t                   uri;
    uint32_t                           clientId;
    lwm2m_bootstrap_result_callback_t  resultCallback;
    void                              *userData;
} bs_data_t;

typedef struct
{
    iowa_content_format_t  format;
    uint32_t               counter;
    uint8_t                token[COAP_MSG_TOKEN_MAX_LEN];
    uint8_t                tokenLen;
    size_t                 bufferLength;
    uint8_t               *buffer;
} lwm2m_value_t;

// defined in acl.c
bool aclRightsServerCheck(iowa_context_t contextP, uint16_t objectId, uint16_t instanceId, uint16_t serverId, uint8_t right);
iowa_status_t aclRightsServerSet(iowa_context_t contextP, uint16_t objectId, uint16_t instanceId, uint16_t serverId, uint8_t accessRights);
iowa_status_t aclRightsServerClear(iowa_context_t contextP, uint16_t objectId, uint16_t instanceId, uint16_t serverId);
iowa_status_t aclRightsObjectClear(iowa_context_t contextP, uint16_t objectId, uint16_t instanceId);

//defined in server.c
void serverNotificationResult(iowa_context_t contextP, lwm2m_observation_t *observationP, uint8_t code, iowa_coap_message_t *responseP);

// defined in attributes.c

// Set the attribute of the given uri.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
// - uriP: uri to write attribute.
// - serverP: server related to the attribute.
// - optionP: list of option which contains the attribute to write.
iowa_status_t attributesWrite(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, lwm2m_server_t *serverP, iowa_coap_option_t *optionP);

// Return the attribute of the given uri.
// Returned value: true if there is attribute or false.
// Parameters:
// - serverP: server related to the attribute.
// - uriP: uri to get attribute.
// - attrP: empty attr stuct to fill.
// - useInheritance: get also the inherited attribute.
// - getDefault: get the default server attribute if attribute aren't set on the uri.
bool attributesGet(lwm2m_server_t *serverP, iowa_lwm2m_uri_t *uriP, attributes_t *attrP, bool useInheritance, bool getDefault);

// Free the attribute list of the given server.
// Parameters:
// - serverP: server on which attribute list is freed.
void attributesRemoveFromServer(lwm2m_server_t *serverP);

// defined in packet.c
void lwm2m_client_handle_out_of_bound_request(iowa_coap_peer_t *fromPeer, uint8_t code, iowa_coap_message_t *requestP, void *userData, iowa_context_t contextP);
void lwm2m_client_handle_request(iowa_coap_peer_t *fromPeer, uint8_t code, iowa_coap_message_t *requestP, void *userData, iowa_context_t contextP);
void lwm2m_server_handle_request(iowa_coap_peer_t *fromPeer, uint8_t code, iowa_coap_message_t *requestP, void *userData, iowa_context_t contextP);
void lwm2m_bootstrap_server_handle_request(iowa_coap_peer_t *fromPeer, uint8_t code, iowa_coap_message_t *requestP, void *userData, iowa_context_t contextP);

// defined in iowa_uri.c

// Get URI from CoAP message
// Returned value: lwm2m_uri_type_t. If any error, return LWM2M_URI_TYPE_UNKNOWN
// Parameters:
// - altPath: alternate path of the uri. Only if LWM2M_ALTPATH_SUPPORT is defined. This can be nil.
// - messageP: Message CoAP contaning the URI wanted. Can not be nil.
// - number: CoAP option number where the URI wanted is set.
// - uriP: OUT. LwM2M URI, useful if result is LWM2M_URI_TYPE_DM.
lwm2m_uri_type_t uri_decode(iowa_coap_message_t *messageP, uint16_t number, iowa_lwm2m_uri_t *uriP);

// Get CoAP option from URI
// Returned value: CoAP option allocated and filled. If any error, return NULL.
// Parameters:
// - number: CoAP option number wanted.
// - altPath: alternate path of the uri. Only if LWM2M_ALTPATH_SUPPORT is defined. This can be nil.
// - uriP: LwM2M URI. This can be nil.
// - buffer: OUT. buffer to store the CoAP option values.
iowa_coap_option_t * uri_encode(uint16_t number, iowa_lwm2m_uri_t *uriP, uint8_t buffer[PRV_URI_BUFFER_SIZE]);

int uri_getNumber(uint8_t * uriString, size_t uriLength);

// defined in objects.c

// Read only readable ressources on a URI.
// Returned value: IOWA_COAP_205_CONTENT in case of success or an error status.
// Parameters:
// - contextP: set in lwm2m_init().
// - uriP: the URI targeted by the operation to append on data.
// - serverShortId: the short ID of the Server making the operation.
// - dataCountP, dataArrayP: OUT. value of LwM2M data.
iowa_status_t object_read(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, uint16_t serverShortId, size_t *dataCountP, iowa_lwm2m_data_t **dataArrayP);

// Read a block of a readable ressource.
// Returned value: IOWA_COAP_205_CONTENT in case of success or an error status.
// Parameters:
// - contextP: set in lwm2m_init().
// - uriP: the URI targeted by the operation to append on data.
// - serverShortId: the short ID of the Server making the operation.
// - blockInfo: the value of the CoAP block2 option.
// - dataCountP, dataArrayP: OUT. value of LwM2M data.
iowa_status_t object_readBlock(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, uint32_t blockInfo, size_t *dataCountP, iowa_lwm2m_data_t **dataArrayP);

// Read ressources on a URI.
// Returned value: IOWA_COAP_205_CONTENT in case of success or an error status.
// Parameters:
// - contextP: set in lwm2m_init().
// - uriP: the URI targeted by the operation to append on data.
// - dataCountP, dataArrayP: OUT. value of LwM2M data.
iowa_status_t object_bootstrapRead(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, size_t *dataCountP, iowa_lwm2m_data_t **dataArrayP);

// Free the memory allocated during a read.
// Parameters:
// - contextP: set in lwm2m_init().
// - dataCount, dataP: value of LwM2M data to free.
void object_free(iowa_context_t contextP, size_t dataCount, iowa_lwm2m_data_t *dataP);

iowa_status_t object_write(iowa_context_t contextP, uint16_t serverShortId, size_t dataCount, iowa_lwm2m_data_t *dataP);
iowa_status_t object_create(iowa_context_t contextP, uint16_t serverShortId, size_t dataCount, iowa_lwm2m_data_t *dataP);
iowa_status_t object_execute(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, uint16_t serverShortId, uint8_t *buffer, size_t length);
iowa_status_t object_delete(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, uint16_t serverShortId);
iowa_status_t object_readRequest(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, uint16_t serverShortId);
iowa_status_t object_checkReadable(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP);
bool object_isAsynchronous(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP);
iowa_lwm2m_data_type_t object_getResourceType(uint16_t objectId, uint16_t resourceId, void *userDataP);
iowa_status_t object_getList(iowa_context_t contextP, uint16_t objectId, link_t **linkP, size_t *nbLinkP);
void object_sendReadEvent(iowa_context_t contextP, uint16_t objectId, uint16_t instanceId);

// get object tree with attributes. Currently attributes inheritance are only available for discover
iowa_status_t object_getTree(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, lwm2m_server_t *serverP, link_t **linkP, size_t *nbLinkP);

uint16_t object_getInstanceIndex(lwm2m_object_t *objectP, uint16_t id);
iowa_status_t object_bootstrapWrite(iowa_context_t contextP, size_t dataCount, iowa_lwm2m_data_t *dataP);
iowa_status_t object_checkWritePayload(iowa_context_t contextP, size_t dataCount, iowa_lwm2m_data_t *dataArray);

// Merge data array into data.
// Returned value: IOWA_COAP_205_CONTENT in case of success or an error status.
// Parameters:
// - dataP: OUT. value of data.
// - dataCountP: OUT. size of data.
// - dataArrayP: IN. value of data array.
// - dataArrayCount: IN. size of data array.
iowa_status_t lwm2mMergeData(iowa_lwm2m_data_t **dataP, size_t *dataCountP, lwm2m_data_array_t *dataArrayP, size_t dataArrayCount);

// defined in management.c
void dm_handleRequest(iowa_context_t contextP, iowa_lwm2m_uri_t * uriP, lwm2m_server_t * serverP, iowa_coap_message_t * messageP);
void asyncOperation_tagOperation(lwm2m_server_t *serverP, iowa_lwm2m_uri_t *uriP);
void asyncOperation_updateValue(iowa_context_t contextP, iowa_sensor_t sensorId);
void asyncOperation_step(iowa_context_t contextP);

/****************************
* defined in observe.c
*/
// Start or cancel an observe.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - uriCount, uriP: the URIs targeted by the operation.
// - serverP: the source of the request.
// - dataCount, dataP: the current value of the URIs targeted.
// - obsOptionP: observe's coap option value to determine if it is to start a new observation or to cancel an existant one.
// - requestP: request's coap message.
// - responseP: OUT. part of the response's coap message.
// - format: preferred format for the response's coap message.
iowa_status_t observe_handleRequest(iowa_context_t contextP, size_t uriCount, iowa_lwm2m_uri_t *uriP, lwm2m_server_t *serverP, size_t dataCount, iowa_lwm2m_data_t *dataP, iowa_coap_option_t *obsOptionP, iowa_coap_message_t *requestP, iowa_coap_message_t *responseP, iowa_content_format_t format);

void observe_cancel(iowa_context_t contextP, lwm2m_server_t *serverP, iowa_coap_message_t *messageP);
iowa_status_t observe_setParameters(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, lwm2m_server_t *serverP);
void observe_delete(lwm2m_observed_t *observedP);
void observeRemoveFromServer(lwm2m_server_t *serverP);
void observe_remove(lwm2m_observation_t * observationP);
void observe_step(iowa_context_t contextP);
void observe_clear(iowa_context_t contextP, iowa_lwm2m_uri_t * uriP);
void observe_handleNotify(iowa_context_t contextP, lwm2m_client_t *clientP, iowa_coap_peer_t *fromPeer, iowa_coap_message_t * messageP);
iowa_status_t observe_updateObserve(iowa_context_t contextP, lwm2m_server_t *serverP, lwm2m_observed_t *observedP);

// defined in registration.c
void registration_handleRequest(iowa_context_t contextP, lwm2m_client_t *clientP, iowa_coap_peer_t *fromPeer, iowa_coap_message_t *messageP);
void registration_deregister(iowa_context_t contextP, lwm2m_server_t *serverP);
void registration_resetServersStatus(iowa_context_t contextP);
iowa_status_t registration_step(iowa_context_t contextP);
void registration_removeObservation(iowa_context_t contextP, lwm2m_client_t *clientP);

// defined in bootstrap.c
iowa_status_t bootstrap_step(iowa_context_t contextP);
void bootstrap_handle_command(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP, lwm2m_server_t *serverP, iowa_coap_message_t *messageP);
void bootstrap_handle_finish(iowa_context_t contextP, iowa_coap_message_t *messageP, iowa_coap_peer_t *fromPeer);
void bootstrap_handle_request(iowa_context_t contextP, iowa_coap_peer_t *fromPeerP, iowa_coap_message_t *messageP);

// defined in iowa_value.c

// Create if necessary a storage queue for a Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the Server which the storage queue has to be created.
iowa_status_t valueCreateStorageQueue(iowa_context_t contextP, lwm2m_server_t *serverP);

// Delete if necessary a storage queue associated to a Server.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the Server which the storage queue has to be deleted.
void valueDeleteStorageQueue(iowa_context_t contextP, lwm2m_server_t *serverP);

// Store a value to a storage queue.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the Server which the value has to be stored in the storage queue.
// - valueP: the value to store.
iowa_status_t valueStoreToStorageQueue(iowa_context_t contextP, lwm2m_server_t *serverP, lwm2m_value_t *valueP);

// Peek a value from a storage queue.
// Returned value: the value or NULL if no value or NULL if there was an error.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the Server which the value has to be retrieved from the storage queue.
lwm2m_value_t * valuePeekFromStorageQueue(iowa_context_t contextP, lwm2m_server_t *serverP);

// Delete the last value of a storage queue.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the Server which the value has to be deleted from the storage queue.
void valueRemoveFromStorageQueue(iowa_context_t contextP, lwm2m_server_t *serverP);

// Free a value.
// Returned value: none.
// Parameters:
// - valueP: the value to free.
void valueFree(lwm2m_value_t *valueP);

// Get content format from CoAP message.
// Returned value: content format found or IOWA_CONTENT_FORMAT_UNSET
// Parameters:
// - messageP: CoAP message.
// - number: option of CoAP message.
// Note: if number is IOWA_COAP_OPTION_ACCEPT and content format not found, content format will be LWM2M_DEFAULT_CONTENT_FORMAT
iowa_content_format_t utils_getMediaType(iowa_coap_message_t * messageP, uint16_t number);

// Check if alt path is valid.
// Returned value: true if valid.
// Parameters:
// - altpath: pointer to the alt path.
bool utils_isAltPathValid(const char *altPath);

// Connect to the given server.
// Returned value: IOWA_COAP_NO_ERROR or an error.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the Server we want to connect to.
// - messageCb: The callback called when a CoAP request or a CoAP response is received, or when a confirmable transmission failed.
// - eventCb: The callback called when the peer generates an event.
iowa_status_t utilsConnectServer(iowa_context_t contextP, lwm2m_server_t *serverP, coap_message_callback_t messageCb, coap_event_callback_t eventCb);

// Connect to the given server using the MSISDN.
// Returned value: IOWA_COAP_NO_ERROR or an error.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the server we want to connect to.
// - messageCb: The callback called when a CoAP request or a CoAP response is received, or when a confirmable transmission failed.
// - eventCb: The callback called when the peer generates an event.
iowa_status_t utilsConnectServerMsisdn(iowa_context_t contextP, lwm2m_server_t *serverP, coap_message_callback_t messageCb, coap_event_callback_t eventCb);

// Disconnect the server and delete it peer.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the server we want to disconnect to.
void utilsDisconnectServer(iowa_context_t contextP, lwm2m_server_t *serverP);

// Free a server.
// Parameters:
// - contextP: returned by iowa_init().
// - serverP: the server we want to free.
void utilsFreeServer(iowa_context_t contextP, lwm2m_server_t *serverP);

// Get the identifier of the given peer.
// Parameters:
// - contextP: returned by iowa_init().
// - fromPeer: pointer to the peer.
// - nameP: pointer to the buffer to allocate.
iowa_status_t utils_getPeerIdentifier(iowa_context_t contextP, iowa_coap_peer_t *fromPeer, char **nameP);

void convertLwm2mClientToUserClient(iowa_context_t contextP, lwm2m_client_t *lwm2mClientP, iowa_client_t *userClientP, bool duplicate);

#endif // _IOWA_PRV_LWM2M_INTERNALS_INCLUDE_
