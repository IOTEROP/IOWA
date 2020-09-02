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
* Copyright (c) 2016-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_COMM_INCLUDE_
#define _IOWA_PRV_COMM_INCLUDE_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "iowa_config.h"
#include "iowa_platform.h"

#if defined(IOWA_COAP_CLIENT_MODE) || defined(LWM2M_CLIENT_MODE)
#define IOWA_COMM_CLIENT_MODE
#endif


/************************************************
 * Datatypes
 */

typedef enum
{
    COMM_EVENT_UNDEFINED = 0,
    COMM_EVENT_CONNECTED,
    COMM_EVENT_DISCONNECTED,
    COMM_EVENT_DATA_AVAILABLE
} comm_event_t;

#define STR_COMM_EVENT(M)                                         \
((M) == COMM_EVENT_UNDEFINED ? "COMM_EVENT_UNDEFINED" :           \
((M) == COMM_EVENT_CONNECTED ? "COMM_EVENT_CONNECTED" :           \
((M) == COMM_EVENT_DISCONNECTED ? "COMM_EVENT_DISCONNECTED" :     \
((M) == COMM_EVENT_DATA_AVAILABLE ? "COMM_EVENT_DATA_AVAILABLE" : \
"Unknown"))))

typedef struct _comm_context_t * comm_context_t;

typedef struct _comm_channel_t comm_channel_t;

// The callback called when a channel generates an event.
// fromChannel: the channel which generated the event.
// event: the event.
// userData: the commChannelCreate() parameter.
// contextP: the IOWA context.
typedef void(*comm_event_callback_t)(comm_channel_t *fromChannel,
                                     comm_event_t event,
                                     void *userData,
                                     iowa_context_t contextP);

typedef void(*comm_new_channel_callback_t)(iowa_context_t contextP,
                                           comm_channel_t *fromChannel,
                                           void * userData);

struct _comm_channel_t
{
    iowa_connection_type_t  type;
    void                   *connP;
    comm_event_callback_t   eventCallback;
    void                   *userData;
};

struct _comm_context_t
{
    size_t           channelCount;
    comm_channel_t **channelArray;    // Dynamically-allocated array of created channels
    comm_new_channel_callback_t newChannelCallback;
    void *callbackUserData;
};

/************************************************
* APIs
*/

// Initialize a Comm context.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: a initialized iowa context.
uint8_t commInit(iowa_context_t contextP);

// Close a Comm stack context.
// Parameters:
// - contextP: as returned by iowa_init().
void commClose(iowa_context_t contextP);

#ifdef IOWA_COMM_CLIENT_MODE
// Create a Comm channel.
// Returned value: A new initialized channel or null in case of error.
// Parameters:
// - contextP: as returned by commInit().
// - type: the type of channel.
// - hostname: the destination hostname.
// - port: the destination port.
// - eventCallback: the callback called when an event occurs on the channel.
// - callbackUserData: past as parameter to eventCallback. This can be nil.
comm_channel_t * commChannelCreate(iowa_context_t contextP,
                                   iowa_connection_type_t type,
                                   char *hostname,
                                   char *port,
                                   comm_event_callback_t eventCallback,
                                   void *callbackUserData);
#endif // IOWA_COMM_CLIENT_MODE

// Close a Comm channel.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - channelP: a Comm channel.
void commChannelDelete(iowa_context_t contextP,
                       comm_channel_t *channelP);

// Find a Comm channel.
// Returned value: A channel or null in case of error.
// Parameters:
// - connP: an opaque connection type.
comm_channel_t * commChannelFind(iowa_context_t contextP,
                                 void *connP);

// Send data on a channel.
// Returned value: the number of bytes sent or a negative number in case of error.
// Parameters:
// - contextP: as returned by iowa_init().
// - channelP: a Comm channel.
// - buffer, length: data to send.
int commSend(iowa_context_t contextP,
             comm_channel_t *channelP,
             uint8_t * buffer,
             size_t length);

// Read data on a channel.
// Returned value: the number of bytes read or a negative number in case of error.
// Parameters:
// - contextP: as returned by iowa_init().
// - channelP: a Comm channel.
// - buffer: to store the read data.
// - length: the number of bytes to read.
int commRecv(iowa_context_t contextP,
             comm_channel_t *channelP,
             uint8_t * buffer,
             size_t length);

// Monitor channels during the specified time.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: as returned by iowa_init().
uint8_t commSelect(iowa_context_t contextP);

#endif // _IOWA_PRV_COMM_INCLUDE_
