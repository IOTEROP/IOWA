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
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_COAP_INCLUDE_
#define _IOWA_COAP_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa.h"
#include "iowa_security.h"

/**************************************************************
* Contants
**************************************************************/

/************************************************
* URI Schemes
*/

#define IOWA_URI_SCHEME_COAP_UDP      "coap://"
#define IOWA_URI_SCHEME_COAP_SEC_UDP  "coaps://"

#define IOWA_URI_SCHEME_COAP_TCP      "coap+tcp://"
#define IOWA_URI_SCHEME_COAP_SEC_TCP  "coaps+tcp://"

#define IOWA_URI_SCHEME_COAP_SMS         "coap+sms://"
#define IOWA_URI_SCHEME_COAP_BINARY_SMS  "sms://"

#define IOWA_URI_SCHEME_LORAWAN  "lorawan://"

/************************************************
* CoAP Format
*/

#define IOWA_COAP_FORMAT_TEXT         0
#define IOWA_COAP_FORMAT_LINK_FORMAT  40
#define IOWA_COAP_FORMAT_XML          41
#define IOWA_COAP_FORMAT_OCTET_STREAM 42
#define IOWA_COAP_FORMAT_EXI          47
#define IOWA_COAP_FORMAT_JSON         50

/**************************************************************
 * Types
 **************************************************************/

typedef struct _iowa_coap_peer_t iowa_coap_peer_t;
typedef struct _iowa_coap_message_t iowa_coap_message_t;

typedef enum
{
    COAP_EVENT_UNDEFINED = 0,
    COAP_EVENT_CONNECTED,
    COAP_EVENT_DISCONNECTED
} iowa_coap_peer_event_t;

// The callback called when the response to a CoAP request is received,
// or when a confirmable transmission failed.
// fromPeer: the peer which sent the CoAP packet.
// code: the code of the message or the status of the transmission.
// messageP: the received CoAP request. This can be nil.
// userData: the iowa_coap_peer_new() parameter.
// contextP: the IOWA context on which iowa_coap_peer_new() was called.
typedef void(*iowa_coap_result_callback_t)(iowa_coap_peer_t *fromPeer,
                                           uint8_t code,
                                           iowa_coap_message_t *messageP,
                                           void *userData,
                                           iowa_context_t contextP);

// The callback called when a peer generates an event.
// fromPeer: the peer which generated the event.
// event: the event.
// userData: the iowa_coap_peer_new() parameter.
// contextP: the IOWA context on which iowa_coap_peer_new() was called.
typedef void(*iowa_coap_peer_event_callback_t)(iowa_coap_peer_t *fromPeer,
                                               iowa_coap_peer_event_t event,
                                               void *userData,
                                               iowa_context_t contextP);

/**************************************************************
 * APIs
 **************************************************************/

// Create a CoAP peer.
// Returned value: A new initialized CoAP peer or null in case of error.
// Parameters:
// - contextP: as returned by iowa_init().
// - uri: the URI of the peer.
// - securityMode: the security to use.
// - eventCb: the callback called when an event occurs on the peer.
// - callbackUserData: past as parameter to eventCallback. This can be nil.
iowa_coap_peer_t *iowa_coap_peer_new(iowa_context_t contextP,
                                     const char *uri,
                                     iowa_security_mode_t securityMode,
                                     iowa_coap_peer_event_callback_t eventCb,
                                     void *callbackUserData);

// Close a CoAP peer.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: a CoAP peer.
void iowa_coap_peer_delete(iowa_context_t contextP,
                           iowa_coap_peer_t *peerP);

// Connect to a CoAP peer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: a CoAP peer.
iowa_status_t iowa_coap_peer_connect(iowa_context_t contextP,
                                     iowa_coap_peer_t *peerP);

// Disconnect from a CoAP peer.
// Returned value: none.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: a CoAP peer.
void iowa_coap_peer_disconnect(iowa_context_t contextP,
                               iowa_coap_peer_t *peerP);

// Send a CoAP GET request to a peer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: the CoAP peer to send the GET request to.
// - path: the path component of the uri to retrieve. This can be nil.
// - query: the query component of the uri to retrieve. This can be nil.
// - resultCb: the callback to call when a reply is received or when the transmission fails.
// - userData: past as parameter to resultCallback. This can be nil.
iowa_status_t iowa_coap_peer_get(iowa_context_t contextP,
                                 iowa_coap_peer_t *peerP,
                                 const char *path,
                                 const char *query,
                                 iowa_coap_result_callback_t resultCb,
                                 void *userData);

// Retrieve the pointer to the payload of a CoAP message.
// Returned value: The length in bytes of the payload.
// Parameters:
// - messageP: the CoAP message to inspect.
// - formatP: OUT. the content format of the payload. This may be IOWA_CONTENT_FORMAT_UNSET.
// - payloadP: OUT. a pointer to the payload of the message.
size_t iowa_coap_message_get_payload(iowa_coap_message_t *messageP,
                                     iowa_content_format_t *formatP,
                                     uint8_t **payloadP);

// Retrieve block information in a CoAP message.
// Returned value: IOWA_COAP_NO_ERROR in case of success, or IOWA_COAP_404_NOT_FOUND if the CoAP message has no block information.
// Parameters:
// - messageP: the CoAP message to inspect.
// - numberP: OUT. the block number.
// - moreP: OUT. true if there are more blocks coming.
// - sizeP: OUT. the size of the block.
iowa_status_t iowa_coap_message_get_block_info(iowa_coap_message_t *messageP,
                                               uint32_t *numberP,
                                               bool *moreP,
                                               uint16_t *sizeP);

// When receiving a reply with a block option, request the next block.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: the CoAP peer to send the message to.
// - messageP: the CoAP message containing the previous block.
// - resultCb: The callback to call when the next block is received.
// - userData: past as parameter to resultCallback. This can be nil.
iowa_status_t iowa_coap_block_request_next(iowa_context_t contextP,
                                           iowa_coap_peer_t *peerP,
                                           iowa_coap_message_t *messageP,
                                           iowa_coap_result_callback_t resultCb,
                                           void *userData);

// Useful during block transfer to request a specific block.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: as returned by iowa_init().
// - peerP: the CoAP peer to send the message to.
// - messageP: the CoAP message containing the block transfer.
// - blockNumber: the block number.
// - resultCb: The callback to call when the next block is received.
// - userData: past as parameter to resultCallback. This can be nil.
uint8_t iowa_coap_block_request_block_number(iowa_context_t contextP,
                                             iowa_coap_peer_t *peerP,
                                             iowa_coap_message_t *messageP,
                                             uint32_t blockNumber,
                                             iowa_coap_result_callback_t resultCb,
                                             void *userData);

/**************************************************************
 * Helper Functions
 **************************************************************/

// Parse an URI and may return its type, hostname, port, path, and query.
// Returned value: '0' in case of success or an error code in the form of a CoAP code.
// Parameters:
// - uri: the URI to parse.
// - typeP: OUT. the connection type.
// - hostnameP: OUT. the hostname. This can be nil.
// - portP: OUT. the port. This can be nil.
// - pathP: OUT. the path. This can be nil.
// - queryP: OUT. the path. This can be nil.
// - isSecureP: OUT. inform if the connection is secured.
iowa_status_t iowa_coap_uri_parse(const char *uri,
                                  iowa_connection_type_t *typeP,
                                  char **hostnameP,
                                  char **portP,
                                  char **pathP,
                                  char **queryP,
                                  bool *isSecureP);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_COAP_INCLUDE_
