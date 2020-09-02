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
* Copyright (c) 2017-2019 IoTerop.
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
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
#if !defined(IOWA_BUFFER_SIZE)
#error "The buffer size must be defined."
#endif
#endif

/**********************************************
* Check IOWA configuration.
**********************************************/

// Check storage context support

// Check at least one transport is defined
#if !defined(IOWA_UDP_SUPPORT) && !defined(IOWA_TCP_SUPPORT) && !defined(IOWA_LORAWAN_SUPPORT) && !defined(IOWA_SMS_SUPPORT)
#error "No transport is enabled."
#endif

// Check the LwM2M and/or CoAP role of the device
#if !defined(LWM2M_CLIENT_MODE) && !defined(LWM2M_SERVER_MODE) && !defined(LWM2M_BOOTSTRAP_SERVER_MODE) && !defined(IOWA_COAP_CLIENT_MODE) && !defined(IOWA_COAP_SERVER_MODE)
#error "At least one LwM2M or CoAP role must be defined."
#endif

/**********************************************
* Check LWM2M features.
**********************************************/

// Check bootstrap support

// Check right format support activated for composite operation

#endif
