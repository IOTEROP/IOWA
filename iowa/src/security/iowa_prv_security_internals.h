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

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_prv_core.h"
#include "iowa_prv_security.h"
#include "iowa_config.h"

/**************************************************************
* Includes
*/

#if (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY)
#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"
#include "mbedtls/timing.h"
#include "mbedtls/ssl_cookie.h"
#ifdef IOWA_SECURITY_OSCORE_SUPPORT
#include "mbedtls/hkdf.h"
#endif // IOWA_SECURITY_OSCORE_SUPPORT
#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#endif // IOWA_SECURITY_CERTIFICATE_SUPPORT
#elif IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_TINYDTLS
#include "tinydtls.h"
#include "dtls.h"
#include "dtls_debug.h"
#endif // IOWA_SECURITY_LAYER

#if defined(IOWA_COAP_CLIENT_MODE) || defined(LWM2M_CLIENT_MODE)
#define IOWA_SECURITY_CLIENT_MODE
#endif

#if defined(IOWA_COAP_SERVER_MODE) || defined(LWM2M_SERVER_MODE) || defined(LWM2M_BOOTSTRAP_SERVER_MODE) || defined(LWM2M_CLIENT_INCOMING_CONNECTION_SUPPORT)
#define IOWA_SECURITY_SERVER_MODE
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
    iowa_connection_type_t          type;
    char                           *uri;
    security_event_callback_t       eventCb;
    void                           *userDataCb;
    bool                            isSecure;
    iowa_security_state_t           state;
    uint16_t                        shortServerID;
#ifdef IOWA_SECURITY_CLIENT_MODE
    iowa_security_mode_t            securityMode;
#endif
#if (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY)
    // Common
    mbedtls_ssl_context      sslContext;
    mbedtls_ssl_config       conf;
    int                     *ciphersuites;
    int32_t                  startTime;
    uint32_t                 timeout;
    bool                     dataAvailable;
#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
    // Certificate
    mbedtls_x509_crt        *caCert;
    mbedtls_x509_crt        *cert;
    mbedtls_pk_context      *privateKey;
#endif
#ifdef MBEDTLS_SSL_DTLS_CONNECTION_ID
    uint8_t                  connId[MBEDTLS_CONN_ID_LENGTH];
#endif
    mbedtls_ssl_cookie_ctx   cookieContext;
#elif IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_TINYDTLS
    dtls_context_t          *sslContext;
    session_t               *sslSession;
    dtls_handler_t          *dtlsHandler;
    dtls_ecdsa_key_t        *ecdsaKey;
    uint8_t                 *decodedBuffer;
    size_t                   decodedBufferSize;
    bool                     clientSide;
#elif IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
    void                    *userInternalsP;
#endif // IOWA_SECURITY_LAYER
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

#ifdef __cplusplus
}
#endif

#endif // _IOWA_SECURITY_INTERNALS_INCLUDE_
