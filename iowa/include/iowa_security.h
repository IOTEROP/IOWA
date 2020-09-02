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
* Copyright (c) 2019-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_SECURITY_INCLUDE_
#define _IOWA_SECURITY_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa.h"

/**************************************************************
* Types
**************************************************************/

// The internal security session. This is an opaque type as the user do not need to modify it.
typedef struct _iowa_security_session_t *iowa_security_session_t;

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

typedef uint8_t iowa_security_mode_t;

#define IOWA_SEC_PRE_SHARED_KEY 0x00
#define IOWA_SEC_RAW_PUBLIC_KEY 0x01
#define IOWA_SEC_CERTIFICATE    0x02
#define IOWA_SEC_NONE           0x03
#define IOWA_SEC_OSCORE         0xF0

typedef enum
{
    IOWA_SEC_READ,
    IOWA_SEC_FREE,
    IOWA_SEC_CREATE,
    IOWA_SEC_DELETE,
    IOWA_SEC_CHECK
} iowa_security_operation_t;

typedef struct
{
    uint8_t *identity;
    size_t   identityLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} iowa_psk_data_t;

typedef struct
{
    uint8_t *caCertificate;
    size_t   caCertificateLen;
    uint8_t *certificate;
    size_t   certificateLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} iowa_certificate_data_t;

typedef struct
{
    uint8_t *publicKeyX;
    size_t   publicKeyXLen;
    uint8_t *publicKeyY;
    size_t   publicKeyYLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} iowa_rpk_data_t;

typedef struct
{
    uint8_t *senderId;
    size_t   senderIdLen;
    uint8_t *recipientId;
    size_t   recipientIdLen;
    uint8_t *masterSecret;
    size_t   masterSecretLen;
} iowa_oscore_data_t;

typedef struct
{
    iowa_security_mode_t securityMode;
    union
    {
        iowa_psk_data_t         pskData;
        iowa_certificate_data_t certData;
        iowa_rpk_data_t         rpkData;
        iowa_oscore_data_t      oscoreData;
    } protocol;
} iowa_security_data_t;

typedef enum
{
    SECURITY_STATE_DISCONNECTED = 0,
    SECURITY_STATE_DISCONNECTING,
    SECURITY_STATE_INIT_HANDSHAKE,
    SECURITY_STATE_HANDSHAKING,
    SECURITY_STATE_HANDSHAKE_DONE,
    SECURITY_STATE_CONNECTED,
    SECURITY_STATE_CONNECTION_FAILING,
    SECURITY_STATE_CONNECTION_FAILED
} iowa_security_state_t;

// The enumeration defining the security event
typedef enum
{
    SECURITY_EVENT_UNDEFINED = 0,
    SECURITY_EVENT_DISCONNECTED,
    SECURITY_EVENT_CONNECTED,
    SECURITY_EVENT_DATA_AVAILABLE
} iowa_security_event_t;

// The enumeration of HMAC algorithms used by OSCORE. See table 7 of RFC 8152.
typedef enum
{
    SECURITY_HMAC_UNDEFINED = 0,
    SECURITY_HMAC_SHA256_64 = 4,
    SECURITY_HMAC_SHA256    = 5,
    SECURITY_HMAC_SHA384    = 6,
    SECURITY_HMAC_SHA512    = 7
} iowa_security_hash_t;

// The enumeration of AEAD algorithms used by OSCORE. See table 10 of RFC 8152.
typedef enum
{
    SECURITY_AEAD_UNDEFINED          = 0,
    SECURITY_AEAD_AES_CCM_16_64_128  = 10,
    SECURITY_AEAD_AES_CCM_16_64_256  = 11,
    SECURITY_AEAD_AES_CCM_64_64_128  = 12,
    SECURITY_AEAD_AES_CCM_64_64_256  = 13,
    SECURITY_AEAD_AES_CCM_16_128_128 = 30,
    SECURITY_AEAD_AES_CCM_16_128_256 = 31,
    SECURITY_AEAD_AES_CCM_64_128_128 = 32,
    SECURITY_AEAD_AES_CCM_64_128_256 = 33
} iowa_security_aead_t;

/**************************************************************
* Security helper functions.
**************************************************************/

// Store opaque data in a security session.
// Returned value: none.
// Parameters:
// - securityS: a security session.
// - internalsP: pointer to opaque data to store.
void iowa_security_session_set_user_internals(iowa_security_session_t securityS,
                                              void *internalsP);

// Retrieve opaque data stored in a security session with iowa_security_session_set_user_internals().
// Returned value: pointer to stored opaque data.
// Parameters:
// - securityS: a security session.
void * iowa_security_session_get_user_internals(iowa_security_session_t securityS);

// Get the state of a security session.
// Returned value: the state of a security session.
// Parameters:
// - securityS: a security session.
iowa_security_state_t iowa_security_session_get_state(iowa_security_session_t securityS);

// Set the state of a security session.
// Returned value: none.
// Parameters:
// - securityS: a security session.
// - state: the new state of the security session.
void iowa_security_session_set_state(iowa_security_session_t securityS,
                                     iowa_security_state_t state);

// Get the security session associated to a Server.
// Returned value: a security session.
// Parameters:
// - contextP: returned by iowa_init().
// - shortServerId: the Short Server ID.
iowa_security_session_t iowa_security_get_server_session(iowa_context_t contextP,
                                                         uint16_t shortServerId);

// Get the security session associated to a Client.
// Returned value: a security session.
// Parameters:
// - contextP: returned by iowa_init().
// - clientId: the Client's ID.
iowa_security_session_t iowa_security_get_client_session(iowa_context_t contextP,
                                                         uint32_t clientId);

// Call the event callback of a security session.
// Returned value: none.
// Parameters:
// - securityS: a security session.
// - event: the event past as parameter to the callback.
void iowa_security_session_generate_event(iowa_security_session_t securityS,
                                          iowa_security_event_t event);

// Get the type of the underlying connection of a security session.
// Returned value: the type of the underlying connection.
// Parameters:
// - securityS: a security session.
iowa_connection_type_t iowa_security_session_get_connection_type(iowa_security_session_t securityS);

// Get the security mode of a security session.
// Returned value: the security mode.
// Parameters:
// - securityS: a security session.
iowa_security_mode_t iowa_security_session_get_security_mode(iowa_security_session_t securityS);

// Get the URI of the peer of a security session.
// Returned value: the URI of the peer.
// Parameters:
// - securityS: a security session.
const char * iowa_security_session_get_uri(iowa_security_session_t securityS);

// Set the delay before the next scheduled operation on a security session.
// Returned value: the IOWA timeout.
// Parameters:
// - securityS: a security session.
// - delay: the time in seconds before iowa_user_security_step() needs to be call again.
void iowa_security_session_set_step_delay(iowa_security_session_t securityS,
                                          int32_t delay);

// Get the IOWA context in which a security session exists.
// Returned value: the IOWA context.
// Parameters:
// - securityS: a security session.
iowa_context_t iowa_security_session_get_context(iowa_security_session_t securityS);

// Get the pointer to application-specific data of the IOWA context in which a security session exists.
// Returned value: the pointer to application-specific data as past to iowa_init().
// Parameters:
// - securityS: a security session.
void * iowa_security_session_get_context_user_data(iowa_security_session_t securityS);

// Send data on the underlying connection of a security session.
// Returned value: the number of bytes sent or a negative number in case of error.
// Parameters:
// - securityS: a security session.
// - buffer, length: data to send.
int iowa_security_connection_send(iowa_security_session_t securityS,
                                  uint8_t *buffer,
                                  size_t length);

// Read data on the underlying connection of a security session.
// Returned value: the number of bytes read or a negative number in case of error.
// Parameters:
// - securityS: a security session.
// - buffer: to store the read data.
// - length: the number of bytes to read.
int iowa_security_connection_recv(iowa_security_session_t securityS,
                                  uint8_t *buffer,
                                  size_t length);

/**************************************************************
* Security implementation abstraction functions
* To be implemented by the user
**************************************************************/

// Initialize implementation internals data of a new security session (client side).
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - securityS: the session to initialize.
iowa_status_t iowa_user_security_create_client_session(iowa_security_session_t securityS);

// Initialize implementation internals data of a new security session (server side).
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - securityS: the session to initialize.
iowa_status_t iowa_user_security_create_server_session(iowa_security_session_t securityS);

// Delete implementation internals data of a security session.
// Returned value: none.
// Parameters:
// - securityS: the session to delete.
void iowa_user_security_delete_session(iowa_security_session_t securityS);

// Handle an incoming packet while the security session is in the SECURITY_STATE_HANDSHAKING state.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - securityS: the session to use.
iowa_status_t iowa_user_security_handle_handshake_packet(iowa_security_session_t securityS);

// Do a security state machine step: handle handshaking, timeout, ...
// This function should update the IOWA context global timeout.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - securityS: the session to update.
iowa_status_t iowa_user_security_step(iowa_security_session_t securityS);

// Send data on a secure connection.
// Returned value: the number of bytes sent or a negative number in case of error.
// Parameters:
// - securityS: the session to use.
// - buffer, length: data to send.
int iowa_user_security_send(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length);

// Receive data on a secure connection.
// Returned value: the number of bytes received or a negative number in case of error.
// Parameters:
// - securityS: the session to use.
// - buffer: to store the read data.
// - length: the number of bytes to read.
int iowa_user_security_recv(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length);

// Close a secure connection to a peer.
// Returned value: none.
// Parameters:
// - securityS: the session to close.
void iowa_user_security_disconnect(iowa_security_session_t securityS);

/**************************************************************
* Security implementation abstraction functions used by OSCORE
* To be implemented by the user
**************************************************************/

// Derive a new key using HMAC-based Extract-and-Expand Key Derivation Function from RFC 5869
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - hash: the hash algorithm to use.
// - IKM, IKMLength: the input keying material.
// - salt, saltLength: the salt value. These may be nil and 0 respectively.
// - info, infoLength: the context and application specific information. These may be nil and 0 respectively.
// - OKM: a buffer to store the output keying material.
// - OKMLength: The length in bytes of OKM
iowa_status_t iowa_user_security_HKDF(iowa_security_hash_t hash,
                                      uint8_t *IKM, size_t IKMLength,
                                      uint8_t *salt, size_t saltLength,
                                      uint8_t *info, size_t infoLength,
                                      uint8_t *OKM, size_t OKMLength);

// Encrypt data.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - aead: the AEAD algorithm to use.
// - key, keyLength: the key to use to encrypt.
// - nonce, nonceLength: the nonce to use. These may be nil and 0 respectively.
// - aad, aadLength: the additional data to authenticate. These may be nil and 0 respectively.
// - plainData, plainDataLength: the data to encrypt.
// - encryptedData: a buffer to store the encrypted data.
// - encryptedDataLengthP: IN/OUT. The length in bytes of encryptedData.
// - tag, tagLength: a buffer to store the authentication tag. These may be nil and 0 respectively.
iowa_status_t iowa_user_security_AEAD_encrypt(iowa_security_aead_t aead,
                                              uint8_t *key, size_t keyLength,
                                              uint8_t *nonce, size_t nonceLength,
                                              uint8_t *aad, size_t aadLength,
                                              uint8_t *plainData, size_t plainDataLength,
                                              uint8_t *encryptedData, size_t *encryptedDataLengthP,
                                              uint8_t *tag, size_t tagLength);

// Decrypt data.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - aead: the AEAD algorithm to use.
// - key, keyLength: the key to use to decrypt.
// - nonce, nonceLength: the nonce to use. These may be nil and 0 respectively.
// - aad, aadLength: the additional data to authenticate. These may be nil and 0 respectively.
// - tag, tagLength: the authentication tag. These may be nil and 0 respectively.
// - encryptedData, encryptedDataLength: the data to decrypt.
// - plainData: a buffer to store the decrypted data.
// - plainDataLengthP: IN/OUT. The length in bytes of plainData.
iowa_status_t iowa_user_security_AEAD_decrypt(iowa_security_aead_t aead,
                                              uint8_t *key, size_t keyLength,
                                              uint8_t *nonce, size_t nonceLength,
                                              uint8_t *aad, size_t aadLength,
                                              uint8_t *tag, size_t tagLength,
                                              uint8_t *encryptedData, size_t encryptedDataLength,
                                              uint8_t *plainData, size_t *plainDataLengthP);

#ifdef __cplusplus
}
#endif

#endif
