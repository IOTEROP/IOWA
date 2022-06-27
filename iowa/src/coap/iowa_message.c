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
* Copyright (c) 2017-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_coap_internals.h"
#include <stdbool.h>

/************************************************
* Constants
*/

#define PRV_DATAGRAM_MSG_HEADER_LENGTH       4

#define PRV_DATAGRAM_MSG_HEADER_VERSION      0x40
#define PRV_DATAGRAM_MSG_HEADER_VERSION_MASK 0xC0U
#define PRV_DATAGRAM_MSG_HEADER_TYPE_SHIFT   4U
#define PRV_DATAGRAM_MSG_HEADER_TYPE_MASK    0x30U
#define PRV_DATAGRAM_MSG_HEADER_TOKEN_MASK   0x0FU
#define PRV_DATAGRAM_MSG_TOKEN_OFFSET   4

#define PRV_STREAM_MSG_MIN_HEADER_LENGTH 2
#define PRV_STREAM_MSG_MAX_HEADER_LENGTH 6
#define PRV_STREAM_MSG_HEADER_LEN_MASK   0xF0U
#define PRV_STREAM_MSG_HEADER_LEN_SHIFT  4U
#define PRV_STREAM_MSG_HEADER_TOKEN_MASK 0x0FU
#define PRV_STREAM_MSG_LENGTH_LIMIT_1    13
#define PRV_STREAM_MSG_LENGTH_LIMIT_2    269
#define PRV_STREAM_MSG_LENGTH_LIMIT_3    65805
#define PRV_STREAM_MSG_LENGTH_EXTEND_1   0x0D
#define PRV_STREAM_MSG_LENGTH_EXTEND_2   0x0E
#define PRV_STREAM_MSG_LENGTH_EXTEND_3   0x0F

size_t coapMessageSerializeDatagram(iowa_coap_message_t *messageP,
                                    uint8_t **bufferP)
{
    size_t bufferLength;
    uint8_t *buffer;
    size_t index;
    iowa_coap_option_t *optionP;
    uint16_t prevNumber;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering");

    // Compute serialized length
    bufferLength = PRV_DATAGRAM_MSG_HEADER_LENGTH + (size_t)messageP->tokenLength + messageP->payload.length;

    if (messageP->payload.length != 0)
    {
        bufferLength += 1;
    }

    prevNumber = 0;
    for (optionP = messageP->optionList; optionP != NULL; optionP = optionP->next)
    {
        if (optionP->number < prevNumber)
        {
            IOWA_LOG_WARNING(IOWA_PART_COAP, "Exit on error: options are not in order.");
            return 0;
        }
        bufferLength += option_getSerializedLength(optionP, iowa_coap_option_is_integer);
        prevNumber = optionP->number;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Estimated length: %u", bufferLength);

    // Allocate buffer for serialized packet
    buffer = (uint8_t *)iowa_system_malloc(bufferLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (buffer == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(bufferLength);
        return 0;
    }
#endif
    memset(buffer, 0, bufferLength);

    // Set CoAP header
    buffer[0] = (uint8_t)(PRV_DATAGRAM_MSG_HEADER_VERSION + (messageP->type << PRV_DATAGRAM_MSG_HEADER_TYPE_SHIFT) + messageP->tokenLength);
    buffer[1] = messageP->code;
    buffer[2] = (uint8_t)(((uint16_t)(messageP->id & 0xFF00)) >> 8);
    buffer[3] = (uint8_t)(messageP->id & 0xFF);

    // Add token if any
    if (messageP->tokenLength > 0 && messageP->tokenLength <= COAP_MSG_TOKEN_MAX_LEN)
    {
        memcpy(buffer + PRV_DATAGRAM_MSG_TOKEN_OFFSET, messageP->token, messageP->tokenLength);
    }
    else
    {
        messageP->tokenLength = 0;
    }

    index = (size_t)(PRV_DATAGRAM_MSG_TOKEN_OFFSET + messageP->tokenLength);

    index += option_serialize(messageP->optionList, buffer + index, iowa_coap_option_is_integer);

    // Add payload if any
    if (messageP->payload.length != 0)
    {
        buffer[index] = PRV_MSG_PAYLOAD_MARKER;
        index++;
        memcpy(buffer + index, messageP->payload.data, messageP->payload.length);
        index += messageP->payload.length;
    }

    *bufferP = buffer;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Serialized message is %u bytes.", index);

    return index;
}

size_t messageDatagramParseHeader(uint8_t *buffer,
                                  size_t bufferLength,
                                  iowa_coap_message_t **messageP)
{
    uint8_t tokenLen;

    *messageP = NULL;

    if (bufferLength < PRV_DATAGRAM_MSG_HEADER_LENGTH)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Buffer length is only %u.", bufferLength);
        return 0;
    }

    if ((buffer[0] & PRV_DATAGRAM_MSG_HEADER_VERSION_MASK) != PRV_DATAGRAM_MSG_HEADER_VERSION)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Wrong version %u.", (buffer[0] & PRV_DATAGRAM_MSG_HEADER_VERSION_MASK));
        return 0;
    }

    tokenLen = (buffer[0] & PRV_DATAGRAM_MSG_HEADER_TOKEN_MASK);
    if (tokenLen > COAP_MSG_TOKEN_MAX_LEN)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Token length is too big: %u.", tokenLen);
        return 0;
    }
    if ((size_t)(PRV_DATAGRAM_MSG_HEADER_LENGTH + tokenLen) > bufferLength)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Buffer is too short (%u bytes) for the declared token length (%u).", bufferLength, tokenLen);
        return 0;
    }

    *messageP = (iowa_coap_message_t *)iowa_system_malloc(sizeof(iowa_coap_message_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*messageP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_coap_message_t));
        return 0;
    }
#endif
    memset(*messageP, 0, sizeof(iowa_coap_message_t));

    (*messageP)->type = ((uint8_t)(buffer[0] & PRV_DATAGRAM_MSG_HEADER_TYPE_MASK)) >> PRV_DATAGRAM_MSG_HEADER_TYPE_SHIFT;
    (*messageP)->code = buffer[1];
    (*messageP)->id = ((uint16_t) buffer[2] << 8) + buffer[3];
    if (tokenLen > 0)
    {
        (*messageP)->tokenLength = tokenLen;
        memcpy((*messageP)->token, buffer + PRV_DATAGRAM_MSG_TOKEN_OFFSET, tokenLen);
    }

    return (size_t)(tokenLen + PRV_DATAGRAM_MSG_HEADER_LENGTH);
}

uint8_t messageDatagramParse(uint8_t *buffer,
                             size_t bufferLength,
                             iowa_coap_message_t **messageP)
{
    size_t index;
    uint8_t result;
    size_t optLen;

    index = messageDatagramParseHeader(buffer, bufferLength, messageP);
    if (index == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_COAP, "CoAP Header parsing failed.");
        return IOWA_COAP_400_BAD_REQUEST;
    }

    result = option_parse(buffer + index, bufferLength - index, &((*messageP)->optionList), &optLen, iowa_coap_option_is_integer);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_WARNING(IOWA_PART_COAP, "Parsing of the options failed.");
        iowa_coap_message_free(*messageP);
        *messageP = NULL;
        return result;
    }

    index += optLen;

    if (index < bufferLength)
    {
        if (buffer[index] == PRV_MSG_PAYLOAD_MARKER)
        {
            index += 1;
            (*messageP)->payload.length = bufferLength - index;
            (*messageP)->payload.data = buffer + index;
        }
        else
        {
            IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Expected payload marker not found at %u.", bufferLength, index);
            iowa_coap_message_free(*messageP);
            *messageP = NULL;
            return IOWA_COAP_400_BAD_REQUEST;
        }
    }

    return IOWA_COAP_NO_ERROR;
}
