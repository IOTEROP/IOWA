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

typedef struct
{
    union
    {
        double asDouble;
        int64_t asInteger;
    } convert;
} prv_double_convert_t;

#define PRV_FLOAT_VALUE_INFINITY_OR_NAN_MASK 0x7F80000000000000

#define PRV_OBJECT_LINK_TEXT_SEPARATOR  ':'
#define PRV_DECIMAL_POINT               '.'
#define PRV_EXPONENT_MIN                'e'
#define PRV_EXPONENT_MAX                'E'
#define PRV_MINUS_SIGN                  '-'
#define PRV_PLUS_SIGN                   '+'

#define PRV_EXPONENT_INT_MIN_CHAR_COUNT 3
#define PRV_EXPONENT_DEC_MIN_CHAR_COUNT 4

/*************************************************************************************
** Private functions
*************************************************************************************/

static int prv_parseUriNumber(const uint8_t *uriString,
                              size_t uriLength,
                              size_t *headP)
{
    int result;

    if (uriString[*headP] == '/')
    {
        return -1;
    }

    result = 0;
    while (*headP < uriLength && uriString[*headP] != '/')
    {
        if ('0' <= uriString[*headP] && uriString[*headP] <= '9')
        {
            result += uriString[*headP] - '0';
            result *= 10;
        }
        else
        {
            return -1;
        }
        *headP += 1;
    }

    result /= 10;
    return result;
}

static size_t prv_bufferCopy(uint8_t *buffer,
                             size_t bufferLength,
                             uint8_t **bufferCopyP)
{
    if (bufferLength == 0)
    {
        *bufferCopyP = NULL;
        return 0;
    }

    *bufferCopyP = (uint8_t *)iowa_system_malloc(bufferLength);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*bufferCopyP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(bufferLength);
        return 0;
    }
#endif
    memcpy(*bufferCopyP, buffer, bufferLength);

    return bufferLength;
}

static size_t prv_partBufferToInt(uint8_t *buffer,
                                  size_t length,
                                  int64_t *dataP)
{
    size_t i;
    uint64_t value;

    assert(buffer != NULL);
    assert(dataP != NULL);

    value = 0;
    i = 0;
    while (i < length
           && '0' <= buffer[i]
           && buffer[i] <= '9')
    {
        uint64_t digit;

        digit = (uint64_t)(buffer[i] - '0');

        value *= 10;
        value += digit;

        if (value < digit)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Overflow.");
            return 0;
        }

        i++;
    }

    if (value > (uint64_t)INT64_MAX)
    {
        return 0;
    }

    *dataP = (int64_t)value;

    return i;
}

static int prv_getSignFromBufferToNumber(uint8_t *buffer,
                                         size_t *indexP)
{
    assert(buffer != NULL);
    assert(indexP != NULL);

    if ('0' <= buffer[*indexP]
        && buffer[*indexP] <= '9')
    {
        return 1;
    }
    else
    {
        if (buffer[*indexP] == PRV_MINUS_SIGN)
        {
            (*indexP)++;
            return -1;
        }
        else if (buffer[*indexP] == PRV_PLUS_SIGN)
        {
            (*indexP)++;
            return 1;
        }
    }
    return 0;
}

static int prv_isExponentPossibleForIntPart(int64_t *valueP)
{
    int cpt;
    int64_t intValue;
    int sign;

    assert(valueP != NULL);

    if (*valueP < 0)
    {
        intValue = 0 - (*valueP);
        sign = -1;
    }
    else
    {
        intValue = (*valueP);
        sign = 1;
    }

    cpt = 0;

    while (intValue > 0
           && (intValue - ((int64_t)(intValue / 10)) * 10) == 0)
    {
        intValue = (int64_t)(intValue / 10);
        cpt++;
    }
    if (cpt >= PRV_EXPONENT_INT_MIN_CHAR_COUNT)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "exponent found: %d.", cpt);
        (*valueP) = sign * intValue;
        return cpt;
    }

    IOWA_LOG_TRACE(IOWA_PART_DATA, "No exponent possible.");
    return 0;
}

static int prv_isExponentPossibleForDecPart(double *valueP)
{
    int cpt;
    double floatValue;
    int sign;

    assert(valueP != NULL);

    if ((int64_t)(*valueP) != 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "No exponent possible.");
        return 0;
    }

    if (*valueP < 0)
    {
        floatValue = 0 - (*valueP);
        sign = -1;
    }
    else
    {
        floatValue = *valueP;
        sign = 1;
    }

    cpt = 0;

    if ((floatValue - (double)FLT_EPSILON) <= 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "No exponent possible.");
        return 0;
    }

    while ((floatValue - (double)FLT_EPSILON) > 0
           && floatValue < 1)
    {
        floatValue = floatValue * 10;
        cpt++;
    }
    if (cpt >= PRV_EXPONENT_DEC_MIN_CHAR_COUNT)
    {
        while (((floatValue - (double)((int64_t)floatValue)) - (double)FLT_EPSILON) > 0)
        {
            floatValue = floatValue * 10;
            cpt++;
        }
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "exponent found: %d.", cpt);
        (*valueP) = sign * floatValue;
        return cpt;
    }

    IOWA_LOG_TRACE(IOWA_PART_DATA, "No exponent possible.");
    return 0;
}

static size_t prv_intToBufferLength(int64_t data)
{
    size_t cpt;

    if (data == 0)
    {
        return 1;
    }

    cpt = 0;
    if (data < 0)
    {
        data = - data;
        cpt++;
    }

    while (data > 0)
    {
        data = data / 10;
        cpt++;
    }

    return cpt;
}

static size_t prv_intToBuffer(int64_t data,
                              uint8_t *buffer,
                              size_t length)
{
    int index;
    bool minus;
    size_t result;

    assert(buffer != NULL);

    if (data < 0)
    {
        minus = true;
        data = 0 - data;
    }
    else
    {
        minus = false;
    }

    index = (int)(length - 1);
    do
    {
        buffer[index] = (uint8_t)('0' + data % 10);
        data /= 10;
        index--;
    } while (index >= 0 && data > 0);

    if (data > 0)
    {
        return 0;
    }

    if (minus == true)
    {
        if (index == 0)
        {
            return 0;
        }
        buffer[index] = '-';
    }
    else
    {
        index++;
    }

    result = length - (size_t)index;
    if (result < length)
    {
        memmove(buffer, buffer + index, result);
    }

    return result;
}


/*************************************************************************************
** Public functions
*************************************************************************************/

double dataUtilsPower(double number, int64_t power)
{
    int64_t ind;
    double result;

    result = 1;

    if (power < 0)
    {
        assert(number != 0.f);

        power *= (-1);
        number = 1/number;
    }

    for (ind = 0; ind < power; ind++)
    {
        result *= number;
    }

    return result;
}

bool dataUtilsIsInt(double value)
{
    int64_t intVal;
    double floatVal;

    intVal = (int64_t)value;
    floatVal = (double)intVal;
    if (floatVal + (double)FLT_EPSILON < value
        || floatVal - (double)FLT_EPSILON > value)
    {
        return false;
    }
    return true;
}

size_t dataUtilsBufferToInt(uint8_t *buffer,
                            size_t length,
                            int64_t *dataP)
{
    int64_t result;
    int sign;
    size_t index;

    assert(buffer != NULL);
    assert(dataP != NULL);

    if (0 == length)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Length is 0.");
        return 0;
    }

    result = 0;
    index = 0;

    sign = prv_getSignFromBufferToNumber(buffer, &index);
    if (sign == 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }

    if (prv_partBufferToInt(&buffer[index], length - index, &result) != length - index)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }

    *dataP = sign * result;

    return 1;
}

size_t dataUtilsIntToBufferLength(int64_t data,
                                  bool withExponent)
{
    size_t cpt;
    int exponent;

    if (withExponent == true)
    {
        exponent = prv_isExponentPossibleForIntPart(&data);
    }
    else
    {
        exponent = 0;
    }

    cpt = prv_intToBufferLength(data);

    if (exponent != 0)
    {
        cpt += 1 + prv_intToBufferLength(exponent);
    }

    return cpt;
}

size_t dataUtilsIntToBuffer(int64_t data,
                            uint8_t *buffer,
                            size_t length,
                            bool withExponent)
{
    size_t result;
    int exponent;

    assert(buffer != NULL);

    if (withExponent == true)
    {
        exponent = prv_isExponentPossibleForIntPart(&data);
    }
    else
    {
        exponent = 0;
    }

    result = prv_intToBuffer(data, buffer, length);
    if (result == 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Failed to put int to buffer.");
        return 0;
    }

    if (exponent != 0)
    {
        size_t expResult;

        if (result + 1 > length)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Adding exponent not possible.");
            return 0;
        }
        buffer[result] = PRV_EXPONENT_MIN;
        result++;
        expResult = prv_intToBuffer(exponent, buffer + result, length - result);
        if (expResult == 0)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Adding exponent not possible.");
            return 0;
        }
        result += expResult;
    }

    return result;
}

size_t dataUtilsBufferToFloat(uint8_t *buffer,
                              size_t length,
                              double *dataP)
{
    double result;
    int sign;
    size_t index;
    int64_t intPart;
    size_t intResult;

    if (0 == length)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }

    assert(buffer != NULL);
    assert(dataP != NULL);

    index = 0;
    sign = prv_getSignFromBufferToNumber(buffer, &index);
    if (sign == 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }

    intResult = prv_partBufferToInt(&buffer[index], length - index, &intPart);
    index += intResult;
    if (intResult == 0
        || index > length)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }
    result = (double)intPart;

    if (index < length
        && buffer[index] == PRV_DECIMAL_POINT)
    {
        double temp;
        index++;
        if (index > length)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
            return 0;
        }

        intResult = prv_partBufferToInt(&buffer[index], length - index, &intPart);
        index += intResult;
        if (intResult == 0
            || index > length)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
            return 0;
        }
        temp = dataUtilsPower(10, (-1) * (int)(intResult));
        temp = (double)intPart * temp;
        result += temp;
    }

    if (index < length
        && (buffer[index] == PRV_EXPONENT_MIN
        || buffer[index] == PRV_EXPONENT_MAX))
    {
        int exponentSign;

        index++;
        if (index > length)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
            return 0;
        }
        exponentSign = prv_getSignFromBufferToNumber(buffer, &index);
        if (exponentSign == 0
            || index > length)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
            return 0;
        }

        intResult = prv_partBufferToInt(&buffer[index], length - index, &intPart);
        index += intResult;
        if (intResult == 0
            || index > length)
        {
            IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
            return 0;
        }
        intPart *= exponentSign;

        result = result * dataUtilsPower(10, intPart);
    }

    if (index != length)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }

    *dataP = result * sign;
    return 1;
}

size_t dataUtilsFloatToBufferLength(double data,
                                    bool withExponent)
{
    if (data > INT64_MAX || data < INT64_MIN)
    {
#ifdef IOWA_USE_SNPRINTF
        uint8_t buffer[64];

        if (withExponent == true)
        {
            return snprintf((char *)&buffer, 64, "%g", data);
        }
        else
        {
            return snprintf((char *)&buffer, 64, "%f", data);
        }
#else
        return 0;
#endif
    }
    else
    {
        size_t intLength;
        size_t decLength;
        int64_t intPart;
        double decPart;
        int exponent;

        if (withExponent == true)
        {
            exponent = prv_isExponentPossibleForDecPart(&data);
        }
        else
        {
            exponent = 0;
        }

        intPart = (int64_t)data;
        decPart = data - (double)intPart;
        if (decPart < 0)
        {
            decPart = 1 - decPart;
        }
        else
        {
            decPart = 1 + decPart;
        }

        if (decPart <= 1 + (double)FLT_EPSILON)
        {
            decPart = 0;
        }

        if (intPart == 0 && data < 0)
        {
            intLength = 2;
        }
        else
        {
            bool setExponent;

            if (withExponent == false
                || decPart >= (double)FLT_EPSILON)
            {
                setExponent = false;
            }
            else
            {
                setExponent = true;
            }
            intLength = dataUtilsIntToBufferLength(intPart, setExponent);
        }

        decLength = 0;
        if (decPart >= (double)FLT_EPSILON)
        {
            double noiseFloor;

            noiseFloor = (double)FLT_EPSILON;
            do
            {
                decPart *= 10;
                noiseFloor *= 10;
            } while (decPart - (double)((int64_t)decPart) > noiseFloor);

            decLength = prv_intToBufferLength((int64_t)decPart);
        }

        if (exponent != 0)
        {
            if (decLength != 0)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "exponent can not be present if decimal part is.");
                return 0;
            }
            intLength += 2;
            decLength = prv_intToBufferLength(exponent);
        }

        return intLength + decLength;
    }
}

size_t dataUtilsFloatToBuffer(double data,
                              uint8_t *buffer,
                              size_t length,
                              bool withExponent)
{
    if (data > INT64_MAX || data < INT64_MIN)
    {
#ifdef IOWA_USE_SNPRINTF
        if (withExponent == true)
        {
            return snprintf((char *)buffer, 64, "%g", data);
        }
        else
        {
            return snprintf((char *)buffer, 64, "%f", data);
        }
#else
        return 0;
#endif
    }
    else
    {
        size_t intLength;
        size_t decLength;
        int64_t intPart;
        double decPart;
        int exponent;

        if (withExponent == true)
        {
            exponent = prv_isExponentPossibleForDecPart(&data);
        }
        else
        {
            exponent = 0;
        }

        intPart = (int64_t)data;
        decPart = data - (double)intPart;
        if (decPart < 0)
        {
            decPart = 1 - decPart;
        }
        else
        {
            decPart = 1 + decPart;
        }

        if (decPart <= 1 + (double)FLT_EPSILON)
        {
            decPart = 0;
        }

        if (intPart == 0 && data < 0)
        {
            if (length < 4)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "buffer length too short.");
                return 0;
            }
            buffer[0] = '-';
            buffer[1] = '0';
            intLength = 2;
        }
        else
        {
            bool setExponent;

            if (withExponent == false
                || decPart >= (double)FLT_EPSILON)
            {
                setExponent = false;
            }
            else
            {
                setExponent = true;
            }
            intLength = dataUtilsIntToBuffer(intPart, buffer, length, setExponent);

            if (intLength == 0)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "failed to recover integer part.");
                return 0;
            }
        }
        decLength = 0;
        if (decPart >= (double)FLT_EPSILON)
        {
            double noiseFloor;

            if (intLength >= length - 1)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "buffer length too short.");
                return 0;
            }

            noiseFloor = (double)FLT_EPSILON;
            do
            {
                decPart *= 10;
                noiseFloor *= 10;
            } while (decPart - (double)((int64_t)decPart) > noiseFloor);

            decLength = prv_intToBuffer((int64_t)decPart, buffer + intLength, length - intLength);
            if (decLength <= 1)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "failed to recover decimal part.");
                return 0;
            }

            buffer[intLength] = '.';
        }

        if (exponent != 0)
        {
            if (length < intLength + decLength + 2)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "buffer length too short.");
                return 0;
            }
            buffer[intLength] = PRV_EXPONENT_MIN;
            intLength++;
            buffer[intLength] = PRV_MINUS_SIGN;
            intLength++;
            decLength = prv_intToBuffer(exponent, buffer + intLength, length - intLength);
            if (decLength == 0)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "Adding exponent not possible.");
                return 0;
            }
        }

        return intLength + decLength;
    }
}

size_t dataUtilsBufferToObjectLink(uint8_t *buffer,
                                   size_t bufferLength,
                                   iowa_lwm2m_data_t *dataP)
{
    size_t i;
    int64_t objectId;
    int64_t instanceId;

    i = 0;
    while (i < bufferLength
           && buffer[i] != PRV_OBJECT_LINK_TEXT_SEPARATOR)
    {
        i++;
    }
    if (i == bufferLength
        || i == bufferLength - 1)
    {
        return 0;
    }

    if (dataUtilsBufferToInt(buffer, i, &objectId) == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Text to Integer conversion failed");
        return 0;
    }
    if (objectId < 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer value is negative");
        return 0;
    }
    if (objectId > 65535)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "ObjectId is superior to 65535");
        return 0;
    }
    if (dataUtilsBufferToInt(buffer + i + 1, bufferLength - i - 1, &instanceId) == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Text to Integer conversion failed");
        return 0;
    }
    if (instanceId < 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer value is negative");
        return 0;
    }
    if (instanceId > 65535)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "InstanceId is superior to 65535");
        return 0;
    }

    dataP->value.asObjLink.objectId = (uint16_t)objectId;
    dataP->value.asObjLink.instanceId = (uint16_t)instanceId;
    dataP->type = IOWA_LWM2M_TYPE_OBJECT_LINK;

    return 1;
}

size_t dataUtilsObjectLinkToBufferLength(iowa_lwm2m_data_t *dataP)
{
    return dataUtilsIntToBufferLength(dataP->value.asObjLink.objectId, false) + 1 + dataUtilsIntToBufferLength(dataP->value.asObjLink.instanceId, false);
}

size_t dataUtilsObjectLinkToBuffer(iowa_lwm2m_data_t *dataP,
                                   uint8_t *buffer,
                                   size_t bufferLength)
{
    size_t nbLength;
    size_t resultBufferLength;

    nbLength = prv_intToBuffer(dataP->value.asObjLink.objectId, buffer, bufferLength);
    if (nbLength == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer to text conversion failed");
        return 0;
    }

    buffer[nbLength] = PRV_OBJECT_LINK_TEXT_SEPARATOR;
    resultBufferLength = nbLength + 1;

    nbLength = prv_intToBuffer(dataP->value.asObjLink.instanceId, buffer + resultBufferLength, bufferLength - resultBufferLength);
    if (nbLength == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer to text conversion failed");
        return 0;
    }

    resultBufferLength += nbLength;

    return resultBufferLength;
}

size_t dataUtilsBufferToUri(const char *buffer,
                            size_t bufferLength,
                            iowa_lwm2m_uri_t *uriP)
{
    size_t head;
    int readNum;

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "bufferLength: %u, buffer: \"%.*s\"", bufferLength, bufferLength, buffer);

    if (buffer == NULL || bufferLength == 0 || uriP == NULL)
    {
        return 0;
    }

    head = 0;
    LWM2M_URI_RESET(uriP);

    if (buffer[head] != '/')
    {
        return 0;
    }
    head++;
    if (head == bufferLength)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
        return head;
    }

    readNum = prv_parseUriNumber((const uint8_t *)buffer, bufferLength, &head);
    if (readNum < 0 || readNum > IOWA_LWM2M_ID_ALL)
    {
        return 0;
    }
    uriP->objectId = (uint16_t)readNum;

    if (head >= bufferLength)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
        return head;
    }
    if (buffer[head] == '/')
    {
        head += 1;
        if (head >= bufferLength)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
            return head;
        }
    }

    readNum = prv_parseUriNumber((const uint8_t *)buffer, bufferLength, &head);
    if (readNum < 0 || readNum >= IOWA_LWM2M_ID_ALL)
    {
        return 0;
    }
    uriP->instanceId = (uint16_t)readNum;

    if (head >= bufferLength)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
        return head;
    }
    if (buffer[head] == '/')
    {
        head += 1;
        if (head >= bufferLength)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
            return head;
        }
    }

    readNum = prv_parseUriNumber((const uint8_t *)buffer, bufferLength, &head);
    if (readNum < 0 || readNum >= IOWA_LWM2M_ID_ALL)
    {
        return 0;
    }
    uriP->resourceId = (uint16_t)readNum;

    if (head >= bufferLength)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
        return head;
    }
    if (buffer[head] == '/')
    {
        head += 1;
        if (head >= bufferLength)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
            return head;
        }
    }

    readNum = prv_parseUriNumber((const uint8_t *)buffer, bufferLength, &head);
    if (readNum < 0
        || readNum >= IOWA_LWM2M_ID_ALL)
    {
        return 0;
    }
    uriP->resInstanceId = (uint16_t)readNum;

    if (head != bufferLength)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "head: %d; bufferLength: %d", head, bufferLength);
        return 0;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);

    return head;
}

size_t dataUtilsUriToBuffer(iowa_lwm2m_uri_t *uriP,
                            uint8_t *buffer,
                            size_t bufferLength)
{
    size_t head;

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "bufferLength: %u.", bufferLength);

    buffer[0] = '/';
    head = 1;

    if (uriP != NULL
        && uriP->objectId != IOWA_LWM2M_ID_ALL)
    {
        size_t res;

        res = prv_intToBuffer(uriP->objectId, buffer + head, bufferLength - head);
        if (res == 0)
        {
            IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer to text conversion failed.");
            return 0;
        }
        head += res;
        if (head >= bufferLength)
        {
            IOWA_LOG_ERROR(IOWA_PART_DATA, "No enough space.");
            return 0;
        }

        if (uriP->instanceId != IOWA_LWM2M_ID_ALL)
        {
            buffer[head] = '/';
            head++;
            res = prv_intToBuffer(uriP->instanceId, buffer + head, bufferLength - head);
            if (res == 0)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer to text conversion failed.");
                return 0;
            }
            head += res;
            if (head >= bufferLength)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "No enough space.");
                return 0;
            }

            if (uriP->resourceId != IOWA_LWM2M_ID_ALL)
            {
                buffer[head] = '/';
                head++;
                res = prv_intToBuffer(uriP->resourceId, buffer + head, bufferLength - head);
                if (res == 0)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer to text conversion failed.");
                    return 0;
                }
                head += res;
                if (head >= bufferLength)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "No enough space.");
                    return 0;
                }

                if (uriP->resInstanceId != IOWA_LWM2M_ID_ALL)
                {
                    buffer[head] = '/';
                    head++;
                    res = prv_intToBuffer(uriP->resInstanceId, buffer + head, bufferLength - head);
                    if (res == 0)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_DATA, "Integer to text conversion failed.");
                        return 0;
                    }
                    head += res;
                    if (head >= bufferLength)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_DATA, "No enough space.");
                        return 0;
                    }
                }
            }
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "length: %u, buffer: \"%.*s\".", head, head, buffer);

    return head;
}

size_t dataUtilsUriToBufferLength(iowa_lwm2m_uri_t *uriP)
{
    size_t length;

    {
       length = 0;
    }

    switch (dataUtilsGetUriDepth(uriP))
    {
    case LWM2M_URI_DEPTH_RESOURCE_INSTANCE:
        length += dataUtilsIntToBufferLength(uriP->resInstanceId, false) + 1;
    case LWM2M_URI_DEPTH_RESOURCE:
        length += dataUtilsIntToBufferLength(uriP->resourceId, false) + 1;
    case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
        length += dataUtilsIntToBufferLength(uriP->instanceId, false) + 1;
    case LWM2M_URI_DEPTH_OBJECT:
        length += dataUtilsIntToBufferLength(uriP->objectId, false) + 1;
        break;
    case LWM2M_URI_DEPTH_ROOT:
        length = 1;
        break;

    default:
        break;
    }

    return length;
}

lwm2m_uri_depth_t dataUtilsGetUriDepth(iowa_lwm2m_uri_t *uriP)
{
    assert(uriP != NULL);

    if (uriP->objectId != IOWA_LWM2M_ID_ALL)
    {
        if (uriP->instanceId != IOWA_LWM2M_ID_ALL)
        {
            if (uriP->resourceId != IOWA_LWM2M_ID_ALL)
            {
                if (uriP->resInstanceId != IOWA_LWM2M_ID_ALL)
                {
                    return LWM2M_URI_DEPTH_RESOURCE_INSTANCE;
                }
                return LWM2M_URI_DEPTH_RESOURCE;
            }
            return LWM2M_URI_DEPTH_OBJECT_INSTANCE;
        }
        return LWM2M_URI_DEPTH_OBJECT;
    }
    return LWM2M_URI_DEPTH_ROOT;
}

iowa_status_t dataUtilsSetBuffer(uint8_t *buffer,
                                 size_t bufferLength,
                                 iowa_lwm2m_data_t *dataP,
                                 iowa_lwm2m_data_type_t type)
{
    assert(dataP != NULL);

    dataP->type = type;
    dataP->value.asBuffer.length = bufferLength;

    if (bufferLength == 0)
    {
        dataP->value.asBuffer.buffer = NULL;

        return IOWA_COAP_NO_ERROR;
    }

    assert(buffer != NULL);

    dataP->value.asBuffer.buffer = (uint8_t *)iowa_system_malloc(dataP->value.asBuffer.length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (dataP->value.asBuffer.buffer == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(dataP->value.asBuffer.length);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    memcpy(dataP->value.asBuffer.buffer, buffer, dataP->value.asBuffer.length);

    return IOWA_COAP_NO_ERROR;
}

bool dataUtilsGetBaseUri(iowa_lwm2m_data_t *dataP,
                         size_t size,
                         iowa_lwm2m_uri_t *uriP,
                         lwm2m_uri_depth_t *uriDepthP)
{
    size_t index;
    lwm2m_uri_depth_t currentUriDepth;
    lwm2m_uri_depth_t uriDepthToCompare;
    iowa_lwm2m_uri_t currentUri;

    assert(dataP != NULL);
    assert(uriP != NULL);
    assert(uriDepthP != NULL);

    uriP->objectId = dataP[0].objectID;
    uriP->instanceId = dataP[0].instanceID;
    uriP->resourceId = dataP[0].resourceID;
    uriP->resInstanceId = dataP[0].resInstanceID;

    uriDepthToCompare = dataUtilsGetUriDepth(uriP);

    for (index = 1; index < size; index++)
    {
        dataUtilsGetUri(&dataP[index], &currentUri);
        currentUriDepth = dataUtilsGetUriDepth(&currentUri);
        if (uriDepthToCompare > currentUriDepth)
        {
            uriDepthToCompare = currentUriDepth;
        }

        switch (dataUtilsGetUriDepth(uriP))
        {
        case LWM2M_URI_DEPTH_RESOURCE_INSTANCE:
            if (uriP->resInstanceId != dataP[index].resInstanceID)
            {
                uriP->resInstanceId = IOWA_LWM2M_ID_ALL;
            }
        case LWM2M_URI_DEPTH_RESOURCE:
            if (uriP->resourceId != dataP[index].resourceID)
            {
                uriP->resourceId = IOWA_LWM2M_ID_ALL;
            }
        case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
            if (uriP->instanceId != dataP[index].instanceID)
            {
                uriP->instanceId = IOWA_LWM2M_ID_ALL;
            }
        case LWM2M_URI_DEPTH_OBJECT:
            if (uriP->objectId != dataP[index].objectID)
            {
                uriP->objectId = IOWA_LWM2M_ID_ALL;
            }
            break;

        default:
            break;
        }
    }

    *uriDepthP = dataUtilsGetUriDepth(uriP);
    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "uriDepth: %d, uriDepthToCompare: %d.", *uriDepthP, uriDepthToCompare);
    if (uriDepthToCompare <= *uriDepthP)
    {
        return false;
    }
    return true;
}

bool dataUtilsIsInBaseUri(iowa_lwm2m_data_t *dataP,
                          iowa_lwm2m_uri_t *baseUriP,
                          lwm2m_uri_depth_t uriDepth)
{
    assert(dataP != NULL);
    assert(baseUriP != NULL);

    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_RESOURCE_INSTANCE:
        if (dataP->resInstanceID != baseUriP->resInstanceId)
        {
            return false;
        }
    case LWM2M_URI_DEPTH_RESOURCE:
        if (dataP->resourceID != baseUriP->resourceId)
        {
            return false;
        }
    case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
        if (dataP->instanceID != baseUriP->instanceId)
        {
            return false;
        }
    case LWM2M_URI_DEPTH_OBJECT:
        if (dataP->objectID != baseUriP->objectId)
        {
            return false;
        }
        break;

    default:
        break;
    }

    return true;
}

bool dataUtilsIsEqualUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP)
{
    if (uriP->objectId == dataP->objectID
        && uriP->instanceId == dataP->instanceID
        && uriP->resourceId == dataP->resourceID
        && uriP->resInstanceId == dataP->resInstanceID)
    {
        return true;
    }
    return false;
}

void dataUtilsGetUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP)
{
    uriP->objectId = dataP->objectID;
    uriP->instanceId = dataP->instanceID;
    uriP->resourceId = dataP->resourceID;
    uriP->resInstanceId = dataP->resInstanceID;
}

void dataUtilsSetUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP)
{
    dataP->objectID = uriP->objectId;
    dataP->instanceID = uriP->instanceId;
    dataP->resourceID = uriP->resourceId;
    dataP->resInstanceID = uriP->resInstanceId;
    dataP->type = IOWA_LWM2M_TYPE_URI_ONLY;
}

iowa_status_t dataUtilsConvertUndefinedValue(uint8_t *bufferP,
                                             size_t bufferLength,
                                             iowa_lwm2m_data_t *dataP,
                                             iowa_content_format_t format,
                                             data_resource_type_callback_t resTypeCb,
                                             void *userDataP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Entering: bufferP: %p, bufferLength: %zu, dataP: %p, format: %s.", bufferP, bufferLength, dataP, STR_MEDIA_TYPE(format));

    assert(dataP != NULL);

    if (resTypeCb != NULL)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Calling resource type callback with object ID: %d and resource ID: %d.", dataP->objectID, dataP->resourceID);
        dataP->type = resTypeCb(dataP->objectID, dataP->resourceID, userDataP);
    }
    else
    {
        dataP->type = IOWA_LWM2M_TYPE_UNDEFINED;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Retrieved data type: %s.", STR_LWM2M_TYPE(dataP->type));

    switch (format)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
        switch (dataP->type)
        {
        case IOWA_LWM2M_TYPE_UNSIGNED_INTEGER:
            if (dataUtilsBufferToInt(bufferP, bufferLength, &dataP->value.asInteger) == 0)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the integer value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            if (dataP->value.asInteger < 0)
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_DATA, "Integer value is signed: %d.", dataP->value.asInteger);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_INTEGER:
        case IOWA_LWM2M_TYPE_TIME:
            if (dataUtilsBufferToInt(bufferP, bufferLength, &dataP->value.asInteger) == 0)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the integer value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_FLOAT:
            if (dataUtilsBufferToFloat(bufferP, bufferLength, &dataP->value.asFloat) == 0)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the float value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_BOOLEAN:
            switch (bufferLength)
            {
            case 1:
                switch (bufferP[0])
                {
                case '0':
                    dataP->value.asBoolean = false;
                    break;

                case '1':
                    dataP->value.asBoolean = true;
                    break;

                default:
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the boolean value.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
                break;

            default:
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the boolean value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_OBJECT_LINK:
        {
            size_t result;
            size_t colonPos;
            int64_t intFromBuffer;

            if (bufferLength < 3
                || bufferLength > 11)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Invalid string Object Link.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }

            for (colonPos = 0; colonPos < bufferLength; colonPos++)
            {
                if (bufferP[colonPos] == ':')
                {
                    break;
                }
            }
            if (colonPos == 0
                || colonPos == bufferLength
                || colonPos > 5 
                || bufferLength - 1 - colonPos > 5)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Invalid string Object Link.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }

            result = dataUtilsBufferToInt(bufferP, colonPos, &intFromBuffer);
            if (result == 0
                || intFromBuffer < 0
                || intFromBuffer > 65535)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Invalid left integer part for the Object Link.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            dataP->value.asObjLink.objectId = (uint16_t)intFromBuffer;

            result = dataUtilsBufferToInt(bufferP + colonPos + 1, bufferLength - 1 - colonPos, &intFromBuffer);
            if (result == 0
                || intFromBuffer < 0
                || intFromBuffer > 65535)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Invalid right integer part for the Object Link.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            dataP->value.asObjLink.instanceId = (uint16_t)intFromBuffer;

            break;
        }

        case IOWA_LWM2M_TYPE_OPAQUE:
            if (bufferLength > 0)
            {
                size_t decodedSize;

                decodedSize = iowa_utils_base64_get_decoded_size(bufferP, bufferLength);
                if (decodedSize == 0)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to retrieved the Base64 decoded buffer length.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                dataP->value.asBuffer.buffer = (uint8_t *)iowa_system_malloc(decodedSize);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                if (dataP->value.asBuffer.buffer == NULL)
                {
                    IOWA_LOG_ERROR_MALLOC(decodedSize);
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
#endif

                dataP->value.asBuffer.length = iowa_utils_base64_decode(bufferP, bufferLength, dataP->value.asBuffer.buffer, decodedSize);
                if (dataP->value.asBuffer.length != decodedSize)
                {
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to decode the Base64 buffer.");
                    iowa_system_free(dataP->value.asBuffer.buffer);
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
            }
            break;

        case IOWA_LWM2M_TYPE_STRING:
        case IOWA_LWM2M_TYPE_CORE_LINK:
        case IOWA_LWM2M_TYPE_UNDEFINED:
            dataP->value.asBuffer.length = prv_bufferCopy(bufferP, bufferLength, &dataP->value.asBuffer.buffer);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (dataP->value.asBuffer.length == 0
                && bufferLength != 0)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            break;

        default:
            IOWA_LOG_ERROR(IOWA_PART_DATA, "Unrecognized or unsupported data type for the current format.");
            return IOWA_COAP_402_BAD_OPTION;
        }
        break;

    case IOWA_CONTENT_FORMAT_OPAQUE:
        switch (dataP->type)
        {
        case IOWA_LWM2M_TYPE_OPAQUE:
        case IOWA_LWM2M_TYPE_UNDEFINED:
            dataP->value.asBuffer.length = prv_bufferCopy(bufferP, bufferLength, &dataP->value.asBuffer.buffer);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (dataP->value.asBuffer.length == 0
                && bufferLength != 0)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            break;

        default:
            IOWA_LOG_ERROR(IOWA_PART_DATA, "Unrecognized or unsupported data type for the current format.");
            return IOWA_COAP_402_BAD_OPTION;
        }
        break;

    case IOWA_CONTENT_FORMAT_TLV:
        switch (dataP->type)
        {
        case IOWA_LWM2M_TYPE_INTEGER:
        case IOWA_LWM2M_TYPE_TIME:
        case IOWA_LWM2M_TYPE_UNSIGNED_INTEGER:
            switch (bufferLength)
            {
            case 1:
                dataP->value.asInteger = (int8_t)bufferP[0];
                break;

            case 2:
            {
                int16_t value;

                utilsCopyValue(&value, bufferP, bufferLength);

                dataP->value.asInteger = value;
                break;
            }

            case 4:
            {
                int32_t value;

                utilsCopyValue(&value, bufferP, bufferLength);

                dataP->value.asInteger = value;
                break;
            }

            case 8:
                utilsCopyValue(&dataP->value.asInteger, bufferP, bufferLength);
                break;

            default:
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the integer value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }

            if (dataP->type == IOWA_LWM2M_TYPE_UNSIGNED_INTEGER
                && dataP->value.asInteger < 0)
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_DATA, "Integer value is signed: %d.", dataP->value.asInteger);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_FLOAT:
            switch (bufferLength)
            {
            case 4:
            {
                float temp;

                utilsCopyValue(&temp, bufferP, bufferLength);

                dataP->value.asFloat = temp;
            }
            break;

            case 8:
                utilsCopyValue(&dataP->value.asFloat, bufferP, bufferLength);
                break;

            default:
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the float value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_BOOLEAN:
            switch (bufferLength)
            {
            case 1:
                switch (bufferP[0])
                {
                case 0:
                    dataP->value.asBoolean = false;
                    break;

                case 1:
                    dataP->value.asBoolean = true;
                    break;

                default:
                    IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the boolean value.");
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
                break;

            default:
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the boolean value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_OBJECT_LINK:
            switch (bufferLength)
            {
            case 4:
                dataP->value.asObjLink.objectId = (uint16_t)((bufferP[0] << 8) | bufferP[1]);
                dataP->value.asObjLink.instanceId = (uint16_t)((bufferP[2] << 8) | bufferP[3]);
                break;

            default:
                IOWA_LOG_ERROR(IOWA_PART_DATA, "Failed to convert back the Object Link value.");
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case IOWA_LWM2M_TYPE_OPAQUE:
        case IOWA_LWM2M_TYPE_STRING:
        case IOWA_LWM2M_TYPE_CORE_LINK:
        case IOWA_LWM2M_TYPE_UNDEFINED:
            dataP->value.asBuffer.length = prv_bufferCopy(bufferP, bufferLength, &dataP->value.asBuffer.buffer);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (dataP->value.asBuffer.length == 0
                && bufferLength != 0)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            break;

        default:
            IOWA_LOG_ERROR(IOWA_PART_DATA, "Unrecognized or unsupported data type for the current format.");
            return IOWA_COAP_402_BAD_OPTION;
        }
        break;

    default:
        IOWA_LOG_ERROR(IOWA_PART_DATA, "Unrecognized format.");
        return IOWA_COAP_402_BAD_OPTION;
    }

    return IOWA_COAP_NO_ERROR;
}

size_t dataSkipBufferSpace(const uint8_t *bufferP,
                           size_t bufferLength)
{
    size_t i;

    assert(bufferP != NULL || (bufferLength == 0 && bufferP == NULL));

    i = 0;
    while (i < bufferLength
           && isspace(bufferP[i]) != 0)
    {
        i++;
    }

    return i;
}

bool dataUtilsCompareFloatingPointNumbers(double num1,
                                          double num2,
                                          double epsilon)
{
    prv_double_convert_t ulp;
    prv_double_convert_t ulp1;
    prv_double_convert_t ulp2;

    assert(epsilon >= 0.f);

    if (num1 == num2)
    {
        return true;
    }

    ulp1.convert.asDouble = num1;
    ulp2.convert.asDouble = num2;

    if (((ulp1.convert.asInteger & PRV_FLOAT_VALUE_INFINITY_OR_NAN_MASK) == PRV_FLOAT_VALUE_INFINITY_OR_NAN_MASK
        || (ulp2.convert.asInteger & PRV_FLOAT_VALUE_INFINITY_OR_NAN_MASK) == PRV_FLOAT_VALUE_INFINITY_OR_NAN_MASK)
        || ((ulp1.convert.asInteger < 0) != (ulp2.convert.asInteger < 0)))
    {
        return false;
    }
    else
    {
        if (ulp2.convert.asDouble < ulp1.convert.asDouble)
        {
            ulp.convert.asDouble = ulp1.convert.asDouble - ulp2.convert.asDouble;
        }
        else
        {
            ulp.convert.asDouble = ulp2.convert.asDouble - ulp1.convert.asDouble;
        }
    }

    return ulp.convert.asDouble <= epsilon;
}
