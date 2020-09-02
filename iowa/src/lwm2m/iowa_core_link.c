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

#include "iowa_prv_lwm2m_internals.h"

#define PRV_LINK_ITEM_START              '<'
#define PRV_LINK_ITEM_END                '>'
#define PRV_LINK_ITEM_ATTR_END           ','
#define PRV_LINK_ATTR_SEPARATOR          ';'
#define PRV_LINK_ATTR_VALUE_SEPARATOR    '='
#define PRV_LINK_ATTR_VALUE_BUFFER_QUOTE '\"'

#define PRV_LINK_ITEM_START_STR               "<"
#define PRV_LINK_ITEM_START_STR_LEN           (size_t)1
#define PRV_LINK_ITEM_END_STR                 ">"
#define PRV_LINK_ITEM_END_STR_LEN             (size_t)1
#define PRV_LINK_ITEM_ATTR_END_STR            ","
#define PRV_LINK_ITEM_ATTR_END_STR_LEN        (size_t)1
#define PRV_LINK_ATTR_SEPARATOR_STR           ";"
#define PRV_LINK_ATTR_SEPARATOR_STR_LEN       (size_t)1
#define PRV_LINK_ATTR_VALUE_SEPARATOR_STR     "="
#define PRV_LINK_ATTR_VALUE_SEPARATOR_STR_LEN (size_t)1

#define PRV_ATTR_LWM2M_VERSION       "lwm2m"
#define PRV_ATTR_LWM2M_VERSION_LEN   (size_t)5
#define PRV_ATTR_SSID                "ssid"
#define PRV_ATTR_SSID_LEN            (size_t)4
#define PRV_ATTR_URI                 "uri"
#define PRV_ATTR_URI_LEN             (size_t)3
#define PRV_ATTR_CONTENT_FORMAT      "ct"
#define PRV_ATTR_CONTENT_FORMAT_LEN  (size_t)2
#define PRV_ATTR_RESOURCE_TYPE       "rt"
#define PRV_ATTR_RESOURCE_TYPE_LEN   (size_t)2
#define PRV_ATTR_OBJECT_VERSION      "ver"
#define PRV_ATTR_OBJECT_VERSION_LEN  (size_t)3
#define PRV_ATTR_DIMENSION           "dim"
#define PRV_ATTR_DIMENSION_LEN       (size_t)3
#define PRV_ATTR_MIN_PERIOD          "pmin"
#define PRV_ATTR_MIN_PERIOD_LEN      (size_t)4
#define PRV_ATTR_MAX_PERIOD          "pmax"
#define PRV_ATTR_MAX_PERIOD_LEN      (size_t)4
#define PRV_ATTR_LESS_THAN           "lt"
#define PRV_ATTR_LESS_THAN_LEN       (size_t)2
#define PRV_ATTR_GREATER_THAN        "gt"
#define PRV_ATTR_GREATER_THAN_LEN    (size_t)2
#define PRV_ATTR_STEP                "st"
#define PRV_ATTR_STEP_LEN            (size_t)2
#define PRV_ATTR_MIN_EVAL_PERIOD     "epmin"
#define PRV_ATTR_MIN_EVAL_PERIOD_LEN (size_t)5
#define PRV_ATTR_MAX_EVAL_PERIOD     "epmax"
#define PRV_ATTR_MAX_EVAL_PERIOD_LEN (size_t)5

#define PRV_LINK_BUFFER_SIZE 1024

#define PRV_LINK_CONCAT_STR(buf, len, index, str, strLen)    \
{                                                            \
    if ((len)-(index) < (strLen))                            \
    {                                                        \
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "No enough space."); \
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;          \
    }                                                        \
    memcpy((buf)+(index), (str), (strLen));                  \
    (index) += (strLen);                                     \
}

/*************************************************************************************
** Private functions
*************************************************************************************/

static iowa_status_t prv_calculateLinkLength(link_t *linkP,
                                             size_t nbLink,
                                             size_t *lengthP)
{
    size_t index;

    *lengthP = 0;

    for (index = 0; index < nbLink; index++)
    {
        attribute_t *currAttrP;
        *lengthP += dataUtilsUriToBufferLength(&linkP[index].uri) + 2;
        for (currAttrP = linkP[index].attrP; currAttrP != NULL; currAttrP = currAttrP->nextP)
        {
            switch (currAttrP->key)
            {
            case KEY_LWM2M_VERSION:
                *lengthP += PRV_ATTR_LWM2M_VERSION_LEN + 2 + currAttrP->value.asBuffer.length;
                break;

            case KEY_SSID:
                *lengthP += PRV_ATTR_SSID_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_SERVER_URI:
                *lengthP += PRV_ATTR_URI_LEN + 4 + currAttrP->value.asBuffer.length;
                break;

            case KEY_CONTENT_FORMAT:
                *lengthP += PRV_ATTR_CONTENT_FORMAT_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_RESOURCE_TYPE:
                *lengthP += PRV_ATTR_RESOURCE_TYPE_LEN + 2 + currAttrP->value.asBuffer.length;
                break;

            case KEY_OBJECT_VERSION:
                *lengthP += PRV_ATTR_OBJECT_VERSION_LEN + 2 + currAttrP->value.asBuffer.length;
                break;

            case KEY_DIMENSION:
                *lengthP += PRV_ATTR_DIMENSION_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_PERIOD_MINIMUM:
                *lengthP += PRV_ATTR_MIN_PERIOD_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_PERIOD_MAXIMUM:
                *lengthP += PRV_ATTR_MAX_PERIOD_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_LESS_THAN:
            {
                size_t length;

                length = dataUtilsFloatToBufferLength(currAttrP->value.asFloat, false);
                if (length == 0)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Float to text length failed.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                *lengthP += PRV_ATTR_LESS_THAN_LEN + 2 + length;
                break;
            }

            case KEY_GREATER_THAN:
            {
                size_t length;

                length = dataUtilsFloatToBufferLength(currAttrP->value.asFloat, false);
                if (length == 0)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Float to text length failed.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                *lengthP += PRV_ATTR_GREATER_THAN_LEN + 2 + length;
                break;
            }

            case KEY_STEP:
            {
                size_t length;

                length = dataUtilsFloatToBufferLength(currAttrP->value.asFloat, false);
                if (length == 0)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Float to text length failed.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                *lengthP += PRV_ATTR_STEP_LEN + 2 + length;
                break;
            }

            case KEY_EVAL_PERIOD_MINIMUM:
                *lengthP += PRV_ATTR_MIN_EVAL_PERIOD_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_EVAL_PERIOD_MAXIMUM:
                *lengthP += PRV_ATTR_MAX_EVAL_PERIOD_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            default:
                IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Unknown attribute key: %d.", currAttrP->key);
                return IOWA_COAP_400_BAD_REQUEST;
            }
        }

        if (index + 1 < nbLink)
        {
            (*lengthP)++;
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with lengthP: %zu.", *lengthP);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_serializeLinkUri(iowa_lwm2m_uri_t *uriP,
                                          uint8_t *buffer,
                                          size_t bufferLength,
                                          size_t *bufferSizeP)
{
    size_t uriLength;

    *bufferSizeP = 0;

    if (bufferLength < PRV_LINK_ITEM_START_STR_LEN)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "No enough space.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    memcpy(buffer + *bufferSizeP, PRV_LINK_ITEM_START_STR, PRV_LINK_ITEM_START_STR_LEN);
    *bufferSizeP = PRV_LINK_ITEM_START_STR_LEN;
    uriLength = dataUtilsUriToBuffer(uriP, buffer + *bufferSizeP, bufferLength - *bufferSizeP);
    if (uriLength == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "URI to text conversion failed.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    *bufferSizeP += uriLength;

    if (bufferLength - *bufferSizeP < PRV_LINK_ITEM_END_STR_LEN)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "No enough space.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    memcpy(buffer + *bufferSizeP, PRV_LINK_ITEM_END_STR, PRV_LINK_ITEM_END_STR_LEN);
    *bufferSizeP += PRV_LINK_ITEM_END_STR_LEN;

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_serializeLinkBufferAttributes(attribute_t *attrP,
                                                       const char *attribute,
                                                       size_t attributeLength,
                                                       uint8_t *buffer,
                                                       size_t bufferLength,
                                                       size_t *bufferSizeP)
{
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, PRV_LINK_ATTR_SEPARATOR_STR, PRV_LINK_ATTR_SEPARATOR_STR_LEN);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, attribute, attributeLength);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, PRV_LINK_ATTR_VALUE_SEPARATOR_STR, PRV_LINK_ATTR_VALUE_SEPARATOR_STR_LEN);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, attrP->value.asBuffer.buffer, attrP->value.asBuffer.length);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_serializeLinkIntAttributes(attribute_t *attrP,
                                                    const char *attribute,
                                                    size_t attributeLength,
                                                    uint8_t *buffer,
                                                    size_t bufferLength,
                                                    size_t *bufferSizeP)
{
    size_t length;

    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, PRV_LINK_ATTR_SEPARATOR_STR, PRV_LINK_ATTR_SEPARATOR_STR_LEN);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, attribute, attributeLength);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, PRV_LINK_ATTR_VALUE_SEPARATOR_STR, PRV_LINK_ATTR_VALUE_SEPARATOR_STR_LEN);

    length = dataUtilsIntToBuffer(attrP->value.asInteger, buffer + *bufferSizeP, bufferLength - *bufferSizeP, false);
    if (length == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Integer to text conversion failed.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    *bufferSizeP += length;

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_serializeLinkFloatAttributes(attribute_t *attrP,
                                                      const char *attribute,
                                                      size_t attributeLength,
                                                      uint8_t *buffer,
                                                      size_t bufferLength,
                                                      size_t *bufferSizeP)
{
    size_t length;

    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, PRV_LINK_ATTR_SEPARATOR_STR, PRV_LINK_ATTR_SEPARATOR_STR_LEN);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, attribute, attributeLength);
    PRV_LINK_CONCAT_STR(buffer, bufferLength, *bufferSizeP, PRV_LINK_ATTR_VALUE_SEPARATOR_STR, PRV_LINK_ATTR_VALUE_SEPARATOR_STR_LEN);

    length = dataUtilsFloatToBuffer(attrP->value.asFloat, buffer + *bufferSizeP, bufferLength - *bufferSizeP, false);
    if (length == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Float to text conversion failed.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    *bufferSizeP += length;

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_serializeLinkAttributes(attribute_t *attrP,
                                                 uint8_t *buffer,
                                                 size_t bufferLength,
                                                 size_t *bufferSizeP)
{
    iowa_status_t result;
    attribute_t *currAttrP;

    *bufferSizeP = 0;
    result = IOWA_COAP_NO_ERROR;
    for (currAttrP = attrP; currAttrP != NULL && result == IOWA_COAP_NO_ERROR; currAttrP = currAttrP->nextP)
    {
        switch (currAttrP->key)
        {
        case KEY_LWM2M_VERSION:
            result = prv_serializeLinkBufferAttributes(currAttrP, PRV_ATTR_LWM2M_VERSION, PRV_ATTR_LWM2M_VERSION_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_SSID:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_SSID, PRV_ATTR_SSID_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_SERVER_URI:
            result = prv_serializeLinkBufferAttributes(currAttrP, PRV_ATTR_URI, PRV_ATTR_URI_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_CONTENT_FORMAT:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_CONTENT_FORMAT, PRV_ATTR_CONTENT_FORMAT_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_RESOURCE_TYPE:
            result = prv_serializeLinkBufferAttributes(currAttrP, PRV_ATTR_RESOURCE_TYPE, PRV_ATTR_RESOURCE_TYPE_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_OBJECT_VERSION:
            result = prv_serializeLinkBufferAttributes(currAttrP, PRV_ATTR_OBJECT_VERSION, PRV_ATTR_OBJECT_VERSION_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_DIMENSION:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_DIMENSION, PRV_ATTR_DIMENSION_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_PERIOD_MINIMUM:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_MIN_PERIOD, PRV_ATTR_MIN_PERIOD_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_PERIOD_MAXIMUM:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_MAX_PERIOD, PRV_ATTR_MAX_PERIOD_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_LESS_THAN:
            result = prv_serializeLinkFloatAttributes(currAttrP, PRV_ATTR_LESS_THAN, PRV_ATTR_LESS_THAN_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_GREATER_THAN:
            result = prv_serializeLinkFloatAttributes(currAttrP, PRV_ATTR_GREATER_THAN, PRV_ATTR_GREATER_THAN_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_STEP:
            result = prv_serializeLinkFloatAttributes(currAttrP, PRV_ATTR_STEP, PRV_ATTR_STEP_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_EVAL_PERIOD_MINIMUM:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_MIN_EVAL_PERIOD, PRV_ATTR_MIN_EVAL_PERIOD_LEN, buffer, bufferLength, bufferSizeP);
            break;

        case KEY_EVAL_PERIOD_MAXIMUM:
            result = prv_serializeLinkIntAttributes(currAttrP, PRV_ATTR_MAX_EVAL_PERIOD, PRV_ATTR_MAX_EVAL_PERIOD_LEN, buffer, bufferLength, bufferSizeP);
            break;

        default:
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Unknown attribute key: %d.", currAttrP->key);
            return IOWA_COAP_400_BAD_REQUEST;
        }
    }

    return IOWA_COAP_NO_ERROR;
}

static size_t prv_countLink(const uint8_t *buffer,
                            size_t bufferLength)
{
    size_t index;
    size_t nbLink;

    nbLink = 0;
    for (index = 0; index < bufferLength; index++)
    {
        if (buffer[index] == PRV_LINK_ITEM_ATTR_END)
        {
            nbLink++;
        }
    }

    nbLink++;

    return nbLink;
}

static iowa_status_t prv_splitLinkAttribute(const uint8_t *buffer,
                                            size_t bufferLength,
                                            size_t *linkAttributeLengthP,
                                            size_t *keyStartP,
                                            size_t *keyLengthP,
                                            size_t *valueStartP,
                                            size_t *valueLengthP)
{
    *linkAttributeLengthP = 0;

    if (buffer[*linkAttributeLengthP] == PRV_LINK_ITEM_ATTR_END)
    {
        return IOWA_COAP_NO_ERROR;
    }

    if (buffer[*linkAttributeLengthP] == PRV_LINK_ATTR_SEPARATOR)
    {
        (*linkAttributeLengthP)++;
    }
    if (*linkAttributeLengthP == bufferLength)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    *keyStartP = *linkAttributeLengthP;

    while (*linkAttributeLengthP < bufferLength
           && buffer[*linkAttributeLengthP] != PRV_LINK_ATTR_VALUE_SEPARATOR)
    {
        (*linkAttributeLengthP)++;
    }
    if (*linkAttributeLengthP == *keyStartP
        || *linkAttributeLengthP == bufferLength)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    *keyLengthP = *linkAttributeLengthP - *keyStartP;

    (*linkAttributeLengthP)++;
    *valueStartP = *linkAttributeLengthP;

    while (*linkAttributeLengthP < bufferLength
           && buffer[*linkAttributeLengthP] != PRV_LINK_ATTR_SEPARATOR
           && buffer[*linkAttributeLengthP] != PRV_LINK_ITEM_ATTR_END)
    {
        (*linkAttributeLengthP)++;
    }

    *valueLengthP = *linkAttributeLengthP - *valueStartP;

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_deserializeLinkUri(const uint8_t *buffer,
                                            size_t bufferLength,
                                            size_t *bufferSizeP,
                                            iowa_lwm2m_uri_t *uriP)
{
    *bufferSizeP = 0;

    if (buffer[0] != PRV_LINK_ITEM_START)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Starting delimiter '%s' not found.", PRV_LINK_ITEM_START_STR);

        LWM2M_URI_RESET(uriP);
        return IOWA_COAP_NO_ERROR;
    }

    for (*bufferSizeP = 0; *bufferSizeP < bufferLength; (*bufferSizeP)++)
    {
        if (buffer[*bufferSizeP] == PRV_LINK_ITEM_END)
        {
            break;
        }
    }
    if (*bufferSizeP == bufferLength)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Ending delimiter '%s' not found.", PRV_LINK_ITEM_END_STR);
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (dataUtilsBufferToUri((const char *)buffer + 1, *bufferSizeP - 1, uriP) == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert buffer to URI.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
    (*bufferSizeP)++;

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_deserializeLinkBufferAttributes(const uint8_t *buffer,
                                                         size_t bufferLength,
                                                         attribute_key_t key,
                                                         attribute_t **attrP)
{
    attribute_t *attrNodeP;

    attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    attrNodeP->value.asBuffer.buffer = (uint8_t *)iowa_system_malloc(bufferLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP->value.asBuffer.buffer == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(bufferLength);
        iowa_system_free(attrNodeP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memcpy(attrNodeP->value.asBuffer.buffer, buffer, bufferLength);
    attrNodeP->value.asBuffer.length = bufferLength;

    attrNodeP->nextP = NULL;
    attrNodeP->key = key;

    *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_deserializeLinkIntAttributes(uint8_t *buffer,
                                                      size_t bufferLength,
                                                      attribute_key_t key,
                                                      attribute_t **attrP)
{
    attribute_t *attrNodeP;

    attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    if (dataUtilsBufferToInt(buffer, bufferLength, &attrNodeP->value.asInteger) == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert back the integer value.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    attrNodeP->nextP = NULL;
    attrNodeP->key = key;

    *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_deserializeLinkFloatAttributes(uint8_t *buffer,
                                                        size_t bufferLength,
                                                        attribute_key_t key,
                                                        attribute_t **attrP)
{
    attribute_t *attrNodeP;

    attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    if (dataUtilsBufferToFloat(buffer, bufferLength, &attrNodeP->value.asFloat) == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert back the float value.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    attrNodeP->nextP = NULL;
    attrNodeP->key = key;

    *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_deserializeLinkAttributes(uint8_t *buffer,
                                                   size_t bufferLength,
                                                   size_t *bufferSizeP,
                                                   attribute_t **attrP)
{
    iowa_status_t result;

    result = IOWA_COAP_NO_ERROR;
    *bufferSizeP = 0;

    while (*bufferSizeP < bufferLength
           && result == IOWA_COAP_NO_ERROR)
    {
        size_t linkAttributeLength;
        size_t keyStart;
        size_t keyLength;
        size_t valueStart;
        size_t valueLength;

        result = prv_splitLinkAttribute(buffer + *bufferSizeP, bufferLength - *bufferSizeP, &linkAttributeLength, &keyStart, &keyLength, &valueStart, &valueLength);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to split the link attributes.");
            return result;
        }
        if (linkAttributeLength == 0)
        {
            (*bufferSizeP)++;
            break;
        }

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Parse attribute '%.*s' with value: %.*s.", keyLength, buffer + *bufferSizeP + keyStart, valueLength, buffer + *bufferSizeP + valueStart);

        if (keyLength == PRV_ATTR_LWM2M_VERSION_LEN
            && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_LWM2M_VERSION, PRV_ATTR_LWM2M_VERSION_LEN) == 0)
        {
            result = prv_deserializeLinkBufferAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_LWM2M_VERSION, attrP);
        }
        else if (keyLength == PRV_ATTR_SSID_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_SSID, PRV_ATTR_SSID_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_SSID, attrP);
        }
        else if (keyLength == PRV_ATTR_URI_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_URI, PRV_ATTR_URI_LEN) == 0)
        {
            result = prv_deserializeLinkBufferAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_SERVER_URI, attrP);
        }
        else if (keyLength == PRV_ATTR_CONTENT_FORMAT_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_CONTENT_FORMAT, PRV_ATTR_CONTENT_FORMAT_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_CONTENT_FORMAT, attrP);
        }
        else if (keyLength == PRV_ATTR_RESOURCE_TYPE_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_RESOURCE_TYPE, PRV_ATTR_RESOURCE_TYPE_LEN) == 0)
        {
            result = prv_deserializeLinkBufferAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_RESOURCE_TYPE, attrP);
        }
        else if (keyLength == PRV_ATTR_OBJECT_VERSION_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_OBJECT_VERSION, PRV_ATTR_OBJECT_VERSION_LEN) == 0)
        {
            result = prv_deserializeLinkBufferAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_OBJECT_VERSION, attrP);
        }
        else if (keyLength == PRV_ATTR_DIMENSION_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_DIMENSION, PRV_ATTR_DIMENSION_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_DIMENSION, attrP);
        }
        else if (keyLength == PRV_ATTR_MIN_PERIOD_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_MIN_PERIOD, PRV_ATTR_MIN_PERIOD_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_PERIOD_MINIMUM, attrP);
        }
        else if (keyLength == PRV_ATTR_MAX_PERIOD_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_MAX_PERIOD, PRV_ATTR_MAX_PERIOD_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_PERIOD_MAXIMUM, attrP);
        }
        else if (keyLength == PRV_ATTR_LESS_THAN_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_LESS_THAN, PRV_ATTR_LESS_THAN_LEN) == 0)
        {
            result = prv_deserializeLinkFloatAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_LESS_THAN, attrP);
        }
        else if (keyLength == PRV_ATTR_GREATER_THAN_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_GREATER_THAN, PRV_ATTR_GREATER_THAN_LEN) == 0)
        {
            result = prv_deserializeLinkFloatAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_GREATER_THAN, attrP);
        }
        else if (keyLength == PRV_ATTR_STEP_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_STEP, PRV_ATTR_STEP_LEN) == 0)
        {
            result = prv_deserializeLinkFloatAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_STEP, attrP);
        }
        else if (keyLength == PRV_ATTR_MIN_EVAL_PERIOD_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_MIN_EVAL_PERIOD, PRV_ATTR_MIN_EVAL_PERIOD_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_EVAL_PERIOD_MINIMUM, attrP);
        }
        else if (keyLength == PRV_ATTR_MAX_EVAL_PERIOD_LEN
                 && memcmp(buffer + *bufferSizeP + keyStart, PRV_ATTR_MAX_EVAL_PERIOD, PRV_ATTR_MAX_EVAL_PERIOD_LEN) == 0)
        {
            result = prv_deserializeLinkIntAttributes(buffer + *bufferSizeP + valueStart, valueLength, KEY_EVAL_PERIOD_MAXIMUM, attrP);
        }
        else
        {
            result = IOWA_COAP_402_BAD_OPTION;
        }

        *bufferSizeP += linkAttributeLength;
    }

    return result;
}

void prv_freeAttribute(void *nodeP)
{
    attribute_t *attrP;

    attrP = (attribute_t *)nodeP;

    switch (attrP->key)
    {
    case KEY_LWM2M_VERSION:
    case KEY_RESOURCE_TYPE:
    case KEY_OBJECT_VERSION:
    case KEY_SERVER_URI:
        iowa_system_free(attrP->value.asBuffer.buffer);
        break;

    default:
        break;
    }

    iowa_system_free(attrP);
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t coreLinkSerialize(link_t *linkP,
                                size_t nbLink,
                                uint8_t **bufferP,
                                size_t *bufferLengthP)
{
    iowa_status_t result;
    size_t index;
    size_t linkLength;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    *bufferP = NULL;
    *bufferLengthP = 0;

    if (linkP == NULL
        || nbLink == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Nothing to serialize.");
        return IOWA_COAP_NO_ERROR;
    }
    result = prv_calculateLinkLength(linkP, nbLink, &linkLength);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to calculate the length.");
        return result;
    }

    *bufferP = (uint8_t *)iowa_system_malloc(linkLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*bufferP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(linkLength);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    for (index = 0; index < nbLink; index++)
    {
        size_t bufferSize;
        result = prv_serializeLinkUri(&linkP[index].uri, *bufferP + *bufferLengthP, linkLength - *bufferLengthP, &bufferSize);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "URI serialization failed.");
            iowa_system_free(*bufferP);
            *bufferP = NULL;
            *bufferLengthP = 0;

            return result;
        }
        *bufferLengthP += bufferSize;

        result = prv_serializeLinkAttributes(linkP[index].attrP, *bufferP + *bufferLengthP, linkLength - *bufferLengthP, &bufferSize);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Attributes serialization failed.");
            iowa_system_free(*bufferP);
            *bufferP = NULL;
            *bufferLengthP = 0;

            return result;
        }
        *bufferLengthP += bufferSize;

        if (index + 1 < nbLink)
        {
            PRV_LINK_CONCAT_STR(*bufferP, PRV_LINK_BUFFER_SIZE, *bufferLengthP, PRV_LINK_ITEM_ATTR_END_STR, PRV_LINK_ITEM_ATTR_END_STR_LEN);
        }
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t coreLinkDeserialize(uint8_t *buffer,
                                  size_t bufferLength,
                                  link_t **linkP,
                                  size_t *nbLinkP)
{
    size_t index;
    size_t bufferIndex;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    *linkP = NULL;
    *nbLinkP = 0;

    if (buffer == NULL
        || bufferLength == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Nothing to deserialize.");
        return IOWA_COAP_NO_ERROR;
    }

    *nbLinkP = prv_countLink(buffer, bufferLength);
    *linkP = (link_t *)iowa_system_malloc(*nbLinkP * sizeof(link_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*linkP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*nbLinkP * sizeof(link_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*linkP, 0, *nbLinkP * sizeof(link_t));

    bufferIndex = 0;
    for (index = 0; index < *nbLinkP; index++)
    {
        iowa_status_t result;
        size_t bufferSize;
        result = prv_deserializeLinkUri(buffer + bufferIndex, bufferLength - bufferIndex, &bufferSize, &((*linkP)[index].uri));
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "URI deserialization failed.");
            iowa_system_free(*linkP);
            return result;
        }
        bufferIndex += bufferSize;

        result = prv_deserializeLinkAttributes(buffer + bufferIndex, bufferLength - bufferIndex, &bufferSize, &(*linkP)[index].attrP);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Attributes deserialization failed.");
            iowa_system_free(*linkP);
            return result;
        }
        bufferIndex += bufferSize;
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t coreLinkAddBufferAttribute(link_t *linkP,
                                         attribute_key_t key,
                                         const uint8_t *buffer,
                                         size_t bufferLength,
                                         bool addQuotes)
{
    attribute_t *attrNodeP;
    size_t realBufferLength;

    attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    realBufferLength = bufferLength;
    if (addQuotes == true)
    {
        realBufferLength += 2;
    }

    attrNodeP->value.asBuffer.buffer = (uint8_t *)iowa_system_malloc(realBufferLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP->value.asBuffer.buffer == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(bufferLength);
        iowa_system_free(attrNodeP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    attrNodeP->value.asBuffer.length = realBufferLength;

    if (addQuotes == true)
    {
        attrNodeP->value.asBuffer.buffer[0] = PRV_LINK_ATTR_VALUE_BUFFER_QUOTE;
        memcpy(attrNodeP->value.asBuffer.buffer + 1, buffer, bufferLength);
        attrNodeP->value.asBuffer.buffer[bufferLength + 1] = PRV_LINK_ATTR_VALUE_BUFFER_QUOTE;
    }
    else
    {
        memcpy(attrNodeP->value.asBuffer.buffer, buffer, bufferLength);
    }

    attrNodeP->nextP = NULL;
    attrNodeP->key = key;

    linkP->attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(linkP->attrP, attrNodeP);

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t coreLinkAddIntegerAttribute(link_t *linkP,
                                          attribute_key_t key,
                                          int64_t value)
{
    attribute_t *attrNodeP;

    attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (attrNodeP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    attrNodeP->value.asInteger = value;
    attrNodeP->nextP = NULL;
    attrNodeP->key = key;

    linkP->attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(linkP->attrP, attrNodeP);

    return IOWA_COAP_NO_ERROR;
}

attribute_t * coreLinkFind(link_t *linkP,
                           attribute_key_t key)
{
    attribute_t *attrP;

    for (attrP = linkP->attrP; attrP != NULL; attrP = attrP->nextP)
    {
        if (attrP->key == key)
        {
            return attrP;
        }
    }

    return NULL;
}

void coreLinkFree(link_t *linkP,
                  size_t nbLink)
{
    size_t index;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    for (index = 0; index < nbLink; index++)
    {
        IOWA_UTILS_LIST_FREE(linkP[index].attrP, prv_freeAttribute);
    }

    iowa_system_free(linkP);
}
