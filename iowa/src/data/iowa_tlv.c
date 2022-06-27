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
*
**********************************************/

#include "iowa_prv_data_internals.h"
#include <float.h>

#ifdef LWM2M_SUPPORT_TLV

#define PRV_TLV_TYPE_MASK 0xC0U

#define PRV_TLV_TYPE_UNKNOWN           (uint8_t)0xFF
#define PRV_TLV_TYPE_OBJECT_INSTANCE   (uint8_t)0x00
#define PRV_TLV_TYPE_RESOURCE          (uint8_t)0xC0
#define PRV_TLV_TYPE_MULTIPLE_RESOURCE (uint8_t)0x80
#define PRV_TLV_TYPE_RESOURCE_INSTANCE (uint8_t)0x40

#define PRV_64BIT_BUFFER_SIZE 8

#define PRV_TLV_TYPE_LEVEL_STR(S) ((S) == PRV_TLV_TYPE_OBJECT_INSTANCE ? "Object Instance" :     \
                                  ((S) == PRV_TLV_TYPE_RESOURCE ? "Resource" :                   \
                                  ((S) == PRV_TLV_TYPE_MULTIPLE_RESOURCE ? "Multiple Resource" : \
                                  ((S) == PRV_TLV_TYPE_RESOURCE_INSTANCE ? "Resource Instance" : \
                                  "unknown"))))

/*************************************************************************************
** Private functions
*************************************************************************************/

static size_t prv_encodeFloat(double data,
                              uint8_t *dataBuffer)
{
    size_t length;

    if ((data < 0.0 - (double)FLT_MAX) || (data >(double)FLT_MAX))
    {
        length = 8;
        utilsCopyValue(dataBuffer, &data, 8);
    }
    else
    {
        float value;

        length = 4;
        value = (float)data;
        utilsCopyValue(dataBuffer, &value, 4);
    }

    return length;
}

static size_t prv_encodeInt(int64_t data,
                            uint8_t *dataBuffer)
{
    size_t length;

    if (data >= INT8_MIN && data <= INT8_MAX)
    {
        length = 1;
        dataBuffer[0] = (uint8_t)data;
    }
    else if (data >= INT16_MIN
             && data <= INT16_MAX)
    {
        int16_t value;

        value = (int16_t)data;
        length = 2;

        // Keep in mind that shifting negative value is platform dependent
        dataBuffer[0] = (uint8_t)((value >> 8) & 0xFF);
        dataBuffer[1] = (uint8_t)(value & 0xFF);
    }
    else if (data >= INT32_MIN
             && data <= INT32_MAX)
    {
        int32_t value;

        value = (int32_t)data;
        length = 4;
        utilsCopyValue(dataBuffer, &value, length);
    }
    else
    {
        length = 8;
        utilsCopyValue(dataBuffer, &data, length);
    }

    return length;
}

static size_t prv_getHeaderLength(uint16_t id,
                                  size_t dataLength)
{
    size_t length;

    length = 2;

    if (id > 0xFF)
    {
        length += 1;
    }

    if (dataLength > 0xFFFF)
    {
        length += 3;
    }
    else if (dataLength > 0xFF)
    {
        length += 2;
    }
    else if (dataLength > 7)
    {
        length += 1;
    }

    return length;
}

static size_t prv_createHeader(uint8_t *header,
                               uint8_t type,
                               uint16_t id,
                               size_t dataLength)
{
    size_t headerLen;
    size_t offset;

    headerLen = prv_getHeaderLength(id, dataLength);

    header[0] = 0;
    header[0] = (uint8_t)(header[0] | (uint8_t)(type & PRV_TLV_TYPE_MASK));

    if (id > 0xFF)
    {
        header[0] |= 0x20;
        header[1] = ((uint8_t)(id >> 8) & 0XFF);
        header[2] = ((uint8_t)id & 0XFF);
        offset = 3;
    }
    else
    {
        header[1] = (uint8_t)id;
        offset = 2;
    }
    if (dataLength <= 7)
    {
        header[0] = (uint8_t)(header[0] + dataLength);
    }
    else if (dataLength <= 0xFF)
    {
        header[0] |= 0x08;
        header[offset] = (uint8_t)dataLength;
    }
    else if (dataLength <= 0xFFFF)
    {
        header[0] |= 0x10;
        header[offset] = ((uint8_t)(dataLength >> 8) & 0XFF);
        header[offset + 1] = (uint8_t)(dataLength & 0XFF);
    }
    else if (dataLength <= 0xFFFFFF)
    {
        header[0] |= 0x18;
        header[offset] = ((uint8_t)(dataLength >> 16) & 0XFF);
        header[offset + 1] = ((uint8_t)(dataLength >> 8) & 0XFF);
        header[offset + 2] = (uint8_t)(dataLength & 0XFF);
    }

    return headerLen;
}

static size_t prv_lwm2mDecodeTlv(uint8_t *buffer,
                                 size_t bufferLength,
                                 uint8_t *oType,
                                 uint16_t *oId,
                                 size_t *oDataIndex,
                                 size_t *oDataLength)
{
    IOWA_LOG_BUFFER_TRACE(IOWA_PART_DATA, "Decoding TLV part", buffer, bufferLength);

    if (bufferLength < 2)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Buffer length too short (%u).", bufferLength);
        return 0;
    }
    *oDataIndex = 2;

    switch (buffer[0] & PRV_TLV_TYPE_MASK)
    {
    case PRV_TLV_TYPE_OBJECT_INSTANCE:
    case PRV_TLV_TYPE_MULTIPLE_RESOURCE:
    case PRV_TLV_TYPE_RESOURCE:
    case PRV_TLV_TYPE_RESOURCE_INSTANCE:
        *oType = buffer[0] & PRV_TLV_TYPE_MASK;
        break;

    default:
        *oType = PRV_TLV_TYPE_UNKNOWN;
    }
    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Type: %u.", *oType);

    if ((buffer[0] & 0x20) == 0x20)
    {
        // id is 16 bits long
        if (bufferLength < 3)
        {
            return 0;
        }
        *oDataIndex += 1;
        *oId = ((uint16_t) buffer[1]<<8) + buffer[2];
    }
    else
    {
        // id is 8 bits long
        *oId = buffer[1];
    }
    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "ID: %u.", *oId);

    switch (buffer[0] & 0x18)
    {
    case 0x00:
        // no length field
        *oDataLength = buffer[0] & 0x07;
        break;

    case 0x08:
        // length field is 8 bits long
        if (bufferLength < *oDataIndex + 1)
        {
            return 0;
        }
        *oDataLength = buffer[*oDataIndex];
        *oDataIndex += 1;
        break;

    case 0x10:
        // length field is 16 bits long
        if (bufferLength < *oDataIndex + 2)
        {
            return 0;
        }
        *oDataLength = ((size_t) buffer[*oDataIndex]<<8) + buffer[*oDataIndex+1];
        *oDataIndex += 2;
        break;

    case 0x18:
        // length field is 24 bits long
        if (bufferLength < *oDataIndex + 3)
        {
            return 0;
        }
        *oDataLength = (size_t)(((uint32_t)buffer[*oDataIndex]<<16) + (uint16_t)((uint16_t)buffer[*oDataIndex+1]<<8) + buffer[*oDataIndex+2]);
        *oDataIndex += 3;
        break;

    default:
        // can't happen
        return 0;
    }
    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Data index: %u, length: %u bytes.", *oDataIndex, *oDataLength);

    if (*oDataIndex + *oDataLength > bufferLength)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Buffer is too short (%u bytes) for declared data length (%u bytes) at index %u.", bufferLength, *oDataLength, *oDataIndex);
        return 0;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Returning parsed length of %u bytes.", *oDataIndex + *oDataLength);

    return *oDataIndex + *oDataLength;
}

static iowa_status_t prv_getLength(iowa_lwm2m_uri_t *baseUriP,
                                   lwm2m_uri_depth_t uriDepth,
                                   iowa_lwm2m_data_t *dataP,
                                   size_t size,
                                   size_t *lengthP)
{
    size_t i;
    size_t instanceDataLength;
    size_t resourceDataLength;

    *lengthP = 0;

    if (size == 0)
    {
        return IOWA_COAP_NO_ERROR;
    }

    instanceDataLength = 0;
    resourceDataLength = 0;

    for (i = 0; i < size; i++)
    {
        uint16_t resId;

        // Check if data has to be added
        if (dataUtilsIsInBaseUri(dataP + i, baseUriP, uriDepth) == false)
        {
            // Loop on next element
            continue;
        }

        // Check if this is a new instance
        if (i > 0
            && dataP[i].instanceID != dataP[i-1].instanceID)
        {
            *lengthP += prv_getHeaderLength(dataP[i-1].instanceID, instanceDataLength) + instanceDataLength;
            instanceDataLength = 0;
        }

        // Check if this is a multiple resource
        if (dataP[i].resInstanceID != IOWA_LWM2M_ID_ALL)
        {
            resId = dataP[i].resInstanceID;
        }
        else
        {
            resId = dataP[i].resourceID;
        }

        switch (dataP[i].type)
        {
        case IOWA_LWM2M_TYPE_STRING:
        case IOWA_LWM2M_TYPE_CORE_LINK:
        case IOWA_LWM2M_TYPE_OPAQUE:
            resourceDataLength += prv_getHeaderLength(resId, dataP[i].value.asBuffer.length) + dataP[i].value.asBuffer.length;
            break;

        case IOWA_LWM2M_TYPE_UNSIGNED_INTEGER:
            if (dataP[i].value.asInteger < 0)
            {
                IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Unsigned integer value has a negative value: %d", dataP[i].value.asInteger);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            // Fall through
        case IOWA_LWM2M_TYPE_INTEGER:
        case IOWA_LWM2M_TYPE_TIME:
        {
            size_t dataLength;
            uint8_t unusedBuffer[PRV_64BIT_BUFFER_SIZE];

            dataLength = prv_encodeInt(dataP[i].value.asInteger, unusedBuffer);
            resourceDataLength += prv_getHeaderLength(resId, dataLength) + dataLength;
            break;
        }

        case IOWA_LWM2M_TYPE_FLOAT:
        {
            size_t dataLength;

            if (dataP[i].value.asFloat < 0.0 - (double)FLT_MAX
                || dataP[i].value.asFloat > (double)FLT_MAX)
            {
                dataLength = 8;
            }
            else
            {
                dataLength = 4;
            }

            resourceDataLength += prv_getHeaderLength(resId, dataLength) + dataLength;
            break;
        }

        case IOWA_LWM2M_TYPE_BOOLEAN:
            // Booleans are always encoded on one byte
            resourceDataLength += prv_getHeaderLength(resId, 1) + 1;
            break;

        case IOWA_LWM2M_TYPE_OBJECT_LINK:
            // Object Link are always encoded on four bytes
            resourceDataLength += prv_getHeaderLength(resId, 4) + 4;
            break;

        case IOWA_LWM2M_TYPE_UNDEFINED:
        default:
            IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Unknown resource type: %d", dataP[i].type);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }

        // Check if this is a multiple resource
        if (dataP[i].resInstanceID != IOWA_LWM2M_ID_ALL
                 && uriDepth != LWM2M_URI_DEPTH_RESOURCE_INSTANCE)
        {
            // Check if this is a new multiple resource
            if (i + 1 == size
                || dataP[i].resourceID != dataP[i+1].resourceID)
            {
                instanceDataLength += prv_getHeaderLength(dataP[i].resourceID, resourceDataLength) + resourceDataLength;
                resourceDataLength = 0;
            }
        }
        else
        {
            instanceDataLength += resourceDataLength;
            resourceDataLength = 0;
        }
    }

    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_OBJECT:
        *lengthP += prv_getHeaderLength(dataP[i-1].instanceID, instanceDataLength) + instanceDataLength;
        break;

    default:
        *lengthP = instanceDataLength;
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

static size_t prv_checkFormatAndGetDataCount(iowa_lwm2m_uri_t *baseUriP,
                                             uint8_t level,
                                             uint8_t *buffer,
                                             size_t bufferLength)
{
    uint8_t type;
    uint16_t id;
    size_t dataCount;
    size_t dataIndex;
    size_t dataLength;
    size_t index;

    dataCount = 0;

    for (index = 0; index < bufferLength; index += dataIndex + dataLength)
    {
        if (prv_lwm2mDecodeTlv(buffer + index, bufferLength - index, &type, &id, &dataIndex, &dataLength) == 0)
        {
            break;
        }

        switch (type)
        {
        case PRV_TLV_TYPE_OBJECT_INSTANCE:
            if (level != PRV_TLV_TYPE_UNKNOWN)
            {
                IOWA_LOG_WARNING(IOWA_PART_DATA, "Object Instance type is not present at root level.");
                return 0;
            }
            else
            {
                if (baseUriP->resourceId != IOWA_LWM2M_ID_ALL)
                {
                    IOWA_LOG_WARNING(IOWA_PART_DATA, "The base URI points under the Object Instance level.");
                    return 0;
                }
            }

            if (baseUriP->instanceId != IOWA_LWM2M_ID_ALL
                && baseUriP->instanceId != id)
            {
                IOWA_LOG_WARNING(IOWA_PART_DATA, "Object Instance is not under the base URI.");
                return 0;
            }
            break;

        case PRV_TLV_TYPE_RESOURCE:
        case PRV_TLV_TYPE_MULTIPLE_RESOURCE:
            if (level != PRV_TLV_TYPE_UNKNOWN)
            {
                if (level != PRV_TLV_TYPE_OBJECT_INSTANCE)
                {
                    IOWA_LOG_WARNING(IOWA_PART_DATA, "Resource type is not present at root level or under an Object Instance.");
                    return 0;
                }
            }
            else
            {
                if (baseUriP->resourceId != IOWA_LWM2M_ID_ALL
                    && baseUriP->instanceId == IOWA_LWM2M_ID_ALL)
                {
                    IOWA_LOG_WARNING(IOWA_PART_DATA, "The base URI does not contain an Object Instance.");
                    return 0;
                }

                if (type == PRV_TLV_TYPE_RESOURCE
                    && baseUriP->resInstanceId != IOWA_LWM2M_ID_ALL)
                {
                    IOWA_LOG_WARNING(IOWA_PART_DATA, "The base URI points to a Resource Instance.");
                    return 0;
                }

                level = PRV_TLV_TYPE_OBJECT_INSTANCE;
            }
            if (baseUriP->resourceId != IOWA_LWM2M_ID_ALL
                && baseUriP->resourceId != id)
            {
                IOWA_LOG_WARNING(IOWA_PART_DATA, "Resource is not under the base URI.");
                return 0;
            }
            break;

        case PRV_TLV_TYPE_RESOURCE_INSTANCE:
            if (level != PRV_TLV_TYPE_UNKNOWN)
            {
                if (level != PRV_TLV_TYPE_MULTIPLE_RESOURCE)
                {
                    IOWA_LOG_WARNING(IOWA_PART_DATA, "Resource Instance type is not present at root level or under a Multiple Resource.");
                    return 0;
                }
            }
            else
            {
                if (baseUriP->resourceId == IOWA_LWM2M_ID_ALL)
                {
                    IOWA_LOG_WARNING(IOWA_PART_DATA, "The base URI is missing the Resource ID.");
                    return 0;
                }

                level = PRV_TLV_TYPE_MULTIPLE_RESOURCE;
            }
            if (baseUriP->resInstanceId != IOWA_LWM2M_ID_ALL
                && baseUriP->resInstanceId != id)
            {
                IOWA_LOG_WARNING(IOWA_PART_DATA, "Resource Instance is not under the base URI.");
                return 0;
            }
            break;

        default:
            IOWA_LOG_WARNING(IOWA_PART_DATA, "Unknown TLV type.");
            return 0;
        }

        if (type == PRV_TLV_TYPE_OBJECT_INSTANCE
            || type == PRV_TLV_TYPE_MULTIPLE_RESOURCE)
        {
            size_t instanceCount;

            instanceCount = prv_checkFormatAndGetDataCount(baseUriP, type, buffer + index + dataIndex, dataLength);
            if (instanceCount == 0 && dataLength != 0)
            {
                // Propagate the error
                return 0;
            }
            dataCount += instanceCount;
        }
        else
        {
            dataCount++;
        }
    }

    return dataCount;
}

static size_t prv_tlvToLwm2mData(iowa_lwm2m_uri_t *baseUriP,
                                 uint8_t *buffer,
                                 size_t bufferLength,
                                 iowa_lwm2m_data_t *dataP)
{
    uint8_t type;
    uint16_t id;
    size_t dataIndex;
    size_t dataLength;
    size_t index;
    size_t iData;

    iData = 0;

    for (index = 0; index < bufferLength; index += dataIndex + dataLength)
    {
        if (prv_lwm2mDecodeTlv(buffer + index, bufferLength - index, &type, &id, &dataIndex, &dataLength) == 0)
        {
            break;
        }

        switch (type)
        {
        case PRV_TLV_TYPE_OBJECT_INSTANCE:
        {
            iowa_lwm2m_uri_t baseUri;

            LWM2M_URI_RESET(&baseUri);
            baseUri.objectId = baseUriP->objectId;
            baseUri.instanceId = id;

            iData += prv_tlvToLwm2mData(&baseUri, buffer + index + dataIndex , dataLength, dataP + iData);
            break;
        }

        case PRV_TLV_TYPE_MULTIPLE_RESOURCE:
        {
            iowa_lwm2m_uri_t baseUri;

            LWM2M_URI_RESET(&baseUri);
            baseUri.objectId = baseUriP->objectId;
            baseUri.instanceId = baseUriP->instanceId;
            baseUri.resourceId = id;

            iData += prv_tlvToLwm2mData(&baseUri, buffer + index + dataIndex , dataLength, dataP + iData);
            break;
        }

        default:
            dataP[iData].objectID = baseUriP->objectId;
            dataP[iData].instanceID = baseUriP->instanceId;

            if (type == PRV_TLV_TYPE_RESOURCE)
            {
                dataP[iData].resourceID = id;
                dataP[iData].resInstanceID = IOWA_LWM2M_ID_ALL;
            }
            else
            {
                // This is a multiple resource
                dataP[iData].resourceID = baseUriP->resourceId;
                dataP[iData].resInstanceID = id;
            }

            if (IOWA_COAP_NO_ERROR != dataUtilsSetBuffer(buffer + index + dataIndex, dataLength, dataP+iData, IOWA_LWM2M_TYPE_UNDEFINED))
            {
                return 0;
            }

            iData++;
        }
    }

    return iData;
}

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_status_t tlvSerialize(iowa_lwm2m_uri_t *baseUriP,
                           iowa_lwm2m_data_t *dataP,
                           size_t size,
                           uint8_t **bufferP,
                           size_t *bufferLengthP)
{
    // Warning: 'dataP' must be in order to convert correctly
    iowa_status_t result;
    iowa_lwm2m_uri_t baseUri;
    lwm2m_uri_depth_t uriDepth;
    size_t index;
    size_t i;
    size_t instanceDataLength;
    size_t resourceDataLength;

    assert(dataP != NULL);
    assert(size != 0);
    assert(bufferP != NULL);
    assert(bufferLengthP != NULL);

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "size: %d", size);

    if (baseUriP == NULL)
    {
        dataUtilsGetBaseUri(dataP, size, &baseUri, &uriDepth);
    }
    else
    {
        baseUri = *baseUriP;
        uriDepth = dataUtilsGetUriDepth(baseUriP);
    }

    if (uriDepth == LWM2M_URI_DEPTH_ROOT)
    {
        IOWA_LOG_WARNING(IOWA_PART_DATA, "Data contain more than one object.");
        return IOWA_COAP_400_BAD_REQUEST;
    }

    *bufferP = NULL;
    result = prv_getLength(&baseUri, uriDepth, dataP, size, bufferLengthP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_WARNING(IOWA_PART_DATA, "Failed to retrieve the length");
        return result;
    }
    if (*bufferLengthP == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_DATA, "Buffer length is zero");
        return result;
    }

    *bufferP = (uint8_t *)iowa_system_malloc(*bufferLengthP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*bufferP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*bufferLengthP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    index = 0;
    instanceDataLength = 0;
    resourceDataLength = 0;

    for (i = 0; i < size; i++)
    {
        size_t headerLen;
        uint16_t resId;
        uint8_t resType;

        // Check if data has to be added
        if (dataUtilsIsInBaseUri(dataP + i, &baseUri, uriDepth) == false)
        {
            // Loop on next element
            continue;
        }

        // Check if this is a new instance
        if (i > 0
            && dataP[i].instanceID != dataP[i-1].instanceID)
        {
            headerLen = prv_getHeaderLength(dataP[i-1].instanceID, instanceDataLength);
            memmove(*bufferP + index + headerLen - instanceDataLength, *bufferP + index - instanceDataLength, instanceDataLength);

            (void)prv_createHeader(*bufferP + index - instanceDataLength, PRV_TLV_TYPE_OBJECT_INSTANCE, dataP[i-1].instanceID, instanceDataLength);
            index += headerLen;

            instanceDataLength = 0;
        }

        // Check if this is a multiple resource
        if (dataP[i].resInstanceID != IOWA_LWM2M_ID_ALL)
        {
            resId = dataP[i].resInstanceID;
            resType = PRV_TLV_TYPE_RESOURCE_INSTANCE;
        }
        else
        {
            resId = dataP[i].resourceID;
            resType = PRV_TLV_TYPE_RESOURCE;
        }

        switch (dataP[i].type)
        {
        case IOWA_LWM2M_TYPE_STRING:
        case IOWA_LWM2M_TYPE_CORE_LINK:
        case IOWA_LWM2M_TYPE_OPAQUE:
            headerLen = prv_createHeader(*bufferP + index, resType, resId, dataP[i].value.asBuffer.length);
            index += headerLen;
            memcpy(*bufferP + index, dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length);
            index += dataP[i].value.asBuffer.length;

            resourceDataLength += headerLen + dataP[i].value.asBuffer.length;
            break;

        case IOWA_LWM2M_TYPE_INTEGER:
        case IOWA_LWM2M_TYPE_TIME:
        case IOWA_LWM2M_TYPE_UNSIGNED_INTEGER:
        {
            size_t dataLength;
            uint8_t dataBuffer[PRV_64BIT_BUFFER_SIZE];

            dataLength = prv_encodeInt(dataP[i].value.asInteger, dataBuffer);
            headerLen = prv_createHeader(*bufferP + index, resType, resId, dataLength);
            index += headerLen;
            memcpy(*bufferP + index, dataBuffer, dataLength);
            index += dataLength;

            resourceDataLength += headerLen + dataLength;
            break;
        }

        case IOWA_LWM2M_TYPE_FLOAT:
        {
            size_t dataLength;
            uint8_t dataBuffer[PRV_64BIT_BUFFER_SIZE];

            dataLength = prv_encodeFloat(dataP[i].value.asFloat, dataBuffer);
            headerLen = prv_createHeader(*bufferP + index, resType, resId, dataLength);
            index += headerLen;
            memcpy(*bufferP + index, dataBuffer, dataLength);
            index += dataLength;

            resourceDataLength += headerLen + dataLength;
            break;
        }

        case IOWA_LWM2M_TYPE_BOOLEAN:
            // Booleans are always encoded on one byte
            headerLen = prv_createHeader(*bufferP + index, resType, resId, 1);
            index += headerLen;
            (*bufferP)[index] = dataP[i].value.asBoolean ? 1 : 0;
            index += 1;

            resourceDataLength += headerLen + 1;
            break;

        case IOWA_LWM2M_TYPE_OBJECT_LINK:
        {
            // Object Link are always encoded on four bytes
            uint8_t buf[4];

            buf[0] = (uint8_t)((uint16_t)(dataP[i].value.asObjLink.objectId & 0xFF00) >> 8);
            buf[1] = (uint8_t)(dataP[i].value.asObjLink.objectId & 0x00FF);
            buf[2] = (uint8_t)((uint16_t)(dataP[i].value.asObjLink.instanceId & 0xFF00) >> 8);
            buf[3] = (uint8_t)(dataP[i].value.asObjLink.instanceId & 0x00FF);

            // Keep encoding as buffer
            headerLen = prv_createHeader(*bufferP + index, resType, resId, 4);
            index += headerLen;
            memcpy(*bufferP + index, buf, 4);
            index += 4;

            resourceDataLength += headerLen + 4;
            break;
        }

        default:
            result = IOWA_COAP_400_BAD_REQUEST;
            goto exit_function;
        }

        // Check if this is a multiple resource
        if (dataP[i].resInstanceID != IOWA_LWM2M_ID_ALL
                 && uriDepth != LWM2M_URI_DEPTH_RESOURCE_INSTANCE)
        {
            // Check if this is a new multiple resource
            if (i + 1 == size
                || dataP[i].resourceID != dataP[i+1].resourceID)
            {
                headerLen = prv_getHeaderLength(dataP[i].resourceID, resourceDataLength);
                memmove(*bufferP + index + headerLen - resourceDataLength, *bufferP + index - resourceDataLength, resourceDataLength);

                (void)prv_createHeader(*bufferP + index - resourceDataLength, PRV_TLV_TYPE_MULTIPLE_RESOURCE, dataP[i].resourceID, resourceDataLength);
                index += headerLen;

                instanceDataLength += resourceDataLength + headerLen;
                resourceDataLength = 0;
            }
        }
        else
        {
            instanceDataLength += resourceDataLength;
            resourceDataLength = 0;
        }

    }

    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_OBJECT:
    {
        size_t headerLen;

        headerLen = prv_getHeaderLength(dataP[i-1].instanceID, instanceDataLength);
        memmove(*bufferP + index + headerLen - instanceDataLength, *bufferP + index - instanceDataLength, instanceDataLength);

        (void)prv_createHeader(*bufferP + index - instanceDataLength, PRV_TLV_TYPE_OBJECT_INSTANCE, dataP[i-1].instanceID, instanceDataLength);
        break;
    }

    default:
        break;
    }

exit_function:
    if (result != IOWA_COAP_NO_ERROR)
    {
        iowa_system_free(*bufferP);
        *bufferP = NULL;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Returning %u bytes", *bufferLengthP);

    return result;
}

iowa_status_t tlvDeserialize(iowa_lwm2m_uri_t *baseUriP,
                             uint8_t *bufferP,
                             size_t bufferLength,
                             iowa_lwm2m_data_t **dataP,
                             size_t *dataCountP)
{
    uint8_t tlvType;
    iowa_lwm2m_uri_t baseUri;

    IOWA_LOG_BUFFER_TRACE(IOWA_PART_DATA, "Parsing TLV buffer", bufferP, bufferLength);

    *dataP = NULL;

    // The Object ID can not be encoded in a LwM2M TLV record. Thus it must be provided by the base URI.
    if (baseUriP == NULL)
    {
        IOWA_LOG_WARNING(IOWA_PART_DATA, "A base URI is required.");
        return IOWA_COAP_400_BAD_REQUEST;
    }
    else
    {
        if (dataUtilsGetUriDepth(baseUriP) == LWM2M_URI_DEPTH_ROOT)
        {
            IOWA_LOG_WARNING(IOWA_PART_DATA, "Object ID must be part of the base URI.");
            return IOWA_COAP_400_BAD_REQUEST;
        }

        baseUri = *baseUriP;
    }

    tlvType = PRV_TLV_TYPE_UNKNOWN;

    *dataCountP = prv_checkFormatAndGetDataCount(&baseUri, tlvType, bufferP, bufferLength);
    if (*dataCountP == 0)
    {
        return IOWA_COAP_400_BAD_REQUEST;
    }

    *dataP = (iowa_lwm2m_data_t *)iowa_system_malloc(*dataCountP * sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*dataP == NULL)
    {
       IOWA_LOG_ERROR_MALLOC(*dataCountP * sizeof(iowa_lwm2m_data_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*dataP, 0, *dataCountP * sizeof(iowa_lwm2m_data_t));

    if (prv_tlvToLwm2mData(&baseUri, bufferP, bufferLength, *dataP) == 0)
    {
        return IOWA_COAP_400_BAD_REQUEST;
    }

    return IOWA_COAP_NO_ERROR;
}

#endif // LWM2M_SUPPORT_TLV
