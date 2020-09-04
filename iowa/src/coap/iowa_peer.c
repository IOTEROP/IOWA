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

#include "iowa_prv_coap_internals.h"
#include <stdbool.h>


static void prv_freeExchange(coap_exchange_t *exchangeP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Freeing exchange %p.", exchangeP);

    iowa_system_free(exchangeP);
}

#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
static bool prv_removeExchange(iowa_coap_peer_t *peerP,
                               coap_exchange_t *exchangeP)
{
    bool found;

    found = false;

    if (peerP->base.exchangeList == exchangeP)
    {
        found = true;
        peerP->base.exchangeList = peerP->base.exchangeList->next;
    }
    else if (peerP->base.exchangeList != NULL)
    {
        coap_exchange_t *parentP;

        parentP = peerP->base.exchangeList;
        while (parentP->next != NULL
               && found == false)
        {
            if (parentP->next == exchangeP)
            {
                found = true;
                parentP->next = parentP->next->next;
            }
            else
            {
                parentP = parentP->next;
            }
        }
    }

    return found;
}

static void prv_datagramSendResult(iowa_coap_peer_t *fromPeer,
                                   uint8_t code,
                                   iowa_coap_message_t *messageP,
                                   void *userData,
                                   iowa_context_t contextP)
{
    coap_exchange_t *exchangeP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "fromPeer: %p, code: %u.%02u, messageP: %p.", fromPeer, code >> 5, code & 0x1F, messageP);

    exchangeP = (coap_exchange_t *)userData;

    if (messageP == NULL)
    {
        if (true == prv_removeExchange(fromPeer, exchangeP))
        {
            IOWA_LOG_TRACE(IOWA_PART_COAP, "Forward reply to the upper layer.");
            exchangeP->callback(fromPeer, code, messageP, exchangeP->userData, contextP);
            prv_freeExchange(exchangeP);
        }
    }
}
#endif

static uint8_t prv_send(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP,
                        iowa_coap_message_t *messageP,
                        coap_message_callback_t resultCallback,
                        void *userData)
{

    uint8_t result;

#if !defined(IOWA_UDP_SUPPORT) && !defined(IOWA_LORAWAN_SUPPORT) && !defined(IOWA_SMS_SUPPORT)
    (void)resultCallback;
    (void)userData;
#endif

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering.");


#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
    switch (peerP->base.type)
    {
    case IOWA_CONN_DATAGRAM:
    case IOWA_CONN_LORAWAN:
    case IOWA_CONN_SMS:
        if (messageP->type == IOWA_COAP_TYPE_CONFIRMABLE
            || messageP->type == IOWA_COAP_TYPE_NON_CONFIRMABLE)
        {
            messageP->id = ((coap_peer_datagram_t *)peerP)->nextMID;
            ((coap_peer_datagram_t *)peerP)->nextMID++;
            if (((coap_peer_datagram_t *)peerP)->nextMID == COAP_RESERVED_MID)
            {
                ((coap_peer_datagram_t *)peerP)->nextMID++;
            }
        }
        break;

    default:
        break;
    }
#endif

    COAP_LOG_MESSAGE("Sending", peerP->base.type, messageP);

    switch (peerP->base.type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        result = messageSendUDP(contextP, peerP, messageP, resultCallback, userData);
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_COAP, "Unsupported connection type: %d.", peerP->base.type);
        result = IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_coap_peer_t *peer_new(iowa_connection_type_t type)
{
    iowa_coap_peer_t *peerP;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering");

    switch (type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        peerP = (iowa_coap_peer_t *)iowa_system_malloc(sizeof(coap_peer_datagram_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (peerP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(coap_peer_datagram_t));
            return NULL;
    }
#endif
        memset(peerP, 0, sizeof(coap_peer_datagram_t));
        ((coap_peer_datagram_t *)peerP)->ackTimeout = COAP_UDP_ACK_REAL_TIMEOUT;
        ((coap_peer_datagram_t *)peerP)->maxRetransmit = COAP_UDP_MAX_RETRANSMIT;
        ((coap_peer_datagram_t *)peerP)->transmitWait = COAP_UDP_MAX_TRANSMIT_WAIT;
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Unknown connection type %d.", type);
        return NULL;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Returning %p", peerP);
    return peerP;
}

void peer_free(iowa_context_t contextP,
               iowa_coap_peer_t *peerP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Freeing peer %p.", peerP);

    if (peerP->base.securityS != NULL)
    {
        securityDeleteSession(contextP, peerP->base.securityS);
        peerP->base.securityS = NULL;
    }

    switch (peerP->base.type)
    {
    case IOWA_CONN_DATAGRAM:
    case IOWA_CONN_LORAWAN:
    case IOWA_CONN_SMS:
        IOWA_UTILS_LIST_FREE(((coap_peer_datagram_t *)peerP)->transactionList, transactionFree);
        IOWA_UTILS_LIST_FREE(((coap_peer_datagram_t *)peerP)->ackList, acknowledgeFree);
        break;

    default:
        break;
    }

    iowa_system_free(peerP);
}

#ifdef IOWA_COAP_CLIENT_MODE
iowa_coap_peer_t *coapPeerCreate(iowa_context_t contextP,
                                 const char *uri,
                                 iowa_security_mode_t securityMode,
                                 coap_message_callback_t requestCallback,
                                 coap_event_callback_t eventCallback,
                                 void *callbackUserData)
{
    security_event_callback_t securityEventCallback;
    iowa_coap_peer_t *peerP;
    iowa_security_session_t securityS;
    iowa_connection_type_t type;
    char *hostname;
    char *port;
    bool isSecure;
    iowa_security_mode_t transportSecurityMode;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "uri: \"%s\".", uri);

    if (IOWA_COAP_NO_ERROR != iowa_coap_uri_parse(uri, &type, &hostname, &port, NULL, NULL, &isSecure))
    {
        return NULL;
    }

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if ((securityMode & IOWA_SEC_OSCORE) == IOWA_SEC_OSCORE)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Request to use OSCORE but IOWA_COAP_OSCORE_SUPPORT was not defined.");
        return NULL;
    }
#endif
    transportSecurityMode = securityMode;

    securityS = securityClientNewSession(contextP, uri, transportSecurityMode);
    if (securityS == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Cannot create a new security session.");
        iowa_system_free(hostname);
        iowa_system_free(port);
        return NULL;
    }

    peerP = peer_new(type);
    if (peerP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Cannot create a new peer.");

        securityDeleteSession(contextP, securityS);
        iowa_system_free(hostname);
        iowa_system_free(port);
        return NULL;
    }
    peerP->base.securityS = securityS;
    peerP->base.type = type;

    switch (peerP->base.type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        securityEventCallback = udpSecurityEventCb;
        break;
#endif

    default:
        securityEventCallback = NULL;
        break;
    }

    securitySetEventCallback(contextP, securityS, securityEventCallback, (void *)peerP);

    switch (peerP->base.type)
    {
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
#endif
        ((coap_peer_datagram_t *)peerP)->nextMID = COAP_FIRST_MID;
        ((coap_peer_datagram_t *)peerP)->lastMID = COAP_RESERVED_MID;
        break;
#endif

    default:
        break;
    }

    coapPeerSetCallbacks(peerP, requestCallback, eventCallback, callbackUserData);

    contextP->coapContextP->peerList = (iowa_coap_peer_t *)IOWA_UTILS_LIST_ADD(contextP->coapContextP->peerList, peerP);

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Created peer %p.", peerP);

    iowa_system_free(hostname);
    iowa_system_free(port);

    return peerP;
}
#endif

void coapPeerDelete(iowa_context_t contextP,
                    iowa_coap_peer_t *peerP)
{
    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Closing peer %p.", peerP);

    if (peerP != NULL
        && peerP->base.type != IOWA_CONN_UNDEFINED)
    {
        iowa_connection_type_t savedType;

        savedType = peerP->base.type;
        peerP->base.type = IOWA_CONN_UNDEFINED;

        coapPeerDisconnect(contextP, peerP);

        contextP->coapContextP->peerList = (iowa_coap_peer_t *)IOWA_UTILS_LIST_REMOVE(contextP->coapContextP->peerList, peerP);

        while (peerP->base.exchangeList != NULL)
        {
            coap_exchange_t *exchangeP;

            exchangeP = peerP->base.exchangeList;
            peerP->base.exchangeList = peerP->base.exchangeList->next;

            if (exchangeP->callback != NULL)
            {
                exchangeP->callback(peerP, IOWA_COAP_503_SERVICE_UNAVAILABLE, NULL, exchangeP->userData, contextP);
            }
            prv_freeExchange(exchangeP);
        }

        switch (savedType)
        {
        case IOWA_CONN_DATAGRAM:
        case IOWA_CONN_LORAWAN:
        case IOWA_CONN_SMS:
            while (((coap_peer_datagram_t *)peerP)->transactionList != NULL)
            {
                coap_transaction_t *transacP;

                transacP = ((coap_peer_datagram_t *)peerP)->transactionList;
                ((coap_peer_datagram_t *)peerP)->transactionList = ((coap_peer_datagram_t *)peerP)->transactionList->next;

                if (transacP->callback != NULL)
                {
                    transacP->callback(peerP, IOWA_COAP_503_SERVICE_UNAVAILABLE, NULL, transacP->userData, contextP);
                }
                transactionFree(transacP);
            }
            IOWA_UTILS_LIST_FREE(((coap_peer_datagram_t *)peerP)->ackList, acknowledgeFree);
            break;

        default:
            break;
        }

        peer_free(contextP, peerP);
    }
}

void coapPeerSetCallbacks(iowa_coap_peer_t *peerP,
                          coap_message_callback_t requestCallback,
                          coap_event_callback_t eventCallback,
                          void *callbackUserData)
{
    peerP->base.requestCallback = requestCallback;
    peerP->base.eventCallback = eventCallback;
    peerP->base.userData = callbackUserData;
}

uint8_t coapPeerConnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP)
{
    return securityConnect(contextP, peerP->base.securityS);
}

void coapPeerDisconnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP)
{
    if (peerP->base.securityS != NULL)
    {
        securityDisconnect(contextP, peerP->base.securityS);
    }
}

iowa_security_state_t coapPeerGetConnectionState(iowa_coap_peer_t *peerP)
{
    iowa_security_state_t state;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Entering for peer %p.", peerP);

    state = iowa_security_session_get_state(peerP->base.securityS);

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting for peer %p with state %s.", peerP, STR_SECURITY_STATE(state));

    return state;
}

iowa_security_session_t coapPeerGetSecuritySession(iowa_coap_peer_t *peerP)
{
    return peerP->base.securityS;
}

iowa_connection_type_t coapPeerGetConnectionType(iowa_coap_peer_t *peerP)
{
    return peerP->base.type;
}

uint8_t peerSend(iowa_context_t contextP,
                 iowa_coap_peer_t *peerP,
                 iowa_coap_message_t *messageP,
                 coap_message_callback_t resultCallback,
                 void *userData)
{

    uint8_t result;
    coap_exchange_t *exchangeP;
    coap_message_callback_t intermediateCallback;
    void *intermediateUserdata;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Entering with peerP: %p, messageP: %p.", peerP, messageP);

    intermediateCallback = resultCallback;
    intermediateUserdata = userData;

    if (resultCallback != NULL
        && COAP_IS_REQUEST(messageP->code))
    {

        exchangeP = (coap_exchange_t *)iowa_system_malloc(sizeof(coap_exchange_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (exchangeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(coap_exchange_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memset(exchangeP, 0, sizeof(coap_exchange_t));

        exchangeP->tokenLength = messageP->tokenLength;
        memcpy(exchangeP->token, messageP->token, messageP->tokenLength);
        exchangeP->callback = resultCallback;
        exchangeP->userData = userData;
    }
    else
    {
        exchangeP = NULL;
    }

#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
    switch (peerP->base.type)
    {
    case IOWA_CONN_DATAGRAM:
    case IOWA_CONN_LORAWAN:
    case IOWA_CONN_SMS:
        if (exchangeP != NULL
            && messageP->type == IOWA_COAP_TYPE_CONFIRMABLE)
        {
            intermediateCallback = prv_datagramSendResult;
            intermediateUserdata = exchangeP;
        }
        break;

    default:
        break;
    }
#endif

    result = prv_send(contextP, peerP, messageP, intermediateCallback, intermediateUserdata);

    if (result == IOWA_COAP_NO_ERROR)
    {
        if (exchangeP != NULL)
        {
            peerP->base.exchangeList = (coap_exchange_t *)IOWA_UTILS_LIST_ADD(peerP->base.exchangeList, exchangeP);
        }
    }
    else
    {
        prv_freeExchange(exchangeP);
    }

    if (!COAP_IS_REQUEST(messageP->code)
        && result == IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE)
    {
        iowa_coap_message_t *errorReplyP;

        errorReplyP = iowa_coap_message_new(messageP->type, IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE, messageP->tokenLength, messageP->token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (errorReplyP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP message.");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        errorReplyP->id = messageP->id;

        (void)prv_send(contextP, peerP, errorReplyP, NULL, NULL);
        iowa_coap_message_free(errorReplyP);
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

int peerSendBuffer(iowa_context_t contextP,
                   iowa_coap_peer_t *peerP,
                   uint8_t *buffer,
                   size_t bufferLength)
{

    return securitySend(contextP, peerP->base.securityS, buffer, bufferLength);
}

int peerRecvBuffer(iowa_context_t contextP,
                   iowa_coap_peer_t *peerP,
                   uint8_t *buffer,
                   size_t bufferLength)
{
    return securityRecv(contextP, peerP->base.securityS, buffer, bufferLength);
}

int32_t coapPeerGetMaxTxWait(iowa_coap_peer_t *peerP)
{
    switch (peerP->base.type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        return ((coap_peer_datagram_t *)peerP)->transmitWait;
#endif

    default:
        break;
    }

    return 0;
}

void peerHandleMessage(iowa_context_t contextP,
                       iowa_coap_peer_t *peerP,
                       iowa_coap_message_t *messageP,
                       bool truncated,
                       size_t maxPayloadSize)
{

    uint8_t code;

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "peerP: %p, truncated: %s, maxPayloadSize: %u.", peerP, truncated ? "true" : "false", maxPayloadSize);
    COAP_LOG_MESSAGE("Handling", peerP->base.type, messageP);

    if (truncated == true)
    {
        code = IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
    }
    else
    {
        code = messageP->code;
    }

#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
    if ((peerP->base.type == IOWA_CONN_DATAGRAM
         || peerP->base.type == IOWA_CONN_LORAWAN
         || peerP->base.type == IOWA_CONN_SMS)
        && code == IOWA_COAP_CODE_EMPTY
        && messageP->type == IOWA_COAP_TYPE_ACKNOWLEDGEMENT)
    {
        goto exit;
    }
#endif
    if (!COAP_IS_REQUEST(messageP->code))
    {
        coap_exchange_t *exchangeP;
        coap_exchange_t *parentP;

        IOWA_LOG_INFO(IOWA_PART_COAP, "Looking for matching exchange.");

        exchangeP = peerP->base.exchangeList;
        parentP = NULL;
        while (exchangeP != NULL)
        {
            if (exchangeP->tokenLength == messageP->tokenLength
                && 0 == memcmp(exchangeP->token, messageP->token, exchangeP->tokenLength))
            {
                break;
            }
            else
            {
                parentP = exchangeP;
                exchangeP = exchangeP->next;
            }
        }
        if (exchangeP != NULL)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Matching exchange found (%p).", (void *)exchangeP);

            if (parentP != NULL)
            {
                parentP->next = exchangeP->next;
            }
            else
            {
                peerP->base.exchangeList = exchangeP->next;
            }
            {
                exchangeP->callback(peerP, code, messageP, exchangeP->userData, contextP);
            }
            prv_freeExchange(exchangeP);

            IOWA_LOG_INFO(IOWA_PART_COAP, "Exiting.");

            goto exit;
        }
    }
    else if (truncated == true)
    {
        iowa_coap_message_t *responseP;

        responseP = iowa_coap_message_prepare_response(messageP, IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (responseP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create response packet.");
            goto exit;
        }
#endif

        (void)peerSend(contextP, peerP, responseP, NULL, NULL);

        iowa_coap_message_free(responseP);
        goto exit;
    }

    IOWA_LOG_INFO(IOWA_PART_COAP, "No matching exchange found.");

    if (peerP->base.requestCallback != NULL)
    {
        peerP->base.requestCallback(peerP, code, messageP, peerP->base.userData, contextP);
    }

exit:
    IOWA_LOG_INFO(IOWA_PART_COAP, "Exiting.");
}

void coapPeerGenerateToken(iowa_coap_peer_t *peerP,
                           uint8_t length,
                           uint8_t *tokenP)
{
    uint8_t i;
    int32_t curTime;
    uint8_t newToken[COAP_MSG_TOKEN_MAX_LEN];
    coap_exchange_t *exchangeP;

    curTime = iowa_system_gettime();
    curTime = (curTime * (size_t)peerP) + curTime;
    memcpy(newToken, &curTime, 4);

    curTime = (curTime * (size_t)tokenP) + curTime;
    memcpy(newToken + 4, &curTime, 4);

    i = 0;
    exchangeP = peerP->base.exchangeList;
    while (exchangeP != NULL)
    {
        if (length == exchangeP->tokenLength
            && 0 == memcmp(newToken, exchangeP->token, length))
        {
            uint8_t temp;

            i++;
            i = i % COAP_MSG_TOKEN_MAX_LEN;
            temp = newToken[length - 1];
            newToken[length - 1] = newToken[i] + 1;
            newToken[i] = temp;

            exchangeP = peerP->base.exchangeList;
        }
        else
        {
            exchangeP = exchangeP->next;
        }
    }

    for (i = 0; i < length; i++)
    {
        tokenP[i] = newToken[i];
    }
}
