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
* Copyright (c) 2019-2020 IoTerop.
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

#ifdef LWM2M_ALTPATH_SUPPORT
static iowa_status_t prv_calculateLinkLength(link_t *linkP,
                                             size_t nbLink,
                                             const char *altPath,
                                             size_t *lengthP)
#else
static iowa_status_t prv_calculateLinkLength(link_t *linkP,
                                             size_t nbLink,
                                             size_t *lengthP)
#endif
{
    size_t index;

    *lengthP = 0;

    for (index = 0; index < nbLink; index++)
    {
        attribute_t *currAttrP;
#ifdef LWM2M_ALTPATH_SUPPORT
        *lengthP += dataUtilsUriToBufferLength(&linkP[index].uri, altPath) + 2; // '<' + 'URI' + '>'
#else
        *lengthP += dataUtilsUriToBufferLength(&linkP[index].uri) + 2; // '<' + 'URI' + '>'
#endif
        for (currAttrP = linkP[index].attrP; currAttrP != NULL; currAttrP = currAttrP->nextP)
        {
            // *lengthP += ';' + 'KEY' + '=' + 'VALUE'
            switch (currAttrP->key)
            {
            case KEY_LWM2M_VERSION:
                *lengthP += PRV_ATTR_LWM2M_VERSION_LEN + 2 + currAttrP->value.asBuffer.length;
                break;

            case KEY_SSID:
                *lengthP += PRV_ATTR_SSID_LEN + 2 + dataUtilsIntToBufferLength(currAttrP->value.asInteger, false);
                break;

            case KEY_SERVER_URI:
                *lengthP += PRV_ATTR_URI_LEN + 4 + currAttrP->value.asBuffer.length; // ... + '"' + '"'
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
            (*lengthP)++; // ','
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with lengthP: %zu.", *lengthP);

    return IOWA_COAP_NO_ERROR;
}

#ifdef LWM2M_ALTPATH_SUPPORT
static iowa_status_t prv_serializeLinkUri(iowa_lwm2m_uri_t *uriP,
                                          const char *altPath,
                                          uint8_t *buffer,
                                          size_t bufferLength,
                                          size_t *bufferSizeP)
#else
static iowa_status_t prv_serializeLinkUri(iowa_lwm2m_uri_t *uriP,
                                          uint8_t *buffer,
                                          size_t bufferLength,
                                          size_t *bufferSizeP)
#endif
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
#ifdef LWM2M_ALTPATH_SUPPORT
    uriLength = dataUtilsUriToBuffer(uriP, altPath, buffer + *bufferSizeP, bufferLength - *bufferSizeP);
#else
    uriLength = dataUtilsUriToBuffer(uriP, buffer + *bufferSizeP, bufferLength - *bufferSizeP);
#endif
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

#ifdef LWM2M_ALTPATH_SUPPORT
iowa_status_t coreLinkSerialize(link_t *linkP,
                                size_t nbLink,
                                const char *altPath,
                                uint8_t **bufferP,
                                size_t *bufferLengthP)
#else
iowa_status_t coreLinkSerialize(link_t *linkP,
                                size_t nbLink,
                                uint8_t **bufferP,
                                size_t *bufferLengthP)
#endif
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
#ifdef LWM2M_ALTPATH_SUPPORT
    result = prv_calculateLinkLength(linkP, nbLink, altPath, &linkLength);
#else
    result = prv_calculateLinkLength(linkP, nbLink, &linkLength);
#endif
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
#ifdef LWM2M_ALTPATH_SUPPORT
        result = prv_serializeLinkUri(&linkP[index].uri, altPath, *bufferP + *bufferLengthP, linkLength - *bufferLengthP, &bufferSize);
#else
        result = prv_serializeLinkUri(&linkP[index].uri, *bufferP + *bufferLengthP, linkLength - *bufferLengthP, &bufferSize);
#endif
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
