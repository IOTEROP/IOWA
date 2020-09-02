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
#define IOWA_COAP_OPTION_OBSERVE        (uint16_t)6    // integer value
#define IOWA_COAP_OPTION_URI_PORT       (uint16_t)7    // integer value
#define IOWA_COAP_OPTION_LOCATION_PATH  (uint16_t)8
#define IOWA_COAP_OPTION_OSCORE         (uint16_t)9
#define IOWA_COAP_OPTION_URI_PATH       (uint16_t)11
#define IOWA_COAP_OPTION_CONTENT_FORMAT (uint16_t)12   // integer value
#define IOWA_COAP_OPTION_MAX_AGE        (uint16_t)14   // integer value
#define IOWA_COAP_OPTION_URI_QUERY      (uint16_t)15
#define IOWA_COAP_OPTION_ACCEPT         (uint16_t)17   // integer value
#define IOWA_COAP_OPTION_LOCATION_QUERY (uint16_t)20
#define IOWA_COAP_OPTION_BLOCK_2        (uint16_t)23   // integer value
#define IOWA_COAP_OPTION_BLOCK_1        (uint16_t)27   // integer value
#define IOWA_COAP_OPTION_SIZE_2         (uint16_t)28   // integer value
#define IOWA_COAP_OPTION_PROXY_URI      (uint16_t)35
#define IOWA_COAP_OPTION_PROXY_SCHEME   (uint16_t)39
#define IOWA_COAP_OPTION_SIZE_1         (uint16_t)60   // integer value
#define IOWA_COAP_OPTION_NO_RESPONSE    (uint16_t)258  // integer value

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

// The CoAP stack internal context.
// This can be an opaque type as the user do not need to modify it.
typedef struct _coap_context_t *coap_context_t;

// A CoAP peer.
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
    uint16_t              id;          // if equal to 0, this is set by the CoAP engine when sending the message
    uint8_t               tokenLength; // '0' means no token.
    uint8_t               token[COAP_MSG_TOKEN_MAX_LEN];
    iowa_coap_option_t   *optionList;
    iowa_buffer_t         payload;
    iowa_linked_buffer_t *userBufferList;  // user-provided buffers that will be freed by iowa_coap_message_free().
};

// The callback called when a CoAP request or a CoAP response is received,
// or when a confirmable transmission failed.
// code: the code of the message or the status of the transmission.
// fromPeer: the peer which sent the CoAP packet.
// messageP: the received CoAP request. This can be nil.
// userData: the coap_configure_server() parameter.
// contextP: as returned by iowa_init().
typedef void(*coap_message_callback_t)(iowa_coap_peer_t *fromPeer,
                                       uint8_t code,
                                       iowa_coap_message_t *messageP,
                                       void *userData,
                                       iowa_context_t contextP);

// The callback called when a peer generates an event.
// fromPeer: the peer which sent the CoAP packet.
// event: the event.
// userData: the coapPeerCreate() or coapPeerSetCallbacks() parameter.
// contextP: as returned by iowa_init().
typedef void(*coap_event_callback_t)(iowa_coap_peer_t *fromPeer,
                                     iowa_coap_peer_event_t event,
                                     void *userData,
                                     iowa_context_t contextP);

// Callback to test if option is integer.
// optionP: the option to test.
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

// Implemented in iowa_option.c
size_t option_getSerializedLength(iowa_coap_option_t * optionP, coap_option_callback_t isIntegerCallback);
size_t option_serialize(iowa_coap_option_t * optionList, uint8_t * buffer, coap_option_callback_t isIntegerCallback);
uint8_t option_parse(uint8_t * buffer, size_t bufferLength, iowa_coap_option_t * *optionListP, size_t * lengthP, coap_option_callback_t isIntegerCallback);

/************************************************
* APIs
*/

// Initialize a CoAP stack context.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: a initialized iowa context.
uint8_t coapInit(iowa_context_t contextP);

// Close a CoAP stack context.
// Parameters:
// - contextP: as returned by iowa_init().
void coapClose(iowa_context_t contextP);

// This function performs any pending operations (typically message re-transmission)
// and returns in timeoutP the delay before the next scheduled operation.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: as returned by iowa_init().
// Note: implementation is single-threaded.
uint8_t coapStep(iowa_context_t contextP);

// Set the request and event callbacks of a CoAP peer.
// Returned value: none.
// Parameters:
// - peerP: a CoAP peer.
// - requestCallback: the callback called when a CoAP *request* is received. This can be nil.
// - eventCallback: the callback called when an event occurs on the peer.
// - callbackUserData: past as parameter to requestCallback and eventCallback. This can be nil.
void coapPeerSetCallbacks(iowa_coap_peer_t *peerP,
                          coap_message_callback_t requestCallback,
                          coap_event_callback_t eventCallback,
                          void *callbackUserData);

// Create a CoAP peer.
// Returned value: A new initialized CoAP peer or null in case of error.
// Parameters:
// - contextP: as returned by iowa_init().
// - uri: the URI of the peer.
// - securityMode: the security to use.
// - requestCallback: the callback called when a CoAP *request* is received. This can be nil.
// - eventCallback: the callback called when an event occurs on the peer.
// - callbackUserData: past as parameter to requestCallback and eventCallback. This can be nil.
iowa_coap_peer_t * coapPeerCreate(iowa_context_t contextP,
                                  const char *uri,
                                  iowa_security_mode_t securityMode,
                                  coap_message_callback_t requestCallback,
                                  coap_event_callback_t eventCallback,
                                  void *callbackUserData);

// Create a CoAP peer from an established connection.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: as returned by iowa_init().
// - type: the type of connection.
// - connP: an opaque connection type.
// - isSecure: use security or not.
// - requestCallback: the callback called when a CoAP *request* is received. This can be nil.
// - eventCallback: the callback called when an event occurs on the peer.
// - callbackUserData: past as parameter to requestCallback and eventCallback. This can be nil.
// - peerP: OUT. The new CoAP peer.
uint8_t coapPeerNew(iowa_context_t contextP,
                    iowa_connection_type_t type,
                    void *connP,
                    bool isSecure,
                    coap_message_callback_t requestCallback,
                    coap_event_callback_t eventCallback,
                    void *callbackUserData,
                    iowa_coap_peer_t **peerP);

// Close a CoAP peer.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: a CoAP peer.
void coapPeerDelete(iowa_context_t contextP,
                    iowa_coap_peer_t *peerP);

// Connect to a CoAP peer.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: a CoAP peer.
uint8_t coapPeerConnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP);

// Disconnect from a CoAP peer.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: a CoAP peer.
void coapPeerDisconnect(iowa_context_t contextP,
                        iowa_coap_peer_t *peerP);

// Get the connection state of a CoAP peer.
// Returned value: iowa_security_state_t.
// Parameters:
// - peerP: a CoAP peer.
iowa_security_state_t coapPeerGetConnectionState(iowa_coap_peer_t *peerP);

// Get the security session of a CoAP peer.
// Returned value: iowa_security_session_t.
// Parameters:
// - peerP: a CoAP peer.
iowa_security_session_t coapPeerGetSecuritySession(iowa_coap_peer_t *peerP);

// Get the connection type of a CoAP peer.
// Returned value: iowa_connection_type_t.
// Parameters:
// - peerP: a CoAP peer.
iowa_connection_type_t coapPeerGetConnectionType(iowa_coap_peer_t *peerP);

// Send a CoAP message to a peer.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: the CoAP peer to send the message to.
// - messageP: the CoAP message to send. If the message ID is 0, an ID is assigned by the CoAP engine.
// - resultCallback: For confirmable messages, the callback called when an acknowledgement is received or when the transmission fails.
//                   This must be nil for other types of message.
// - userData: past as parameter to resultCallback. This can be nil.
uint8_t coapSend(iowa_context_t contextP,
                 iowa_coap_peer_t *peerP,
                 iowa_coap_message_t *messageP,
                 coap_message_callback_t resultCallback,
                 void *userData);

// Cancel a pending message relative to a CoAP peer.
// Returned value: none.
// Parameters:
// - peerP: the CoAP peer the message was sent to.
// - token: message to cancel identified by its token.
// - tokenLength: length in bytes of 'token'.
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

// Create and send a CoAP response message from a received CoAP message.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: the CoAP peer which sent the message.
// - messageP: the received CoAP message.
// - code: the CoAP response message code.
void coapSendResponse(iowa_context_t contextP,
                      iowa_coap_peer_t *peerP,
                      iowa_coap_message_t *messageP,
                      uint8_t code);

// Create and send a CoAP reset message from a received CoAP message.
// Returned value: none.
// Parameters:
// - peerP: the CoAP peer which sent the message.
// - messageP: the received CoAP message.
void coapSendReset(iowa_context_t contextP,
                   iowa_coap_peer_t *peerP,
                   iowa_coap_message_t *messageP);

// Serialize a CoAP message for datagram stransports (e.g. UDP)..
// Returned value: the length of the serialized buffer.
// Parameters:
// - messageP: the CoAP message to serialize.
// - bufferP: OUT. the serialized buffer.
size_t coapMessageSerializeDatagram(iowa_coap_message_t *messageP,
                                    uint8_t **bufferP);

// Serialize a CoAP message for stream stransports (e.g. TCP).
// Returned value: the length of the serialized buffer.
// Parameters:
// - messageP: the CoAP message to serialize.
// - bufferP: OUT. the serialized buffer.
size_t coapMessageSerializeStream(iowa_coap_message_t *messageP,
                                  uint8_t **bufferP);

// Decode block information.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - value: the block value to decode.
// - numberP: OUT. the block number.
// - moreP: OUT. true if there are more blocks coming.
// - sizeP: OUT. the size of the block.
uint8_t coapDecodeBlockInfo(uint32_t value,
                            uint32_t *numberP,
                            bool *moreP,
                            uint16_t *sizeP);

// Encode block information.
// Parameters:
// - number: the block number.
// - more: true if there are more blocks coming.
// - size: the size of the block.
// - valueP: OUT. the encoded block value.
uint8_t coapEncodeBlockInfo(uint32_t number,
                            bool more,
                            uint16_t size,
                            uint32_t *valueP);

// Add an user buffer to the CoAP message.
// Parameters:
// - messageP: the message to add the buffer too. Not tested for validity.
// - buffer: The buffer to add.
void coapMessageAddUserBuffer(iowa_coap_message_t *messageP,
                              iowa_buffer_t buffer);

/****************************
 * For iowa_coap_option_t
 */

// Create a CoAP option.
// Returned value: If created, the option else NULL.
// Parameters:
// - number: the CoAP option number.
iowa_coap_option_t *iowa_coap_option_new(uint16_t number);

// Free a CoAP option.
// Returned value: none.
// Parameters:
// - optionP: the CoAP option to free.
void iowa_coap_option_free(iowa_coap_option_t *optionP);

// Create CoAP options for each segment of a path.
// Returned value: If created, the options else NULL.
// - number: the CoAP option number.
// - path: the path. Must be valid until the option is freed.
// - delimiter: the delimiter used to obtain the segments.
iowa_coap_option_t *iowa_coap_path_to_option(uint16_t number,
                                             const char *path,
                                             char delimiter);

// Compare CoAP options to each segment of a path.
// Returned value: True if paths match. False otherwise.
// - optionP: the options to compare.
// - path: the path.
// - delimiter: the delimiter used to obtain the segments.
bool iowa_coap_option_compare_to_path(const iowa_coap_option_t *optionP,
                                      const char *path,
                                      char delimiter);

// Determine if a CoAP option contains a integer.
// Returned value: True if option contains an integer. False if it contains a buffer.
// - optionP: the option to check.
bool iowa_coap_option_is_integer(const iowa_coap_option_t *optionP);

/****************************
 * For iowa_coap_message_t
 */

// Create a CoAP message.
// Returned value: If created, the message else NULL.
// Parameters:
// - type: the CoAP message type. Unused for stream transports.
// - code: the CoAP message code.
// - tokenLength: length in bytes of 'token'.
// - token: the CoAP message token.
iowa_coap_message_t *iowa_coap_message_new(uint8_t type,
                                           uint8_t code,
                                           uint8_t tokenLength,
                                           uint8_t *token);

// Free a CoAP message.
// Returned value: none.
// Parameters:
// - messageP: the CoAP message to free.
void iowa_coap_message_free(iowa_coap_message_t *messageP);

// Prepare a CoAP response message from a received CoAP message.
// Returned value: If created, the response message else NULL.
// Parameters:
// - messageP: the received CoAP message.
// - code: the CoAP response message code.
iowa_coap_message_t *iowa_coap_message_prepare_response(iowa_coap_message_t *messageP,
                                                        uint8_t code);

// Add a CoAP option to a CoAP message.
// Returned value: none.
// Parameters:
// - messageP: the CoAP message in which the option is added.
// - optionP: the CoAP option to add.
void iowa_coap_message_add_option(iowa_coap_message_t *messageP,
                                  iowa_coap_option_t *optionP);

// Find a CoAP option in a CoAP message.
// Returned value: If found, the option else NULL.
// Parameters:
// - messageP: the CoAP message in which the option is searched.
// - number: the CoAP option number.
iowa_coap_option_t *iowa_coap_message_find_option(iowa_coap_message_t *messageP,
                                                  uint16_t number);

#endif // _IOWA_PRV_COAP_INCLUDE_
