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
    uint64_t          recipientSequenceNumber;
    challenge_data_t *challengeDataP;
};

/************************************************
* Functions
*/









iowa_buffer_t oscore_coseComputeNonce(iowa_buffer_t id,
                                      uint64_t partialIV,
                                      iowa_buffer_t commonIV);







iowa_buffer_t oscore_coseComputeAAD(iowa_buffer_t id,
                                    uint64_t partialIV,
                                    iowa_buffer_t options);










iowa_status_t oscore_coseDeriveKey(iowa_buffer_t ikm,
                                   iowa_buffer_t context,
                                   uint8_t type,
                                   iowa_buffer_t id,
                                   iowa_buffer_t *keyP,
                                   iowa_buffer_t *aadP);









iowa_status_t oscore_coseEncrypt(iowa_buffer_t key,
                                 iowa_buffer_t nonce,
                                 iowa_buffer_t aad,
                                 iowa_buffer_t plainBuffer,
                                 iowa_buffer_t *encryptedBufferP);









iowa_status_t oscore_coseDecrypt(iowa_buffer_t key,
                                 iowa_buffer_t nonce,
                                 iowa_buffer_t aad,
                                 iowa_buffer_t encryptedBuffer,
                                 iowa_buffer_t *plainBufferP);


#endif
