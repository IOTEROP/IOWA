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
* Copyright (c) 2018-2019 IoTerop.
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
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Julien Vermillard - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Ville Skyttä - Please refer to git log
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

#ifndef _IOWA_PRV_LWM2M_INCLUDE_
#define _IOWA_PRV_LWM2M_INCLUDE_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#include "iowa_config.h"
#include "iowa_client.h"
#include "iowa_IPSO_ID.h"
#include "iowa_LwM2M_ID.h"
#include "iowa_server.h"
#include "iowa_prv_coap.h"
#include "iowa_prv_security.h"
#include "iowa_prv_timer.h"

#define STR_LWM2M_TYPE(M)                                                       \
((M) == IOWA_LWM2M_TYPE_UNDEFINED ? "LWM2M_TYPE_UNDEFINED" :                    \
((M) == IOWA_LWM2M_TYPE_STRING ? "LWM2M_TYPE_STRING" :                          \
((M) == IOWA_LWM2M_TYPE_OPAQUE ? "LWM2M_TYPE_OPAQUE" :                          \
((M) == IOWA_LWM2M_TYPE_INTEGER ? "LWM2M_TYPE_INTEGER" :                        \
((M) == IOWA_LWM2M_TYPE_FLOAT ? "LWM2M_TYPE_FLOAT" :                            \
((M) == IOWA_LWM2M_TYPE_BOOLEAN ? "LWM2M_TYPE_BOOLEAN" :                        \
((M) == IOWA_LWM2M_TYPE_CORE_LINK ? "LWM2M_TYPE_CORE_LINK" :                    \
((M) == IOWA_LWM2M_TYPE_OBJECT_LINK ? "LWM2M_TYPE_OBJECT_LINK" :                \
((M) == IOWA_LWM2M_TYPE_TIME ? "LWM2M_TYPE_TIME" :                              \
((M) == IOWA_LWM2M_TYPE_UNSIGNED_INTEGER ? "IOWA_LWM2M_TYPE_UNSIGNED_INTEGER" : \
"Unknown"))))))))))

#define LWM2M_MAJOR_VERSION    1
#define LWM2M_MINOR_VERSION    0
#define LWM2M_REVISION_VERSION 2

#ifndef IOWA_PEER_IDENTIFIER_SIZE
#define IOWA_PEER_IDENTIFIER_SIZE 32 // Default value
#endif

// TLV must be support LwM2M version 1.0 is not removed
#ifndef LWM2M_VERSION_1_0_REMOVE
#ifndef LWM2M_SUPPORT_TLV
#define LWM2M_SUPPORT_TLV
#endif
#endif // LWM2M_VERSION_1_0_REMOVE


// LwM2M Servers support all the content formats and LwM2M Clients must at least support one of the following content formats (by priority order):
// - CBOR
// - SenML JSON
// - In 1.0, default format is TLV
#ifdef LWM2M_CLIENT_MODE

#if   defined(LWM2M_SUPPORT_TLV)
#define LWM2M_DEFAULT_CONTENT_FORMAT IOWA_CONTENT_FORMAT_TLV
#else
#error "Default content format not found." // Should never happen
#endif

#define LWM2M_DEFAULT_CONTENT_SINGLE_FORMAT LWM2M_DEFAULT_CONTENT_FORMAT

#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
#define IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
#endif

#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
#define IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
#endif

#else // Not LWM2M_CLIENT_MODE

#ifndef LWM2M_VERSION_1_0_REMOVE
#define LWM2M_DEFAULT_CONTENT_FORMAT IOWA_CONTENT_FORMAT_TLV
#define LWM2M_DEFAULT_CONTENT_SINGLE_FORMAT IOWA_CONTENT_FORMAT_TLV
#else
#define LWM2M_DEFAULT_CONTENT_FORMAT IOWA_CONTENT_FORMAT_SENML_CBOR
#define LWM2M_DEFAULT_CONTENT_SINGLE_FORMAT IOWA_CONTENT_FORMAT_CBOR
#endif // LWM2M_VERSION_1_0_REMOVE

#endif

// If Observe-composite is support, then Read-composite is support too

#define LWM2M_OBSERVATION_MID_ARRAY_SIZE 4

#define LWM2M_DEFAULT_LIFETIME        86400

#define LWM2M_RESERVED_FIRST_ID 0

#define LWM2M_LORAWAN_DEFAULT_LIFETIME 2592000

#define LWM2M_CONTENT_FORMAT_CORE_LINK 40

/*
 * URI
 *
 * objectId is always set
 * instanceId or resourceId are set according to the flag bit-field
 *
 */

#define LWM2M_URI_IS_SET_OBJECT(uri)            ((uri)->objectId != IOWA_LWM2M_ID_ALL)
#define LWM2M_URI_IS_SET_INSTANCE(uri)          ((uri)->instanceId != IOWA_LWM2M_ID_ALL)
#define LWM2M_URI_IS_SET_RESOURCE(uri)          ((uri)->resourceId != IOWA_LWM2M_ID_ALL)
#define LWM2M_URI_IS_SET_RESOURCE_INSTANCE(uri) ((uri)->resInstanceId != IOWA_LWM2M_ID_ALL)

#define LWM2M_URI_RESET(uri)               memset((uri), 0xFF, sizeof(iowa_lwm2m_uri_t))
#define LWM2M_URI_ARE_EQUAL(uri1, uri2)    (memcmp((uri1), (uri2), sizeof(iowa_lwm2m_uri_t)) == 0)
#define LWM2M_URI_COPY(uriDest, uriSource) memcpy((uriDest), (uriSource), sizeof(iowa_lwm2m_uri_t))

/*
 * LWM2M Servers
 *
 * Since LWM2M Server Object instances are not accessible to LWM2M servers,
 * there is no need to store them as lwm2m_objects_t
 */

typedef enum
{
    STATE_DISCONNECTED = 0,        // connection not initialized
    STATE_WAITING_CONNECTION,      // connection initialized
    STATE_REG_REGISTERING,         // registration request pending
    STATE_REG_REGISTERED,          // successfully registered
    STATE_REG_FAILED,              // last registration failed
    STATE_REG_UPDATE_PENDING,      // registration update pending
    STATE_BS_HOLD_OFF,             // bootstrap hold off time
    STATE_BS_REQUEST_PENDING,      // bootstrap request pending
    STATE_BS_BOOTSTRAPPING,        // boostrap ongoing
    STATE_BS_FINISHED,             // bootstrap done
    STATE_BS_FAILED                // bootstrap failed
} lwm2m_status_t;

typedef uint8_t lwm2m_binding_t;

#define BINDING_UNKNOWN 0x00
#define BINDING_U       0x01 // UDP
#define BINDING_T       0x02 // TCP
#define BINDING_S       0x04 // SMS
#define BINDING_N       0x08 // Non-IP
#define BINDING_Q       0x10 // Queue mode

/*
 * LWM2M result callback
 *
 * When used with an observe, if 'data' is not nil, 'status' holds the observe counter.
 */
typedef void (*lwm2m_result_callback_t) (uint32_t clientID, iowa_lwm2m_uri_t *uriP, iowa_status_t status, iowa_content_format_t format, uint8_t *data, int dataLength, void *userData, iowa_context_t contextP);

/*
 * LWM2M Observations
 *
 * Used to store observation of remote clients resources.
 * status STATE_REG_REGISTERING means the observe request was sent to the client but not yet answered.
 * status STATE_REG_REGISTERED means the client acknowledged the observe request.
 * status STATE_REG_FAILED means the user canceled the request before the client answered it.
 */

typedef struct _lwm2m_observation_
{
    struct _lwm2m_observation_  *next;  // matches lwm2m_list_t::next
    uint16_t                     id;    // matches lwm2m_list_t::id
    struct _lwm2m_client_       *clientP;
    size_t                       uriCount;
    iowa_lwm2m_uri_t            *uriP;
    lwm2m_status_t               status;
    iowa_response_callback_t     responseCb;
    void                        *userDataP;
} lwm2m_observation_t;

/*
 * LWM2M Link Attributes
 *
 * Used for observation parameters.
 *
 */

#define LWM2M_ATTR_FLAG_MIN_PERIOD      (uint8_t)0x01
#define LWM2M_ATTR_FLAG_MAX_PERIOD      (uint8_t)0x02
#define LWM2M_ATTR_FLAG_GREATER_THAN    (uint8_t)0x04
#define LWM2M_ATTR_FLAG_LESS_THAN       (uint8_t)0x08
#define LWM2M_ATTR_FLAG_STEP            (uint8_t)0x10
#define LWM2M_ATTR_FLAG_MIN_EVAL_PERIOD (uint8_t)0x20
#define LWM2M_ATTR_FLAG_MAX_EVAL_PERIOD (uint8_t)0x40

typedef struct _attributes_t
{
    struct _attributes_t *nextP;
    iowa_lwm2m_uri_t      uri;
    uint8_t               flags;
    uint32_t              minPeriod;
    uint32_t              maxPeriod;
    double                greaterThan;
    double                lessThan;
    double                step;
} attributes_t;

/*
 * LWM2M Clients
 */

typedef struct _lwm2m_client_
{
    struct _lwm2m_client_         *next;       // matches lwm2m_list_t::next
    uint32_t                       internalID; // matches lwm2m_list_t::id
    char                          *name;
    lwm2m_binding_t                binding;
    char                          *msisdn;
    char                          *altPath;
    iowa_supported_format_t        supportedFormats;
    uint32_t                       lifetime;
    iowa_timer_t                  *timerP;
    iowa_content_format_t          multiResourcesFormat;
    iowa_content_format_t          singleResourceFormat;
    iowa_content_format_t          multiResourcesAcceptFormat;
    iowa_content_format_t          singleResourceAcceptFormat;
    iowa_coap_peer_t              *peerP;
    size_t                         objectLinkCount;
    iowa_lwm2m_object_link_t      *objectLinkArray;
    lwm2m_observation_t           *observationList;
    iowa_lwm2m_protocol_version_t  lwm2mVersion;
    uint8_t                        flags;
} lwm2m_client_t;

/*
 * LWM2M data array
 */
typedef struct
{
    iowa_lwm2m_data_t *dataP;
    size_t dataCount;
} lwm2m_data_array_t;

/*
 * LWM2M observed resources
 */

typedef struct
{
    uint8_t     flags; // possibilities: LWM2M_ATTR_FLAG_MIN_PERIOD; LWM2M_ATTR_FLAG_MAX_PERIOD
    uint32_t    minPeriod;
    uint32_t    maxPeriod;
} lwm2m_time_attributes_t;

typedef struct
{
    uint8_t     flags; // possibilities: LWM2M_ATTR_FLAG_GREATER_THAN; LWM2M_ATTR_FLAG_LESS_THAN; LWM2M_ATTR_FLAG_STEP; LWM2M_ATTR_FLAG_MIN_PERIOD; LWM2M_ATTR_FLAG_MAX_PERIOD; LWM2M_ATTR_FLAG_MIN_EVAL_PERIOD; LWM2M_ATTR_FLAG_MAX_EVAL_PERIOD
    double      greaterThan;
    double      lessThan;
    double      step;
} lwm2m_uri_attributes_t;

typedef struct
{
    uint8_t                     flags; // possibilities: LWM2M_OBSERVE_FLAG_FLOAT; LWM2M_OBSERVE_FLAG_INTEGER; LWM2M_OBSERVE_FLAG_URI_UNSET; LWM2M_OBSERVE_FLAG_UPDATE
    iowa_lwm2m_uri_t            uri;
    lwm2m_uri_attributes_t     *uriAttrP;
    // Use only if observe on resourceId if tansmission failed stockage of last LWM2M_VALUE_ARRAY_SIZE values
    union
    {
        int64_t asInteger;
        double  asFloat;
    } lastValue;
} lwm2m_observed_uri_info_t;

typedef struct _lwm2m_observed_
{
    struct _lwm2m_observed_ *next;

    uint8_t                     flags; // possibilities: LWM2M_OBSERVE_FLAG_UPDATE
    size_t                      uriCount;
    lwm2m_observed_uri_info_t  *uriInfoP;
    lwm2m_time_attributes_t    *timeAttrP;
    iowa_content_format_t       format;
    uint8_t                     token[COAP_MSG_TOKEN_MAX_LEN];
    uint8_t                     tokenLen;
    int32_t                     lastTime;
    uint32_t                    counter;
    uint16_t                    lastMid[LWM2M_OBSERVATION_MID_ARRAY_SIZE];
} lwm2m_observed_t;

typedef struct _lwm2m_async_operation_
{
    struct _lwm2m_async_operation_ *next;

    uint8_t                 flags; // possibilities: LWM2M_OBSERVE_FLAG_UPDATE
    iowa_content_format_t   format;
    uint8_t                 token[COAP_MSG_TOKEN_MAX_LEN];
    uint8_t                 tokenLen;
    size_t                  uriCount;
    iowa_lwm2m_uri_t        uriArray[];
} lwm2m_async_operation_t;

typedef enum
{
    STATE_INITIAL = 0,
    STATE_BOOTSTRAP,
    STATE_DEVICE_MANAGEMENT
} lwm2m_client_state_t;

typedef struct
{
    uint8_t *pubKeyIdentity;
    size_t   pubKeyIdentityLen;
    uint8_t *serverPubKey;
    size_t   serverPubKeyLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} bootstrap_data_t;

#if !defined(IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE) || !defined(IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE)
typedef struct
{
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
    uint16_t priorityOrder;
    int32_t initialDelayTimer;
    bool blockOnFailure;
    bool bootstrapOnFailure;
#endif // IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
    uint8_t retryCount;
    int32_t retryDelayTimer;
    uint8_t sequenceRetryCount;
    int32_t sequenceDelayTimer;
#endif // IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
} lwm2m_server_registration_t;
#endif // !defined(IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE) || !defined(IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE)

#define LWM2M_OSCORE_FLAG_SECURITY_DATA_ADDED              0x01
#define LWM2M_OSCORE_FLAG_SECURITY_DATA_CLIENT_APPLICATION 0x02
#define LWM2M_OSCORE_FLAG_OBJECT_LINK_ADDED                0x04

typedef struct
{
    uint16_t          objInstId;
    uint8_t           flags;
} lwm2m_oscore_t;

#define LWM2M_SERVER_FLAG_SECURITY_DATA_ADDED              0x0001
#define LWM2M_SERVER_FLAG_SECURITY_DATA_CLIENT_APPLICATION 0x0002
#define LWM2M_SERVER_FLAG_RUNTIME_UPDATE                   0x0004
#define LWM2M_SERVER_FLAG_UPDATE                           0x0008
#define LWM2M_SERVER_FLAG_DISABLE                          0x0010
#define LWM2M_SERVER_FLAG_AVAILABLE                        0x0020
#define LWM2M_SERVER_FLAG_MSISDN_FALLBACK                  0x0040
#define LWM2M_SERVER_FLAG_BOOTSTRAP_TRIGGER                0x0080
#define LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT               0x0100
#define LWM2M_SERVER_FLAG_LORAWAN_FALLBACK                 0x0200
#define LWM2M_SERVER_FLAG_OBSERVE_SENDING                  0x0400

typedef struct
{
    uint16_t                 flags;
    iowa_coap_peer_t        *peerP;
    lwm2m_status_t           status;
    uint8_t                  update;
    char                    *location;
    lwm2m_observed_t        *observedList;
    attributes_t            *attributesList;
    iowa_timer_t            *updateTimerP;
    iowa_timer_t            *lifetimeTimerP;
#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
    uint8_t                  retryCount;
    uint8_t                  sequenceRetryCount;
#endif // IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
} lwm2m_server_runtime_t;

typedef struct _lwm2m_server_
{
    struct _lwm2m_server_        *next;         // matches lwm2m_list_t::next
    uint16_t                      shortId;      // matches lwm2m_list_t::id
    uint16_t                      secObjInstId;
    uint16_t                      srvObjInstId;
    char                         *uri;
    iowa_lwm2m_protocol_version_t lwm2mVersion;
    int32_t                       lifetime;     // lifetime of the registration in sec or 0 if default value (86400 sec), also used as hold off time for bootstrap servers
    lwm2m_binding_t               binding;      // client connection mode with this server
#ifndef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
    uint32_t                      defaultPmin;
    uint32_t                      defaultPmax;
#endif
    bool                          notifStoring;
#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
    int32_t                       disableTimeout;
#endif
    iowa_security_mode_t          securityMode;
#if !defined(IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE) || !defined(IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE)
    lwm2m_server_registration_t   registrationProcedure;
#endif
    lwm2m_server_runtime_t        runtime;
} lwm2m_server_t;

/*
 * LwM2M ACL object
 */

typedef struct _acl_server_flags_t
{
    struct _acl_server_flags_t *nextP;
    uint16_t serverId;
    uint8_t  flags;
} acl_server_flags_t;

typedef struct _lwm2m_acl_t
{
    struct _lwm2m_acl_t *nextP;
    uint16_t             aclInstanceId;
    uint16_t             objectId;
    uint16_t             instanceId;
    uint16_t             ownerId;
    acl_server_flags_t  *aclServerFlagsP;
} lwm2m_acl_t;

/*
 * LWM2M Context
 */

// In all the following APIs, the session handle MUST uniquely identify a peer.

// LWM2M bootstrap callback
// When a LWM2M client requests bootstrap information, the callback is called with status IOWA_COAP_NO_ERROR, uriP is nil and
// name is set. The callback must return a IOWA_COAP_* error code. IOWA_COAP_204_CHANGED for success.
// After a lwm2m_bootstrap_delete() or a lwm2m_bootstrap_write(), the callback is called with the status returned by the
// client, the URI of the operation (may be nil) and name is nil.
typedef void (*lwm2m_bootstrap_callback_t) (uint32_t clientId, iowa_status_t status, iowa_lwm2m_uri_t *uriP, void *userData);

typedef void (*lwm2m_bootstrap_result_callback_t) (uint32_t clientID, iowa_lwm2m_uri_t *uriP, iowa_status_t status, void *userData, iowa_context_t contextP);

// Internal resource flags
#define INTERNAL_RESOURCE_FLAG_FRAGMENTATION_SUPPORT 0x80 // The resource supports fragmentation

#define IS_RSC_USEABLE(R)      ((R).operations != IOWA_OPERATION_NONE)
#define IS_RSC_READABLE(R)     (((R).operations & IOWA_OPERATION_READ) == IOWA_OPERATION_READ)
#define IS_RSC_WRITABLE(R)     (((R).operations & IOWA_OPERATION_WRITE) == IOWA_OPERATION_WRITE)
#define IS_RSC_EXECUTABLE(R)   (((R).operations & IOWA_OPERATION_EXECUTE) == IOWA_OPERATION_EXECUTE)
#define IS_RSC_MULTIPLE(R)     (((R).flags & IOWA_RESOURCE_FLAG_MULTIPLE) == IOWA_RESOURCE_FLAG_MULTIPLE)
#define IS_RSC_MANDATORY(R)    (((R).flags & IOWA_RESOURCE_FLAG_MANDATORY) == IOWA_RESOURCE_FLAG_MANDATORY)
#define IS_RSC_SUPPORT_FRAG(R) (((R).flags & INTERNAL_RESOURCE_FLAG_FRAGMENTATION_SUPPORT) == INTERNAL_RESOURCE_FLAG_FRAGMENTATION_SUPPORT)
#define IS_RSC_STREAMABLE(R)   (((R).flags & IOWA_RESOURCE_FLAG_STREAMABLE) == IOWA_RESOURCE_FLAG_STREAMABLE)
#define IS_RSC_ASYNCHRONOUS(R) (((R).flags & IOWA_RESOURCE_FLAG_ASYNCHRONOUS) == IOWA_RESOURCE_FLAG_ASYNCHRONOUS)

#define PRV_RWE_OP_MASK (uint8_t)(IOWA_OPERATION_READ | IOWA_OPERATION_WRITE | IOWA_OPERATION_EXECUTE)

#define LWM2M_SINGLE_OBJECT_INSTANCE_ID 0

typedef enum
{
    OBJECT_SINGLE,
    OBJECT_MULTIPLE,
    OBJECT_SINGLE_ADVANCED,
    OBJECT_MULTIPLE_ADVANCED
} lwm2m_object_type_t;

typedef struct
{
    uint16_t  id;
    uint16_t  resCount;
    uint16_t *resArray;
} lwm2m_instance_details_t;

typedef struct _lwm2m_object_t
{
    struct _lwm2m_object_t     *next;
    uint16_t                    objID;
    lwm2m_object_type_t         type;
    uint16_t                    resourceCount;
    iowa_lwm2m_resource_desc_t *resourceArray;
    iowa_RWE_callback_t         dataCb;
    iowa_CD_callback_t          instanceCb;
    iowa_RI_callback_t          resInstanceCb;
    void                       *userData;
    uint16_t                    instanceCount;
    lwm2m_instance_details_t   *instanceArray;
} lwm2m_object_t;

typedef struct _lwm2m_context_t lwm2m_context_t;

struct _lwm2m_context_t
{
#ifdef LWM2M_CLIENT_MODE
    lwm2m_client_state_t  state;
    char                 *endpointName;
    lwm2m_server_t       *serverList;
    lwm2m_object_t       *objectList;
    uint8_t               internalFlag;
#endif // LWM2M_CLIENT_MODE
    void                 *userData;
};

#define LWM2M_CLIENT_FLAG_CLOSED      0x01

// initialize a LwM2M context.
iowa_status_t lwm2m_init(iowa_context_t contextP);

// close a LwM2M context.
void lwm2m_close(iowa_context_t contextP);

// perform any required pending operation and adjust timeoutP to the maximal time interval to wait in seconds.
iowa_status_t lwm2m_step(iowa_context_t contextP);

// configure the client side with the Endpoint Name, binding, MSISDN (can be nil), alternative path
// for objects (can be nil) and a list of objects.
// LWM2M Security Object (ID 0) must be present with either a bootstrap server or a LWM2M server and
// its matching LWM2M Server Object (ID 1) instance
iowa_status_t lwm2m_configure(iowa_context_t contextP, const char * endpointName, const char * msisdn);

#define LWM2M_UPDATE_FLAG_NONE     0x00
#define LWM2M_UPDATE_FLAG_OBJECTS  0x01
#define LWM2M_UPDATE_FLAG_LIFETIME 0x02

// Send a registration update to a server or to all the connected servers if the pointer is nil.
void lwm2mUpdateRegistration(iowa_context_t contextP, lwm2m_server_t *serverP, uint8_t update);

void lwm2m_resource_value_changed(iowa_context_t contextP, iowa_lwm2m_uri_t *uriP);

// Device Management APIs
int lwm2m_dm_discover(iowa_context_t contextP, uint32_t clientID, iowa_lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_execute(iowa_context_t contextP, uint32_t clientID, iowa_lwm2m_uri_t * uriP, iowa_content_format_t format, uint8_t * buffer, int length, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_create(iowa_context_t contextP, uint32_t clientID, iowa_lwm2m_uri_t * uriP, iowa_content_format_t format, uint8_t * buffer, int length, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_delete(iowa_context_t contextP, uint32_t clientID, iowa_lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);

// Boostrap Interface APIs
// if uriP is nil, a "Delete /" is sent to the client
int lwm2m_bootstrap_delete(iowa_context_t contextP, uint32_t clientId, iowa_lwm2m_uri_t *uriP, lwm2m_bootstrap_result_callback_t resultCallback, void *userData);
int lwm2m_bootstrap_write(iowa_context_t contextP, uint32_t clientId, iowa_lwm2m_uri_t *uriP, iowa_content_format_t format, uint8_t *buffer, size_t length, lwm2m_bootstrap_result_callback_t resultCallback, void *userData);
int lwm2m_bootstrap_finish(iowa_context_t contextP, uint32_t clientId, lwm2m_bootstrap_result_callback_t resultCallback, void *userData);

// Server APIs
void lwm2m_bootstrap_server_close(iowa_context_t contextP, lwm2m_server_t *serverP);
void lwm2m_server_close(iowa_context_t contextP, lwm2m_server_t *serverP, bool sendDeregistration);

iowa_status_t objectAddInstance(iowa_context_t contextP, uint16_t objectID, uint16_t instanceID, uint16_t resourceCount, uint16_t *resourceArray);
iowa_status_t objectRemoveInstance(iowa_context_t contextP, uint16_t objectID, uint16_t instanceID);
iowa_status_t customObjectAdd(iowa_context_t contextP,
                              uint16_t objectID, lwm2m_object_type_t type, uint16_t instanceCount, void *instanceIDs, uint16_t resourceCount, iowa_lwm2m_resource_desc_t *resourceArray,
                              iowa_RWE_callback_t dataCallback, iowa_CD_callback_t instanceCallback, iowa_RI_callback_t resInstanceCallback, void *userData);
void customObjectDelete(lwm2m_object_t *objectP);
iowa_status_t customObjectRemove(iowa_context_t contextP, uint16_t objectID);
void customObjectResourceChanged(iowa_context_t contextP, uint16_t objectID, uint16_t instanceID, uint16_t resourceID);
bool customObjectHasResource(iowa_context_t contextP, uint16_t objectID, uint16_t instanceID, uint16_t resourceID);

// CoRE Link APIs
typedef enum
{
    KEY_UNKNOWN,
    KEY_LWM2M_VERSION,
    KEY_SSID,
    KEY_SERVER_URI,
    KEY_CONTENT_FORMAT,
    KEY_RESOURCE_TYPE,
    KEY_OBJECT_VERSION,
    KEY_DIMENSION,
    KEY_PERIOD_MINIMUM,
    KEY_PERIOD_MAXIMUM,
    KEY_LESS_THAN,
    KEY_GREATER_THAN,
    KEY_STEP,
    KEY_EVAL_PERIOD_MINIMUM,
    KEY_EVAL_PERIOD_MAXIMUM
} attribute_key_t;

typedef struct _attribute_t
{
    struct _attribute_t *nextP;
    attribute_key_t      key;
    union
    {
        int64_t asInteger;
        double  asFloat;
        struct
        {
            size_t   length;
            uint8_t *buffer;
        } asBuffer;
    } value;
} attribute_t;

typedef struct
{
    iowa_lwm2m_uri_t uri;
    attribute_t *attrP;
} link_t;

// Serialize the link array
// Returned value: IOWA_COAP_NO_ERROR or an error.
// Parameters:
// - linkP: pointer to the array.
// - nbLink: number of element in the array.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
iowa_status_t coreLinkSerialize(link_t *linkP, size_t nbLink, uint8_t **bufferP, size_t *bufferLengthP);

// Serialize the link array
// Returned value: IOWA_COAP_NO_ERROR or an error.
// Parameters:
// - buffer: buffer to deserialize.
// - bufferLength: length of the buffer.
// - linkP, nbLinkP: OUT. deserialized, dynamically allocated link array.
iowa_status_t coreLinkDeserialize(uint8_t *buffer, size_t bufferLength, link_t **linkP, size_t *nbLinkP);

void lwm2m_uri_set(iowa_lwm2m_uri_t *uriP, uint16_t objectId, uint16_t instanceId, uint16_t resourceId, uint16_t resInstanceId);

// Add a buffer attribute to the link.
// Returned value: IOWA_COAP_NO_ERROR or an error.
// Parameters:
// - linkP: pointer to the link.
// - key: the key of the attribute.
// - buffer: buffer to copy.
// - bufferLength: size of the buffer.
// - addQuotes: if quotes has to be added to the buffer.
iowa_status_t coreLinkAddBufferAttribute(link_t *linkP, attribute_key_t key, const uint8_t *buffer, size_t bufferLength, bool addQuotes);

// Add an integer attribute to the link.
// Returned value: IOWA_COAP_NO_ERROR or an error.
// Parameters:
// - linkP: pointer to the link.
// - key: the key of the attribute.
// - value: value of the attribute.
iowa_status_t coreLinkAddIntegerAttribute(link_t *linkP, attribute_key_t key, int64_t value);

// Find a specific attribute inside a link.
// Returned value: an attribute_t or null in case of error.
// Parameters:
// - linkP: pointer to the link.
// - key: the key of the desired attribute.
attribute_t * coreLinkFind(link_t *linkP, attribute_key_t key);

// Free a link array and their attributes.
// Returned value: none.
// Parameters:
// - linkP: pointer to the array.
// - nbLink: number of element in the array.
void coreLinkFree(link_t *linkP, size_t nbLink);

// Misc APIs

// Free Client and remove related timer, peer and observations on Server mode.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - clientP: client to free.
// Note: only if LWM2M_BOOTSTRAP_SERVER_MODE or LWM2M_SERVER_MODE is defined
void utils_freeClient(iowa_context_t contextP, lwm2m_client_t *clientP);

// Get binding from string.
// Returned value: strBindingLen is succeed or 0 if any error.
// Parameters:
// - strBinding, strBindingLen: string which should contain binding information.
// - bindingP: OUT. binding which can be from BINDING_U, BINDING_T, BINDING_S, BINDING_N and/or BINDING_Q
size_t utils_stringToBinding(uint8_t *strBinding, size_t strBindingLen, lwm2m_binding_t *bindingP);

// Generate the binding parameter string.
// Returned value: binding string length.
// Parameters:
// - binding: binding which is a combination of BINDING_U, BINDING_T, BINDING_S, and BINDING_N
// - queueMode: indicate if there is queue mode or not
// - strBindingP: pointer to store the generated string. To be freed by the caller.
size_t utils_bindingToString(lwm2m_binding_t binding, bool queueMode, uint8_t **strBindingP);

/****************************
 * Data push operation
 */

// For send API if LWM2M_CLIENT_MODE is defined
typedef struct
{
    lwm2m_server_t *serverP;
    iowa_response_callback_t responseCb;
    void *userDataP;
} client_operation_data_t;

// Check data push validity on LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: as returned by iowa_init().
// - clientP: request's client.
// - fromPeerP: peer source.
// - requestP: request CoAP message.
iowa_status_t lwm2mServerDataPushCheck(iowa_context_t contextP,
                                       lwm2m_client_t *clientP,
                                       iowa_coap_peer_t *fromPeerP,
                                       iowa_coap_message_t *requestP);

#endif // _IOWA_PRV_LWM2M_INCLUDE_
