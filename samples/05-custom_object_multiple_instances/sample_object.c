/**********************************************
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
    sample_instance_values_t *instanceValues;
    size_t instanceIndex;
    size_t i;

    // Retrieve our instances values
    instanceValues = (sample_instance_values_t *)userData;

    for (i = 0; i < numData; i++)
    {
        // First we need to determine which instance is targeted
        // We do this only when needed
        if (i == 0
            || dataP[i].instanceID != dataP[i - 1].instanceID)
        {
            instanceIndex = 0;

            while (instanceValues[instanceIndex].id != dataP[i].instanceID)
            {
                instanceIndex++;
                // We only declared three instances so this should not happen
                if (instanceIndex >= 3)
                {
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }
        }

        // This part is the same as in the custom_object_baseline sample, except that we retrieve values for the matching instance
        switch (dataP[i].resourceID)
        {
        case 5500:
            if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asBoolean = instanceValues[instanceIndex].booleanValue;
            }
            break;

        case 5503:
           if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asInteger = instanceValues[instanceIndex].integerValue;
            }
            else if (operation == IOWA_DM_WRITE)
            {
                instanceValues[instanceIndex].integerValue = (int) dataP[i].value.asInteger;
            }
            break;

        case 5750:
            if (operation == IOWA_DM_READ)
            {
                if (instanceValues[instanceIndex].stringValue != NULL)
                {
                    // For the sake of the example, we return a copy of our string value.
                    dataP[i].value.asBuffer.length = strlen(instanceValues[instanceIndex].stringValue);
                    dataP[i].value.asBuffer.buffer = (uint8_t *)strdup(instanceValues[instanceIndex].stringValue);
                    if (dataP[i].value.asBuffer.buffer == NULL)
                    {
                        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    }
                }
                else
                {
                    dataP[i].value.asBuffer.length = 0;
                    dataP[i].value.asBuffer.buffer = NULL;
                }
            }
            else if (operation == IOWA_DM_WRITE)
            {
                free(instanceValues[instanceIndex].stringValue);
                instanceValues[instanceIndex].stringValue = (char *)malloc(dataP[i].value.asBuffer.length + 1);
                if (instanceValues[instanceIndex].stringValue == NULL)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
                memcpy(instanceValues[instanceIndex].stringValue, dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length);
                instanceValues[instanceIndex].stringValue[dataP[i].value.asBuffer.length] = 0;
            }
            else if (operation == IOWA_DM_FREE)
            {
                // IOWA has no longer use of the string value. We can free it.
                free(dataP[i].value.asBuffer.buffer);
            }
            break;

            default:
                // Should not happen
                return IOWA_COAP_404_NOT_FOUND;
        }
    }

    return IOWA_COAP_NO_ERROR;
}
