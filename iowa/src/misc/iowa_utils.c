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

#include "iowa_prv_misc.h"

static char b64ClassicAlphabet[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static char b64UriSafeAlphabet[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};

/*************************************************************************************
** Private functions
*************************************************************************************/

static bool prv_b64Revert(uint8_t input,
                          uint8_t *output,
                          uint8_t mode)
{
    if (input >= 'A' && input <= 'Z')
    {
        *output = (uint8_t)(input - 'A');
    }
    else if (input >= 'a' && input <= 'z')
    {
        *output = (uint8_t)(26 + input - 'a');
    }
    else if (input >= '0' && input <= '9')
    {
        *output = (uint8_t)(52 + input - '0');
    }
    else
    {
        switch (input)
        {
        case '+':
            if (mode == BASE64_MODE_CLASSIC)
            {
                *output = 62;
            }
            else
            {
                return false;
            }
            break;
        case '/':
            if (mode == BASE64_MODE_CLASSIC)
            {
                *output = 63;
            }
            else
            {
                return false;
            }
            break;
        case '-':
            if (mode == BASE64_MODE_URI_SAFE)
            {
                *output = 62;
            }
            else
            {
                return false;
            }
            break;
        case '_':
            if (mode == BASE64_MODE_URI_SAFE)
            {
                *output = 63;
            }
            else
            {
                return false;
            }
            break;

        default:
            return false;
        }
    }

    return true;
}

/*************************************************************************************
** Public functions
*************************************************************************************/
#ifdef LWM2M_CLIENT_MODE
iowa_sensor_t iowa_utils_uri_to_sensor(iowa_lwm2m_uri_t *uriP)
{
    switch (uriP->objectId)
    {
    case IOWA_LWM2M_CELLULAR_CONNECTIVITY_OBJECT_ID:
    case IOWA_LWM2M_CONNECTIVITY_MONITORING_OBJECT_ID:
    case IOWA_LWM2M_CONNECTIVITY_STATS_OBJECT_ID:
    case IOWA_LWM2M_BEARER_SELECTION_OBJECT_ID:
    case IOWA_LWM2M_AT_COMMAND_OBJECT_ID:
    case IOWA_LWM2M_LOCATION_OBJECT_ID:
    case IOWA_LWM2M_SOFTWARE_COMPONENT_OBJECT_ID:
    case IOWA_LWM2M_SOFTWARE_MANAGEMENT_OBJECT_ID:
    case IOWA_LWM2M_DIGITAL_OUTPUT_OBJECT_ID:
    case IOWA_LWM2M_LIGHT_CONTROL_OBJECT_ID:
    case IOWA_LWM2M_ACCELEROMETER_OBJECT_ID:
    case IOWA_LWM2M_MAGNETOMETER_OBJECT_ID:
    case IOWA_LWM2M_GYROMETER_OBJECT_ID:
    case IOWA_LWM2M_GPS_OBJECT_ID:
    case IOWA_IPSO_ANALOG_INPUT:
    case IOWA_IPSO_GENERIC:
    case IOWA_IPSO_ILLUMINANCE:
    case IOWA_IPSO_TEMPERATURE:
    case IOWA_IPSO_HUMIDITY:
    case IOWA_IPSO_BAROMETER:
    case IOWA_IPSO_VOLTAGE:
    case IOWA_IPSO_CURRENT:
    case IOWA_IPSO_FREQUENCY:
    case IOWA_IPSO_DEPTH:
    case IOWA_IPSO_PERCENTAGE:
    case IOWA_IPSO_ALTITUDE:
    case IOWA_IPSO_LOAD:
    case IOWA_IPSO_PRESSURE:
    case IOWA_IPSO_LOUDNESS:
    case IOWA_IPSO_CONCENTRATION:
    case IOWA_IPSO_ACIDITY:
    case IOWA_IPSO_CONDUCTIVITY:
    case IOWA_IPSO_POWER:
    case IOWA_IPSO_POWER_FACTOR:
    case IOWA_IPSO_RATE:
    case IOWA_IPSO_DISTANCE:
    case IOWA_IPSO_ENERGY:
    case IOWA_IPSO_DIRECTION:
    case IOWA_IPSO_LEVEL_CONTROL:
    case IOWA_IPSO_DIGITAL_INPUT:
    case IOWA_IPSO_PRESENCE:
    case IOWA_IPSO_ON_OFF_SWITCH:
    case IOWA_IPSO_PUSH_BUTTON:
        switch (uriP->instanceId)
        {
        case IOWA_LWM2M_ID_ALL:
            return IOWA_INVALID_SENSOR_ID;

        default:
            return OBJECT_INSTANCE_ID_TO_SENSOR(uriP->objectId, uriP->instanceId);
        }
        break;

#ifdef IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
        if (uriP->instanceId == LWM2M_SINGLE_OBJECT_INSTANCE_ID)
        {
            switch (uriP->resourceId)
            {
            case IOWA_LWM2M_DEVICE_ID_AVAILABLE_POWER_SRC:
            case IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE:
            case IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT:
            case IOWA_LWM2M_ID_ALL:
                if (uriP->resInstanceId != IOWA_LWM2M_ID_ALL)
                {
                    return OBJECT_INSTANCE_ID_TO_SENSOR(uriP->objectId, uriP->resInstanceId);
                }
                return IOWA_INVALID_SENSOR_ID;

            case IOWA_LWM2M_DEVICE_ID_CURRENT_TIME:
                if (uriP->resInstanceId == IOWA_LWM2M_ID_ALL)
                {
                    return IOWA_DEVICE_TIME_SENSOR_ID;
                }
                return IOWA_INVALID_SENSOR_ID;

            default:
                return IOWA_INVALID_SENSOR_ID;
            }
        }
        break;
#endif

    default:
        break;
    }

    return IOWA_INVALID_SENSOR_ID;
}

iowa_lwm2m_uri_t iowa_utils_sensor_to_uri(iowa_sensor_t id)
{
    iowa_lwm2m_uri_t uri;
    size_t objectId;

    LWM2M_URI_RESET(&uri);

    objectId = GET_OBJECT_ID_FROM_SENSOR(id);

    uri.objectId = objectId;

    switch (objectId)
    {
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
        uri.instanceId = LWM2M_SINGLE_OBJECT_INSTANCE_ID;
        if (id == IOWA_DEVICE_TIME_SENSOR_ID)
        {
            uri.resourceId = IOWA_LWM2M_DEVICE_ID_CURRENT_TIME;
        }
        else
        {
            uri.resInstanceId = GET_INSTANCE_ID_FROM_SENSOR(id);
        }
        break;

    default:
        uri.instanceId = GET_INSTANCE_ID_FROM_SENSOR(id);
        break;
    }

    return uri;
}
#endif

size_t iowa_utils_base64_get_encoded_size(size_t rawBufferLen)
{
    size_t resultLen;

    resultLen = 4 * (rawBufferLen / 3);
    if (rawBufferLen % 3)
    {
        resultLen += 4;
    }

    return resultLen;
}

size_t iowa_utils_base64_get_decoded_size(uint8_t *base64Buffer,
                                          size_t base64BufferLen)
{
    size_t resultLen;

    if (base64Buffer == NULL
        || base64BufferLen == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "Base64 buffer is empty.");
        return 0;
    }

    resultLen = utils_b64GetDecodedSize(base64Buffer, base64BufferLen, true);

    return resultLen;
}

size_t iowa_utils_base64_encode(uint8_t * rawBuffer,
                                size_t rawBufferLen,
                                uint8_t * base64Buffer,
                                size_t base64BufferLen)
{
    size_t resultLen;

    if (base64Buffer == NULL
        || rawBuffer == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "Input or output buffers can not be nil.");
        return 0;
    }

    resultLen = iowa_utils_base64_get_encoded_size(rawBufferLen);
    if (resultLen > base64BufferLen)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                           "Cannot encode buffer, output buffer is too small. Current length: %d, required length: %d",
                           base64BufferLen,
                           resultLen);
        return 0;
    }

    utils_b64Encode(rawBuffer, rawBufferLen, base64Buffer, &base64BufferLen, BASE64_MODE_CLASSIC);

    return resultLen;
}

size_t iowa_utils_base64_decode(uint8_t * base64Buffer,
                                size_t base64BufferLen,
                                uint8_t * rawBuffer,
                                size_t rawBufferLen)
{
    size_t resultLen;

    if (base64Buffer == NULL
        || rawBuffer == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "Input or output buffers can not be nil.");
        return 0;
    }

    resultLen = iowa_utils_base64_get_decoded_size(base64Buffer, base64BufferLen);
    if (resultLen == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "Cannot decode the Base64 buffer.");
        return 0;
    }
    else if (resultLen > rawBufferLen)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                           "Output buffer is too small. Current length: %d, required length: %d.",
                           rawBufferLen,
                           resultLen);
        return 0;
    }

    resultLen = utils_b64Decode(base64Buffer, base64BufferLen, rawBuffer, BASE64_MODE_CLASSIC);

    return resultLen;
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

char * utilsStrdup(const char *str)
{
    char *dest;

    if (str == NULL)
    {
        return NULL;
    }

    dest = (char *)iowa_system_malloc(strlen(str) + 1);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (dest == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(strlen(str) + 1);
        return NULL;
    }
#endif
    strcpy(dest, str);

    return dest;
}

void * utilsCalloc(size_t number, size_t size)
{
    size_t totalSize;
    void *pointer;

    totalSize = number*size;
    pointer = iowa_system_malloc(totalSize);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (pointer == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(totalSize);
        return NULL;
    }
#endif

    return memset(pointer, 0, totalSize);
}

char * utilsBufferToString(const uint8_t *buffer,
                           size_t size)
{
    char *dest;

    if (buffer == NULL)
    {
        return NULL;
    }

    dest = (char *)iowa_system_malloc(size + 1);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (dest == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(size + 1);
        return NULL;
    }
#endif
    memcpy(dest, buffer, size);
    dest[size] = '\0';

    return dest;
}

bool utilsCmpBufferWithString(const uint8_t *buffer,
                              size_t size,
                              const char *str)
{
    if (buffer == NULL
        || str == NULL)
    {
        return ((const void *)buffer == (const void *)str);
    }
    if (size != strlen(str))
    {
        return false;
    }

    // Both are non null and have the same size
    return memcmp(buffer, str, size) == 0;
}

size_t utilsStrlen(const char *str)
{
    if (str == NULL)
    {
        return 0;
    }

    return strlen(str);
}

size_t utilsStringCopy(char *buffer, size_t length, const char *str)
{
    size_t strLength;

    assert(buffer != NULL);

    strLength = utilsStrlen(str);
    if (strLength == 0
        || strLength >= length)
    {
        return 0;
    }

    strcpy(buffer, str);

    return strLength;
}

void utilsCopyValue(void *dst,
                    const void *src,
                    size_t len)
{
    assert(dst != NULL);
    assert(src != NULL);

#ifdef LWM2M_BIG_ENDIAN
    memcpy(dst, src, len);
#else
#ifdef LWM2M_LITTLE_ENDIAN
    size_t i;

    for (i = 0; i < len; i++)
    {
        ((uint8_t *)dst)[i] = ((const uint8_t *)src)[len - 1 - i];
    }
#endif // LWM2M_LITTLE_ENDIAN
#endif // LWM2M_BIG_ENDIAN
}

void utils_b64Encode(uint8_t *rawBuffer,
                     size_t rawBufferLen,
                     uint8_t *base64Buffer,
                     size_t *base64BufferLenP,
                     uint8_t mode)
{
    size_t dataIndex;
    size_t resultIndex;
    char *alphabet;

    switch (mode)
    {
    case BASE64_MODE_URI_SAFE:
        alphabet = b64UriSafeAlphabet;
        break;

    case BASE64_MODE_CLASSIC:
    default:
        alphabet = b64ClassicAlphabet;
        break;
    }

    *base64BufferLenP = 0;

    dataIndex = 0;
    resultIndex = 0;
    while (dataIndex < rawBufferLen)
    {
        switch (rawBufferLen - dataIndex)
        {
        case 0:
            // should never happen
            break;

        case 1:
            base64Buffer[resultIndex] = (uint8_t)(alphabet[rawBuffer[dataIndex] >> 2]);
            base64Buffer[resultIndex + 1] = (uint8_t)(alphabet[(uint8_t)(((uint8_t)(rawBuffer[dataIndex] & 0x03)) << 4)]);
            switch (mode)
            {
            case BASE64_MODE_URI_SAFE:
                *base64BufferLenP += 2;
                break;

            case BASE64_MODE_CLASSIC:
            default:
                *base64BufferLenP += 4;
                base64Buffer[resultIndex + 2] = (uint8_t)BASE64_PADDING;
                base64Buffer[resultIndex + 3] = (uint8_t)BASE64_PADDING;
                break;
            }
            break;

        case 2:
            base64Buffer[resultIndex] = (uint8_t)(alphabet[rawBuffer[dataIndex] >> 2]);
            base64Buffer[resultIndex + 1] = (uint8_t)(alphabet[(uint8_t)(((uint8_t)(rawBuffer[dataIndex] & 0x03)) << 4)
                                                               | (uint8_t)(rawBuffer[dataIndex + 1] >> 4)]);
            base64Buffer[resultIndex + 2] = (uint8_t)(alphabet[(uint8_t)((uint8_t)(rawBuffer[dataIndex + 1] & 0x0F) << 2)]);
            switch (mode)
            {
            case BASE64_MODE_URI_SAFE:
                *base64BufferLenP += 3;
                break;

            case BASE64_MODE_CLASSIC:
            default:
                *base64BufferLenP += 4;
                base64Buffer[resultIndex + 3] = (uint8_t)BASE64_PADDING;
                break;
            }
            break;

        default:
            *base64BufferLenP += 4;
            base64Buffer[resultIndex] = (uint8_t)(alphabet[rawBuffer[dataIndex] >> 2]);
            base64Buffer[resultIndex + 1] = (uint8_t)(alphabet[(uint8_t)((uint8_t)(rawBuffer[dataIndex] & 0x03) << 4)
                                                               | (uint8_t)(rawBuffer[dataIndex + 1] >> 4)]);
            base64Buffer[resultIndex + 2] = (uint8_t)(alphabet[(uint8_t)((uint8_t)(rawBuffer[dataIndex + 1] & 0x0F) << 2)
                                                               | (uint8_t)(rawBuffer[dataIndex + 2] >> 6)]);
            base64Buffer[resultIndex + 3] = (uint8_t)(alphabet[(rawBuffer[dataIndex + 2] & 0x3F)]);
            break;
        }

        dataIndex += 3;
        resultIndex += 4;
    }
}

size_t utils_b64Decode(uint8_t *base64Buffer,
                       size_t base64BufferLen,
                       uint8_t *rawBuffer,
                       uint8_t mode)
{
    size_t dataIndex;
    size_t resultIndex;

    if (mode == BASE64_MODE_CLASSIC)
    {
        uint8_t i;

        if (base64BufferLen % 4 != 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Base64 buffer is not a multiple of 4.");
            return 0;
        }

        i = 0;

        // Remove padding
        while (base64Buffer[base64BufferLen - 1] == BASE64_PADDING)
        {
            i++;
            if (i > 2)
            {
                IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Base64 buffer has too much padding.");
                return 0;
            }
            base64BufferLen--;
        }
    }

    dataIndex = 0;
    resultIndex = 0;
    while ((dataIndex < base64BufferLen)
           && (base64BufferLen - dataIndex) >= 4)
    {
        uint8_t tmp[4];
        uint8_t i;

        for (i = 0; i < 4; i++)
        {
            if (false == prv_b64Revert(base64Buffer[dataIndex + i], tmp + i, mode))
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                                   "Invalid character in input buffer at position %u [0x%02X]",
                                   dataIndex + i, base64Buffer[dataIndex + i]);
                return 0;
            }
        }

        rawBuffer[resultIndex] = (uint8_t)(tmp[0] << 2) | (uint8_t)(tmp[1] >> 4);
        rawBuffer[resultIndex + 1] = (uint8_t)(tmp[1] << 4) | (uint8_t)(tmp[2] >> 2);
        rawBuffer[resultIndex + 2] = (uint8_t)(tmp[2] << 6) | tmp[3];

        dataIndex += 4;
        resultIndex += 3;
    }
    switch (base64BufferLen - dataIndex)
    {
    case 0:
        break;
    case 2:
    {
        uint8_t tmp[2];

        if (false == prv_b64Revert(base64Buffer[base64BufferLen - 2], tmp, mode))
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                               "Invalid character in input buffer at position %u [0x%02X]",
                               base64BufferLen - 2,
                               base64Buffer[base64BufferLen - 2]);
            return 0;
        }
        if (false == prv_b64Revert(base64Buffer[base64BufferLen - 1], tmp + 1, mode))
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                               "Invalid character in input buffer at position %u [0x%02X]",
                               base64BufferLen - 1,
                               base64Buffer[base64BufferLen - 1]);
            return 0;
        }

        rawBuffer[resultIndex] = (uint8_t)(tmp[0] << 2) | (uint8_t)(tmp[1] >> 4);
        resultIndex += 1;
    }
    break;
    case 3:
    {
        uint8_t tmp[3];

        if (false == prv_b64Revert(base64Buffer[base64BufferLen - 3], tmp, mode))
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                               "Invalid character in input buffer at position %u [0x%02X]",
                               base64BufferLen - 3,
                               base64Buffer[base64BufferLen - 3]);
            return 0;
        }
        if (false == prv_b64Revert(base64Buffer[base64BufferLen - 2], tmp + 1, mode))
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                               "Invalid character in input buffer at position %u [0x%02X]",
                               base64BufferLen - 2,
                               base64Buffer[base64BufferLen - 2]);
            return 0;
        }
        if (false == prv_b64Revert(base64Buffer[base64BufferLen - 1], tmp + 2, mode))
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM,
                               "Invalid character in input buffer at position %u [0x%02X]",
                               base64BufferLen - 1,
                               base64Buffer[base64BufferLen - 1]);
            return 0;
        }

        rawBuffer[resultIndex] = (uint8_t)(tmp[0] << 2) | (uint8_t)(tmp[1] >> 4);
        rawBuffer[resultIndex + 1] = (uint8_t)(tmp[1] << 4) | (uint8_t)(tmp[2] >> 2);
        resultIndex += 2;
    }
    break;
    default:
        IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "Unexpected error occurred during decoding the Base64 buffer");
        resultIndex = 0;
        break;
    }

    return resultIndex;
}

size_t utils_b64GetDecodedSize(uint8_t *base64Buffer,
                               size_t base64BufferLen,
                               bool withPadding)
{
    size_t resultLen;
    uint8_t lenRemainder;

    assert(base64Buffer != NULL);
    assert(base64BufferLen != 0);

    lenRemainder = (uint8_t) (base64BufferLen % 4);

    if (withPadding == true)
    {
        if (lenRemainder != 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Base64 buffer is not a multiple of 4.");
            return 0;
        }
    }
    else
    {
        if (lenRemainder == 1)
        {
            IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Base64 buffer is too short.");
            return 0;
        }
    }

    resultLen = (base64BufferLen >> 2) * 3;

    if (withPadding == true)
    {
        // Check the padding and adjust the raw length. Here the length of the base64BufferLen is > 0 and % 4
        // Maximum Base64 padding is 2
        if (base64Buffer[base64BufferLen - 3] == BASE64_PADDING)
        {
            IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Base64 buffer has too much padding.");
            return 0;
        }

        resultLen -= (size_t)(base64Buffer[base64BufferLen - 1] == BASE64_PADDING) + (size_t)(base64Buffer[base64BufferLen - 2] == BASE64_PADDING);
    }
    else
    {
        // Maximum Base64 padding is 2
        if (lenRemainder == 2)
        {
            resultLen += 1;
        }
        if (lenRemainder == 3)
        {
            resultLen += 2;
        }
    }

    return resultLen;
}
