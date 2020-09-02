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

#include "iowa_prv_coap_internals.h"
#include <stdbool.h>

/************************************************
* Constants
*/

#define PRV_DATAGRAM_MSG_HEADER_LENGTH       4

#define PRV_DATAGRAM_MSG_HEADER_VERSION      0x40
#define PRV_DATAGRAM_MSG_HEADER_VERSION_MASK 0xC0
#define PRV_DATAGRAM_MSG_HEADER_TYPE_SHIFT   4
#define PRV_DATAGRAM_MSG_HEADER_TYPE_MASK    0x30
#define PRV_DATAGRAM_MSG_HEADER_TOKEN_MASK   0x0F
#define PRV_DATAGRAM_MSG_TOKEN_OFFSET   4

#define PRV_STREAM_MSG_MIN_HEADER_LENGTH 2
#define PRV_STREAM_MSG_MAX_HEADER_LENGTH 6
#define PRV_STREAM_MSG_HEADER_LEN_MASK   0xF0
#define PRV_STREAM_MSG_HEADER_LEN_SHIFT  4
#define PRV_STREAM_MSG_HEADER_TOKEN_MASK 0x0F
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
    buffer[2] = (uint8_t)((messageP->id & 0xFF00) >> 8);
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

    (*messageP)->type = (buffer[0] & PRV_DATAGRAM_MSG_HEADER_TYPE_MASK) >> PRV_DATAGRAM_MSG_HEADER_TYPE_SHIFT;
    (*messageP)->code = buffer[1];
    (*messageP)->id = (uint16_t)((buffer[2] << 8) + buffer[3]);
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

size_t coapMessageSerializeStream(iowa_coap_message_t *messageP,
                                  uint8_t **bufferP)
{
    size_t bufferLength;
    size_t messageLength;
    size_t optionLength;
    uint8_t *buffer;
    size_t index;
    iowa_coap_option_t *optionP;
    uint16_t prevNumber;
    size_t optionIndex;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering");

    // Compute conservative serialized length
    bufferLength = (size_t)(PRV_STREAM_MSG_MAX_HEADER_LENGTH + messageP->tokenLength);
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

    // Serialize option in a temporary spot
    optionIndex = (size_t)(PRV_STREAM_MSG_MAX_HEADER_LENGTH + messageP->tokenLength);
    optionLength = option_serialize(messageP->optionList, buffer + optionIndex, iowa_coap_option_is_integer);
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Serialized options length: %u.", optionLength);

    // Compute length field (options + payload)
    messageLength = optionLength + messageP->payload.length;
    if (messageLength < optionLength)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Exit on error: integer overflow.");
        iowa_system_free(buffer);
        return 0;
    }
    if (messageP->payload.length != 0)
    {
        messageLength += 1;
    }
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Message length: %u.", messageLength);

    // Set CoAP TCP header

    if (messageLength >= PRV_STREAM_MSG_LENGTH_LIMIT_3)
    {
        buffer[0] = PRV_STREAM_MSG_LENGTH_EXTEND_3 << PRV_STREAM_MSG_HEADER_LEN_SHIFT;
        messageLength -= PRV_STREAM_MSG_LENGTH_LIMIT_3;
        buffer[1] = (messageLength >> 24) & 0xFF;
        buffer[2] = (messageLength >> 16) & 0xFF;
        buffer[3] = (messageLength >> 8) & 0xFF;
        buffer[4] = messageLength & 0xFF;
        index = 5;
    }
    else if (messageLength >= PRV_STREAM_MSG_LENGTH_LIMIT_2)
    {
        buffer[0] = PRV_STREAM_MSG_LENGTH_EXTEND_2 << PRV_STREAM_MSG_HEADER_LEN_SHIFT;
        messageLength -= PRV_STREAM_MSG_LENGTH_LIMIT_2;
        buffer[1] = (messageLength >> 8) & 0xFF;
        buffer[2] = messageLength & 0xFF;
        index = 3;
    }
    else if (messageLength >= PRV_STREAM_MSG_LENGTH_LIMIT_1)
    {
        buffer[0] = PRV_STREAM_MSG_LENGTH_EXTEND_1 << PRV_STREAM_MSG_HEADER_LEN_SHIFT;
        messageLength -= PRV_STREAM_MSG_LENGTH_LIMIT_1;
        buffer[1] = messageLength & 0xFF;
        index = 2;
    }
    else
    {
        buffer[0] = (uint8_t)(messageLength << PRV_STREAM_MSG_HEADER_LEN_SHIFT);
        index = 1;
    }

    buffer[0] |= messageP->tokenLength;

    buffer[index] = messageP->code;
    index++;

    // Add token if any
    if (messageP->tokenLength > 0 && messageP->tokenLength <= COAP_MSG_TOKEN_MAX_LEN)
    {
        memcpy(buffer + index, messageP->token, messageP->tokenLength);
        index += messageP->tokenLength;
    }
    else
    {
        messageP->tokenLength = 0;
    }

    // Move options to the right place
    if (optionLength != 0)
    {
        memmove(buffer + index, buffer + optionIndex, optionLength);
        index += optionLength;
    }

    // Add payload marker if needed
    if (messageP->payload.length != 0)
    {
        buffer[index] = PRV_MSG_PAYLOAD_MARKER;
        index++;
    }

    *bufferP = buffer;
    return index;
}

uint8_t messageStreamParseLengthField(uint8_t lenField)
{
    uint8_t headerLength;
    uint8_t length;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Length field: 0x%08X.", lenField);

    length = (lenField & PRV_STREAM_MSG_HEADER_LEN_MASK) >> PRV_STREAM_MSG_HEADER_LEN_SHIFT;

    switch (length)
    {
    case PRV_STREAM_MSG_LENGTH_EXTEND_1:
        headerLength = 2;
        break;

    case PRV_STREAM_MSG_LENGTH_EXTEND_2:
        headerLength = 3;
        break;

    case PRV_STREAM_MSG_LENGTH_EXTEND_3:
        headerLength = 4;
        break;

    default:
        // At least one remaining byte for the code
        headerLength = 1;
        break;
    }

    length = lenField & PRV_STREAM_MSG_HEADER_TOKEN_MASK;

    headerLength = (uint8_t)(headerLength + length);

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Parsed header length: %u.", headerLength);

    if (headerLength > PRV_STREAM_MSG_MAX_HEADER_AND_TOKEN_LENGTH)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Parsed header length %u is superior to the max allowed header length (%u).", headerLength, PRV_STREAM_MSG_MAX_HEADER_AND_TOKEN_LENGTH);
        headerLength = 0;
    }

    return headerLength;
}

uint8_t messageStreamParseHeader(uint8_t *buffer,
                                 size_t bufferLength,
                                 iowa_coap_message_t **messageP,
                                 size_t *lengthP)
{
    size_t bodyLength;
    size_t index;
    uint8_t tokenLen;

    *messageP = NULL;

    if (bufferLength < PRV_STREAM_MSG_MIN_HEADER_LENGTH)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Buffer length (%u) is too small, expecting at least %u.", bufferLength, PRV_STREAM_MSG_MAX_HEADER_LENGTH);
        return IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE;
    }

    bodyLength = (buffer[0] & PRV_STREAM_MSG_HEADER_LEN_MASK) >> PRV_STREAM_MSG_HEADER_LEN_SHIFT;
    tokenLen = buffer[0] & PRV_STREAM_MSG_HEADER_TOKEN_MASK;

    if (tokenLen > COAP_MSG_TOKEN_MAX_LEN)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Token length is too big: %u.", tokenLen);
        return IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
    }

    switch (bodyLength)
    {
    case PRV_STREAM_MSG_LENGTH_EXTEND_1:
        bodyLength = (size_t)(PRV_STREAM_MSG_LENGTH_LIMIT_1 + buffer[1]);
        index = 2;
        break;

    case PRV_STREAM_MSG_LENGTH_EXTEND_2:
        bodyLength = (size_t)(PRV_STREAM_MSG_LENGTH_LIMIT_2 + (buffer[1] << 8) + buffer[2]);
        index = 3;
        break;

    case PRV_STREAM_MSG_LENGTH_EXTEND_3:
        bodyLength = (size_t)(PRV_STREAM_MSG_LENGTH_LIMIT_3 + (buffer[1] << 24) + (buffer[2] << 16) + (buffer[3] << 8) + buffer[4]);
        index = 5;
        break;

    default:
        index = 1;
        break;
    }

    if (index + 1 + tokenLen > bufferLength)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Buffer is too short (%u bytes) for the declared token length (%u).", bufferLength, tokenLen);
        return IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE;
    }

    *messageP = (iowa_coap_message_t *)iowa_system_malloc(sizeof(iowa_coap_message_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*messageP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_coap_message_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*messageP, 0, sizeof(iowa_coap_message_t));

    // On stream, mark all messages as NON so higher layer don't have to check the peer type
    (*messageP)->type = IOWA_COAP_TYPE_NON_CONFIRMABLE;

    (*messageP)->code = buffer[index];
    index += 1;

    if (tokenLen > 0)
    {
        (*messageP)->tokenLength = tokenLen;
        memcpy((*messageP)->token, buffer + index, tokenLen);
    }

    *lengthP = bodyLength;

    return IOWA_COAP_NO_ERROR;
}

iowa_coap_message_t * messageDuplicate(iowa_coap_message_t *messageP,
                                       bool withMemory)
{
    iowa_coap_message_t *msgCopyP;
    iowa_coap_option_t *optionP;
    iowa_linked_buffer_t *bufferP;
    size_t bufferIndex;

    bufferP = NULL;

    msgCopyP = iowa_coap_message_new(messageP->type, messageP->code, messageP->tokenLength, messageP->token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (msgCopyP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP message.");
        return NULL;
    }
#endif

    if (messageP->type == IOWA_COAP_TYPE_ACKNOWLEDGEMENT
        || messageP->type == IOWA_COAP_TYPE_RESET)
    {
        msgCopyP->id = messageP->id;
    }
    else
    {
        msgCopyP->id = COAP_RESERVED_MID;
    }

    msgCopyP->payload = messageP->payload;

    if (withMemory == true)
    {
        size_t bufferLength;

        optionP = messageP->optionList;
        bufferLength = 0;
        while (optionP != NULL)
        {
            if (!iowa_coap_option_is_integer(optionP))
            {
                bufferLength += optionP->length;
            }
            optionP = optionP->next;
        }
        if (bufferLength != 0)
        {
            bufferP = (iowa_linked_buffer_t *)iowa_system_malloc(sizeof(iowa_linked_buffer_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (bufferP == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(sizeof(iowa_linked_buffer_t));
                iowa_coap_message_free(msgCopyP);
                return NULL;
            }
#endif
            memset(bufferP, 0, sizeof(iowa_linked_buffer_t));

            bufferP->data = (uint8_t *)iowa_system_malloc(bufferLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (bufferP->data == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(bufferLength);
                iowa_system_free(bufferP);
                iowa_coap_message_free(msgCopyP);
                return NULL;
            }
#endif
            bufferP->length = bufferLength;
        }
    }


    optionP = messageP->optionList;
    bufferIndex = 0;
    while (optionP != NULL)
    {
        iowa_coap_option_t *optCopyP;

        optCopyP = iowa_coap_option_new(optionP->number);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (optCopyP == NULL)
        {
            iowa_coap_message_free(msgCopyP);
            IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP option.");
            return NULL;
        }
#endif
        if (iowa_coap_option_is_integer(optionP))
        {
            optCopyP->value.asInteger = optionP->value.asInteger;
        }
        else
        {
            if (withMemory == true)
            {
                optCopyP->length = optionP->length;
                if (optionP->length != 0)
                {
                    memcpy(bufferP->data + bufferIndex, optionP->value.asBuffer, optionP->length);
                    optCopyP->value.asBuffer = bufferP->data + bufferIndex;
                    bufferIndex += optionP->length;
                }
            }
            else
            {
                optCopyP->value.asBuffer = optionP->value.asBuffer;
            }
        }

        iowa_coap_message_add_option(msgCopyP, optCopyP);

        optionP = optionP->next;
    }

    if (bufferP != NULL)
    {
        bufferP->next = msgCopyP->userBufferList;
        msgCopyP->userBufferList = bufferP;
    }

    return msgCopyP;
}

void coapMessageAddUserBuffer(iowa_coap_message_t *messageP,
                              iowa_buffer_t buffer)
{
    iowa_linked_buffer_t *linkBufferP;

    linkBufferP = (iowa_linked_buffer_t *)iowa_system_malloc(sizeof(iowa_linked_buffer_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (linkBufferP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_linked_buffer_t));
        return;
    }
#endif

    linkBufferP->data = buffer.data;
    linkBufferP->length = buffer.length;
    linkBufferP->next = messageP->userBufferList;
    messageP->userBufferList = linkBufferP;
}
