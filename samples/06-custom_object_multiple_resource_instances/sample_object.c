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
    sample_object_values_t *objectValuesP;
    size_t i;

    // Retrieve our Object values
    objectValuesP = (sample_object_values_t *)userData;

    for (i = 0; i < numData; i++)
    {
        switch (dataP[i].resourceID)
        {
        case 5500:
            if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asBoolean = objectValuesP->booleanValue;
            }
            break;

        case 5503:
           if (operation == IOWA_DM_READ)
            {
                switch (dataP[i].resInstanceID)
                {
                case 1:
                    dataP[i].value.asInteger = objectValuesP->integerValueRes1;
                    break;
                case 2:
                    dataP[i].value.asInteger = objectValuesP->integerValueRes2;
                    break;
                case 3:
                    dataP[i].value.asInteger = objectValuesP->integerValueRes3;
                    break;
                case 6:
                    dataP[i].value.asInteger = objectValuesP->integerValueRes6;
                    break;
                default:
                    // Should not happen
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }
            else if (operation == IOWA_DM_WRITE)
            {
                switch (dataP[i].resInstanceID)
                {
                case 1:
                    objectValuesP->integerValueRes1 = dataP[i].value.asInteger;
                    break;
                case 2:
                    objectValuesP->integerValueRes2 = dataP[i].value.asInteger;
                    break;
                case 3:
                    objectValuesP->integerValueRes3 = dataP[i].value.asInteger;
                    break;
                case 6:
                    objectValuesP->integerValueRes6 = dataP[i].value.asInteger;
                    break;
                default:
                    // Should not happen
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }
            break;

        case 5750:
            if (operation == IOWA_DM_READ)
            {
                if (objectValuesP->stringValue != NULL)
                {
                    // For the sake of the example, we return a copy of our string value.
                    dataP[i].value.asBuffer.length = strlen(objectValuesP->stringValue);
                    dataP[i].value.asBuffer.buffer = strdup(objectValuesP->stringValue);
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
                free(objectValuesP->stringValue);
                objectValuesP->stringValue = (char *)malloc(dataP[i].value.asBuffer.length + 1);
                if (objectValuesP->stringValue == NULL)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
                memcpy(objectValuesP->stringValue, dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length);
                objectValuesP->stringValue[dataP[i].value.asBuffer.length] = 0;
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

iowa_status_t sample_object_resInstance_dataCallback(uint16_t objectID,
                                                     uint16_t instanceID,
                                                     uint16_t resourceID,
                                                     uint16_t *nbResInstanceP,
                                                     uint16_t **resInstanceArrayP,
                                                     void *userData,
                                                     iowa_context_t contextP)
{
    (void)objectID;
    (void)instanceID;
    (void)userData;
    (void)contextP;

    if (resourceID == 5503)
    {
        // only our resource with ID 5503 is multiple
        // we declare that it has 4 instances
        *resInstanceArrayP = (uint16_t *)malloc(4 * sizeof(uint16_t));
        if (*resInstanceArrayP == NULL)
        {
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }

        // our resource instance IDs are 1, 2, 3, and 6
        (*resInstanceArrayP)[0] = 1;
        (*resInstanceArrayP)[1] = 2;
        (*resInstanceArrayP)[2] = 3;
        (*resInstanceArrayP)[3] = 6;

        *nbResInstanceP = 4;
    }
    else
    {
        // This should not happen
        return IOWA_COAP_404_NOT_FOUND;
    }

    return IOWA_COAP_NO_ERROR;
}
