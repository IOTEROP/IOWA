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

    assert(uriString != NULL);
    assert(headP != NULL);

    if (uriString[*headP] == '/')
    {
        // empty Object Instance ID with resource ID is not allowed
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

// Get buffer part into integer
// Returned value: index to when the part stopped (when buffer contains a character not between '0' and '9')
// Parameters:
// - buffer, length: buffer to look
// - dataP: OUT. integer got.
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

// Get buffer part into integer
// Returned value: sign value (-1 or 1) or 0 if error
// Parameters:
// - buffer: buffer to look
// - indexP: INOUT. buffer index to look.
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

// change value and indicate exponent if it is possible for integer value
// return exponent if possible or 0
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


// change value and indicate exponent if it is possible for decimal part of value
// return exponent if possible or 0
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

// get the length of the conversion of an integer into a buffer
// return the length of the conversion
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

// Convert an integer value (data) into a buffer
// return the length of the conversion
static size_t prv_intToBuffer(int64_t data,
                              uint8_t *buffer,
                              size_t length)
{
    int index;
    bool minus;
    size_t result;

    assert(buffer != NULL);
    assert(length != 0);

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
        // Out of the buffer
        return 0;
    }

    if (minus == true)
    {
        if (index < 0)
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
        assert(number != 0.);

        power *= (-1);
        number = 1/number;
    }

    for (ind = 0; ind < power; ind++)
    {
        result *= number;
    }

    return result;
}

size_t dataUtilsBufferToInt(uint8_t *buffer,
                            size_t length,
                            int64_t *dataP)
{
    int64_t result;
    int sign;
    size_t index;

    assert(dataP != NULL);
    assert((buffer != NULL && length != 0) || length == 0);

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

    assert(buffer != NULL && length != 0);

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

    assert(dataP != NULL);
    assert((buffer != NULL && length != 0) || length == 0);

    if (0 == length)
    {
        IOWA_LOG_TRACE(IOWA_PART_DATA, "Not a number.");
        return 0;
    }

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
    if (data > (double)INT64_MAX
        || data < (double)INT64_MIN)
    {
        IOWA_LOG_WARNING(IOWA_PART_DATA, "Number is too large, set IOWA_USE_SNPRINTF to serialize it without error.");
        return 0;
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
            // deal with numbers between -1 and 0
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
            // Call the utils function to take exponent in account
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
            } while (decPart - (double)((int64_t)decPart) > noiseFloor);  // Compare the integer part of decPart with possible noise

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
    assert(buffer != NULL && length != 0);

    if (data > (double)INT64_MAX
        || data < (double)INT64_MIN)
    {
        return 0;
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
            // deal with numbers between -1 and 0
            if (length < 4)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "buffer length too short.");
                return 0;   // "-0.n"
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
            // Call the utils function to take exponent in account
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
            } while (decPart - (double)((int64_t)decPart) > noiseFloor); // Compare the integer part of decPart with possible noise

            decLength = prv_intToBuffer((int64_t)decPart, buffer + intLength, length - intLength);
            if (decLength <= 1)
            {
                IOWA_LOG_TRACE(IOWA_PART_DATA, "failed to recover decimal part.");
                return 0;
            }

            // replace the leading 1 with a dot
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

    assert((buffer != NULL && bufferLength != 0) || bufferLength == 0);
    assert(dataP != NULL);

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

size_t dataUtilsObjectLinkToBuffer(iowa_lwm2m_data_t *dataP,
                                   uint8_t *buffer,
                                   size_t bufferLength)
{
    size_t nbLength;
    size_t resultBufferLength;

    assert(dataP != NULL);
    assert(buffer != NULL && bufferLength != 0);

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

#ifdef LWM2M_ALTPATH_SUPPORT
size_t dataUtilsBufferToUri(const char *buffer,
                            size_t bufferLength,
                            iowa_lwm2m_uri_t *uriP,
                            char **altPathP)
#else
size_t dataUtilsBufferToUri(const char *buffer,
                            size_t bufferLength,
                            iowa_lwm2m_uri_t *uriP)
#endif
{
    size_t head;
    int readNum;

    assert((buffer != NULL && bufferLength != 0) || bufferLength == 0);
    assert(uriP != NULL);

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "bufferLength: %u, buffer: \"%.*s\"", bufferLength, bufferLength, buffer);

    if (bufferLength == 0)
    {
        return 0;
    }

    // Skip any white space
    head = 0;
    LWM2M_URI_RESET(uriP);

    // Check the URI starts with a '/'
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
#ifdef LWM2M_ALTPATH_SUPPORT
    if (altPathP != NULL)
    {
        // Check alternate path
        if (*altPathP == NULL)
        {
            // Check if there is an alternate path
            size_t bufferIndex;
            bool altPathFound;

            altPathFound = false;

            for (bufferIndex = head; bufferIndex < bufferLength; bufferIndex++)
            {
                if (buffer[bufferIndex] == '/')
                {
                    break;
                }

                if (altPathFound == false
                    && (buffer[bufferIndex] < '0' || buffer[bufferIndex] > '9'))
                {
                    // This is not an integral number URI
                    altPathFound = true;
                }
            }

            if (altPathFound == true)
            {
                *altPathP = (char *)iowa_system_malloc(bufferIndex - head + 1);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                if (*altPathP == NULL)
                {
                    IOWA_LOG_ERROR_MALLOC(bufferIndex - head + 1);
                    return 0;
                }
#endif
                memcpy(*altPathP, buffer + head, bufferIndex - head);
                (*altPathP)[bufferIndex - head] = '\0';

                head = bufferIndex + 1;
                if (head >= bufferLength)
                {
                    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
                    return head;
                }
            }
        }
        else
        {
            // Check if the alternate path is present
            size_t altPathLength;

            altPathLength = strlen(*altPathP);
            if (head + altPathLength > bufferLength
                || memcmp(buffer + head, *altPathP, altPathLength) != 0)
            {
                return 0;
            }
            head += altPathLength;
            if (head == bufferLength)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Parsed characters: %u", head);
                return head;
            }

            // Check the URI begins with a '/'
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
        }
    }
#endif

    // Read object ID
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

    // Read instance ID
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

    // Read resource ID
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

    // Read resource instance ID
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

#ifdef LWM2M_ALTPATH_SUPPORT
size_t dataUtilsUriToBuffer(iowa_lwm2m_uri_t *uriP,
                            const char *altPath,
                            uint8_t *buffer,
                            size_t bufferLength)
#else
size_t dataUtilsUriToBuffer(iowa_lwm2m_uri_t *uriP,
                            uint8_t *buffer,
                            size_t bufferLength)
#endif
{
    size_t head;

    assert(uriP != NULL);
    assert(buffer != NULL && bufferLength != 0);

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "bufferLength: %u.", bufferLength);

    buffer[0] = '/';
    head = 1;

#ifdef LWM2M_ALTPATH_SUPPORT
    if (altPath != NULL)
    {
        size_t altPathLength;

        altPathLength = strlen(altPath);
        if (altPath[altPathLength - 1] == '/')
        {
            altPathLength--;
        }
        if (altPath[0] == '/')
        {
            altPath += 1;
            altPathLength--;
        }
        if (head + altPathLength >= bufferLength)
        {
            IOWA_LOG_ERROR(IOWA_PART_DATA, "No enough space.");
            return 0;
        }
        memcpy(buffer + head, altPath, altPathLength);
        head += altPathLength;
    }
#endif

    if (uriP != NULL
        && uriP->objectId != IOWA_LWM2M_ID_ALL)
    {
        size_t res;

#ifdef LWM2M_ALTPATH_SUPPORT
        if (altPath != NULL)
        {
            if (head + 1 >= bufferLength)
            {
                IOWA_LOG_ERROR(IOWA_PART_DATA, "No enough space.");
                return 0;
            }

            buffer[head] = '/';
            head++;
        }
#endif

        // Add the Object ID
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
            // Add the Instance Object ID
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
                // Add the Resource ID
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
                    // Add the Resource Instance ID
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

#ifdef LWM2M_ALTPATH_SUPPORT
size_t dataUtilsUriToBufferLength(iowa_lwm2m_uri_t *uriP,
                                  const char *altPath)
#else
size_t dataUtilsUriToBufferLength(iowa_lwm2m_uri_t *uriP)
#endif
{
    size_t length;
    assert(uriP != NULL);

#ifdef LWM2M_ALTPATH_SUPPORT
    if (altPath != NULL)
    {
        length = strlen(altPath);
        if (length >= 2 && altPath[length - 1] == '/')
        {
            length--;
        }
        if (altPath[0] != '/')
        {
            length++;
        }
    }
    else
#endif
    {
       length = 0;
    }

    switch (dataUtilsGetUriDepth(uriP))
    {
    case LWM2M_URI_DEPTH_RESOURCE_INSTANCE:
        length += dataUtilsIntToBufferLength(uriP->resInstanceId, false) + 1;
        // Fall through
    case LWM2M_URI_DEPTH_RESOURCE:
        length += dataUtilsIntToBufferLength(uriP->resourceId, false) + 1;
        // Fall through
    case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
        length += dataUtilsIntToBufferLength(uriP->instanceId, false) + 1;
        // Fall through
    case LWM2M_URI_DEPTH_OBJECT:
        length += dataUtilsIntToBufferLength(uriP->objectId, false) + 1;
        break;
    case LWM2M_URI_DEPTH_ROOT:
#ifdef LWM2M_ALTPATH_SUPPORT
        if (length == 0)
        {
            length = 1;
        }
#else
        length = 1;
#endif
        break;

    default:
        // Do nothing
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
    assert((buffer != NULL && bufferLength != 0) || bufferLength == 0);

    dataP->type = type;
    dataP->value.asBuffer.length = bufferLength;

    if (bufferLength == 0)
    {
        dataP->value.asBuffer.buffer = NULL;

        return IOWA_COAP_NO_ERROR;
    }

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

    assert(dataP != NULL && size != 0);
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
            // Fall through
        case LWM2M_URI_DEPTH_RESOURCE:
            if (uriP->resourceId != dataP[index].resourceID)
            {
                uriP->resourceId = IOWA_LWM2M_ID_ALL;
            }
            // Fall through
        case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
            if (uriP->instanceId != dataP[index].instanceID)
            {
                uriP->instanceId = IOWA_LWM2M_ID_ALL;
            }
            // Fall through
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
        // Fall through
    case LWM2M_URI_DEPTH_RESOURCE:
        if (dataP->resourceID != baseUriP->resourceId)
        {
            return false;
        }
        // Fall through
    case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
        if (dataP->instanceID != baseUriP->instanceId)
        {
            return false;
        }
        // Fall through
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
    assert(dataP != NULL);
    assert(uriP != NULL);

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
    assert(dataP != NULL);
    assert(uriP != NULL);

    uriP->objectId = dataP->objectID;
    uriP->instanceId = dataP->instanceID;
    uriP->resourceId = dataP->resourceID;
    uriP->resInstanceId = dataP->resInstanceID;
}

bool dataUtilsCompareFloatingPointNumbers(double num1,
                                          double num2)
{
    // Check if the number are already equals
    if (num1 == num2)
    {
        return true;
    }

    return (((num1 - num2) <= FLT_EPSILON)
            && ((num1-num2) >= -FLT_EPSILON));
}


/**************************************************************
 * Half float conversion
 **************************************************************/
