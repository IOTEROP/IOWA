/**********************************************
 *
 * Copyright (c) 2016-2020 IoTerop.
 * All rights reserved.
 *
 * This program and the accompanying materials
 * are made available under the terms of
 * IoTerop’s IOWA License (LICENSE.TXT) which
 * accompany this distribution.
 *
 **********************************************/

/**********************************************
*
* In this file, you can define the compilation
* flags instead of specifying them on the
* compiler command-line.
*
**********************************************/

#ifndef _IOWA_CONFIG_INCLUDE_
#define _IOWA_CONFIG_INCLUDE_

/**********************************************
*
* Platform configuration.
*
**********************************************/

/**********************************************
* To specify the endianness of your platform.
* One and only one must be defined.
*/
// #define LWM2M_BIG_ENDIAN
#define LWM2M_LITTLE_ENDIAN

/************************************************
* To specify the size of the static buffer used
* to received datagram packets.
*/
#define IOWA_BUFFER_SIZE 512

/**********************************************
*
* IOWA configuration.
*
**********************************************/

/**********************************************
* Support of transports.
*/
#define IOWA_UDP_SUPPORT
// #define IOWA_TCP_SUPPORT
// #define IOWA_LORAWAN_SUPPORT
// #define IOWA_SMS_SUPPORT

/***********************************************
* To enable logs
* By level:
*     - IOWA_LOG_LEVEL_NONE (default)
*     - IOWA_LOG_LEVEL_ERROR
*     - IOWA_LOG_LEVEL_WARNING
*     - IOWA_LOG_LEVEL_INFO
*     - IOWA_LOG_LEVEL_TRACE
*
* and by components:
*     - IOWA_PART_ALL (default)
*     - IOWA_PART_BASE
*     - IOWA_PART_COAP
*     - IOWA_PART_COMM
*     - IOWA_PART_DATA
*     - IOWA_PART_LWM2M
*     - IOWA_PART_OBJECT
*     - IOWA_PART_SECURITY
*     - IOWA_PART_SYSTEM
*/
#define IOWA_LOG_LEVEL IOWA_LOG_LEVEL_INFO
// #define IOWA_LOG_PART IOWA_PART_ALL

/**********************************************
* To enable LWM2M features.
**********************************************/

/**********************************************
* To specify the LWM2M role of your device.
* Several of them can be defined at the same time.
*/
#define LWM2M_CLIENT_MODE
// #define LWM2M_SERVER_MODE
// #define LWM2M_BOOTSTRAP_SERVER_MODE

#endif
