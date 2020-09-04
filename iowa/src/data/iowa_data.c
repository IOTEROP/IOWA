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
* Copyright (c) 2018-2019 IoTerop.
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

static void prv_dataInsertionSortSmallToLarge(size_t dataCount,
                                              iowa_lwm2m_data_t *dataArrayP)
{
    size_t indTolook;
    size_t indToCompare;
    iowa_lwm2m_data_t dataCurrent;

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

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Entering: dataCount: %zu, dataArrayP: %p.", dataCount, dataArrayP);

    for (i = 0; i < dataCount; i++)
    {
        switch (dataArrayP[i].type)
        {
        case IOWA_LWM2M_TYPE_UNDEFINED:
        case IOWA_LWM2M_TYPE_CORE_LINK:
        case IOWA_LWM2M_TYPE_STRING:
        case IOWA_LWM2M_TYPE_OPAQUE:
            iowa_system_free(dataArrayP[i].value.asBuffer.buffer);
            break;

        default:
            break;
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

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Entering: dataP: %p, dataCount: %u, contentFormatP: %s.", dataP, dataCount, STR_MEDIA_TYPE(*contentFormatP));

    *bufferP = NULL;
    *bufferLengthP = 0;

    if (dataCount == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_DATA, "No data to serialize. Exiting.");
        return IOWA_COAP_NO_ERROR;
    }

    switch (*contentFormatP)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
    case IOWA_CONTENT_FORMAT_OPAQUE:
        if (dataCount != 1
            || dataP[0].resourceID == IOWA_LWM2M_ID_ALL)
        {
            *contentFormatP = LWM2M_DEFAULT_CONTENT_FORMAT;
            IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Single content format cannot be used on Object or Instance Object level. New content format: %s.", STR_MEDIA_TYPE(*contentFormatP));
        }
        if (*contentFormatP == IOWA_CONTENT_FORMAT_OPAQUE
            && dataP[0].type != IOWA_LWM2M_TYPE_OPAQUE)
        {
            *contentFormatP = LWM2M_DEFAULT_CONTENT_FORMAT;
            IOWA_LOG_ARG_WARNING(IOWA_PART_DATA, "Opaque format is reserved to opaque resource. New content format: %s.", STR_MEDIA_TYPE(*contentFormatP));
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

    sortedDataP = dataP;
    sortedDataCount = dataCount;

    switch (*contentFormatP)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
        result = textSerialize(sortedDataP, bufferP, bufferLengthP);
        break;

    case IOWA_CONTENT_FORMAT_OPAQUE:
        result = opaqueSerialize(sortedDataP, bufferP, bufferLengthP);
        break;

#ifdef LWM2M_SUPPORT_TLV
    case IOWA_CONTENT_FORMAT_TLV_OLD:
    case IOWA_CONTENT_FORMAT_TLV:
        result = tlvSerialize(baseUriP, sortedDataP, sortedDataCount, bufferP, bufferLengthP);
        break;
#endif

    default:
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

    *dataP = NULL;
    *dataCountP = 0;

    switch (contentFormat)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
        result = textDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP, resTypeCb, userDataP);
        break;

    case IOWA_CONTENT_FORMAT_OPAQUE:
        result = opaqueDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP, resTypeCb, userDataP);
        break;

#ifdef LWM2M_SUPPORT_TLV
    case IOWA_CONTENT_FORMAT_TLV_OLD:
    case IOWA_CONTENT_FORMAT_TLV:
        result = tlvDeserialize(baseUriP, bufferP, bufferLength, dataP, dataCountP, resTypeCb, userDataP);
        break;
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_DATA, "Content format %s is not supported.", STR_MEDIA_TYPE(contentFormat));
        result = IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "Exiting with result: %u.%02u, dataP: %p, dataCountP: %zu.", (result & 0xFF) >> 5, (result & 0x1F), *dataP, *dataCountP);

    prv_dataInsertionSortSmallToLarge(*dataCountP, *dataP);

    return result;
}


