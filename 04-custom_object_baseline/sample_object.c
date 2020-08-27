/**********************************************
 *
 * Copyright (c) 2020 IoTerop.
 * All rights reserved.
 *
 **********************************************/

/**************************************************
 *
 * This is the implementation of a simple custom
 * LwM2M Object.
 *
 **************************************************/

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Header file containing the definition of the sample custom Object
#include "sample_object.h"

iowa_status_t sample_object_dataCallback(iowa_dm_operation_t operation,
                                         iowa_lwm2m_data_t *dataP,
                                         size_t numData,
                                         void *userData,
                                         iowa_context_t iowaH)
{
    sample_object_values_t *objectValuesP;
    size_t i;

    // Retrieve our Object values
    objectValuesP = (sample_object_values_t *)userData;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0; i < numData; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 5500:
                dataP[i].value.asBoolean = objectValuesP->booleanValue;
                break;

            case 5503:
                dataP[i].value.asInteger = objectValuesP->integerValue;
                break;

            case 5750:
                if (objectValuesP->stringValue != NULL)
                {
                    dataP[i].value.asBuffer.length = strlen(objectValuesP->stringValue);
                }
                else
                {
                    dataP[i].value.asBuffer.length = 0;
                }
                dataP[i].value.asBuffer.buffer = objectValuesP->stringValue;
                break;

            default:
                // Should not happen
                return IOWA_COAP_404_NOT_FOUND;
            }
        }
        break;

    case IOWA_DM_WRITE:
        for (i = 0; i < numData; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 5503:
                objectValuesP->integerValue = dataP[i].value.asInteger;
                break;

            case 5750:
                free(objectValuesP->stringValue);
                objectValuesP->stringValue = (char *)malloc(dataP[i].value.asBuffer.length + 1);
                if (objectValuesP->stringValue == NULL)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
                memcpy(objectValuesP->stringValue, dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length);
                objectValuesP->stringValue[dataP[i].value.asBuffer.length] = 0;
                break;

            default:
                // Should not happen
                return IOWA_COAP_404_NOT_FOUND;
            }
        }
        break;

    default:
        // Should not happen
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    return IOWA_COAP_NO_ERROR;
}