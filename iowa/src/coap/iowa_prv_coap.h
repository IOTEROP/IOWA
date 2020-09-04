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

#ifndef _IOWA_PRV_COAP_INCLUDE_
#define _IOWA_PRV_COAP_INCLUDE_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "iowa_config.h"
#include "iowa_coap.h"
#include "iowa_prv_comm.h"
#include "iowa_prv_security.h"

/************************************************
* Datagram Message Types
*/

#define IOWA_COAP_TYPE_CONFIRMABLE     (uint8_t)0
#define IOWA_COAP_TYPE_NON_CONFIRMABLE (uint8_t)1
#define IOWA_COAP_TYPE_ACKNOWLEDGEMENT (uint8_t)2
#define IOWA_COAP_TYPE_RESET           (uint8_t)3

/************************************************
* CoAP Codes
*/

#define IOWA_COAP_CODE_EMPTY  (uint8_t)0x00
#define IOWA_COAP_CODE_GET    (uint8_t)0x01
#define IOWA_COAP_CODE_POST   (uint8_t)0x02
#define IOWA_COAP_CODE_PUT    (uint8_t)0x03
#define IOWA_COAP_CODE_DELETE (uint8_t)0x04
#define IOWA_COAP_CODE_FETCH  (uint8_t)0x05
#define IOWA_COAP_CODE_PATCH  (uint8_t)0x06
#define IOWA_COAP_CODE_IPATCH (uint8_t)0x07

/************************************************
* CoAP Option Numbers
*/

#define IOWA_COAP_OPTION_IF_MATCH       (uint16_t)1
#define IOWA_COAP_OPTION_URI_HOST       (uint16_t)3
#define IOWA_COAP_OPTION_ETAG           (uint16_t)4
#define IOWA_COAP_OPTION_IF_NONE_MATCH  (uint16_t)5
#define IOWA_COAP_OPTION_OBSERVE        (uint16_t)6
#define IOWA_COAP_OPTION_URI_PORT       (uint16_t)7
#define IOWA_COAP_OPTION_LOCATION_PATH  (uint16_t)8
#define IOWA_COAP_OPTION_OSCORE         (uint16_t)9
#define IOWA_COAP_OPTION_URI_PATH       (uint16_t)11
#define IOWA_COAP_OPTION_CONTENT_FORMAT (uint16_t)12
#define IOWA_COAP_OPTION_MAX_AGE        (uint16_t)14
#define IOWA_COAP_OPTION_URI_QUERY      (uint16_t)15
#define IOWA_COAP_OPTION_ACCEPT         (uint16_t)17
#define IOWA_COAP_OPTION_LOCATION_QUERY (uint16_t)20
#define IOWA_COAP_OPTION_BLOCK_2        (uint16_t)23
#define IOWA_COAP_OPTION_BLOCK_1        (uint16_t)27
#define IOWA_COAP_OPTION_SIZE_2         (uint16_t)28
#define IOWA_COAP_OPTION_PROXY_URI      (uint16_t)35
#define IOWA_COAP_OPTION_PROXY_SCHEME   (uint16_t)39
#define IOWA_COAP_OPTION_SIZE_1         (uint16_t)60
#define IOWA_COAP_OPTION_NO_RESPONSE    (uint16_t)258

#define COAP_DEFAULT_PORT      "5683"
#define COAP_DEFAULT_PORT_SEC  "5684"

#define COAP_CODE_CLASS_MASK  0xE0

#define COAP_CLASS_REQUEST 0x00

#define COAP_IS_REQUEST(code) (((code) & COAP_CODE_CLASS_MASK) == COAP_CLASS_REQUEST)

#define STR_COAP_EVENT(M)                                     \
((M) == COAP_EVENT_UNDEFINED ? "COAP_EVENT_UNDEFINED" :       \
((M) == COAP_EVENT_CONNECTED ? "COAP_EVENT_CONNECTED" :       \
((M) == COAP_EVENT_DISCONNECTED ? "COAP_EVENT_DISCONNECTED" : \
"Unknown")))

#define COAP_MSG_TOKEN_MAX_LEN 8



typedef struct _coap_context_t *coap_context_t;


typedef struct _coap_transaction_t coap_transaction_t;
typedef struct _coap_ack_t coap_ack_t;
typedef struct _coap_exchange_t coap_exchange_t;
typedef struct _block_transfer_t block_transfer_t;
typedef struct _oscore_peer_context_t oscore_peer_context_t;

typedef struct
{
    uint8_t *data;
    size_t   length;
} iowa_buffer_t;

typedef struct _iowa_linked_buffer_t
{
    struct _iowa_linked_buffer_t *next;
    uint8_t *data;
    size_t   length;
} iowa_linked_buffer_t;

#define IOWA_BUFFER_EMPTY (iowa_buffer_t){NULL, 0}

typedef struct _iowa_coap_option_t
{
    struct _iowa_coap_option_t *next;

    uint16_t      number;
    uint16_t      length;
    union
    {
        uint8_t *asBuffer;
        uint32_t  asInteger;
    } value;
} iowa_coap_option_t;

struct _iowa_coap_message_t
{
    uint8_t               type;
    uint8_t               code;
    uint16_t              id;
    uint8_t               tokenLength;
    uint8_t               token[COAP_MSG_TOKEN_MAX_LEN];
    iowa_coap_option_t   *optionList;
    iowa_buffer_t         payload;
    iowa_linked_buffer_t *userBufferList;
};








typedef void(*coap_message_callback_t)(iowa_coap_peer_t *fromPeer,
                                       uint8_t code,
                                       iowa_coap_message_t *messageP,
                                       void *userData,
                                       iowa_context_t contextP);






typedef void(*coap_event_callback_t)(iowa_coap_peer_t *fromPeer,
                                     iowa_coap_peer_event_t event,
                                     void *userData,
                                     iowa_context_t contextP);



typedef bool(*coap_option_callback_t) (const iowa_coap_option_t *optionP);

typedef struct _coap_peer_base_t
{
    struct _iowa_coap_peer_t *next;
    iowa_connection_type_t    type;
    coap_message_callback_t   requestCallback;
    coap_event_callback_t     eventCallback;
    coap_exchange_t          *exchangeList;
    void                     *userData;
    iowa_security_session_t   securityS;
} coap_peer_base_t;

struct _iowa_coap_peer_t
{
    coap_peer_base_t    base;
};


size_t option_getSerializedLength(iowa_coap_option_t * optionP, coap_option_callback_t isIntegerCallback);
size_t option_serialize(iowa_coap_option_t * optionList, uint8_t * buffer, coap_option_callback_t isIntegerCallback);
uint8_t option_parse(uint8_t * buffer, size_t bufferLength, iowa_coap_option_t * *optionListP, size_t * lengthP, coap_option_callback_t isIntegerCallback);

/************************************************
* APIs
*/





uint8_t coapInit(iowa_context_t contextP);




void coapClose(iowa_context_t contextP);







uint8_t coapStep(iowa_context_t contextP);








void coapPeerSetCallbacks(iowa_coap_peer_t *peerP,
                          coap_message_callback_t requestCallback,
                          coap_event_callback_t eventCallback,
                          void *callbackUserData);










iowa_coap_peer_t * coapPeerCreate(iowa_context_t contextP,
                                  const char *uri,
                                  iowa_security_mode_t securityMode,
                                  coap_message_callback_t requestCallback,
                                  coap_event_callback_t eventCallback,
                                  void *callbackUserData);












uint8_t coapPeerNew(iowa_context_t contextP,
                    iowa_connection_type_t type,
                    void *connP,
                    bool isSecure,
                    coap_message_callback_t requestCallback,
                    coap_event_callback_t eventCallback,
                    void *callbackUserData,
                    iowa_coap_peer_t **peerP);






void coapPeerDelete(iowa_context_t contextP,
                    iowa_coap_peer_t *peerP);






uint8_t coapPeerConnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP);






void coapPeerDisconnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP);





iowa_security_state_t coapPeerGetConnectionState(iowa_coap_peer_t *peerP);





iowa_security_session_t coapPeerGetSecuritySession(iowa_coap_peer_t *peerP);





iowa_connection_type_t coapPeerGetConnectionType(iowa_coap_peer_t *peerP);










uint8_t coapSend(iowa_context_t contextP,
                 iowa_coap_peer_t *peerP,
                 iowa_coap_message_t *messageP,
                 coap_message_callback_t resultCallback,
                 void *userData);







void coapCancelSend(iowa_coap_peer_t *peerP,
                    uint8_t *token,
                    uint8_t tokenLength);

int32_t coapPeerGetMaxTxWait(iowa_coap_peer_t *peerP);

void coapPeerGenerateToken(iowa_coap_peer_t *peerP,
                           uint8_t length,
                           uint8_t *tokenP);

/************************************************
* Utility functions
*/








void coapSendResponse(iowa_context_t contextP,
                      iowa_coap_peer_t *peerP,
                      iowa_coap_message_t *messageP,
                      uint8_t code);






void coapSendReset(iowa_context_t contextP,
                   iowa_coap_peer_t *peerP,
                   iowa_coap_message_t *messageP);






size_t coapMessageSerializeDatagram(iowa_coap_message_t *messageP,
                                    uint8_t **bufferP);






size_t coapMessageSerializeStream(iowa_coap_message_t *messageP,
                                  uint8_t **bufferP);








uint8_t coapDecodeBlockInfo(uint32_t value,
                            uint32_t *numberP,
                            bool *moreP,
                            uint16_t *sizeP);







uint8_t coapEncodeBlockInfo(uint32_t number,
                            bool more,
                            uint16_t size,
                            uint32_t *valueP);





void coapMessageAddUserBuffer(iowa_coap_message_t *messageP,
                              iowa_buffer_t buffer);

/****************************
 * For iowa_coap_option_t
 */





iowa_coap_option_t *iowa_coap_option_new(uint16_t number);





void iowa_coap_option_free(iowa_coap_option_t *optionP);






iowa_coap_option_t *iowa_coap_path_to_option(uint16_t number,
                                             const char *path,
                                             char delimiter);






bool iowa_coap_option_compare_to_path(const iowa_coap_option_t *optionP,
                                      const char *path,
                                      char delimiter);




bool iowa_coap_option_is_integer(const iowa_coap_option_t *optionP);

/****************************
 * For iowa_coap_message_t
 */








iowa_coap_message_t *iowa_coap_message_new(uint8_t type,
                                           uint8_t code,
                                           uint8_t tokenLength,
                                           uint8_t *token);





void iowa_coap_message_free(iowa_coap_message_t *messageP);






iowa_coap_message_t *iowa_coap_message_prepare_response(iowa_coap_message_t *messageP,
                                                        uint8_t code);






void iowa_coap_message_add_option(iowa_coap_message_t *messageP,
                                  iowa_coap_option_t *optionP);






iowa_coap_option_t *iowa_coap_message_find_option(iowa_coap_message_t *messageP,
                                                  uint16_t number);

#endif
