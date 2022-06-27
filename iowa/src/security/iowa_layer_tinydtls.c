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

#include "iowa_prv_security_internals.h"

#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_TINYDTLS

#pragma message("The configuration flags IOWA_SECURITY_LAYER_TINYDTLS will be deprecated in the next IOWA release. It is advised to set the flag IOWA_SECURITY_LAYER_USER instead and link with functions in samples/secure_client_tinydtls/user_security_tinydtls.c.")

#define PRV_HANDSHAKE_TIMEOUT 1
#define PRV_SUCCESSFUL        0
#define PRV_ERROR             1

/*************************************************************************************
** Private functions
*************************************************************************************/

static int prv_tinydtlsSendFunc(struct dtls_context_t *dtlsContext,
                                session_t *session,
                                uint8_t *buffer,
                                size_t len)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = dtlsContext->app;

    return commSend(securityS->contextP, securityS->channelP, buffer, len);
}

static int prv_tinydtlsRecvFunc(struct dtls_context_t *dtlsContext,
                                session_t *session,
                                uint8_t *buffer,
                                size_t len)
{
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = dtlsContext->app;

    securityS->decodedBufferSize = len;
    memcpy(securityS->decodedBuffer, buffer, securityS->decodedBufferSize);

    return securityS->decodedBufferSize;
}

static int prv_tinydtlsPskCallback(struct dtls_context_t *dtlsContext,
                                   const session_t *session,
                                   dtls_credentials_type_t type,
                                   const unsigned char *identity,
                                   size_t identityLen,
                                   unsigned char *resultCb,
                                   size_t resultLength)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;
    iowa_security_data_t securityData;
    iowa_status_t result;
    int returnLength;
    uint8_t *peerIndentity;
    size_t peerIdentityLen;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    if (type == DTLS_PSK_HINT)
    {
        return 0;
    }

    securityS = dtlsContext->app;

    memset(&securityData, 0, sizeof(iowa_security_data_t));
    securityData.securityMode = IOWA_SEC_PRE_SHARED_KEY;

    if (securityS->clientSide == true)
    {
        peerIndentity = (uint8_t *)securityS->uri;
        peerIdentityLen = strlen(securityS->uri);
    }
    else
    {
        peerIndentity = (uint8_t *)identity;
        peerIdentityLen = identityLen;
    }

    CRIT_SECTION_LEAVE(securityS->contextP);
    result = iowa_system_security_data(peerIndentity, peerIdentityLen, IOWA_SEC_READ, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Failed to retrieve the PSK key (%u.%02u).", (result & 0xFF) >> 5, (result & 0x1F));
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    // tinyDTLS retrieves the identity-key pair in two pass
    switch (type)
    {
    case DTLS_PSK_IDENTITY:
        if (securityData.protocol.pskData.identityLen == 0
            || securityData.protocol.pskData.identity == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot set PSK identity: buffer is nil.");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }
        if (securityData.protocol.pskData.identityLen > resultLength)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot set PSK identity: buffer too small.");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }

        memcpy(resultCb, securityData.protocol.pskData.identity, securityData.protocol.pskData.identityLen);
        returnLength = securityData.protocol.pskData.identityLen;
        break;

    case DTLS_PSK_KEY:
        if (securityData.protocol.pskData.privateKeyLen == 0
            || securityData.protocol.pskData.privateKey == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot set PSK key: buffer is nil.");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }
        if (securityData.protocol.pskData.privateKeyLen > resultLength)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot set PSK key: buffer too small.");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }

        memcpy(resultCb, securityData.protocol.pskData.privateKey, securityData.protocol.pskData.privateKeyLen);
        returnLength = securityData.protocol.pskData.privateKeyLen;
        break;

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Unsupported request type: %d.", type);
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    CRIT_SECTION_LEAVE(securityS->contextP);
    (void)iowa_system_security_data(peerIndentity, peerIdentityLen, IOWA_SEC_FREE, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    return returnLength;
}

#ifdef IOWA_SECURITY_CLIENT_MODE
static int prv_tinydtlsRpkCallback(struct dtls_context_t *dtlsContext,
                                   const session_t *session,
                                   const dtls_ecdsa_key_t **ecdsaKey)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;
    iowa_security_data_t securityData;
    iowa_status_t result;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = dtlsContext->app;

    if (securityS->ecdsaKey == NULL)
    {
        securityS->ecdsaKey = (dtls_ecdsa_key_t *)iowa_system_malloc(sizeof(dtls_ecdsa_key_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (securityS->ecdsaKey == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(dtls_ecdsa_key_t));
            return PRV_ERROR;
        }
#endif
        securityS->ecdsaKey->curve = DTLS_ECDH_CURVE_SECP256R1;
    }

    securityData.securityMode = IOWA_SEC_RAW_PUBLIC_KEY;

    CRIT_SECTION_LEAVE(securityS->contextP);
    result = iowa_system_security_data((uint8_t *)securityS->uri, strlen(securityS->uri), IOWA_SEC_READ, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "No RPK key-identity pair found.");
        return PRV_ERROR;
    }

    securityS->ecdsaKey->priv_key = securityData.protocol.rpkData.privateKey;
    securityS->ecdsaKey->pub_key_x = securityData.protocol.rpkData.publicKeyX;
    securityS->ecdsaKey->pub_key_y = securityData.protocol.rpkData.publicKeyY;

    CRIT_SECTION_LEAVE(securityS->contextP);
    (void)iowa_system_security_data((uint8_t *)securityS->uri, strlen(securityS->uri), IOWA_SEC_FREE, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    *ecdsaKey = securityS->ecdsaKey;

    return PRV_SUCCESSFUL;
}

static int prv_tinydtlsVerifyRpkCallback(struct dtls_context_t *dtlsContext,
                                         const session_t *session,
                                         const unsigned char *otherPubX,
                                         const unsigned char *otherPubY,
                                         size_t keySize)
{
    (void)dtlsContext;
    (void)session;
    (void)otherPubX;
    (void)otherPubY;
    (void)keySize;

    return 0;
}
#endif

static void prv_connectionFailing(iowa_security_session_t securityS)
{
    tinydtlsDisconnect(securityS);
    securityS->contextP->timeout = 0;
    securityS->state = SECURITY_STATE_CONNECTION_FAILED;
    SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DISCONNECTED);
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

#ifdef IOWA_SECURITY_CLIENT_MODE
iowa_status_t tinydtlsCreateClientSession(iowa_security_session_t securityS)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS->clientSide = true;

    // Init tinyDTLS context
    dtls_init();
    securityS->sslContext = dtls_new_context(securityS);
    if (securityS->sslContext == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to create the DTLS context.");
        goto error;
    }

    // Set the callbacks
    securityS->dtlsHandler = (dtls_handler_t *)iowa_system_malloc(sizeof(dtls_handler_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->dtlsHandler == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(dtls_handler_t));
        goto error;
    }
#endif
    memset(securityS->dtlsHandler, 0, sizeof(dtls_handler_t));

    securityS->dtlsHandler->write = prv_tinydtlsSendFunc;
    securityS->dtlsHandler->read = prv_tinydtlsRecvFunc;

    switch (securityS->securityMode)
    {
    case IOWA_SEC_PRE_SHARED_KEY:
        securityS->dtlsHandler->get_psk_info = prv_tinydtlsPskCallback;
        break;

    case IOWA_SEC_RAW_PUBLIC_KEY:
        securityS->dtlsHandler->get_ecdsa_key = prv_tinydtlsRpkCallback;
        securityS->dtlsHandler->verify_ecdsa_key = prv_tinydtlsVerifyRpkCallback;
        break;

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Unknown security mode: %d.", securityS->securityMode);
        goto error;
    }

    dtls_set_handler(securityS->sslContext, securityS->dtlsHandler);

    // Init tinyDTLS session
    securityS->sslSession = (session_t *)iowa_system_malloc(sizeof(session_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->sslSession == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(session_t));
        goto error;
    }
#endif
    memset(securityS->sslSession, 0, sizeof(session_t));

    return IOWA_COAP_NO_ERROR;

error:
    tinydtlsDeleteSession(securityS);
    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}
#endif

#ifdef IOWA_SECURITY_SERVER_MODE
iowa_status_t tinydtlsCreateServerSession(iowa_security_session_t securityS)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    // Init tinyDTLS context
    dtls_init();
    securityS->sslContext = dtls_new_context(securityS);
    if (securityS->sslContext == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to create the DTLS context.");
        goto error;
    }

    // Set the callbacks
    securityS->dtlsHandler = (dtls_handler_t *)iowa_system_malloc(sizeof(dtls_handler_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->dtlsHandler == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(dtls_handler_t));
        goto error;
    }
#endif
    memset(securityS->dtlsHandler, 0, sizeof(dtls_handler_t));

    securityS->dtlsHandler->write = prv_tinydtlsSendFunc;
    securityS->dtlsHandler->read = prv_tinydtlsRecvFunc;

    // TODO: Currently, RPK is not supported in server mode
    securityS->dtlsHandler->get_psk_info = prv_tinydtlsPskCallback;

    dtls_set_handler(securityS->sslContext, securityS->dtlsHandler);

    // Init tinyDTLS session
    securityS->sslSession = (session_t *)iowa_system_malloc(sizeof(session_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->sslSession == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(session_t));
        goto error;
    }
#endif
    memset(securityS->sslSession, 0, sizeof(session_t));

    return IOWA_COAP_NO_ERROR;

error:
    tinydtlsDeleteSession(securityS);
    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}
#endif

void tinydtlsDeleteSession(iowa_security_session_t securityS)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    if (securityS->sslContext != NULL)
    {
        dtls_peer_t *peer;

        peer = dtls_get_peer(securityS->sslContext, securityS->sslSession);
        if (peer != NULL)
        {
            dtls_reset_peer(securityS->sslContext, peer);
        }

        dtls_free_context(securityS->sslContext);
    }

    iowa_system_free(securityS->sslSession);
    iowa_system_free(securityS->dtlsHandler);
    iowa_system_free(securityS->ecdsaKey);
}

iowa_status_t tinydtlsStep(iowa_security_session_t securityS)
{
    switch (securityS->state)
    {
    case SECURITY_STATE_DISCONNECTING:
        tinydtlsDisconnect(securityS);
        securityS->state = SECURITY_STATE_DISCONNECTED;
        SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DISCONNECTED);
        break;

    case SECURITY_STATE_INIT_HANDSHAKE:
    {
        dtls_peer_t *peerP;

        peerP = dtls_get_peer(securityS->sslContext, securityS->sslSession);
        if (peerP != NULL
            && peerP->state != DTLS_STATE_INIT)
        {
            IOWA_LOG_WARNING(IOWA_PART_SECURITY, "Session state is not Hello Request.");
            return IOWA_COAP_412_PRECONDITION_FAILED;
        }

        if (securityS->clientSide == true)
        {
            if (dtls_get_peer(securityS->sslContext, securityS->sslSession) == NULL)
            {
                int tinyResult;

                // Start a fresh handshake
                tinyResult = dtls_connect(securityS->sslContext, securityS->sslSession);
                if (tinyResult < PRV_SUCCESSFUL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to initiate the handshake.");
                    prv_connectionFailing(securityS);
                    break;
                }
            }

            securityS->state = SECURITY_STATE_HANDSHAKING;
            securityS->contextP->timeout = 0;
        }
        break;
    }

    case SECURITY_STATE_HANDSHAKING:
    {

        dtls_peer_t *peerP;

        peerP = dtls_get_peer(securityS->sslContext, securityS->sslSession);
        if (peerP == NULL)
        {
            if (securityS->clientSide == true)
            {
                IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Handshaking error.");
                prv_connectionFailing(securityS);
            }

            // Do nothing more if the peer has not found
            break;
        }

        switch (peerP->state)
        {
        case DTLS_STATE_CLOSING:
        case DTLS_STATE_CLOSED:
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Handshaking error.");
            prv_connectionFailing(securityS);
            break;

        case DTLS_STATE_CONNECTED:
            IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake done.");
            securityS->contextP->timeout = 0;
            securityS->state = SECURITY_STATE_CONNECTED;
            SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_CONNECTED);
            break;

        default:
        {
            clock_time_t nextTime;

            // Check next retransmission time
            dtls_check_retransmit(securityS->sslContext, &nextTime);

            if (nextTime != 0)
            {
                int32_t currentTime;
                int32_t delay;

                // Calculate the delay before the next retransmission
                currentTime = iowa_system_gettime();
                delay = nextTime - currentTime;

                if (delay < securityS->contextP->timeout)
                {
                    securityS->contextP->timeout = delay;
                }
            }
        }
        }
    }
    break;

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t tinydtlsHandleHandshakePacket(iowa_security_session_t securityS)
{
    uint8_t buffer[IOWA_BUFFER_SIZE];
    int bufferLength;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    bufferLength = commRecv(securityS->contextP, securityS->channelP, buffer, IOWA_BUFFER_SIZE);
    if (bufferLength > 0)
    {
        if (dtls_handle_message(securityS->sslContext, securityS->sslSession, buffer, bufferLength) != PRV_SUCCESSFUL)
        {
            prv_connectionFailing(securityS);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
    }
    else
    {
        prv_connectionFailing(securityS);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    return IOWA_COAP_NO_ERROR;
}

void tinydtlsDisconnect(iowa_security_session_t securityS)
{
    dtls_peer_t *peerP;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    // Close current session
    (void)dtls_close(securityS->sslContext, securityS->sslSession);

    peerP = dtls_get_peer(securityS->sslContext, securityS->sslSession);
    if (peerP != NULL)
    {
        dtls_reset_peer(securityS->sslContext, peerP);
    }
}

int tinydtlsSend(iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length)
{
    int res;

    res = dtls_write(securityS->sslContext, securityS->sslSession, buffer, length);
    if (res < PRV_SUCCESSFUL)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot write the buffer.");
        return res;
    }

    return length;
}

int tinydtlsRecv(iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length)
{
    int bufferLength;

    bufferLength = commRecv(securityS->contextP, securityS->channelP, buffer, (size_t)length);

    // Point to the buffer got from the argument
    // The pointer is then used by the received callback to write the message to the buffer
    securityS->decodedBuffer = buffer;

    if (dtls_handle_message(securityS->sslContext, securityS->sslSession, buffer, bufferLength) != PRV_SUCCESSFUL)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Cannot handle the buffer.");
        return 0;
    }

    return securityS->decodedBufferSize;
}
#endif // IOWA_SECURITY_LAYER_USER
