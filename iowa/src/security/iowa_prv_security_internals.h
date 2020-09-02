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

#ifndef _IOWA_PRV_SECURITY_INTERNALS_INCLUDE_
#define _IOWA_PRV_SECURITY_INTERNALS_INCLUDE_

#include "iowa_prv_core.h"
#include "iowa_prv_security.h"
#include "iowa_config.h"

/**************************************************************
* Includes
*/

#if defined(IOWA_COAP_CLIENT_MODE) || defined(LWM2M_CLIENT_MODE)
#define IOWA_SECURITY_CLIENT_MODE
#endif

/**************************************************************
* Defines
*/

#define STR_MBEDTLS_STATE(M)                                                                            \
((M) == MBEDTLS_SSL_HELLO_REQUEST ? "MBEDTLS_SSL_HELLO_REQUEST" :                                       \
((M) == MBEDTLS_SSL_CLIENT_HELLO ? "MBEDTLS_SSL_CLIENT_HELLO" :                                         \
((M) == MBEDTLS_SSL_SERVER_HELLO ? "MBEDTLS_SSL_SERVER_HELLO" :                                         \
((M) == MBEDTLS_SSL_SERVER_CERTIFICATE ? "MBEDTLS_SSL_SERVER_CERTIFICATE" :                             \
((M) == MBEDTLS_SSL_SERVER_KEY_EXCHANGE ? "MBEDTLS_SSL_SERVER_KEY_EXCHANGE" :                           \
((M) == MBEDTLS_SSL_CERTIFICATE_REQUEST ? "MBEDTLS_SSL_CERTIFICATE_REQUEST" :                           \
((M) == MBEDTLS_SSL_SERVER_HELLO_DONE ? "MBEDTLS_SSL_SERVER_HELLO_DONE" :                               \
((M) == MBEDTLS_SSL_CLIENT_CERTIFICATE ? "MBEDTLS_SSL_CLIENT_CERTIFICATE" :                             \
((M) == MBEDTLS_SSL_CLIENT_KEY_EXCHANGE ? "MBEDTLS_SSL_CLIENT_KEY_EXCHANGE" :                           \
((M) == MBEDTLS_SSL_CERTIFICATE_VERIFY ? "MBEDTLS_SSL_CERTIFICATE_VERIFY" :                             \
((M) == MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC ? "MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC" :               \
((M) == MBEDTLS_SSL_CLIENT_FINISHED ? "MBEDTLS_SSL_CLIENT_FINISHED" :                                   \
((M) == MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC ? "MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC" :               \
((M) == MBEDTLS_SSL_SERVER_FINISHED ? "MBEDTLS_SSL_SERVER_FINISHED" :                                   \
((M) == MBEDTLS_SSL_FLUSH_BUFFERS ? "MBEDTLS_SSL_FLUSH_BUFFERS" :                                       \
((M) == MBEDTLS_SSL_HANDSHAKE_WRAPUP ? "MBEDTLS_SSL_HANDSHAKE_WRAPUP" :                                 \
((M) == MBEDTLS_SSL_HANDSHAKE_OVER ? "MBEDTLS_SSL_HANDSHAKE_OVER" :                                     \
((M) == MBEDTLS_SSL_SERVER_NEW_SESSION_TICKET ? "MBEDTLS_SSL_SERVER_NEW_SESSION_TICKET" :               \
((M) == MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT ? "MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT" : \
"Unknown")))))))))))))))))))

#define SESSION_CALL_EVENT_CALLBACK(S, E)                       \
    if ((S)->eventCb != NULL)                                   \
    {                                                           \
        (S)->eventCb((S), (E), (S)->userDataCb, (S)->contextP); \
    }

#define MBEDTLS_CONN_ID_LENGTH 8

/**************************************************************
* Structures
*/

struct _iowa_security_context_t
{
    iowa_security_session_t sessionList;
};

struct _iowa_security_session_t
{
    void                           *nextP;
    iowa_context_t                  contextP;
    comm_channel_t                 *channelP;
    char                           *uri;
    security_event_callback_t       eventCb;
    void                           *userDataCb;
    bool                            isSecure;
    iowa_security_state_t           state;
#ifdef IOWA_SECURITY_CLIENT_MODE
    iowa_security_mode_t            securityMode;
#endif
};

/**************************************************************
* Security layers API
**************************************************************/

/**************************************************************
* MbedTLS API
*/

iowa_status_t mbedtlsCreateClientSession(iowa_security_session_t securityS);
iowa_status_t mbedtlsCreateServerSession(iowa_security_session_t securityS);
void mbedtlsDeleteSession(iowa_security_session_t securityS);
iowa_status_t mbedtlsStep(iowa_security_session_t securityS);
iowa_status_t mbedtlsHandleHandshakePacket(iowa_security_session_t securityS);
iowa_status_t mbedtlsConnect(iowa_security_session_t securityS);
void mbedtlsDisconnect(iowa_security_session_t securityS);
int mbedtlsSend(iowa_security_session_t securityS, uint8_t *buffer, size_t length);
int mbedtlsRecv(iowa_security_session_t securityS, uint8_t *buffer, size_t length);

/**************************************************************
* tinyDTLS API
*/

iowa_status_t tinydtlsCreateClientSession(iowa_security_session_t securityS);
iowa_status_t tinydtlsCreateServerSession(iowa_security_session_t securityS);
void tinydtlsDeleteSession(iowa_security_session_t securityS);
iowa_status_t tinydtlsStep(iowa_security_session_t securityS);
iowa_status_t tinydtlsHandleHandshakePacket(iowa_security_session_t securityS);
void tinydtlsDisconnect(iowa_security_session_t securityS);
int tinydtlsSend(iowa_security_session_t securityS, uint8_t *buffer, size_t length);
int tinydtlsRecv(iowa_security_session_t securityS, uint8_t *buffer, size_t length);

#endif
