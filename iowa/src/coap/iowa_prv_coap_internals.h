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

#ifndef _IOWA_PRV_COAP_INTERNALS_INCLUDE_
#define _IOWA_PRV_COAP_INTERNALS_INCLUDE_

#include <string.h>

#include "iowa_config.h"
#include "iowa_utils.h"

#ifdef LWM2M_CLIENT_MODE
#define IOWA_COAP_CLIENT_MODE
#endif

#include "iowa_prv_coap.h"
#include "iowa_prv_logger.h"
#include "iowa_prv_misc.h"
#include "iowa_platform.h"
#include "iowa_prv_core.h"

/************************************************
* Constants
*/

#define PRV_STREAM_MSG_MAX_HEADER_AND_TOKEN_LENGTH 5 + COAP_MSG_TOKEN_MAX_LEN

#define PRV_OPT_HEADER_LENGTH              1
#define PRV_OPT_HEADER_CONSERVATIVE_LENGTH 3
#define PRV_OPT_DELTA_SHIFT                4
#define PRV_OPT_DELTA_MASK                 0xF0
#define PRV_OPT_LENGTH_MASK                0x0F
#define PRV_OPT_LIMIT_1                    13
#define PRV_OPT_LIMIT_2                    269
#define PRV_INTEGER_LIMIT                  1000000000
#define PRV_OPT_EXTEND_1                   0x0D
#define PRV_OPT_EXTEND_2                   0x0E
#define PRV_OPT_EXTEND_FORBIDDEN           0x0F

#define PRV_MSG_PAYLOAD_MARKER 0xFF

#define COAP_RESERVED_MID 0

#define COAP_FIRST_MID 107

#define COAP_BLOCK_OPTION_MAX_LENGTH 4

#define COAP_UDP_ACK_TIMEOUT       2
#define COAP_UDP_ACK_RANDOM_FACTOR 1.5
#define COAP_UDP_ACK_REAL_TIMEOUT  3
#define COAP_UDP_MAX_RETRANSMIT    4
#define COAP_UDP_NSTART            1
#define COAP_UDP_MAX_TRANSMIT_WAIT (uint32_t)(((COAP_UDP_ACK_TIMEOUT * ( (1 << (COAP_UDP_MAX_RETRANSMIT + 1) ) - 1) * COAP_UDP_ACK_RANDOM_FACTOR)))

#define COAP_SMS_ACK_TIMEOUT       10
#define COAP_SMS_ACK_RANDOM_FACTOR 1.5
#define COAP_SMS_ACK_REAL_TIMEOUT  15
#define COAP_SMS_MAX_RETRANSMIT    4
#define COAP_SMS_NSTART            1
#define COAP_SMS_MAX_TRANSMIT_WAIT (uint32_t)(((COAP_SMS_ACK_TIMEOUT * ( (1 << (COAP_SMS_MAX_RETRANSMIT + 1) ) - 1) * COAP_SMS_ACK_RANDOM_FACTOR)))

#define COAP_LORAWAN_ACK_REAL_TIMEOUT  0
#define COAP_LORAWAN_MAX_RETRANSMIT    0
#define COAP_LORAWAN_NSTART            1
#define COAP_LORAWAN_MAX_TRANSMIT_WAIT 0

/************************************************
 * Macros
 */

#define STR_SECURITY_EVENT(M) ((M) == SECURITY_EVENT_UNDEFINED ? "SECURITY_EVENT_UNDEFINED" :           \
                              ((M) == SECURITY_EVENT_DISCONNECTED ? "SECURITY_EVENT_DISCONNECTED" :     \
                              ((M) == SECURITY_EVENT_CONNECTED ? "SECURITY_EVENT_CONNECTED" :           \
                              ((M) == SECURITY_EVENT_DATA_AVAILABLE ? "SECURITY_EVENT_DATA_AVAILABLE" : \
                              "Unknown"))))

#define PRV_STR_MESSAGE_TYPE(S) ((S) == IOWA_COAP_TYPE_CONFIRMABLE ? "Confirmable" :         \
                                ((S) == IOWA_COAP_TYPE_NON_CONFIRMABLE ? "Non confirmable" : \
                                ((S) == IOWA_COAP_TYPE_ACKNOWLEDGEMENT ? "Acknowledgement" : \
                                ((S) == IOWA_COAP_TYPE_RESET ? "Reset" :                     \
                                "Unknown"))))

#define PRV_STR_COAP_CODE(S) ((S) == IOWA_COAP_CODE_EMPTY ? "Empty" :                                               \
                             ((S) == IOWA_COAP_CODE_GET ? "Get" :                                                   \
                             ((S) == IOWA_COAP_CODE_POST ? "Post" :                                                 \
                             ((S) == IOWA_COAP_CODE_PUT ? "Put" :                                                   \
                             ((S) == IOWA_COAP_CODE_DELETE ? "Delete" :                                             \
                             ((S) == IOWA_COAP_CODE_FETCH ? "Fetch" :                                               \
                             ((S) == IOWA_COAP_CODE_PATCH ? "Patch" :                                               \
                             ((S) == IOWA_COAP_CODE_IPATCH ? "iPatch" :                                             \
                             ((S) == IOWA_COAP_201_CREATED ? "Created" :                                       \
                             ((S) == IOWA_COAP_202_DELETED ? "Deleted" :                                       \
                             ((S) == IOWA_COAP_203_VALID ? "Valid" :                                           \
                             ((S) == IOWA_COAP_204_CHANGED ? "Changed" :                                       \
                             ((S) == IOWA_COAP_205_CONTENT ? "Content" :                                       \
                             ((S) == IOWA_COAP_231_CONTINUE ? "Continue" :                                     \
                             ((S) == IOWA_COAP_400_BAD_REQUEST ? "Bad Request" :                               \
                             ((S) == IOWA_COAP_401_UNAUTHORIZED ? "Unauthorized" :                             \
                             ((S) == IOWA_COAP_402_BAD_OPTION ? "Bad Option" :                                 \
                             ((S) == IOWA_COAP_403_FORBIDDEN ? "Forbidden" :                                   \
                             ((S) == IOWA_COAP_404_NOT_FOUND ? "Not Found" :                                   \
                             ((S) == IOWA_COAP_405_METHOD_NOT_ALLOWED ? "Method Not Allowed" :                 \
                             ((S) == IOWA_COAP_406_NOT_ACCEPTABLE ? "Not Acceptable" :                         \
                             ((S) == IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE ? "Request Entity Incomplete" :   \
                             ((S) == IOWA_COAP_409_CONFLICT ? "Conflict" :                                     \
                             ((S) == IOWA_COAP_412_PRECONDITION_FAILED ? "Precondition Failed" :               \
                             ((S) == IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE ? "Request Entity Too Large" :     \
                             ((S) == IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT ? "Unsupported Content Format" : \
                             ((S) == IOWA_COAP_422_UNPROCESSABLE_ENTITY ? "Unprocessable Entity" :             \
                             ((S) == IOWA_COAP_500_INTERNAL_SERVER_ERROR ? "Internal Server Error" :           \
                             ((S) == IOWA_COAP_501_NOT_IMPLEMENTED ? "Not Implemented" :                       \
                             ((S) == IOWA_COAP_502_BAD_GATEWAY ? "Bad Gateway" :                               \
                             ((S) == IOWA_COAP_503_SERVICE_UNAVAILABLE ? "Service Unavailable" :               \
                             ((S) == IOWA_COAP_504_GATEWAY_TIMEOUT ? "Gateway Timeout" :                       \
                             ((S) == IOWA_COAP_505_PROXYING_NOT_SUPPORTED ? "Proxying Not Supported" :         \
                             "Unknown")))))))))))))))))))))))))))))))))

#define PRV_STR_COAP_OPTION(S) ((S) == IOWA_COAP_OPTION_IF_MATCH ? "If Match" :             \
                               ((S) == IOWA_COAP_OPTION_URI_HOST ? "URI Host" :             \
                               ((S) == IOWA_COAP_OPTION_ETAG ? "ETAG" :                     \
                               ((S) == IOWA_COAP_OPTION_IF_NONE_MATCH ? "If None Match" :   \
                               ((S) == IOWA_COAP_OPTION_OBSERVE ? "Observe" :               \
                               ((S) == IOWA_COAP_OPTION_URI_PORT ? "URI Port" :             \
                               ((S) == IOWA_COAP_OPTION_LOCATION_PATH ? "Location Path" :   \
                               ((S) == IOWA_COAP_OPTION_OSCORE ? "OSCORE" :                 \
                               ((S) == IOWA_COAP_OPTION_URI_PATH ? "URI Path" :             \
                               ((S) == IOWA_COAP_OPTION_CONTENT_FORMAT ? "Content Format" : \
                               ((S) == IOWA_COAP_OPTION_MAX_AGE ? "Max Age" :               \
                               ((S) == IOWA_COAP_OPTION_URI_QUERY ? "URI Query" :           \
                               ((S) == IOWA_COAP_OPTION_ACCEPT ? "Accept" :                 \
                               ((S) == IOWA_COAP_OPTION_LOCATION_QUERY ? "Location Query" : \
                               ((S) == IOWA_COAP_OPTION_BLOCK_2 ? "Block 2" :               \
                               ((S) == IOWA_COAP_OPTION_BLOCK_1 ? "Block 1" :               \
                               ((S) == IOWA_COAP_OPTION_SIZE_2 ? "Size 2" :                 \
                               ((S) == IOWA_COAP_OPTION_PROXY_URI ? "Proxy URI" :           \
                               ((S) == IOWA_COAP_OPTION_PROXY_SCHEME ? "Proxy Scheme" :     \
                               ((S) == IOWA_COAP_OPTION_SIZE_1 ? "Size 1" :                 \
                               ((S) == IOWA_COAP_OPTION_NO_RESPONSE ? "No Response" :       \
                               "Unknown")))))))))))))))))))))


#define PEER_CALL_EVENT_CALLBACK(C, P, E)                                               \
    if ((P)->base.eventCallback != NULL)                                                \
    {                                                                                   \
        (P)->base.eventCallback((iowa_coap_peer_t *)(P), (E), (P)->base.userData, (C)); \
    }

/************************************************
 * Datatypes
 */

typedef enum
{
    COAP_STREAM_STATE_CSM_WAIT = 0,
    COAP_STREAM_STATE_CSM_SENT,
    COAP_STREAM_STATE_OK
} coap_stream_state_t;

struct _coap_transaction_t
{
    struct _coap_transaction_t *next;
    uint16_t                    mID;
    uint8_t                     retrans_counter;
    int32_t                     retrans_time;
    size_t                      buffer_len;
    uint8_t                    *buffer;
    coap_message_callback_t     callback;
    void                       *userData;
};

struct _coap_ack_t
{
    struct _coap_ack_t *next;
    uint16_t            mID;
    int32_t             validity_time;
    size_t              buffer_len;
    uint8_t            *buffer;
};

struct _coap_exchange_t
{
    struct _coap_exchange_t *next;
    uint8_t                  token[COAP_MSG_TOKEN_MAX_LEN];
    uint8_t                  tokenLength;
    coap_message_callback_t  callback;
    void                    *userData;
};

struct _block_transfer_t
{
    struct _block_transfer_t *next;
    iowa_coap_peer_t    *peerP;
    iowa_coap_message_t *messageP;
    iowa_coap_option_t  *optionP;
    bool            isMessageDuplicated;
    uint8_t         szx;
    size_t          payloadLength;
    uint8_t        *payload;
    coap_message_callback_t  resultCallback;
    void                    *userData;
    size_t          index;
};

typedef struct
{
    coap_peer_base_t    base;
    uint8_t             ackTimeout;
    uint8_t             maxRetransmit;
    uint16_t            transmitWait;
    uint16_t            nextMID;
    uint16_t            lastMID;
    coap_transaction_t *transactionList;
    coap_ack_t         *ackList;
} coap_peer_datagram_t;

typedef struct
{
    coap_peer_base_t     base;
    coap_stream_state_t  state;
} coap_peer_stream_t;

struct _coap_context_t
{
    iowa_coap_peer_t              *peerList;
};

typedef struct
{
    iowa_coap_result_callback_t      messageCallback;
    iowa_coap_peer_event_callback_t  eventCallback;
    iowa_coap_message_t             *messageP;
    void                            *callbackUserData;
} application_coap_data_t;

/************************************************
* Functions
*/


void coapUtilsfreeApplicationData(application_coap_data_t *dataP);
void coapMessageCallback(iowa_coap_peer_t *fromPeer, uint8_t code, iowa_coap_message_t *messageP, void *userData, iowa_context_t contextP);


void messageLog(const char *function, unsigned int line, const char *info, iowa_connection_type_t type, iowa_coap_message_t *messageP);
#define COAP_LOG_MESSAGE(I, T, M) messageLog(__func__, __LINE__, (I), (T), (M))


void peer_free(iowa_context_t contextP, iowa_coap_peer_t *peerP);
iowa_coap_peer_t * peer_new(iowa_connection_type_t type);
uint8_t peerSend(iowa_context_t contextP, iowa_coap_peer_t *peerP, iowa_coap_message_t *messageP, coap_message_callback_t resultCallback, void *userData);
int peerSendBuffer(iowa_context_t contextP, iowa_coap_peer_t *peerP, uint8_t *buffer, size_t bufferLength);
int peerRecvBuffer(iowa_context_t contextP, iowa_coap_peer_t *peerP, uint8_t *buffer, size_t bufferLength);
void peerHandleMessage(iowa_context_t contextP, iowa_coap_peer_t *peerP, iowa_coap_message_t *messageP, bool truncated, size_t maxPayloadSize);


void transactionFree(coap_transaction_t *transacP);
uint8_t transactionNew(iowa_context_t contextP, coap_peer_datagram_t *peerP, iowa_coap_message_t *messageP, uint8_t *buffer, size_t bufferLength, coap_message_callback_t resultCallback, void *userData);
uint8_t transactionStep(iowa_context_t contextP, coap_peer_datagram_t *peerP, int32_t currentTime, int32_t *timeoutP);
void transactionHandleMessage(iowa_context_t contextP, coap_peer_datagram_t *peerP, iowa_coap_message_t *messageP, bool truncated, size_t maxPayloadSize);
void acknowledgeFree(coap_ack_t *ackP);


size_t messageDatagramParseHeader(uint8_t *buffer, size_t bufferLength, iowa_coap_message_t **messageP);
uint8_t messageDatagramParse(uint8_t *buffer, size_t bufferLength, iowa_coap_message_t **messageP);
iowa_coap_message_t *messageDuplicate(iowa_coap_message_t *messageP, bool withMemory);
uint8_t messageStreamParseLengthField(uint8_t lenField);
uint8_t messageStreamParseHeader(uint8_t *buffer, size_t bufferLength, iowa_coap_message_t **messageP, size_t *lengthP);


uint8_t blockPush(iowa_context_t contextP, iowa_coap_peer_t *peerP, coap_exchange_t *exchangeP, size_t maxPayloadSize);
void blockHandleMessage(iowa_context_t contextP, iowa_coap_peer_t *peerP, coap_exchange_t *exchangeP, iowa_coap_message_t *replyP, bool truncated);
uint8_t blockSend413Reply(iowa_context_t contextP, iowa_coap_peer_t *peerP, iowa_coap_message_t *messageP, size_t maxPayloadSize);
uint8_t blockRequestNumber(iowa_context_t contextP, iowa_coap_peer_t *peerP, iowa_coap_message_t *messageP, uint32_t number, bool more, uint8_t szx, coap_message_callback_t  resultCallback, void *userData);
void blockResizePayload(iowa_coap_message_t *messageP);
void blockFree(block_transfer_t *blockP);


uint8_t messageSendLoRaWAN(iowa_context_t contextP, iowa_coap_peer_t *peerBaseP, iowa_coap_message_t *messageP, coap_message_callback_t resultCallback, void *userData);
void lorawanSecurityEventCb(iowa_security_session_t securityS, iowa_security_event_t event, void *userData, iowa_context_t contextP);


uint8_t messageSendUDP(iowa_context_t contextP, iowa_coap_peer_t *peerBaseP, iowa_coap_message_t *messageP, coap_message_callback_t resultCallback, void *userData);
void udpSecurityEventCb(iowa_security_session_t securityS, iowa_security_event_t event, void *userData, iowa_context_t contextP);


uint8_t messageSendTCP(iowa_context_t contextP, iowa_coap_peer_t *peerBaseP, iowa_coap_message_t *messageP);
uint8_t peerConnectTCP(iowa_context_t contextP, coap_peer_stream_t *peerP);
void tcpSecurityEventCb(iowa_security_session_t securityS, iowa_security_event_t event, void *userData, iowa_context_t contextP);


uint8_t messageSendSMS(iowa_context_t contextP, iowa_coap_peer_t *peerBaseP, iowa_coap_message_t *messageP, coap_message_callback_t resultCallback, void *userData);
void smsSecurityEventCb(iowa_security_session_t securityS, iowa_security_event_t event, void *userData, iowa_context_t contextP);








oscore_peer_context_t *oscore_contextCreateFromURI(iowa_context_t contextP,
                                                   const char *uri);







iowa_status_t oscore_contextCreateFromOption(iowa_context_t contextP,
                                             iowa_coap_option_t *optionP,
                                             oscore_peer_context_t **peerContextPP);





void oscore_contextDelete(oscore_peer_context_t *peerContextP);







iowa_status_t oscore_encryptMessage(iowa_context_t contextP,
                                    oscore_peer_context_t *peerContextP,
                                    iowa_coap_message_t *plainMessageP,
                                    iowa_coap_message_t **encryptedMessageP);







iowa_status_t oscore_decryptMessage(iowa_context_t contextP,
                                    iowa_coap_peer_t *peerP,
                                    oscore_peer_context_t *peerContextP,
                                    iowa_coap_message_t *encryptedMessageP,
                                    iowa_coap_message_t **plainMessageP);

#endif
