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
* Copyright (c) 2018-2020 IoTerop.
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

/*************************************************************************************
** Private functions
*************************************************************************************/

// Use Insertion Algorithm to sort iowa_lwm2m_data_t
// Returned value: None.
// Parameters:
// - dataCount: data Array size.
// - dataArrayP: data to sort.
// - level: level to sort data.
static void prv_dataInsertionSortSmallToLarge(size_t dataCount,
                                              iowa_lwm2m_data_t *dataArrayP)
{
    size_t indTolook;
    size_t indToCompare;
    iowa_lwm2m_data_t dataCurrent;

    assert((dataArrayP != NULL && dataCount != 0) || dataCount == 0);

    for (indTolook = 0; indTolook < dataCount; indTolook++)
    {
        memcpy(&dataCurrent, &dataArrayP[indTolook], sizeof(iowa_lwm2m_data_t));

        for (indToCompare = indTolook; indToCompare > 0; indToCompare--)
        {
            if (dataArrayP[indToCompare - 1].objectID < dataCurrent.objectID
                || (dataArrayP[indToCompare - 1].objectID == dataCurrent.objectID && (dataArrayP[indToCompare - 1].instanceID <= dataCurrent.instanceID)))
            {
                break;
            }

            memcpy(&dataArrayP[indToCompare], &dataArrayP[indToCompare - 1], sizeof(iowa_lwm2m_data_t));
        }

        memcpy(&dataArrayP[indToCompare], &dataCurrent, sizeof(iowa_lwm2m_data_t));
    }
}

/*************************************************************************************
** Public functions
*************************************************************************************/

void dataLwm2mFree(size_t dataCount,
                   iowa_lwm2m_data_t *dataArrayP)
{
    size_t i;

    assert((dataArrayP != NULL && dataCount != 0) || (dataCount == 0));

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Entering: dataCount: %zu, dataArrayP: %p.", dataCount, dataArrayP);

    for (i = 0; i < dataCount; i++)
    {
        if (IOWA_LWM2M_TYPE_UNDEFINED == dataArrayP[i].type
            || IOWA_LWM2M_TYPE_CORE_LINK == dataArrayP[i].type
            || IOWA_LWM2M_TYPE_STRING == dataArrayP[i].type
            || IOWA_LWM2M_TYPE_OPAQUE == dataArrayP[i].type)
        {
            iowa_system_free(dataArrayP[i].value.asBuffer.buffer);
        }
    }

    iowa_system_free(dataArrayP);
}

iowa_status_t dataLwm2mSerialize(iowa_lwm2m_uri_t *baseUriP,
                                 iowa_lwm2m_data_t *dataP,
                                 size_t dataCount,
                                 iowa_content_format_t *contentFormatP,
                                 uint8_t **bufferP,
                                 size_t *bufferLengthP)
{
    iowa_status_t result;
    iowa_lwm2m_data_t *sortedDataP;
    size_t sortedDataCount;

#if !defined(LWM2M_SUPPORT_TLV) && !defined(LWM2M_SUPPORT_JSON)
    (void)baseUriP;
#endif

    assert(contentFormatP != NULL);
    assert(bufferP != NULL);
    assert(bufferLengthP != NULL);
    assert((dataP != NULL && dataCount != 0) || (dataCount == 0));

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Entering: dataP: %p, dataCount: %u, contentFormatP: %s.", dataP, dataCount, STR_MEDIA_TYPE(*contentFormatP));

    *bufferP = NULL;
    *bufferLengthP = 0;

    // Check arguments
    if (dataCount == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_DATA, "No data to serialize. Exiting.");
        return IOWA_COAP_NO_ERROR;
    }

    // Check content format
    switch (*contentFormatP)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
    case IOWA_CONTENT_FORMAT_OPAQUE:
        if (dataCount != 1)
        {
            *contentFormatP = LWM2M_DEFAULT_CONTENT_FORMAT;
            IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Single content format cannot be used for multiple resources. New content format: %s.", STR_MEDIA_TYPE(*contentFormatP));
        }
        else if (dataP[0].resourceID == IOWA_LWM2M_ID_ALL
                 || (*contentFormatP == IOWA_CONTENT_FORMAT_OPAQUE && dataP[0].type != IOWA_LWM2M_TYPE_OPAQUE)
                 || (dataP[0].resInstanceID != IOWA_LWM2M_ID_ALL && baseUriP != NULL && baseUriP->resInstanceId == IOWA_LWM2M_ID_ALL))
        {
            *contentFormatP = LWM2M_DEFAULT_CONTENT_FORMAT;
            IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Requested format is not acceptable. New content format: %s.", STR_MEDIA_TYPE(*contentFormatP));
        }
        break;

#ifdef LWM2M_SUPPORT_TLV
    case IOWA_CONTENT_FORMAT_TLV_OLD:
    case IOWA_CONTENT_FORMAT_TLV:
        break;
#endif

    default:
        *contentFormatP = LWM2M_DEFAULT_CONTENT_FORMAT;
        IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "New content format: %s.", STR_MEDIA_TYPE(*contentFormatP));
    }

    // Check the data
    sortedDataP = dataP;
    sortedDataCount = dataCount;

    // Serialize the data
    if (IOWA_CONTENT_FORMAT_TEXT == *contentFormatP)
    {
        result = textSerialize(sortedDataP, bufferP, bufferLengthP);
    }
    else if (IOWA_CONTENT_FORMAT_OPAQUE == *contentFormatP)
    {
        result = opaqueSerialize(sortedDataP, bufferP, bufferLengthP);
    }
#ifdef LWM2M_SUPPORT_TLV
    else if (IOWA_CONTENT_FORMAT_TLV_OLD == *contentFormatP
             || IOWA_CONTENT_FORMAT_TLV == *contentFormatP)
    {
        result = tlvSerialize(baseUriP, sortedDataP, sortedDataCount, bufferP, bufferLengthP);
    }
#endif
    else
    {
        // Should not happen
        result = IOWA_COAP_400_BAD_REQUEST;
    }

    if (result != IOWA_COAP_NO_ERROR)
    {
        *bufferP = NULL;
        *bufferLengthP = 0;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Exiting with result: %u.%02u, bufferP: %p, bufferLengthP: %zu.", (result & 0xFF) >> 5, (result & 0x1F), *bufferP, *bufferLengthP);

    return result;
}

iowa_status_t dataLwm2mDeserialize(iowa_lwm2m_uri_t *baseUriP,
                                   uint8_t *bufferP,
                                   size_t bufferLength,
                                   iowa_content_format_t contentFormat,
                                   iowa_lwm2m_data_t **dataP,
                                   size_t *dataCountP,
                                   data_resource_type_callback_t resTypeCb,
                                   void *userDataP)
{
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Entering: bufferP: %p, bufferLength: %u, contentFormat: %s.", bufferP, bufferLength, STR_MEDIA_TYPE(contentFormat));

    assert(dataP != NULL);
    assert(dataCountP != NULL);

    *dataP = NULL;
    *dataCountP = 0;

    switch (contentFormat)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
        result = textDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP);
        break;

    case IOWA_CONTENT_FORMAT_OPAQUE:
        result = opaqueDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP);
        break;

#ifdef LWM2M_SUPPORT_TLV
    case IOWA_CONTENT_FORMAT_TLV_OLD:
    case IOWA_CONTENT_FORMAT_TLV:
        result = tlvDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP);
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_DATA, "Content format %s is not supported.", STR_MEDIA_TYPE(contentFormat));
        result = IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT;
    }

    if (IOWA_COAP_NO_ERROR == result)
    {
        result = dataLwm2mConsolidate(*dataCountP, *dataP, contentFormat, resTypeCb, userDataP);
    }

    prv_dataInsertionSortSmallToLarge(*dataCountP, *dataP);

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Exiting with result: %u.%02u, dataP: %p, dataCountP: %zu.", (result & 0xFF) >> 5, (result & 0x1F), *dataP, *dataCountP);

    return result;
}

iowa_status_t dataLwm2mConsolidate(size_t dataCount,
                                   iowa_lwm2m_data_t *dataArray,
                                   iowa_content_format_t contentFormat,
                                   data_resource_type_callback_t resTypeCb,
                                   void *userDataP)
{
    size_t i;
    double epsilon;
    double delta;
    uint8_t *tmpP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "Entering. dataCount: %u, dataArrayP: %p, resTypeCb: %p.", dataCount, dataArray, resTypeCb);

    for (i = 0; i < dataCount; i++)
    {
        iowa_lwm2m_data_type_t type;

        tmpP = NULL;

        if (IOWA_LWM2M_TYPE_URI_ONLY == dataArray[i].type)
        {
            continue;
        }

        if (IOWA_LWM2M_ID_ALL == dataArray[i].resourceID)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u does not have the resource ID set.", i);
            goto exit_error;
        }

        if (IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == dataArray[i].type
            && dataArray[i].value.asInteger < 0)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u does not have a avlid unsigned integer value.", i);
            goto exit_error;
        }

        if (NULL == resTypeCb)
        {
            type = IOWA_LWM2M_TYPE_UNDEFINED;
            IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "No expected type for /%d/%d/%d/%d.", dataArray[i].objectID, dataArray[i].instanceID, dataArray[i].resourceID, dataArray[i].resInstanceID);
        }
        else
        {
            type = resTypeCb(dataArray[i].objectID, dataArray[i].resourceID, userDataP);
            IOWA_LOG_ARG_TRACE(IOWA_PART_DATA, "For /%d/%d/%d/%d expected type is: %s and current type is: %s.", dataArray[i].objectID, dataArray[i].instanceID, dataArray[i].resourceID, dataArray[i].resInstanceID, STR_LWM2M_TYPE(type), STR_LWM2M_TYPE(dataArray[i].type));
        }

        if (dataArray[i].type == type)
        {
            continue;
        }

        if (IOWA_LWM2M_TYPE_STRING == type)
        {
            if (IOWA_LWM2M_TYPE_NULL == dataArray[i].type)
            {
                dataArray[i].type = IOWA_LWM2M_TYPE_STRING;
                dataArray[i].value.asBuffer.length = 0;
                dataArray[i].value.asBuffer.buffer = NULL;
            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                dataArray[i].type = IOWA_LWM2M_TYPE_STRING;
            }
#endif
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not a string as expected.", i);
                goto exit_error;
            }
        }
        else if (IOWA_LWM2M_TYPE_OPAQUE == type)
        {
            if (IOWA_LWM2M_TYPE_NULL == dataArray[i].type)
            {
                dataArray[i].value.asBuffer.length = 0;
                dataArray[i].value.asBuffer.buffer = NULL;
            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                dataArray[i].type = IOWA_LWM2M_TYPE_OPAQUE;
            }
#endif
            else if (IOWA_LWM2M_TYPE_STRING == dataArray[i].type)
            {
                if (dataArray[i].value.asBuffer.length > 0)
                {
                    size_t decodedSize;

                    decodedSize = utils_b64GetDecodedSize(dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length, true);
                    if (decodedSize == 0)
                    {
                        IOWA_LOG_INFO(IOWA_PART_DATA, "Failed to retrieve the Base64 decoded buffer length.");
                        goto exit_error;;
                    }

                    tmpP = dataArray[i].value.asBuffer.buffer;
                    dataArray[i].value.asBuffer.buffer = (uint8_t *)iowa_system_malloc(decodedSize);
    #ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                    if (dataArray[i].value.asBuffer.buffer == NULL)
                    {
                        IOWA_LOG_ERROR_MALLOC(decodedSize);
                        goto exit_error;;
                    }
    #endif

                    dataArray[i].value.asBuffer.length = utils_b64Decode(tmpP, dataArray[i].value.asBuffer.length, dataArray[i].value.asBuffer.buffer, BASE64_MODE_CLASSIC);
                    if (dataArray[i].value.asBuffer.length != decodedSize)
                    {
                        IOWA_LOG_INFO(IOWA_PART_DATA, "Failed to decode the Base64 buffer.");
                        goto exit_error;
                    }
                }
            }
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an opaque as expected.", i);
                goto exit_error;
            }

            dataArray[i].type = IOWA_LWM2M_TYPE_OPAQUE;

        }
        else if (IOWA_LWM2M_TYPE_INTEGER == type
                || IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == type
                || IOWA_LWM2M_TYPE_TIME == type)
        {
            if (IOWA_LWM2M_TYPE_FLOAT == dataArray[i].type)
            {
                // 32 bits floating point has 23 bits of mantissa ; we accept a maximum precision loss of about a millionth of the value
                epsilon = dataArray[i].value.asFloat / (1024L*1024);

                // Let's use absolute values
                if (epsilon < 0)
                {
                    epsilon = -epsilon;
                }

                // Cap our acceptable precision loss, for large values
                if (epsilon > 0.5)
                {
                    epsilon = 0.5;
                }

                delta = ((double)(int64_t)dataArray[i].value.asFloat) - dataArray[i].value.asFloat;

                if (delta < 0)
                {
                    delta = -delta;
                }

                // If the conversion is precise enough
                if (delta <= epsilon)
                {
                    dataArray[i].value.asInteger = (int64_t)dataArray[i].value.asFloat;
                    dataArray[i].type = IOWA_LWM2M_TYPE_INTEGER;
                }
                else
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Float value %lf of Data #%u is not an integer as expected.", dataArray[i].value.asFloat, i);
                    goto exit_error;
                }
            }
            else if (IOWA_LWM2M_TYPE_STRING == dataArray[i].type)
            {
                int64_t value;

                if (dataUtilsBufferToInt(dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length, &value) == 0)
                {
                    IOWA_LOG_INFO(IOWA_PART_DATA, "Failed to convert back the integer value.");
                    goto exit_error;
                }

                tmpP = dataArray[i].value.asBuffer.buffer;
                dataArray[i].value.asInteger = value;

            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                tmpP = dataArray[i].value.asBuffer.buffer;

                if (1 == dataArray[i].value.asBuffer.length)
                {
                    dataArray[i].value.asInteger = (int8_t)tmpP[0];
                }
                else if (2 == dataArray[i].value.asBuffer.length)
                {
                    if (IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == type)
                    {
                        uint16_t value;

                        utilsCopyValue(&value, tmpP, dataArray[i].value.asBuffer.length);

                        dataArray[i].value.asInteger = value;
                    }
                    else
                    {
                        int16_t value;

                        utilsCopyValue(&value, tmpP, dataArray[i].value.asBuffer.length);

                        dataArray[i].value.asInteger = value;
                    }
                }
                else if (4 == dataArray[i].value.asBuffer.length)
                {
                    if (IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == type)
                    {
                        uint32_t value;

                        utilsCopyValue(&value, tmpP, dataArray[i].value.asBuffer.length);

                        dataArray[i].value.asInteger = value;
                    }
                    else
                    {
                        int32_t value;

                        utilsCopyValue(&value, tmpP, dataArray[i].value.asBuffer.length);

                        dataArray[i].value.asInteger = value;
                    }
                }
                else if (8 == dataArray[i].value.asBuffer.length)
                {
                    if (IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == type)
                    {
                        uint64_t value;

                        utilsCopyValue(&value, tmpP, dataArray[i].value.asBuffer.length);

                        dataArray[i].value.asInteger = value;
                    }
                    else
                    {
                        utilsCopyValue(&(dataArray[i].value.asInteger), tmpP, dataArray[i].value.asBuffer.length);
                    }
                }
                else
                {
                    IOWA_LOG_INFO(IOWA_PART_DATA, "Failed to convert the integer value.");
                    tmpP = NULL;
                    goto exit_error;
                }
            }
#endif
            else if (dataArray[i].type != IOWA_LWM2M_TYPE_INTEGER
                     && dataArray[i].type != IOWA_LWM2M_TYPE_UNSIGNED_INTEGER
                     && dataArray[i].type != IOWA_LWM2M_TYPE_TIME)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an integer as expected.", i);
                goto exit_error;
            }

            if (IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == type
                && dataArray[i].value.asInteger < 0)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an unsigned integer as expected.", i);
                tmpP = NULL;
                goto exit_error;
            }

            dataArray[i].type = type;

        }
        else if (IOWA_LWM2M_TYPE_FLOAT == type)
        {
            if (IOWA_LWM2M_TYPE_INTEGER == dataArray[i].type
                || IOWA_LWM2M_TYPE_UNSIGNED_INTEGER == dataArray[i].type)
            {
                dataArray[i].value.asFloat = (double)dataArray[i].value.asInteger;
            }
            else if (IOWA_LWM2M_TYPE_STRING == dataArray[i].type)
            {
                double value;

                if (dataUtilsBufferToFloat(dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length, &value) == 0)
                {
                    IOWA_LOG_INFO(IOWA_PART_DATA, "Failed to convert back the integer value.");
                    goto exit_error;
                }

                tmpP = dataArray[i].value.asBuffer.buffer;
                dataArray[i].value.asFloat = value;
            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                if (4 == dataArray[i].value.asBuffer.length)
                {
                    float value;

                    utilsCopyValue(&value, dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);

                    dataArray[i].value.asFloat = value;
                }
                else if (8 == dataArray[i].value.asBuffer.length)
                {
                    double value;

                    utilsCopyValue(&value, dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);

                    dataArray[i].value.asFloat = value;
                }
                else
                {
                    IOWA_LOG_INFO(IOWA_PART_DATA, "Failed to convert the floating point value.");
                    goto exit_error;
                }
            }
#endif
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an integer as expected.", i);
                goto exit_error;
            }

            dataArray[i].type = IOWA_LWM2M_TYPE_FLOAT;
        }
        else if (IOWA_LWM2M_TYPE_BOOLEAN == type)
        {
            uint8_t falseValue;
            uint8_t trueValue;

            if (IOWA_LWM2M_TYPE_STRING == dataArray[i].type)
            {
                falseValue = '0';
                trueValue = '1';
            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                falseValue = 0;
                trueValue = 1;
            }
#endif
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an boolean as expected.", i);
                goto exit_error;
            }

            if (dataArray[i].value.asBuffer.length != 1)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "String value of Data #%u is not a boolean as expected.", i);
                goto exit_error;
            }

            tmpP = dataArray[i].value.asBuffer.buffer;

            if (falseValue == dataArray[i].value.asBuffer.buffer[0])
            {
                dataArray[i].value.asBoolean = false;
            }
            else if (trueValue == dataArray[i].value.asBuffer.buffer[0])
            {
                dataArray[i].value.asBoolean = true;
            }
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "String value of Data #%u is not a boolean as expected.", i);
                tmpP = NULL;
                goto exit_error;
            }

            dataArray[i].type = IOWA_LWM2M_TYPE_BOOLEAN;
        }
        else if (IOWA_LWM2M_TYPE_CORE_LINK == type)
        {
            if (IOWA_LWM2M_TYPE_STRING == dataArray[i].type)
            {
                dataArray[i].type = IOWA_LWM2M_TYPE_CORE_LINK;
            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                dataArray[i].type = IOWA_LWM2M_TYPE_CORE_LINK;
            }
#endif
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not a CoRE Link string as expected.", i);
                goto exit_error;
            }
        }
        else if (IOWA_LWM2M_TYPE_OBJECT_LINK == type)
        {
            if (IOWA_LWM2M_TYPE_STRING == dataArray[i].type)
            {
                size_t stringLength;
                size_t res;

                tmpP = dataArray[i].value.asBuffer.buffer;
                stringLength = dataArray[i].value.asBuffer.length;

                res = dataUtilsBufferToObjectLink(tmpP, stringLength, dataArray + i);
                if (res == 0)
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an Object link as expected.", i);
                    tmpP = NULL;
                    goto exit_error;
                }
            }
#ifdef LWM2M_SUPPORT_TLV
            else if (IOWA_LWM2M_TYPE_UNDEFINED == dataArray[i].type)
            {
                if (4 == dataArray[i].value.asBuffer.length)
                {
                    uint32_t value;

                    tmpP = dataArray[i].value.asBuffer.buffer;

                    utilsCopyValue(&value, dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);

                    dataArray[i].value.asObjLink.objectId = (value >> 16) & 0xFFFF;
                    dataArray[i].value.asObjLink.instanceId = value & 0xFFFF;
                }
                else
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Failed to convert the value.", i);
                    goto exit_error;
                }
            }
#endif
            else
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u is not an Object link as expected.", i);
                goto exit_error;
            }

            dataArray[i].type = IOWA_LWM2M_TYPE_OBJECT_LINK;
        }
        else if (IOWA_LWM2M_TYPE_UNDEFINED == type)
        {
            if (contentFormat != IOWA_CONTENT_FORMAT_OPAQUE)
            {
                dataArray[i].type = IOWA_LWM2M_TYPE_UNDEFINED;
            }
        }
        else
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Data #%u type is unknown.", i);
            goto exit_error;
        }

        iowa_system_free(tmpP);
    }

    IOWA_LOG_INFO(IOWA_PART_DATA, "Exiting on success.");

    return IOWA_COAP_NO_ERROR;

exit_error:
    iowa_system_free(tmpP);

    IOWA_LOG_INFO(IOWA_PART_DATA, "Exiting on error.");

    return IOWA_COAP_406_NOT_ACCEPTABLE;
}
