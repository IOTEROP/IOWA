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

#ifndef _IOWA_PRV_CORE_CHECK_CONFIG_
#define _IOWA_PRV_CORE_CHECK_CONFIG_

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************
* Check platform configuration.
**********************************************/

// Check endianness of the platform
#if !defined(LWM2M_BIG_ENDIAN) && !defined(LWM2M_LITTLE_ENDIAN)
#error "At least one endianness format must be defined."
#endif

#if defined(LWM2M_BIG_ENDIAN) && defined(LWM2M_LITTLE_ENDIAN)
#error "LWM2M_BIG_ENDIAN and LWM2M_LITTLE_ENDIAN are defined but only one must be used."
#endif

// Check the buffer size
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
#if !defined(IOWA_BUFFER_SIZE)
#error "The buffer size must be defined."
#endif
#endif

/**********************************************
* Check IOWA configuration.
**********************************************/

// Check storage context support
#if defined(IOWA_STORAGE_CONTEXT_AUTOMATIC_BACKUP) && !defined(IOWA_STORAGE_CONTEXT_SUPPORT)
#error "The storage of context feature is not enabled."
#endif

// Check at least one transport is defined
#if !defined(IOWA_UDP_SUPPORT) && !defined(IOWA_TCP_SUPPORT) && !defined(IOWA_WEBSOCKET_SUPPORT) && !defined(IOWA_LORAWAN_SUPPORT) && !defined(IOWA_SMS_SUPPORT)
#error "No transport is enabled."
#endif

// Check the LwM2M and/or CoAP role of the device
#if !defined(LWM2M_CLIENT_MODE) && !defined(LWM2M_SERVER_MODE) && !defined(LWM2M_BOOTSTRAP_SERVER_MODE) && !defined(IOWA_COAP_CLIENT_MODE) && !defined(IOWA_COAP_SERVER_MODE)
#error "At least one LwM2M or CoAP role must be defined."
#endif

// Check Block-wise transfer is used only on datagram transports
#if defined(IOWA_COAP_BLOCK_SUPPORT) || defined(IOWA_COAP_BLOCK_MINIMAL_SUPPORT)
#if !defined(IOWA_UDP_SUPPORT) && !defined(IOWA_LORAWAN_SUPPORT) && !defined(IOWA_SMS_SUPPORT)
#error "CoAP Block-Wise Transfer is usable only with datagram transports."
#endif
#endif

/**********************************************
* Check LWM2M features.
**********************************************/

// Check bootstrap support
#if !defined(LWM2M_CLIENT_MODE) && defined(LWM2M_BOOTSTRAP)
#error "LWM2M_BOOTSTRAP must be only used when the LwM2M role is client."
#endif

#if defined(LWM2M_BOOTSTRAP_PACK_SUPPORT) && !defined(LWM2M_BOOTSTRAP) && !defined(LWM2M_BOOTSTRAP_SERVER_MODE)
#error "LWM2M_BOOTSTRAP_PACK_SUPPORT requires LWM2M_BOOTSTRAP or LWM2M_BOOTSTRAP_SERVER_MODE"
#endif

// Check right format support activated for read & observe composite operations
#if (defined(LWM2M_READ_COMPOSITE_SUPPORT) || defined(LWM2M_OBSERVE_COMPOSITE_SUPPORT)) \
    && (!defined(LWM2M_SUPPORT_SENML_CBOR) && !defined(LWM2M_SUPPORT_SENML_JSON))
#error "To use composite read or observe operations at least one of the following formats must be supported: SenML CBOR or SenML JSON."
#endif

// Check right format support activated for write composite operation
#if (defined(LWM2M_WRITE_COMPOSITE_SUPPORT)) \
    && (!defined(LWM2M_SUPPORT_SENML_CBOR) && !defined(LWM2M_SUPPORT_SENML_JSON) && !defined(LWM2M_SUPPORT_LWM2M_CBOR))
#error "To use composite write operations at least one of the following formats must be supported: SenML CBOR, SenML JSON, or LwM2M CBOR."
#endif

// Check the LwM2M 1.1 version dependance
#if !defined(LWM2M_VERSION_1_1_SUPPORT) && defined(LWM2M_VERSION_1_0_REMOVE)
#error "At least one LwM2M version must be defined."
#endif

// Check encoding support format in client mode
#if defined(LWM2M_VERSION_1_1_SUPPORT) && defined(LWM2M_CLIENT_MODE) \
    && !defined(LWM2M_SUPPORT_SENML_JSON) && !defined(LWM2M_SUPPORT_SENML_CBOR) && !defined(LWM2M_SUPPORT_LWM2M_CBOR)
#error "LwM2M 1.1 clients must support at least one of the following formats: SenML CBOR, SenML JSON, or LwM2M CBOR."
#endif

// Check format requirements for data push operation
#if defined(LWM2M_DATA_PUSH_SUPPORT) \
    && (!defined(LWM2M_SUPPORT_SENML_CBOR) && !defined(LWM2M_SUPPORT_SENML_JSON) && !defined(LWM2M_SUPPORT_LWM2M_CBOR))
#error "To use data push operation at least one of the following formats must be supported: SenML CBOR, SenML JSON, or LwM2M CBOR."
#endif

/**********************************************
* Check IOWA objects configuration.
**********************************************/

// Check bearer selection object resources
#if defined(IOWA_SUPPORT_BEARER_SELECTION_OBJECT) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_PREFERRED_COMM_BEARER) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_ACCEPTABLE_RSSI_GSM) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_ACCEPTABLE_RSCP_UMTS) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_ACCEPTABLE_RSRP_LTE) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_ACCEPTABLE_RSSI_EV_DO) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_CELL_LOCK_LIST) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_OPERATOR_LIST) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_OPERATOR_LIST_MODE) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_AVAILABLE_PLMNS) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_ACCEPTABLE_RSRP_NB_IOT) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_PLMN_SEARCH_TIMER) \
    && !defined(IOWA_BEARER_SELECTION_SUPPORT_RSC_ATTACH_WO_PDN_CONNECTION)
#error "Bearer Selection object has no resource set."
#endif

// Check Light Control Object resources
#if defined(IOWA_LIGHT_CONTROL_SUPPORT_RSC_CUMULATIVE_ACTIVE_POWER) \
    && (!defined(IOWA_LIGHT_CONTROL_SUPPORT_RSC_ON_TIME) || !defined(IOWA_LIGHT_CONTROL_SUPPORT_RSC_POWER_FACTOR))
#error "Cumulative active power resource cannot be present without Power Factor and On Time resources."
#endif

// Check Server Object resources
#if defined(IOWA_SERVER_SUPPORT_RSC_REGISTRATION_BEHAVIOUR) && !defined(LWM2M_VERSION_1_1_SUPPORT)
#error "Registration behaviour resources can only be supported if LwM2M version 1.1."
#endif
#if defined(IOWA_SERVER_SUPPORT_RSC_COMMUNICATION_ATTEMPTS) && !defined(LWM2M_VERSION_1_1_SUPPORT)
#error "Communication attemps resources can only be supported if LwM2M version 1.1."
#endif
#if defined(IOWA_SERVER_SUPPORT_RSC_BOOTSTRAP_TRIGGER) && !defined(LWM2M_BOOTSTRAP)
#error "Bootstrap Trigger resource can only be supported if bootstrap is supported."
#endif
#if defined(IOWA_SERVER_SUPPORT_RSC_MUTE_SEND) && !defined(LWM2M_DATA_PUSH_SUPPORT)
#error "Mute resource can only be supported if data push is supported."
#endif

/**********************************************
* Check Security configuration.
**********************************************/

#if defined(IOWA_TCP_SUPPORT) && (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_TINYDTLS)
#error "tinyDTLS does not support TLS encryption."
#endif

#ifdef __cplusplus
}
#endif

#endif
