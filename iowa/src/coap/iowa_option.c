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

bool iowa_coap_option_is_integer(const iowa_coap_option_t *optionP)
{
    switch (optionP->number)
    {
    case IOWA_COAP_OPTION_OBSERVE:
    case IOWA_COAP_OPTION_URI_PORT:
    case IOWA_COAP_OPTION_CONTENT_FORMAT:
    case IOWA_COAP_OPTION_MAX_AGE:
    case IOWA_COAP_OPTION_ACCEPT:
    case IOWA_COAP_OPTION_BLOCK_2:
    case IOWA_COAP_OPTION_BLOCK_1:
    case IOWA_COAP_OPTION_SIZE_2:
    case IOWA_COAP_OPTION_SIZE_1:
    case IOWA_COAP_OPTION_NO_RESPONSE:
        return true;
    default:
        return false;
    }
}

size_t option_getSerializedLength(iowa_coap_option_t *optionP,
                                  coap_option_callback_t isIntegerCallback)
{
    size_t length;

    if (optionP == NULL)
    {
        return 0;
    }

    length = PRV_OPT_HEADER_CONSERVATIVE_LENGTH;

    if (isIntegerCallback(optionP))
    {
        uint32_t limit;

        limit = 10;

        while (optionP->value.asInteger > limit
               && limit < PRV_INTEGER_LIMIT)
        {
            length += 1;
            limit *= 10;
        }
    }
    else
    {
        length += optionP->length;
    }

    if (length >= PRV_OPT_HEADER_CONSERVATIVE_LENGTH + PRV_OPT_LIMIT_2)
    {
        length += 2;
    }
    else if (length >= PRV_OPT_HEADER_CONSERVATIVE_LENGTH + PRV_OPT_LIMIT_1)
    {
        length += 1;
    }

    return length;
}

// This function does not test that buffer is large enough to contain the serialized options.
// This is intentional as the caller should have used option_getSerializedLength().
size_t option_serialize(iowa_coap_option_t *optionList,
                        uint8_t *buffer,
                        coap_option_callback_t isIntegerCallback)
{
    size_t index;
    iowa_coap_option_t *optionP;
    uint16_t prevNumber;

    index = 0;
    prevNumber = 0;

    for (optionP = optionList; optionP != NULL; optionP = optionP->next)
    {
        uint16_t delta;
        uint16_t valueLen;
        uint8_t hdrLen;
        uint8_t lengthOffset;
        uint8_t *optBuffer;

        optBuffer = buffer + index;

        delta = (uint16_t)(optionP->number - prevNumber);
        if (delta >= PRV_OPT_LIMIT_2)
        {
            optBuffer[0] = PRV_OPT_EXTEND_2 << PRV_OPT_DELTA_SHIFT;
            delta = (uint16_t)(delta - PRV_OPT_LIMIT_2);
            optBuffer[1] = (uint8_t)((delta & 0xFF00) >> 8);
            optBuffer[2] = (uint8_t)(delta & 0xFF);
            lengthOffset = 3;
            hdrLen = 3;
        }
        else if (delta >= PRV_OPT_LIMIT_1)
        {
            optBuffer[0] = PRV_OPT_EXTEND_1 << PRV_OPT_DELTA_SHIFT;
            delta = (uint16_t)(delta - PRV_OPT_LIMIT_1);
            optBuffer[1] = (uint8_t)(delta & 0xFF);
            lengthOffset = 2;
            hdrLen = 2;
        }
        else
        {
            uint8_t littleDelta;

            littleDelta = (uint8_t)delta;
            optBuffer[0] = (uint8_t)(littleDelta << PRV_OPT_DELTA_SHIFT);
            lengthOffset = 1;
            hdrLen = 1;
        }

        if (isIntegerCallback(optionP))
        {
            // 32-bit integer text value can not be more than 10
            valueLen = 0;
            if ((optionP->value.asInteger & 0xFF000000) != 0)
            {
                optBuffer[hdrLen + valueLen] = (uint8_t)((optionP->value.asInteger >> 24) & 0xFF);
                valueLen = (uint16_t)(valueLen + 1);
            }
            if ((optionP->value.asInteger & 0xFFFF0000) != 0)
            {
                optBuffer[hdrLen + valueLen] = (uint8_t)((optionP->value.asInteger >> 16) & 0xFF);
                valueLen = (uint16_t)(valueLen + 1);
            }
            if ((optionP->value.asInteger & 0xFFFFFF00) != 0)
            {
                optBuffer[hdrLen + valueLen] = (uint8_t)((optionP->value.asInteger >> 8) & 0xFF);
                valueLen = (uint16_t)(valueLen + 1);
            }
            if (optionP->value.asInteger != 0)
            {
                optBuffer[hdrLen + valueLen] = (uint8_t)(optionP->value.asInteger & 0xFF);
                valueLen = (uint16_t)(valueLen + 1);
            }
            optBuffer[0] |= (uint8_t)valueLen;
        }
        else
        {
            valueLen = optionP->length;

            if (valueLen >= PRV_OPT_LIMIT_2)
            {
                optBuffer[0] |= PRV_OPT_EXTEND_2;
                valueLen = (uint16_t)(valueLen - PRV_OPT_LIMIT_2);
                optBuffer[lengthOffset] = (uint8_t)((valueLen & 0xFF00) >> 8);
                optBuffer[lengthOffset + 1] = (uint8_t)(valueLen & 0xFF);
                hdrLen = (uint8_t)(hdrLen + 2);
            }
            else if (valueLen >= PRV_OPT_LIMIT_1)
            {
                optBuffer[0] |= PRV_OPT_EXTEND_1;
                valueLen = (uint16_t)(valueLen -PRV_OPT_LIMIT_1);
                optBuffer[lengthOffset] = (uint8_t)(valueLen & 0xFF);
                hdrLen = (uint8_t)(hdrLen + 1);
            }
            else
            {
                optBuffer[0] |= (uint8_t)valueLen;
            }

            memcpy(optBuffer + hdrLen, optionP->value.asBuffer, optionP->length);
            valueLen = optionP->length;
        }

        index = (size_t)(index + hdrLen + valueLen);
        prevNumber = optionP->number;
    }

    return index;
}

uint8_t option_parse(uint8_t *buffer,
                     size_t bufferLength,
                     iowa_coap_option_t **optionListP,
                     size_t *lengthP,
                     coap_option_callback_t isIntegerCallback)
{
    uint8_t result;
    size_t index;
    iowa_coap_option_t *currOptionP;

    index = 0;
    currOptionP = NULL;

    while (index < bufferLength
           && buffer[index] != PRV_MSG_PAYLOAD_MARKER)
    {
        uint16_t delta;
        uint16_t length;

        delta = (buffer[index] & PRV_OPT_DELTA_MASK) >> PRV_OPT_DELTA_SHIFT;
        length = buffer[index] & PRV_OPT_LENGTH_MASK;
        index += PRV_OPT_HEADER_LENGTH;
        if (index > bufferLength)
        {
            IOWA_LOG_WARNING(IOWA_PART_COAP, "Options are truncated in the received buffer.");
            result = IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
            goto exit_on_error;
        }

        switch (delta)
        {
        case PRV_OPT_EXTEND_FORBIDDEN:
            IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Delta is 0xF for the option at %u.", index);
            result = IOWA_COAP_400_BAD_REQUEST;
            goto exit_on_error;

        case PRV_OPT_EXTEND_1:
            delta = (uint16_t)(PRV_OPT_LIMIT_1 + buffer[index]);
            index++;
            break;

        case PRV_OPT_EXTEND_2:
            delta = (uint16_t)(PRV_OPT_LIMIT_2 + (buffer[index] << 8) + buffer[index + 1]);
            index += 2;
            break;

        default:
            break;
        }

        if (index > bufferLength)
        {
            IOWA_LOG_WARNING(IOWA_PART_COAP, "Options are truncated in the received buffer.");
            result = IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
            goto exit_on_error;
        }

        switch (length)
        {
        case PRV_OPT_EXTEND_FORBIDDEN:
            IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Length is 0xF for the option at %u.", index);
            result = IOWA_COAP_400_BAD_REQUEST;
            goto exit_on_error;

        case PRV_OPT_EXTEND_1:
            length = (uint16_t)(PRV_OPT_LIMIT_1 + buffer[index]);
            index++;
            break;

        case PRV_OPT_EXTEND_2:
            length = (uint16_t)(PRV_OPT_LIMIT_2 + (buffer[index] << 8) + buffer[index + 1]);
            index += 2;
            break;

        default:
            break;
        }

        if (index > bufferLength
            || index + length > bufferLength)
        {
            IOWA_LOG_WARNING(IOWA_PART_COAP, "Options are truncated in the received buffer.");
            result = IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
            goto exit_on_error;
        }

        if (currOptionP != NULL)
        {
            currOptionP->next = iowa_coap_option_new((uint16_t)(currOptionP->number + delta));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (currOptionP->next == NULL)
            {
                IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP option.");
                result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                goto exit_on_error;
            }
#endif
            currOptionP = currOptionP->next;
        }
        else
        {
            *optionListP = iowa_coap_option_new(delta);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*optionListP == NULL)
            {
                IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP option.");
                result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                goto exit_on_error;
            }
#endif
            currOptionP = *optionListP;
        }

        if (length > 0)
        {
            if (isIntegerCallback(currOptionP))
            {
                if (length > 4)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Implementation limit reached for the integer option at %u.", bufferLength, index);
                    result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    goto exit_on_error;
                }
                while (length > 0)
                {
                    currOptionP->value.asInteger = (currOptionP->value.asInteger << 8) + buffer[index];
                    index++;
                    length--;
                }
            }
            else
            {
                currOptionP->length = length;
                currOptionP->value.asBuffer = buffer + index;
            }
        }
        index += length;
    }

    *lengthP = index;

    return IOWA_COAP_NO_ERROR;

exit_on_error:
    iowa_coap_option_free(*optionListP);
    *optionListP = NULL;

    return result;
}

iowa_coap_option_t * iowa_coap_option_new(uint16_t number)
{
    iowa_coap_option_t *optionP;

    optionP = (iowa_coap_option_t *)iowa_system_malloc(sizeof(iowa_coap_option_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (optionP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_coap_option_t));
        return NULL;
    }
#endif
    memset(optionP, 0, sizeof(iowa_coap_option_t));
    optionP->number = number;

    return optionP;
}

void iowa_coap_option_free(iowa_coap_option_t *optionP)
{
    IOWA_UTILS_LIST_FREE(optionP, iowa_system_free);
}

iowa_coap_option_t * iowa_coap_path_to_option(uint16_t number,
                                              const char *path,
                                              char delimiter)
{
    iowa_coap_option_t *optionP;
    iowa_coap_option_t *currentP;
    size_t i;

    optionP = iowa_coap_option_new(number);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (optionP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP option.");
        return NULL;
    }
#endif

    currentP = optionP;
    i = 0;

    while (path[i] != 0)
    {
        size_t end;

        if (path[i] == delimiter)
        {
            i++;
        }
        end = i;
        while (path[end] != delimiter
               && path[end] != 0)
        {
            end++;
        }
        if ((end - i) > 0xFFFF)
        {
            iowa_coap_option_free(optionP);
            return NULL;
        }
        currentP->length = (uint16_t)(end - i);
        currentP->value.asBuffer = (uint8_t *)path + i;

        i = end;
        if (path[i] != 0)
        {
            currentP->next = iowa_coap_option_new(number);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (currentP->next == NULL)
            {
                iowa_coap_option_free(optionP);
                IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP option.");
                return NULL;
            }
#endif
            currentP = currentP->next;
        }
    }

    return optionP;
}

bool iowa_coap_option_compare_to_path(const iowa_coap_option_t *optionP,
                                      const char *path,
                                      char delimiter)
{
    size_t i;
    uint16_t optionNumber;

    optionNumber = optionP->number;
    i = 0;
    while (path[i] != '\0'
           && optionP != NULL
           && optionP->number == optionNumber)
    {
        size_t j;

        switch (i)
        {
        case 0:
            if (path[i] == delimiter)
            {
                // Skip the first delimiter to allow subpath compare
                i++;
            }
            break;

        default:
            if (path[i] != delimiter)
            {
                return false;
            }
            i++;
        }

        // Check the current segment
        for (j=0; j<optionP->length; j++)
        {
            if (path[i] == '\0'
                || optionP->value.asBuffer[j] == delimiter
                || optionP->value.asBuffer[j] != path[i])
            {
                return false;
            }

            i++;
        }

        // Go to next option path
        optionP = optionP->next;
    }

    if (path[i] != '\0'
        || (optionP != NULL
            && optionP->next != NULL
            && optionP->next->number == optionNumber))
    {
        return false;
    }

    return true;
}
