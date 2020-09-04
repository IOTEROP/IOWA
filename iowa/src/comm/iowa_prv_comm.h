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
    comm_channel_t **channelArray;
    comm_new_channel_callback_t newChannelCallback;
    void *callbackUserData;
};

/************************************************
* APIs
*/





uint8_t commInit(iowa_context_t contextP);




void commClose(iowa_context_t contextP);

#ifdef IOWA_COMM_CLIENT_MODE









comm_channel_t * commChannelCreate(iowa_context_t contextP,
                                   iowa_connection_type_t type,
                                   char *hostname,
                                   char *port,
                                   comm_event_callback_t eventCallback,
                                   void *callbackUserData);
#endif






void commChannelDelete(iowa_context_t contextP,
                       comm_channel_t *channelP);





comm_channel_t * commChannelFind(iowa_context_t contextP,
                                 void *connP);







int commSend(iowa_context_t contextP,
             comm_channel_t *channelP,
             uint8_t * buffer,
             size_t length);








int commRecv(iowa_context_t contextP,
             comm_channel_t *channelP,
             uint8_t * buffer,
             size_t length);





uint8_t commSelect(iowa_context_t contextP);

#endif
