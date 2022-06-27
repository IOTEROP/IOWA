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

#include "iowa_prv_security_internals.h"

/*************************************************************************************
** Private functions
*************************************************************************************/

static void prv_commEventCb(comm_channel_t *fromChannel,
                            comm_event_t event,
                            void *userData,
                            iowa_context_t contextP)
{
    iowa_security_session_t securityS;

    (void)fromChannel;

    securityS = (iowa_security_session_t)userData;

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "securityS: %p, event: %s.", securityS, STR_COMM_EVENT(event));

    switch (event)
    {
    case COMM_EVENT_DISCONNECTED:
        securityDisconnect(contextP, securityS);
        break;

    case COMM_EVENT_DATA_AVAILABLE:
#if IOWA_SECURITY_LAYER != IOWA_SECURITY_LAYER_NONE

        switch (securityS->state)
        {
#ifdef IOWA_SECURITY_SERVER_MODE
        case SECURITY_STATE_INIT_HANDSHAKE:
            securityS->state = SECURITY_STATE_HANDSHAKING;
            // Fall through
#endif
        case SECURITY_STATE_HANDSHAKING:
            // Security mode cannot be none on Handshaking
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
            (void)iowa_user_security_handle_handshake_packet(securityS);
#endif
            break;

        case SECURITY_STATE_CONNECTED:
            // Only forward the data to the upper layer if the state is connected
            SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DATA_AVAILABLE);
            break;

        default:
            // Do nothing
            break;
        }
#else
        SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DATA_AVAILABLE);
#endif
        break;

    default:
        break;
    }
}

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_status_t securityInit(iowa_context_t contextP)
{
    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Init security layer");

    contextP->securityContextP = (struct _iowa_security_context_t *)iowa_system_malloc(sizeof(struct _iowa_security_context_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (contextP->securityContextP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _iowa_security_context_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(contextP->securityContextP, 0, sizeof(struct _iowa_security_context_t));

    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Security layer init done");

    return IOWA_COAP_NO_ERROR;
}

void securityClose(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Closing security layer");

    iowa_system_free(contextP->securityContextP);
    contextP->securityContextP = NULL;

    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Security layer closed");
}

iowa_status_t securityStep(iowa_context_t contextP)
{
    iowa_status_t result;
    iowa_security_session_t securityS;

    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Entering currentTime: %u, timeoutP: %u.", contextP->currentTime, contextP->timeout);

    result = IOWA_COAP_NO_ERROR;
    securityS = contextP->securityContextP->sessionList;

    while (securityS != NULL)
    {
        iowa_security_session_t nextSecurityS;

        IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Entering with Security state: %s for securityS: %p.", STR_SECURITY_STATE(securityS->state), securityS);

        // Save the next Security session since the step could delete it
        nextSecurityS = (iowa_security_session_t)securityS->nextP;

        if (securityS->isSecure == true)
        {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
            result = iowa_user_security_step(securityS);
#else
            // Should not happen
            result = IOWA_COAP_501_NOT_IMPLEMENTED;
#endif
        }

        securityS = nextSecurityS;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

#ifdef IOWA_SECURITY_CLIENT_MODE
iowa_security_session_t securityClientNewSession(iowa_context_t contextP,
                                                 const char *uri,
                                                 iowa_security_mode_t securityMode)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;
    iowa_status_t result;
    iowa_connection_type_t type;
    char *hostname;
    char *port;
    bool isSecure;

    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Creating client security session for uri \"%s\".", uri);

    securityS = NULL;

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    // Check security mode with the security layer
    switch (securityMode)
    {
#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
    case IOWA_SEC_CERTIFICATE:
#endif
#ifdef IOWA_SECURITY_RAW_PUBLIC_KEY_SUPPORT
    case IOWA_SEC_RAW_PUBLIC_KEY:
#endif
    case IOWA_SEC_NONE:
    case IOWA_SEC_PRE_SHARED_KEY:
        break;

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Security mode %d is not supported with the current security layer.", securityMode);
        return NULL;
    }
#endif // IOWA_CONFIG_SKIP_ARGS_CHECK

    result = iowa_coap_uri_parse(uri, &type, &hostname, &port, NULL, NULL, &isSecure);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "Failed to parse URI \"%s\".", uri);
        return NULL;
    }

    // Check security mode with is secure
    switch (securityMode)
    {
    case IOWA_SEC_NONE:
        if (isSecure == true)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Security mode %d does not match the right URI schema.", securityMode);
            goto error;
        }
        break;

    default:
        if (isSecure == false)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Security mode %d does not match the right URI schema.", securityMode);
            goto error;
        }
    }

    // Create the security session
    securityS = (struct _iowa_security_session_t *)iowa_system_malloc(sizeof(struct _iowa_security_session_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _iowa_security_session_t));
        goto error;
    }
#endif
    memset(securityS, 0, sizeof(struct _iowa_security_session_t));

    securityS->uri = utilsStrdup(uri);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->uri == NULL)
    {
        iowa_system_free(securityS);
        goto error;
    }
#endif
    securityS->contextP = contextP;
    securityS->securityMode = securityMode;
    securityS->isSecure = isSecure;
    securityS->type = type;
    securityS->shortServerID = IOWA_LWM2M_ID_ALL;

    if (securityS->isSecure == true)
    {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
        result = iowa_user_security_create_client_session(securityS);
#else
        // Should not happen
        result = IOWA_COAP_501_NOT_IMPLEMENTED;
#endif

        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to create the security session.");
            goto error;
        }
    }

    // Add the security session to the list
    contextP->securityContextP->sessionList = (iowa_security_session_t)IOWA_UTILS_LIST_ADD(contextP->securityContextP->sessionList, securityS);

    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Client security session created.");

    iowa_system_free(hostname);
    iowa_system_free(port);

    return securityS;

error:
    if (securityS != NULL)
    {
        iowa_system_free(securityS->uri);
        iowa_system_free(securityS);
    }

    iowa_system_free(hostname);
    iowa_system_free(port);

    return NULL;
}
#endif

void securitySetEventCallback(iowa_context_t contextP,
                              iowa_security_session_t securityS,
                              security_event_callback_t eventCb,
                              void *userDataCb)
{
    (void)contextP;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS->eventCb = eventCb;
    securityS->userDataCb = userDataCb;
}

void securitySetSSID(iowa_security_session_t securityS,
                     uint16_t ssid)
{
    securityS->shortServerID = ssid;
}

void securityDeleteSession(iowa_context_t contextP,
                           iowa_security_session_t securityS)
{
    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Deleting security session %p.", securityS);

    // Remove the session from the list
    contextP->securityContextP->sessionList = (iowa_security_session_t)IOWA_UTILS_LIST_REMOVE(contextP->securityContextP->sessionList, securityS);

    if (securityS->isSecure == true)
    {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
        iowa_user_security_delete_session(securityS);
#endif
    }

    iowa_system_free(securityS->uri);
    iowa_system_free(securityS);

    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Security session %p deleted.", securityS);
}

iowa_status_t securityConnect(iowa_context_t contextP,
                              iowa_security_session_t securityS)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Connecting session %p.", securityS);

#ifdef IOWA_SECURITY_CLIENT_MODE
    if (securityS->channelP == NULL)
    {
        comm_channel_t *channelP;
        iowa_status_t result;
        iowa_connection_type_t type;
        char *hostname;
        char *port;

        result = iowa_coap_uri_parse(securityS->uri, &type, &hostname, &port, NULL, NULL, NULL);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "Failed to parse URI \"%s\".", securityS->uri);
            return result;
        }

        // Create the communication channel
        channelP = commChannelCreate(contextP, type, hostname, port, securityS->shortServerID, prv_commEventCb, (void *)securityS);

        // Free used memory
        iowa_system_free(hostname);
        iowa_system_free(port);

        if (channelP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot open a connection.");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }

        securityS->channelP = channelP;
    }
#endif

    switch (securityS->state)
    {
    case SECURITY_STATE_DISCONNECTED:
        if (securityS->isSecure == false)
        {
            securityS->state = SECURITY_STATE_CONNECTED;
            SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_CONNECTED);
        }
        else
        {
            securityS->state = SECURITY_STATE_INIT_HANDSHAKE;
            contextP->timeout = 0;
        }
        break;

    default:
        // Do nothing
        break;
    }

    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Exiting.");

    return IOWA_COAP_NO_ERROR;
}

void securityDisconnect(iowa_context_t contextP,
                        iowa_security_session_t securityS)
{
    // WARNING: This function is called in a critical section
    (void)contextP;

    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Disconnecting session %p.", securityS);

    switch (securityS->state)
    {
    case SECURITY_STATE_CONNECTED:
        if (securityS->isSecure == true)
        {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
            iowa_user_security_disconnect(securityS);
#endif
        }

        commChannelDelete(contextP, securityS->channelP);
        securityS->channelP = NULL;
        securityS->state = SECURITY_STATE_DISCONNECTED;

        SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DISCONNECTED);
        break;

    default:
        commChannelDelete(contextP, securityS->channelP);
        securityS->channelP = NULL;
        securityS->state = SECURITY_STATE_DISCONNECTED;
    }

    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Exiting.");
}

int securitySend(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length)
{
    // WARNING: This function is called in a critical section
    int bufferSend;

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Sending %d bytes on session %p.", length, securityS);

    if (securityS->isSecure == false)
    {
        bufferSend = commSend(contextP, securityS->channelP, buffer, length);
    }
    else
    {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
        bufferSend = iowa_user_security_send(securityS, buffer, length);
#else
        // Should not happen
        bufferSend = 0;
#endif
    }

    return bufferSend;
}

int securityRecv(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length)
{
    // WARNING: This function is called in a critical section
    int bufferReceived;

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Receiving on session %p in a %d bytes buffer.", securityS, length);

    if (securityS->isSecure == false)
    {
        bufferReceived = commRecv(contextP, securityS->channelP, buffer, length);
    }
    else
    {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
        bufferReceived = iowa_user_security_recv(securityS, buffer, length);
#else
        // Should not happen
        bufferReceived = 0;
#endif
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with: %d.", bufferReceived);

    return bufferReceived;
}

#ifdef IOWA_SECURITY_SERVER_MODE
iowa_security_session_t securityServerNewSession(iowa_context_t contextP,
                                                 iowa_connection_type_t type,
                                                 void *connP,
                                                 bool isSecure)
{
    iowa_security_session_t securityS;
    iowa_status_t result;
    comm_channel_t *channelP;

    IOWA_LOG_INFO(IOWA_PART_SECURITY, "Creating server security session.");

    // Create the security session
    securityS = (struct _iowa_security_session_t *)iowa_system_malloc(sizeof(struct _iowa_security_session_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _iowa_security_session_t));
        return NULL;
    }
#endif
    memset(securityS, 0, sizeof(struct _iowa_security_session_t));

    // Create the communication channel
    result = commChannelNew(contextP, type, connP, prv_commEventCb, (void *)securityS, &channelP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        iowa_system_free(securityS);
        return NULL;
    }

    securityS->isSecure = isSecure;
    securityS->contextP = contextP;
    securityS->channelP = channelP;
    securityS->type = type;

    if (securityS->isSecure == false)
    {
        result = IOWA_COAP_NO_ERROR;
    }
    else
    {
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
        result = iowa_user_security_create_server_session(securityS);
#else
        // Should not happen
        result = IOWA_COAP_501_NOT_IMPLEMENTED;
#endif
    }

    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to create the security session.");

        commChannelDelete(contextP, channelP);
        iowa_system_free(securityS);

        return NULL;
    }

    // Add the security session to the list
    contextP->securityContextP->sessionList = (iowa_security_session_t)IOWA_UTILS_LIST_ADD(contextP->securityContextP->sessionList, securityS);

    IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "Server security session created: %p.", securityS);

    return securityS;
}

size_t securityGetIdentity(iowa_context_t contextP,
                           iowa_security_session_t securityS,
                           uint8_t *bufferP,
                           size_t length)
{
    // WARNING: This function is called in a critical section
    size_t identifierLength;

    switch (securityS->channelP->type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
#endif
#ifdef IOWA_TCP_SUPPORT
    case IOWA_CONN_STREAM:
#endif
#ifdef IOWA_WEBSOCKET_SUPPORT
    case IOWA_CONN_WEBSOCKET:
#endif
#ifdef IOWA_SMS_SUPPORT
    case IOWA_CONN_SMS:
#endif
#ifdef IOWA_LORAWAN_SUPPORT
    case IOWA_CONN_LORAWAN:
#endif
        CRIT_SECTION_LEAVE(contextP);
        identifierLength = iowa_system_connection_get_peer_identifier(securityS->channelP->connP, bufferP, length, contextP->userData);
        CRIT_SECTION_ENTER(contextP);
        break;

    default:
        // Should not happen
        identifierLength = 0;
    }

    return identifierLength;
}

bool securityGetIsSecure(iowa_context_t contextP,
                         iowa_security_session_t securityS)
{
    (void)contextP;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    return securityS->isSecure;
}
#endif // IOWA_SECURITY_SERVER_MODE

void * securityGetConnP(iowa_security_session_t securityS)
{
    if (securityS->channelP == NULL)
    {
        return NULL;
    }
    return securityS->channelP->connP;
}
