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
#include "iowa_prv_security_internals.h"
#include <stdbool.h>

/*************************************************************************************
** Private functions
*************************************************************************************/

#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
static bool prv_exchangeFindCallback(void *nodeP,
                                     void *criteriaP)
{
    return nodeP == criteriaP;
}

static void prv_datagramSendResult(iowa_coap_peer_t *fromPeer,
                                   uint8_t code,
                                   iowa_coap_message_t *messageP,
                                   void *userData,
                                   iowa_context_t contextP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "fromPeer: %p, code: %u.%02u, messageP: %p.", fromPeer, code >> 5, code & 0x1F, messageP);

    if (messageP == NULL)
    {
        coap_exchange_t *exchangeP;
        coap_exchange_t *exchangeFoundP;

        exchangeP = (coap_exchange_t *)userData;
        exchangeFoundP = NULL;

        fromPeer->base.exchangeList = (coap_exchange_t *)IOWA_UTILS_LIST_FIND_AND_REMOVE(fromPeer->base.exchangeList, prv_exchangeFindCallback, exchangeP, &exchangeFoundP);
        if (exchangeFoundP != NULL)
        {
            IOWA_LOG_TRACE(IOWA_PART_COAP, "Forward reply to the upper layer.");
            exchangeFoundP->callback(fromPeer, code, messageP, exchangeFoundP->userData, contextP);
            iowa_system_free(exchangeFoundP);
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
    // WARNING: This function is called in a critical section
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
#endif // IOWA_UDP_SUPPORT

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        result = messageSendTCP(contextP, peerP, messageP);
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_COAP, "Unsupported connection type: %d.", peerP->base.type);
        result = IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

static iowa_coap_peer_t *peer_new(iowa_connection_type_t type)
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
        ((coap_peer_datagram_t *)peerP)->transmitWait = COAP_COMPUTE_MAX_TRANSMIT_WAIT(COAP_UDP_ACK_REAL_TIMEOUT, COAP_UDP_MAX_RETRANSMIT);
        break;
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        peerP = (iowa_coap_peer_t *)iowa_system_malloc(sizeof(coap_peer_stream_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (peerP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(coap_peer_stream_t));
            return NULL;
        }
#endif
        memset(peerP, 0, sizeof(coap_peer_stream_t));
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Unknown connection type %d.", type);
        peerP = NULL;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Returning %p", peerP);
    return peerP;
}

static void peer_free(iowa_context_t contextP,
               iowa_coap_peer_t *peerP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Freeing peer %p.", peerP);

    securityDeleteSession(contextP, peerP->base.securityS);

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

#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
static iowa_status_t prv_datagramConfig(coap_peer_datagram_t *peerP,
                                        bool set,
                                        iowa_coap_setting_id_t settingId,
                                        void *argP)
{
    switch (settingId)
    {
    case IOWA_COAP_SETTING_ACK_TIMEOUT:
        if (set == true)
        {
            peerP->ackTimeout = *((uint8_t *)argP);
            peerP->transmitWait = COAP_COMPUTE_MAX_TRANSMIT_WAIT(peerP->ackTimeout, peerP->maxRetransmit);
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "RFC7252 peer %p new ACK_TIMEOUT: %u, new TRANSMIT_WAIT: %u.", peerP, peerP->ackTimeout, peerP->transmitWait);
        }
        else
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "RFC7252 peer %p ACK_TIMEOUT is %u.", peerP, peerP->ackTimeout);
            *((uint8_t *)argP) = peerP->ackTimeout;
        }
        break;

    case IOWA_COAP_SETTING_MAX_RETRANSMIT:
        if (set == true)
        {
            peerP->maxRetransmit = *((uint8_t *)argP);
            peerP->transmitWait = COAP_COMPUTE_MAX_TRANSMIT_WAIT(peerP->ackTimeout, peerP->maxRetransmit);
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "RFC7252 peer %p new MAX_RETRANSMIT: %u, new TRANSMIT_WAIT: %u.", peerP, peerP->maxRetransmit, peerP->transmitWait);
        }
        else
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "RFC7252 peer %p MAX_RETRANSMIT is %u.", peerP, peerP->maxRetransmit);
            *((uint8_t *)argP) = peerP->maxRetransmit;
        }
        break;

        default:
            IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Unknown setting: %u.", settingId);
            return IOWA_COAP_405_METHOD_NOT_ALLOWED;
        }

    return IOWA_COAP_NO_ERROR;
}
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
static iowa_status_t prv_streamConfig(coap_peer_stream_t *peerP,
                                      bool set,
                                      iowa_coap_setting_id_t settingId,
                                      void *argP)
{
    switch (settingId)
    {
    default:
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Unknown setting: %u.", settingId);
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    return IOWA_COAP_NO_ERROR;
}
#endif

/*************************************************************************************
** Public functions
*************************************************************************************/

#ifdef IOWA_COAP_CLIENT_MODE
iowa_coap_peer_t *coapPeerCreate(iowa_context_t contextP,
                                 const char *uri,
                                 iowa_security_mode_t securityMode,
                                 coap_message_callback_t requestCallback,
                                 coap_event_callback_t eventCallback,
                                 void *callbackUserData)
{
    // WARNING: This function is called in a critical section
    security_event_callback_t securityEventCallback;
    iowa_coap_peer_t *peerP;
    iowa_security_session_t securityS;
    iowa_connection_type_t type;
    iowa_security_mode_t transportSecurityMode;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "uri: \"%s\".", uri);

    if (IOWA_COAP_NO_ERROR != iowa_coap_uri_parse(uri, &type, NULL, NULL, NULL, NULL, NULL))
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
        return NULL;
    }

    peerP = peer_new(type);
    if (peerP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Cannot create a new peer.");

        securityDeleteSession(contextP, securityS);
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

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        securityEventCallback = tcpSecurityEventCb;
        break;
#endif

    default:
        // Should not happen
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
    {
#if IOWA_SECURITY_LAYER != IOWA_SECURITY_LAYER_NONE
        int result;

        CRIT_SECTION_LEAVE(contextP);
        result = iowa_system_random_vector_generator((uint8_t *)&(((coap_peer_datagram_t *)peerP)->nextMID),
                                                            sizeof(((coap_peer_datagram_t *)peerP)->nextMID),
                                                            contextP->userData);
        CRIT_SECTION_ENTER(contextP);

        if (0 != result)
#endif
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "iowa_system_random_vector_generator() failed or is not implemented. Using %u as first MID.", COAP_FIRST_MID);

            ((coap_peer_datagram_t *)peerP)->nextMID = COAP_FIRST_MID;
        }
        break;
    }
#endif // defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)

    default:
        break;
    }

    coapPeerSetCallbacks(peerP, requestCallback, eventCallback, callbackUserData);

    contextP->coapContextP->peerList = (iowa_coap_peer_t *)IOWA_UTILS_LIST_ADD(contextP->coapContextP->peerList, peerP);

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Created peer %p.", peerP);

    return peerP;
}
#endif // IOWA_COAP_CLIENT_MODE

#ifdef IOWA_COAP_SERVER_MODE

uint8_t coapPeerNew(iowa_context_t contextP,
                    iowa_connection_type_t type,
                    void *connP,
                    bool isSecure,
                    coap_message_callback_t requestCallback,
                    coap_event_callback_t eventCallback,
                    void *callbackUserData,
                    iowa_coap_peer_t **peerP)
{
    security_event_callback_t securityEventCallback;
    iowa_security_session_t securityS;

    securityS = securityServerNewSession(contextP, type, connP, isSecure);
    if (securityS == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Cannot create a new security session.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    *peerP = peer_new(type);
    if (*peerP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new peer.");
        securityDeleteSession(contextP, securityS);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    (*peerP)->base.type = type;
    (*peerP)->base.securityS = securityS;

    switch ((*peerP)->base.type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        securityEventCallback = udpSecurityEventCb;
        break;
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        securityEventCallback = tcpSecurityEventCb;
        break;
#endif

    default:
        // Should not happen
        securityEventCallback = NULL;
        break;
    }

    securitySetEventCallback(contextP, securityS, securityEventCallback, (void *)*peerP);

    switch ((*peerP)->base.type)
    {
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
#endif
    {
#if IOWA_SECURITY_LAYER != IOWA_SECURITY_LAYER_NONE
        int result;

        CRIT_SECTION_LEAVE(contextP);
        result = iowa_system_random_vector_generator((uint8_t *)&(((coap_peer_datagram_t *)(*peerP))->nextMID),
                                                      sizeof(((coap_peer_datagram_t *)(*peerP))->nextMID),
                                                      contextP->userData);
        CRIT_SECTION_ENTER(contextP);

        if (0 != result)
#endif
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "iowa_system_random_vector_generator() failed or is not implemented. Using %u as first MID.", COAP_FIRST_MID);

            ((coap_peer_datagram_t *)(*peerP))->nextMID = COAP_FIRST_MID;
        }
        break;
    }
#endif // defined(IOWA_UDP_SUPPORT) || defined(IOWA_LORAWAN_SUPPORT) || defined(IOWA_SMS_SUPPORT)

    default:
        break;
    }

    coapPeerSetCallbacks(*peerP, requestCallback, eventCallback, callbackUserData);

    contextP->coapContextP->peerList = (iowa_coap_peer_t *)IOWA_UTILS_LIST_ADD(contextP->coapContextP->peerList, (*peerP));

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Created peer %p.", *peerP);

    return IOWA_COAP_NO_ERROR;
}

#endif // IOWA_COAP_SERVER_MODE

void coapPeerDelete(iowa_context_t contextP,
                    iowa_coap_peer_t *peerP)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Closing peer %p.", peerP);

    if (peerP != NULL
        && peerP->base.type != IOWA_CONN_UNDEFINED)
    {
        iowa_connection_type_t savedType;

        // Mark the peer has being deleted in case a transaction callback would delete the peer.
        savedType = peerP->base.type;
        peerP->base.type = IOWA_CONN_UNDEFINED;

        coapPeerDisconnect(contextP, peerP);

        contextP->coapContextP->peerList = (iowa_coap_peer_t *)IOWA_UTILS_LIST_REMOVE(contextP->coapContextP->peerList, peerP);

        while (peerP->base.exchangeList != NULL)
        {
            coap_exchange_t *exchangeP;

            exchangeP = peerP->base.exchangeList;
            peerP->base.exchangeList = exchangeP->next;

            if (exchangeP->callback != NULL)
            {
                exchangeP->callback(peerP, IOWA_COAP_503_SERVICE_UNAVAILABLE, NULL, exchangeP->userData, contextP);
            }
            iowa_system_free(exchangeP);
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

iowa_status_t coapPeerConfiguration(iowa_coap_peer_t *peerP,
                                    bool set,
                                    iowa_coap_setting_id_t settingId,
                                    void *argP)
{
    iowa_status_t result;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "%s setting %u of peer %p.", set?"Writing":"Reading", settingId, peerP);

    switch (settingId)
    {
    case IOWA_COAP_SETTING_URI_LENGTH:
        if (set == true)
        {
            result = IOWA_COAP_405_METHOD_NOT_ALLOWED;
        }
        else
        {
            *((size_t *)argP) = utilsStrlen(peerP->base.securityS->uri);
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "CoAP peer %p URI length is %u.", peerP, *((size_t *)argP));

            result = IOWA_COAP_NO_ERROR;
        }
        break;

    case IOWA_COAP_SETTING_URI:
        if (set == true)
        {
            result = IOWA_COAP_405_METHOD_NOT_ALLOWED;
        }
        else
        {
            if (peerP->base.securityS->uri != NULL)
            {
                utilsStringCopy((char *)argP, utilsStrlen(peerP->base.securityS->uri) + 1, peerP->base.securityS->uri);
                IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "CoAP peer %p URI is \"%s\".", peerP, peerP->base.securityS->uri);
            }
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "CoAP peer %p has no URI.", peerP);
                *((char *)argP) = 0;
            }

            result = IOWA_COAP_NO_ERROR;
        }
        break;

    default:
        switch (peerP->base.type)
        {
#ifdef IOWA_UDP_SUPPORT
        case IOWA_CONN_DATAGRAM:
            result = prv_datagramConfig((coap_peer_datagram_t *)peerP, set, settingId, argP);
            break;
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
        case IOWA_CONN_STREAM:
        case IOWA_CONN_WEBSOCKET:
            result = prv_streamConfig((coap_peer_stream_t *)peerP, set, settingId, argP);
            break;
#endif

        default:
            IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Unknown peer type: %u.", peerP->base.type);
            result = IOWA_COAP_422_UNPROCESSABLE_ENTITY;
        }
    }

    return result;
}

uint8_t coapPeerConnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP)
{
    // WARNING: This function is called in a critical section
    return securityConnect(contextP, peerP->base.securityS);
}

void coapPeerDisconnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP)
{
    // WARNING: This function is called in a critical section
    securityDisconnect(contextP, peerP->base.securityS);
}

iowa_security_state_t coapPeerGetConnectionState(iowa_coap_peer_t *peerP)
{
    iowa_security_state_t state;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Entering for peer %p.", peerP);

    state = iowa_security_session_get_state(peerP->base.securityS);

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    if ((state == SECURITY_STATE_CONNECTED)
        && ((peerP->base.type == IOWA_CONN_STREAM)
            || (peerP->base.type == IOWA_CONN_WEBSOCKET))
        && (((coap_peer_stream_t *)peerP)->state != COAP_STREAM_STATE_OK))
    {
        IOWA_LOG_TRACE(IOWA_PART_COAP, "TCP CSM exchange is not done yet.");
        state = SECURITY_STATE_HANDSHAKE_DONE;
    }
#endif

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting for peer %p with state %s.", peerP, STR_SECURITY_STATE(state));

    return state;
}

iowa_security_session_t coapPeerGetSecuritySession(iowa_coap_peer_t *peerP)
{
    if (NULL == peerP)
    {
        return NULL;
    }

    return peerP->base.securityS;
}

iowa_connection_type_t coapPeerGetConnectionType(iowa_coap_peer_t *peerP)
{
    if (NULL == peerP)
    {
        return IOWA_CONN_UNDEFINED;
    }

    return peerP->base.type;
}

uint8_t peerSend(iowa_context_t contextP,
                 iowa_coap_peer_t *peerP,
                 iowa_coap_message_t *messageP,
                 coap_message_callback_t resultCallback,
                 void *userData)
{
    // WARNING: This function is called in a critical section
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
            // We need to intercept the result from the transport for separate response or reliable transport
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
            // Send was successful, enqueue the exchange
            peerP->base.exchangeList = (coap_exchange_t *)IOWA_UTILS_LIST_ADD(peerP->base.exchangeList, exchangeP);
        }
    }
    else
    {
        // an error occurred, free the exchange
        iowa_system_free(exchangeP);
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
    // WARNING: This function is called in a critical section
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
    // peerP->transmitWait is contained inside a uint16_t. And thus, by definition, UINT16_MAX (65535) can be stored inside a int32_t
    switch (peerP->base.type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        return ((coap_peer_datagram_t *)peerP)->transmitWait;
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        return COAP_TCP_MAX_TRANSMIT_WAIT;
#endif

    default:
        break;
    }

    return -1;
}

int32_t coapPeerGetExchangeLifetime(iowa_coap_peer_t *peerP)
{
    int32_t exchangeLifetime;

    switch (peerP->base.type)
    {
#ifdef IOWA_UDP_SUPPORT
    case IOWA_CONN_DATAGRAM:
        exchangeLifetime = COAP_COMPUTE_MAX_TRANSMIT_SPAN(((coap_peer_datagram_t *)peerP)->ackTimeout, ((coap_peer_datagram_t *)peerP)->maxRetransmit);
        break;
#endif

#if defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT)
    case IOWA_CONN_STREAM:
    case IOWA_CONN_WEBSOCKET:
        exchangeLifetime = INT32_MAX;
        break;
#endif

    default:
        exchangeLifetime = 0;
        break;
    }

    if (exchangeLifetime != 0
        && exchangeLifetime != INT32_MAX)
    {
        exchangeLifetime += 2 * COAP_MAX_LATENCY + COAP_PROCESSING_DELAY;
    }

    return exchangeLifetime;
}

void peerHandleMessage(iowa_context_t contextP,
                       iowa_coap_peer_t *peerP,
                       iowa_coap_message_t *messageP,
                       bool truncated,
                       size_t maxPayloadSize)
{
    // WARNING: This function is called in a critical section
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
        // ignore
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
            iowa_system_free(exchangeP);

            IOWA_LOG_INFO(IOWA_PART_COAP, "Exiting.");

            goto exit;
        }
    }
    else if (truncated == true)
    {
        // This is a request too big for our MTU
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

    // Either this is request or no matching exchange was found
    if (peerP->base.requestCallback != NULL)
    {
        peerP->base.requestCallback(peerP, code, messageP, peerP->base.userData, contextP);
    }
#ifdef IOWA_COAP_SERVER_MODE
    else if (COAP_IS_REQUEST(messageP->code))
    {
        IOWA_LOG_INFO(IOWA_PART_COAP, "Replying with a 4.04 code.");

        coapSendResponse(contextP, peerP, messageP, IOWA_COAP_404_NOT_FOUND);
    }
#endif

exit:
    IOWA_LOG_INFO(IOWA_PART_COAP, "Exiting.");
}

uint8_t coapPeerGenerateToken(iowa_coap_peer_t *peerP,
                              uint8_t *lengthP,
                              uint8_t *tokenP)
{
    size_t i;
    int32_t curTime;
    uint8_t newToken[COAP_MSG_TOKEN_MAX_LEN];
    coap_exchange_t *exchangeP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Entering peerP: %p, exchangeList: %p", peerP, peerP->base.exchangeList);

    // initialize the token length.
    *lengthP = 1;

    // generate some number
    curTime = iowa_system_gettime();
    curTime = (curTime * (size_t)peerP) + curTime;
    memcpy(newToken, &curTime, 4);

    curTime = (curTime * (size_t)tokenP) + curTime;
    memcpy(newToken + 4, &curTime, 4);

    // check it is not already in use
    i = 0;
    exchangeP = peerP->base.exchangeList;
    while (exchangeP != NULL)
    {
        if (*lengthP == exchangeP->tokenLength
            && 0 == memcmp(newToken, exchangeP->token, *lengthP))
        {
            uint8_t temp;

            // change the token
            i++;
            if (i > (1 << ((uint8_t)(*lengthP * 8))))
            {
                IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "No more token possibilities with length of %d bytes.", *lengthP);

                (*lengthP)++;
                if (*lengthP > COAP_MSG_TOKEN_MAX_LEN)
                {
                    IOWA_LOG_ERROR(IOWA_PART_COAP, "No more token possibilities.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
            }

            temp = newToken[*lengthP - 1];
            newToken[*lengthP - 1] = newToken[i % COAP_MSG_TOKEN_MAX_LEN] + 1;
            newToken[i % COAP_MSG_TOKEN_MAX_LEN] = temp;

            // retest
            exchangeP = peerP->base.exchangeList;
        }
        else
        {
          exchangeP = exchangeP->next;
        }
    }

    memcpy(tokenP, newToken, *lengthP);

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Exiting");

    return IOWA_COAP_NO_ERROR;
}
