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
* Copyright (c) 2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_SECURITY_INCLUDE_
#define _IOWA_PRV_SECURITY_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_security.h"
#include "iowa_config.h"

#include <stddef.h>
#include <stdint.h>

/**************************************************************
* Defines
*/

#define IOWA_SECURITY_LAYER_NONE                0
#define IOWA_SECURITY_LAYER_MBEDTLS             1
#define IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY    2
#define IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY 3
#define IOWA_SECURITY_LAYER_TINYDTLS            4
#define IOWA_SECURITY_LAYER_USER                5

#ifndef IOWA_SECURITY_LAYER
#define IOWA_SECURITY_LAYER IOWA_SECURITY_LAYER_NONE
#endif

#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_TINYDTLS
#define IOWA_SECURITY_PRE_SHARED_KEY_SUPPORT
#define IOWA_SECURITY_RAW_PUBLIC_KEY_SUPPORT
#endif
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY
#define IOWA_SECURITY_OSCORE_SUPPORT
#endif
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY
#define IOWA_SECURITY_OSCORE_SUPPORT
#define IOWA_SECURITY_PRE_SHARED_KEY_SUPPORT
#endif
#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS
#define IOWA_SECURITY_PRE_SHARED_KEY_SUPPORT
#define IOWA_SECURITY_CERTIFICATE_SUPPORT
#define IOWA_SECURITY_OSCORE_SUPPORT
#endif

#define STR_SECURITY_STATE(M)                                                     \
((M) == SECURITY_STATE_DISCONNECTED ? "SECURITY_STATE_DISCONNECTED" :             \
((M) == SECURITY_STATE_DISCONNECTING ? "SECURITY_STATE_DISCONNECTING" :           \
((M) == SECURITY_STATE_INIT_HANDSHAKE ? "SECURITY_STATE_INIT_HANDSHAKE" :         \
((M) == SECURITY_STATE_HANDSHAKING ? "SECURITY_STATE_HANDSHAKING" :               \
((M) == SECURITY_STATE_HANDSHAKE_DONE ? "SECURITY_STATE_HANDSHAKE_DONE" :         \
((M) == SECURITY_STATE_CONNECTED ? "SECURITY_STATE_CONNECTED" :                   \
((M) == SECURITY_STATE_CONNECTION_FAILING ? "SECURITY_STATE_CONNECTION_FAILING" : \
((M) == SECURITY_STATE_CONNECTION_FAILED ? "SECURITY_STATE_CONNECTION_FAILED" :   \
"Unknown"))))))))

/**************************************************************
* Types
*/

// The internal security context. This is an opaque type as the user do not need to modify it.
typedef struct _iowa_security_context_t *iowa_security_context_t;

// The security event callback.
// securityS: the security session as returned by iowa_security_new_session().
// event: the security event.
// userData: the securitySetEventCallback() parameter.
// contextP: as returned by iowa_init().
typedef void(*security_event_callback_t)(iowa_security_session_t securityS,
                                         iowa_security_event_t event,
                                         void *userData,
                                         iowa_context_t contextP);

/*************************************
* API
*/

// Initialize a security context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t securityInit(iowa_context_t contextP);

// Close a security context.
// Parameters:
// - contextP: returned by iowa_init().
void securityClose(iowa_context_t contextP);

// Do a security step: handle handshaking, timeout, ...
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t securityStep(iowa_context_t contextP);

// Initialize a new security session (client side).
// Returned value: A new initialized security session or null in case of error.
// Parameters:
// - contextP: returned by iowa_init().
// - uri: URI of the server, used as identity.
// - securityMode: Inform which security mode to use.
iowa_security_session_t securityClientNewSession(iowa_context_t contextP,
                                                 const char *uri,
                                                 iowa_security_mode_t securityMode);

// Initialize a new security session (server side).
// Returned value: A new initialized security session or null in case of error.
// Parameters:
// - contextP: returned by iowa_init().
// - type: the connection type.
// - connP: the connection as returned by iowa_system_connection_open().
// - isSecure: inform if the connection is secure.
iowa_security_session_t securityServerNewSession(iowa_context_t contextP,
                                                 iowa_connection_type_t type,
                                                 void *connP,
                                                 bool isSecure);

// Close a security session.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the session to close, returned by iowa_security_new_session().
void securityDeleteSession(iowa_context_t contextP,
                           iowa_security_session_t securityS);

// Set the event callback for a security session.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the session to close, returned by iowa_security_new_session().
// - eventCb: the event callback.
// - userDataCb: the user data pass to the event callback when called.
void securitySetEventCallback(iowa_context_t contextP,
                              iowa_security_session_t securityS,
                              security_event_callback_t eventCb,
                              void *userDataCb);

// Open a secure connection to a peer.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the session to open, returned by iowa_security_new_session().
iowa_status_t securityConnect(iowa_context_t contextP,
                              iowa_security_session_t securityS);

// Close a secure connection to a peer.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the session to close, returned by securityConnect().
void securityDisconnect(iowa_context_t contextP,
                        iowa_security_session_t securityS);

// Send data on a secure connection.
// Returned value: the number of bytes sent or a negative number in case of error.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the session to use, returned by securityConnect().
// - buffer, length: data to send.
int securitySend(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);

// Receive data on a secure connection.
// Returned value: the number of bytes received or a negative number in case of error.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the session to use, returned by securityConnect().
// - buffer: to store the read data.
// - length: the number of bytes to read.
int securityRecv(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);

// Return if a security session is encrypted.
// Returned value: 'true' if encrypted, 'false' otherwise.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the secure connection, returned by securityConnect().
bool securityGetIsSecure(iowa_context_t contextP,
                         iowa_security_session_t securityS);

// Retrieve the identity of the peer of a secure connection.
// Returned value: the length in bytes of the identity.
// Parameters:
// - contextP: returned by iowa_init().
// - securityS: the secure connection, returned by securityConnect().
// - buffer: to store the indentity.
// - length: the length in bytes of buffer.
size_t securityGetIdentity(iowa_context_t contextP,
                           iowa_security_session_t securityS,
                           uint8_t *buffer,
                           size_t length);

// Get the connection platform of a Security session.
// Returned value: the connP.
// Parameters:
// - securityS: the secure connection, returned by securityConnect().
void * securityGetConnP(iowa_security_session_t securityS);

// Set the Short Server ID associated to a Security session.
// Parameters:
// - securityS: the secure connection, returned by securityConnect().
// - ssid: a Short Server ID.
void securitySetSSID(iowa_security_session_t securityS,
                     uint16_t ssid);

// Store a key.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - uri: the URI of the peer the key is associated to.
// - securityDataP: data containing the key.
iowa_status_t securityAddKey(iowa_context_t contextP,
                             const char *uri,
                             iowa_security_data_t *securityDataP);

// Remove keys.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - uri: the URI of the peer the keys are associated to.
iowa_status_t securityRemoveKey(iowa_context_t contextP,
                                const char *uri);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_PRV_SECURITY_INCLUDE_
