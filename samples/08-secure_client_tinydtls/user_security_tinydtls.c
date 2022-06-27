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
* Copyright (c) 2017-2022 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

/*********************************************************************************
*
* This file is an example of providing your own (D)TLS implementation
* to be used with IOWA. Here we are using tinydtls as a "custom" implementation
* and we support only the Pre-Shared Key mode.
*
**********************************************************************************/

#include "iowa_security.h"

#include "iowa_config.h"
#include "iowa_platform.h"
#include "iowa_prv_logger.h"

// tinydtls headers
#include "tinydtls.h"
#include "dtls.h"
#include "dtls_debug.h"

/*************************************************************************************
** Private types
*************************************************************************************/

#define PRV_SUCCESSFUL        0
#define PRV_ERROR             1

// Internal structure holding the tinydtls data
typedef struct
{
    dtls_context_t   *sslContext;
    session_t        *sslSession;
    dtls_handler_t   *dtlsHandler;
    dtls_ecdsa_key_t *ecdsaKey;
    uint8_t          *decodedBuffer;
    size_t            decodedBufferSize;
    bool              clientSide;
} tinydtls_internal_t;

/*************************************************************************************
** Private functions
*************************************************************************************/

// Used to free the allocated tinydtls_internal_t
static void prv_internalsFree(tinydtls_internal_t *internalsP)
{
    if (internalsP->sslContext != NULL)
    {
        dtls_free_context(internalsP->sslContext);
    }
    iowa_system_free(internalsP->sslSession);
    iowa_system_free(internalsP->dtlsHandler);
    iowa_system_free(internalsP->ecdsaKey);
    iowa_system_free(internalsP);
}

// function called by tinydtls to send a buffer to a peer
static int prv_tinydtlsSendFunc(struct dtls_context_t *dtlsContext,
                                session_t *session,
                                uint8_t *buffer,
                                size_t len)
{
    iowa_security_session_t securityS;

    (void)session;

    securityS = dtlsContext->app;

    return iowa_security_connection_send(securityS, buffer, len);
}

// function called by tinydtls to read data received from a peer
static int prv_tinydtlsRecvFunc(struct dtls_context_t *dtlsContext,
                                session_t *session,
                                uint8_t *buffer,
                                size_t len)
{
    iowa_security_session_t securityS;
    tinydtls_internal_t *internalsP;

    (void)session;

    securityS = dtlsContext->app;
    internalsP = (tinydtls_internal_t *)iowa_security_session_get_user_internals(securityS);

    internalsP->decodedBufferSize = len;
    memcpy(internalsP->decodedBuffer, buffer, internalsP->decodedBufferSize);

    return internalsP->decodedBufferSize;
}

// function called by tinydtls to retrieve the PSK security credentials
// In this example we are using iowa_system_security_data(). There could
// be a direct access to a security vault.
static int prv_tinydtlsPskCallback(struct dtls_context_t *dtlsContext,
                                   const session_t *session,
                                   dtls_credentials_type_t type,
                                   const unsigned char *identity,
                                   size_t identityLen,
                                   unsigned char *resultCb,
                                   size_t resultLength)
{
    iowa_security_session_t securityS;
    tinydtls_internal_t *internalsP;
    iowa_security_data_t securityData;
    iowa_status_t result;
    int returnLength;
    const char *uri;
    uint8_t *peerIndentity;
    size_t peerIdentityLen;

    (void)session;

    if (type == DTLS_PSK_HINT)
    {
        return 0;
    }

    securityS = dtlsContext->app;
    internalsP = (tinydtls_internal_t *)iowa_security_session_get_user_internals(securityS);

    memset(&securityData, 0, sizeof(iowa_security_data_t));
    securityData.securityMode = IOWA_SEC_PRE_SHARED_KEY;

    uri = iowa_security_session_get_uri(securityS);

    if (internalsP->clientSide == true)
    {
        peerIndentity = (uint8_t *)uri;
        peerIdentityLen = strlen(uri);
    }
    else
    {
        peerIndentity = (uint8_t *)identity;
        peerIdentityLen = identityLen;
    }

    result = iowa_system_security_data(peerIndentity, peerIdentityLen, IOWA_SEC_READ, &securityData, iowa_security_session_get_context_user_data(securityS));
    if (result != IOWA_COAP_NO_ERROR)
    {
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    // tinyDTLS retrieves the identity-key pair in two pass
    switch (type)
    {
    case DTLS_PSK_IDENTITY:
        if (securityData.protocol.pskData.identityLen == 0
            || securityData.protocol.pskData.identity == NULL)
        {
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }
        if (securityData.protocol.pskData.identityLen > resultLength)
        {
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }

        memcpy(resultCb, securityData.protocol.pskData.identity, securityData.protocol.pskData.identityLen);
        returnLength = securityData.protocol.pskData.identityLen;
        break;

    case DTLS_PSK_KEY:
        if (securityData.protocol.pskData.privateKeyLen == 0
            || securityData.protocol.pskData.privateKey == NULL)
        {
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }
        if (securityData.protocol.pskData.privateKeyLen > resultLength)
        {
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }

        memcpy(resultCb, securityData.protocol.pskData.privateKey, securityData.protocol.pskData.privateKeyLen);
        returnLength = securityData.protocol.pskData.privateKeyLen;
        break;

    default:
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    result = iowa_system_security_data(peerIndentity, peerIdentityLen, IOWA_SEC_FREE, &securityData, iowa_security_session_get_context_user_data(securityS));
    if (result != IOWA_COAP_NO_ERROR)
    {
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    return returnLength;
}

static int prv_tinydtlsConnect(tinydtls_internal_t *internalsP)
{
    int result;

    if (internalsP->sslContext->peers == NULL)
    {
        // Start a fresh handshake
        result = dtls_connect(internalsP->sslContext, internalsP->sslSession);
        if (result < PRV_SUCCESSFUL)
        {
            return result;
        }
    }

    return IOWA_COAP_NO_ERROR;
}

static void prv_connectionFailing(iowa_security_session_t securityS)
{
    iowa_user_security_disconnect(securityS);
    iowa_security_session_set_step_delay(securityS, 0);
    iowa_security_session_set_state(securityS, SECURITY_STATE_CONNECTION_FAILED);
    iowa_security_session_generate_event(securityS, SECURITY_EVENT_DISCONNECTED);
}

/*************************************************************************************
** Custom security implementation functions
*************************************************************************************/

// Initialize tinydtls data for a new client-side security session.
iowa_status_t iowa_user_security_create_client_session(iowa_security_session_t securityS)
{
    tinydtls_internal_t *internalsP;

    internalsP = (tinydtls_internal_t *)iowa_system_malloc(sizeof(tinydtls_internal_t));
    if (internalsP == NULL)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    memset(internalsP, 0, sizeof(tinydtls_internal_t));

    internalsP->clientSide = true;

    // Init a tinydtls context
    dtls_init();
    internalsP->sslContext = dtls_new_context(securityS);
    if (internalsP->sslContext == NULL)
    {
        goto error;
    }

    // Set the callbacks
    internalsP->dtlsHandler = (dtls_handler_t *)iowa_system_malloc(sizeof(dtls_handler_t));
    if (internalsP->dtlsHandler == NULL)
    {
        goto error;
    }
    memset(internalsP->dtlsHandler, 0, sizeof(dtls_handler_t));

    internalsP->dtlsHandler->write = prv_tinydtlsSendFunc;
    internalsP->dtlsHandler->read = prv_tinydtlsRecvFunc;

    // Here we support only PSK mode
    switch (iowa_security_session_get_security_mode(securityS))
    {
    case IOWA_SEC_PRE_SHARED_KEY:
        internalsP->dtlsHandler->get_psk_info = prv_tinydtlsPskCallback;
        break;

    default:
        // Unhandled security mode
        goto error;
    }

    dtls_set_handler(internalsP->sslContext, internalsP->dtlsHandler);

    // Init tinydtls session
    internalsP->sslSession = (session_t *)iowa_system_malloc(sizeof(session_t));
    if (internalsP->sslSession == NULL)
    {
        goto error;
    }
    memset(internalsP->sslSession, 0, sizeof(session_t));

    // Store the tinydtls data in the IOWA session
    iowa_security_session_set_user_internals(securityS, internalsP);

    return IOWA_COAP_NO_ERROR;

error:
    prv_internalsFree(internalsP);

    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}

// This function is required only when in Server mode.
iowa_status_t iowa_user_security_create_server_session(iowa_security_session_t securityS)
{
    tinydtls_internal_t *internalsP;

    internalsP = (tinydtls_internal_t *)iowa_system_malloc(sizeof(tinydtls_internal_t));
    if (internalsP == NULL)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    memset(internalsP, 0, sizeof(tinydtls_internal_t));

    internalsP->clientSide = false;

    // Init tinydtls context
    dtls_init();
    internalsP->sslContext = dtls_new_context(securityS);
    if (internalsP->sslContext == NULL)
    {
        goto error;
    }

    // Set the callbacks
    internalsP->dtlsHandler = (dtls_handler_t *)iowa_system_malloc(sizeof(dtls_handler_t));
    if (internalsP->dtlsHandler == NULL)
    {
        goto error;
    }
    memset(internalsP->dtlsHandler, 0, sizeof(dtls_handler_t));

    internalsP->dtlsHandler->write = prv_tinydtlsSendFunc;
    internalsP->dtlsHandler->read = prv_tinydtlsRecvFunc;

    // Here we support only PSK in server mode
    internalsP->dtlsHandler->get_psk_info = prv_tinydtlsPskCallback;

    dtls_set_handler(internalsP->sslContext, internalsP->dtlsHandler);

    // Init tinydtls session
    internalsP->sslSession = (session_t *)iowa_system_malloc(sizeof(session_t));
    if (internalsP->sslSession == NULL)
    {
        goto error;
    }
    memset(internalsP->sslSession, 0, sizeof(session_t));

    iowa_security_session_set_user_internals(securityS, internalsP);

    return IOWA_COAP_NO_ERROR;

error:
    prv_internalsFree(internalsP);
    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}

// Delete tinydtls data of a security session.
void iowa_user_security_delete_session(iowa_security_session_t securityS)
{
    tinydtls_internal_t *internalsP;

    internalsP = iowa_security_session_get_user_internals(securityS);

    prv_internalsFree(internalsP);
}

// Handle an incoming packet during the TLS/DTLS handshake.
iowa_status_t iowa_user_security_handle_handshake_packet(iowa_security_session_t securityS)
{
    tinydtls_internal_t *internalsP;
    uint8_t buffer[IOWA_BUFFER_SIZE];
    int bufferLength;

    internalsP = iowa_security_session_get_user_internals(securityS);

    // Received the incoming packet from the underlying connection
    bufferLength = iowa_security_connection_recv(securityS, buffer, IOWA_BUFFER_SIZE);
    if (bufferLength > 0)
    {
        if (dtls_handle_message(internalsP->sslContext, internalsP->sslSession, buffer, bufferLength) != PRV_SUCCESSFUL)
        {
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
    }
    else
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    return IOWA_COAP_NO_ERROR;
}

// Perform various tinydtls operation depending on the security session state.
iowa_status_t iowa_user_security_step(iowa_security_session_t securityS)
{
    tinydtls_internal_t *internalsP;

    internalsP = iowa_security_session_get_user_internals(securityS);

    switch (iowa_security_session_get_state(securityS))
    {
    case SECURITY_STATE_DISCONNECTING:
        iowa_user_security_disconnect(securityS);
        iowa_security_session_set_state(securityS, SECURITY_STATE_DISCONNECTED);
        iowa_security_session_generate_event(securityS, SECURITY_EVENT_DISCONNECTED);
        break;

    case SECURITY_STATE_INIT_HANDSHAKE:
    {
        dtls_peer_t *peerP;

        peerP = dtls_get_peer(internalsP->sslContext, internalsP->sslSession);
        if (peerP != NULL
            && peerP->state != DTLS_STATE_INIT)
        {
            return IOWA_COAP_412_PRECONDITION_FAILED;
        }

        if (internalsP->clientSide == true)
        {
            if (dtls_get_peer(internalsP->sslContext, internalsP->sslSession) == NULL)
            {
                int tinyResult;

                // Start a fresh handshake
                tinyResult = dtls_connect(internalsP->sslContext, internalsP->sslSession);
                if (tinyResult < PRV_SUCCESSFUL)
                {
                    prv_connectionFailing(securityS);
                    break;
                }
            }

            iowa_security_session_set_state(securityS, SECURITY_STATE_HANDSHAKING);
            iowa_security_session_set_step_delay(securityS, 0);
        }
        break;
    }

    case SECURITY_STATE_HANDSHAKING:
    {
        dtls_peer_t *peerP;

        peerP = dtls_get_peer(internalsP->sslContext, internalsP->sslSession);
        if (peerP == NULL)
        {
            if (internalsP->clientSide == true)
            {
                IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Handshaking error.");
                prv_connectionFailing(securityS);
            }

            // Do nothing more if the peer has not found
            break;
        }

        switch (internalsP->sslContext->peers->state)
        {
        case DTLS_STATE_CLOSING:
        case DTLS_STATE_CLOSED:
            // an error occurred
            prv_connectionFailing(securityS);
            break;

        case DTLS_STATE_CONNECTED:
            // the DTLS session is established
            iowa_security_session_set_state(securityS, SECURITY_STATE_CONNECTED);
            iowa_security_session_generate_event(securityS, SECURITY_EVENT_CONNECTED);
            iowa_security_session_set_step_delay(securityS, 0);
            break;

        default:
        {
            clock_time_t nextTime;

            // Check next retransmission time
            dtls_check_retransmit(internalsP->sslContext, &nextTime);

            if (nextTime != 0)
            {
                // Timydtls uses microseconds
                iowa_security_session_set_step_delay(securityS, nextTime/1000);
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

// Send data on a secure connection.
int iowa_user_security_send(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length)
{
    tinydtls_internal_t *internalsP;
    int res;

    internalsP = iowa_security_session_get_user_internals(securityS);

    res = dtls_write(internalsP->sslContext, internalsP->sslSession, buffer, length);
    if (res < PRV_SUCCESSFUL)
    {
        return res;
    }

    return length;
}

// Receive data on a secure connection.
int iowa_user_security_recv(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length)
{
    tinydtls_internal_t *internalsP;
    int bufferLength;

    internalsP = iowa_security_session_get_user_internals(securityS);

    bufferLength = iowa_security_connection_recv(securityS, buffer, length);

    // Point to the buffer got from the argument
    // The pointer is then used by the received callback to write the message to the buffer
    internalsP->decodedBuffer = buffer;

    if (dtls_handle_message(internalsP->sslContext, internalsP->sslSession, buffer, bufferLength) != PRV_SUCCESSFUL)
    {
        return 0;
    }

    return internalsP->decodedBufferSize;
}

// Close a secure connection to a peer.
void iowa_user_security_disconnect(iowa_security_session_t securityS)
{
    tinydtls_internal_t *internalsP;
    dtls_peer_t *peerP;

    internalsP = iowa_security_session_get_user_internals(securityS);

    // Close current session
    (void)dtls_close(internalsP->sslContext, internalsP->sslSession);

    peerP = dtls_get_peer(internalsP->sslContext, internalsP->sslSession);
    if (peerP != NULL)
    {
        dtls_reset_peer(internalsP->sslContext, peerP);
    }
}
