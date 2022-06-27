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

#ifndef _OSCORE_INTERNALS_H_
#define _OSCORE_INTERNALS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "iowa_config.h"

#include "iowa_prv_coap_internals.h"
#include "iowa_prv_logger.h"
#include "iowa_prv_misc.h"
#include "iowa_prv_core.h"
#include "iowa_platform.h"

/************************************************
* Constants
*/

#define PRV_OSCORE_VERSION 1

#define PRV_OSCORE_ID_CONTEXT_LENGTH 8

#define PRV_STATE_NEW_CLIENT       0
#define PRV_STATE_NEW_SERVER       1
#define PRV_STATE_CHALLENGE_CLIENT 2
#define PRV_STATE_CHALLENGE_SERVER 3
#define PRV_STATE_VALID            4

#define COSE_AEAD_ALG_AES_CCM_16_64_128 10

#define COSE_HKDF_TYPE_KEY 1
#define COSE_HKDF_TYPE_IV  2

#define PRV_INFO_LENGTH_KEY 16
#define PRV_OSCORE_NONCE_LENGTH  13
#define OSCORE_MAX_ID_LENGTH  7

#define PRV_INCOMING false
#define PRV_OUTGOING true

/************************************************
 * Macros
 */


/************************************************
 * Datatypes
 */

typedef struct
{
    iowa_coap_message_t *messageP;
    iowa_buffer_t        payload;
} challenge_data_t;

typedef struct _request_IV_t
{
    struct _request_IV_t *next;
    uint64_t partialIV;
    bool     way;
    bool     keep;
    uint8_t  tokenLength;
    uint8_t  token[COAP_MSG_TOKEN_MAX_LEN];
} request_IV_t;

struct _oscore_peer_context_t
{
    uint8_t           state;
    iowa_buffer_t     masterSecret;
    iowa_buffer_t     senderId;
    iowa_buffer_t     senderKey;
    iowa_buffer_t     recipientId;
    iowa_buffer_t     recipientKey;
    iowa_buffer_t     idContext;
    iowa_buffer_t     commonIV;
    uint64_t          senderSequenceNumber;
    request_IV_t     *requestList;
    challenge_data_t *challengeDataP;
};

/************************************************
* Functions
*/

// Implemented in iowa_cose.c

// Compute a Nonce according to section 5.2 of RFC 8613.
// Returned value: The nonce.
// Parameters:
// - id: The ID linked to the partial IV.
// - partialIV: The partial IV.
// - commonIV: The common IV.
iowa_buffer_t oscore_coseComputeNonce(iowa_buffer_t id,
                                      uint64_t partialIV,
                                      iowa_buffer_t commonIV);

// Compute an AAD according to section 5.4 of RFC 8613.
// Returned value: The AAD.
// Parameters:
// - id: The ID linked to the partial IV.
// - partialIV: The partial IV.
// - options: The serialized Class I CoAP options IV.
iowa_buffer_t oscore_coseComputeAAD(iowa_buffer_t id,
                                    uint64_t partialIV,
                                    iowa_buffer_t options);

// Use HKDF to derive a key.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - ikm: The Input Keying Material.
// - context: The ID Context to use.
// - type: The type of key to derive.
// - id: The id to derive a key for.
// - keyP: OUT. The derived key.
// - aadP: OUT. The generated Additional Authenticated Data.
iowa_status_t oscore_coseDeriveKey(iowa_buffer_t ikm,
                                   iowa_buffer_t context,
                                   uint8_t type,
                                   iowa_buffer_t id,
                                   iowa_buffer_t *keyP,
                                   iowa_buffer_t *aadP);

// Encrypt a buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - key: The key to use to encrypt.
// - nonce: The nonce to use to encrypt.
// - aad: The Additional Authenticated Data to use to encrypt.
// - plainBuffer: The data to encrypt.
// - encryptedBufferP: To store the encrypted data.
iowa_status_t oscore_coseEncrypt(iowa_buffer_t key,
                                 iowa_buffer_t nonce,
                                 iowa_buffer_t aad,
                                 iowa_buffer_t plainBuffer,
                                 iowa_buffer_t *encryptedBufferP);

// Decrypt a buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - key: The key to use to decrypt.
// - nonce: The nonce to use to decrypt.
// - aad: The Additional Authenticated Data to use to encrypt.
// - encryptedBuffer: The data to decrypt.
// - plainBufferP: To store the decrypted data.
iowa_status_t oscore_coseDecrypt(iowa_buffer_t key,
                                 iowa_buffer_t nonce,
                                 iowa_buffer_t aad,
                                 iowa_buffer_t encryptedBuffer,
                                 iowa_buffer_t *plainBufferP);


#ifdef __cplusplus
}
#endif

#endif // _OSCORE_INTERNALS_H_
