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
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
*
**********************************************/

#include "iowa_prv_comm.h"
#include "iowa_prv_logger.h"
#include "iowa_prv_core.h"

/*************************************************************************************
** Private functions
*************************************************************************************/

static comm_channel_t * prv_channelNew(iowa_context_t contextP,
                                       iowa_connection_type_t type,
                                       void *connP)
{
    // WARNING: This function is called in a critical section
    comm_channel_t *channelP;
    comm_channel_t **newChannelArray;

    newChannelArray = (comm_channel_t **)iowa_system_malloc(sizeof(comm_channel_t *) * (contextP->commContextP->channelCount + 1));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (newChannelArray == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(comm_channel_t *) * (contextP->commContextP->channelCount + 1));
        return NULL;
    }
#endif

    if (contextP->commContextP->channelCount > 0)
    {
        memcpy(newChannelArray, contextP->commContextP->channelArray, sizeof(comm_channel_t *) * contextP->commContextP->channelCount);
    }
    newChannelArray[contextP->commContextP->channelCount] = (comm_channel_t *)iowa_system_malloc(sizeof(comm_channel_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (newChannelArray[contextP->commContextP->channelCount] == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(comm_channel_t));
        iowa_system_free(newChannelArray);
        return NULL;
    }
#endif
    channelP = newChannelArray[contextP->commContextP->channelCount];

    memset(channelP, 0, sizeof(comm_channel_t));
    channelP->type = type;
    channelP->connP = connP;

    iowa_system_free(contextP->commContextP->channelArray);
    contextP->commContextP->channelArray = newChannelArray;
    contextP->commContextP->channelCount += 1;

    return channelP;
}

/*************************************************************************************
** Public functions
*************************************************************************************/

uint8_t commInit(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_TRACE(IOWA_PART_COMM, "Entering.");

    contextP->commContextP = (comm_context_t)iowa_system_malloc(sizeof(struct _comm_context_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (contextP->commContextP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _comm_context_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    memset(contextP->commContextP, 0, sizeof(struct _comm_context_t));

    IOWA_LOG_TRACE(IOWA_PART_COMM, "Comm init done.");

    return IOWA_COAP_NO_ERROR;
}

void commClose(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    size_t i;
    comm_context_t commContextP;

    IOWA_LOG_TRACE(IOWA_PART_COMM, "Entering");

    commContextP = contextP->commContextP;
    contextP->commContextP = NULL;

    for (i = 0; i < commContextP->channelCount; i++)
    {
        CRIT_SECTION_LEAVE(contextP);
        iowa_system_connection_close(commContextP->channelArray[i]->connP, contextP->userData);
        CRIT_SECTION_ENTER(contextP);

        iowa_system_free(commContextP->channelArray[i]);
    }

    iowa_system_free(commContextP->channelArray);
    iowa_system_free(commContextP);

    IOWA_LOG_TRACE(IOWA_PART_COMM, "Comm closed.");
}

#ifdef IOWA_COMM_SERVER_MODE
void commServerConfigure(iowa_context_t contextP,
                         comm_new_channel_callback_t newChannelCallback,
                         void *callbackUserData)
{
    contextP->commContextP->newChannelCallback = newChannelCallback;
    contextP->commContextP->callbackUserData = callbackUserData;
}

uint8_t commChannelNew(iowa_context_t contextP,
                       iowa_connection_type_t type,
                       void *connP,
                       comm_event_callback_t eventCallback,
                       void *callbackUserData,
                       comm_channel_t **channelP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Type: %d, connP: %p.", type, connP);

    // WARNING: This function is called in a critical section
    switch (type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        break;
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_COMM, "Unsupported connection type (%d). Make sure IOWA is compiled with the right transport support.", type);
        return IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    *channelP = prv_channelNew(contextP, type, connP);
    if (*channelP == NULL)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    (*channelP)->eventCallback = eventCallback;
    (*channelP)->userData = callbackUserData;

    if (contextP->commContextP->newChannelCallback != NULL)
    {
        contextP->commContextP->newChannelCallback(contextP, *channelP, contextP->commContextP->callbackUserData);
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Exiting with new channel: %p.", *channelP);

    return IOWA_COAP_NO_ERROR;
}

#endif // IOWA_COMM_SERVER_MODE

#ifdef IOWA_COMM_CLIENT_MODE
comm_channel_t * commChannelCreate(iowa_context_t contextP,
                                   iowa_connection_type_t type,
                                   char *hostname,
                                   char *port,
                                   uint16_t ssid,
                                   comm_event_callback_t eventCallback,
                                   void *callbackUserData)
{
    // WARNING: This function is called in a critical section
    void * connP;
    comm_channel_t *channelP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "type: %d, hostname: \"%s\", port: \"%s\".", type, hostname, port);

    CRIT_SECTION_LEAVE(contextP);

    connP = iowa_system_connection_open(type, hostname, port, contextP->userData);

    CRIT_SECTION_ENTER(contextP);
    if (connP == NULL)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COMM, "Opening a connection failed (type: %d, hostname: \"%s\", port: \"%s\").", type, hostname, port);
        return NULL;
    }

    channelP = prv_channelNew(contextP, type, connP);
    if (channelP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COMM, "Failed to create new channel.");
        CRIT_SECTION_LEAVE(contextP);
        iowa_system_connection_close(connP, contextP->userData);
        CRIT_SECTION_ENTER(contextP);
        return NULL;
    }

    channelP->eventCallback = eventCallback;
    channelP->userData = callbackUserData;

    if (channelP->eventCallback != NULL)
    {
        channelP->eventCallback(channelP, COMM_EVENT_CONNECTED, channelP->userData, contextP);
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Exiting with channelP: %p.", channelP);

    return channelP;
}
#endif

void commChannelDelete(iowa_context_t contextP,
                       comm_channel_t *channelP)
{
    // WARNING: This function is called in a critical section
    size_t i;
    comm_channel_t **newChannelArray;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "channelP: %p.", channelP);

    if (NULL == channelP)
    {
        // Don't try to delete an already nil channel structure
        IOWA_LOG_TRACE(IOWA_PART_COMM, "channelP is nil. Exiting.");
        return;
    }

    for (i = 0; i < contextP->commContextP->channelCount; i++)
    {
        if (channelP == contextP->commContextP->channelArray[i])
        {
            break;
        }
    }

    if (i == contextP->commContextP->channelCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "channelP: %p not found.", channelP);
        return;
    }

    if (contextP->commContextP->channelCount == 1)
    {
        newChannelArray = NULL;
    }
    else
    {
        newChannelArray = (comm_channel_t **)iowa_system_malloc(sizeof(comm_channel_t *) * (contextP->commContextP->channelCount - 1));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (newChannelArray == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(comm_channel_t *) * (contextP->commContextP->channelCount - 1));
            return;
        }
#endif
        if (i > 0)
        {
            memcpy(newChannelArray, contextP->commContextP->channelArray, sizeof(comm_channel_t *) * i);
        }
        if (contextP->commContextP->channelCount - i > 1)
        {
            memcpy(newChannelArray + i, contextP->commContextP->channelArray + i + 1, sizeof(comm_channel_t *) * (contextP->commContextP->channelCount - i - 1));
        }
    }

    iowa_system_free(contextP->commContextP->channelArray);
    contextP->commContextP->channelArray = newChannelArray;

    contextP->commContextP->channelCount -= 1;

    if (channelP->connP != NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        iowa_system_connection_close(channelP->connP, contextP->userData);
        CRIT_SECTION_ENTER(contextP);
    }

    iowa_system_free(channelP);

    IOWA_LOG_TRACE(IOWA_PART_COMM, "Exiting.");
}

comm_channel_t * commChannelFind(iowa_context_t contextP,
                                 void *connP)
{
    size_t channelIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Connection %p.", connP);

    for (channelIndex = 0; channelIndex < contextP->commContextP->channelCount; channelIndex++)
    {
        if (contextP->commContextP->channelArray[channelIndex]->connP == connP)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Found matching channel %p.", contextP->commContextP->channelArray[channelIndex]);

            return contextP->commContextP->channelArray[channelIndex];
        }
    }

    IOWA_LOG_TRACE(IOWA_PART_COMM, "Could not find a matching channel.");

    return NULL;
}

int commSend(iowa_context_t contextP,
             comm_channel_t *channelP,
             uint8_t *buffer,
             size_t length)
{
    // WARNING: This function is called in a critical section
    int result;

    IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "On channelP: %p.", channelP);
    IOWA_LOG_BUFFER_INFO(IOWA_PART_COMM, "Sending", buffer, length);

    CRIT_SECTION_LEAVE(contextP);
    result = iowa_system_connection_send(channelP->connP, buffer, length, contextP->userData);
    CRIT_SECTION_ENTER(contextP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Exiting with result %d.", result);

    return result;
}

int commRecv(iowa_context_t contextP,
             comm_channel_t *channelP,
             uint8_t *buffer,
             size_t length)
{
    // WARNING: This function is called in a critical section
    int result;

    IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "Receiving %u bytes on channelP: %p.", length, channelP);

    CRIT_SECTION_LEAVE(contextP);
    result = iowa_system_connection_recv(channelP->connP, buffer, length, contextP->userData);
    CRIT_SECTION_ENTER(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_SYSTEM, "iowa_system_connection_recv() returned %d.", result);

#if (IOWA_LOG_LEVEL >= IOWA_LOG_LEVEL_INFO)
    if (result > 0)
    {
        IOWA_LOG_BUFFER_INFO(IOWA_PART_COMM, "Received", buffer, (size_t)result);
    }
#endif

    return result;
}

uint8_t commSelect(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    int result;
    void **connArray;
    size_t connCount;
    int32_t currentTimeout;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Channel count: %u.", contextP->commContextP->channelCount);

    connArray = NULL;
    if (contextP->commContextP->channelCount > 0)
    {
        size_t connIndex;

        connArray = (void **)iowa_system_malloc(contextP->commContextP->channelCount * sizeof(void *));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (connArray == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(contextP->commContextP->channelCount * sizeof(void *));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        for (connIndex = 0; connIndex < contextP->commContextP->channelCount; connIndex++)
        {
            connArray[connIndex] = contextP->commContextP->channelArray[connIndex]->connP;
        }
        connCount = contextP->commContextP->channelCount;
    }
    else
    {
        connCount = 0;
    }

    currentTimeout = contextP->timeout; // Store the timeout before to leave the critical section to prevent a possible data race condition

    IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "Calling iowa_system_connection_select() for %u connections with a timeout of %ds.", connCount, currentTimeout);

    CRIT_SECTION_LEAVE(contextP);
    result = iowa_system_connection_select(connArray, connCount, currentTimeout, contextP->userData);
    CRIT_SECTION_ENTER(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "iowa_system_connection_select() returned %d.", result);

    if (result < 0)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "iowa_system_connection_select() returned %d. Exiting with error 5.03 (SERVICE UNAVAILABLE).", result);
        iowa_system_free(connArray);

        return IOWA_COAP_503_SERVICE_UNAVAILABLE;
    }
    else if (result > 0
             && connCount != 0
             && contextP->commContextP->channelCount > 0)
    {
        int32_t currentTime;
        comm_channel_t *channelP;
        size_t connIndex;

        // Retrieve the current time before calling the callbacks
        CRIT_SECTION_LEAVE(contextP);
        currentTime = iowa_system_gettime();
        CRIT_SECTION_ENTER(contextP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (currentTime < 0
            || currentTime < contextP->currentTime)
        {
            IOWA_LOG_ERROR_GETTIME(currentTime);
            iowa_system_free(connArray);

            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        contextP->currentTime = currentTime;

        for (connIndex = 0; connIndex < connCount; connIndex++)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_COMM, "Connection index: %u.", connIndex);

            if (connArray[connIndex] != NULL)
            {

                IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "Connection #%u (%p) has data.", connIndex, connArray[connIndex]);

                channelP = commChannelFind(contextP, connArray[connIndex]);
                if (channelP != NULL)
                {
                   IOWA_LOG_ARG_INFO(IOWA_PART_COMM, "Found matching channel %p.", channelP);

                    channelP->eventCallback(channelP, COMM_EVENT_DATA_AVAILABLE, channelP->userData, contextP);
                }
            }
        }
    }

    iowa_system_free(connArray);

    return IOWA_COAP_NO_ERROR;
}
