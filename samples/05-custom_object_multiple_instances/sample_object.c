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

/*************************************************************************************
** Private functions
*************************************************************************************/
// Free a sample instance
void prv_freeInstance(sample_instance_t *instanceP)
{
    free(instanceP->instanceValuesP->stringValue);
    free(instanceP->instanceValuesP);
    free(instanceP);
}

// Find instance by its Id
sample_instance_t *prv_findInstanceById(sample_instance_t *instanceList, uint16_t id)
{
    sample_instance_t *nodeP;

    for (nodeP = instanceList; nodeP != NULL; nodeP = nodeP->next)
    {
        if (nodeP->id == id)
        {
            break;
        }
    }

    return nodeP;
}

// Add instance to sample object's instanceList
sample_instance_t *prv_addInstance(sample_instance_t *instanceList, sample_instance_t *nodeP)
{
    if (nodeP == NULL)
    {
        return NULL;
    }

    nodeP->next = instanceList;

    return nodeP;
}

// Remove instance from sample object's instanceList
sample_instance_t *prv_removeInstanceById(sample_instance_t *instanceList, uint16_t id)
{
    sample_instance_t *nodeP;

    nodeP = instanceList;

    if (nodeP == NULL)
    {
        return NULL;
    }
    else if (nodeP->id == id)
    {
        instanceList = instanceList->next;
        prv_freeInstance(nodeP);
    }
    else
    {
        while (nodeP->next != NULL
                && nodeP->next->id != id)
        {
            nodeP = nodeP->next;
        }
        if (nodeP != NULL)
        {
            sample_instance_t *removedNodeP;

            removedNodeP = nodeP->next;
            nodeP->next = removedNodeP->next;

            prv_freeInstance(removedNodeP);
        }
    }

    return instanceList;
}
/*************************************************************************************
** Public functions
*************************************************************************************/
// Instance Resources callback
iowa_status_t sample_object_dataCallback(iowa_dm_operation_t operation,
                                         iowa_lwm2m_data_t *dataP,
                                         size_t numData,
                                         void *userData,
                                         iowa_context_t iowaH)
{
    sample_object_t *objectP;
    sample_instance_t *instanceP;
    size_t i;

    // Retrieve our Object values
    objectP = (sample_object_t *)userData;

    for (i = 0; i < numData; i++)
    {
        if (i == 0
            || dataP[i].instanceID != dataP[i - 1].instanceID)
        {
            instanceP = prv_findInstanceById(objectP->instanceList, dataP[i].instanceID);
            if (instanceP == NULL)
            {
                return IOWA_COAP_404_NOT_FOUND;
            }
        }

        switch (dataP[i].resourceID)
        {
        case 5500:
            if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asBoolean = instanceP->instanceValuesP->booleanValue;
            }
            break;

        case 5503:
           if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asInteger = instanceP->instanceValuesP->integerValue;
            }
            else if (operation == IOWA_DM_WRITE)
            {
                instanceP->instanceValuesP->integerValue = dataP[i].value.asInteger;
            }
            break;

        case 5750:
            if (operation == IOWA_DM_READ)
            {
                if (instanceP->instanceValuesP->stringValue != NULL)
                {
                    // For the sake of the example, we return a copy of our string value.
                    dataP[i].value.asBuffer.length = strlen(instanceP->instanceValuesP->stringValue);
                    dataP[i].value.asBuffer.buffer = strdup(instanceP->instanceValuesP->stringValue);
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
                free(instanceP->instanceValuesP->stringValue);
                instanceP->instanceValuesP->stringValue = (char *)malloc(dataP[i].value.asBuffer.length + 1);
                if (instanceP->instanceValuesP->stringValue == NULL)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
                memcpy(instanceP->instanceValuesP->stringValue, dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length);
                instanceP->instanceValuesP->stringValue[dataP[i].value.asBuffer.length] = 0;
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

// Object instances callback
iowa_status_t sample_object_InstanceCallback(iowa_dm_operation_t operation,
                                             uint16_t objectID,
                                             uint16_t instanceID,
                                             void *userData,
                                             iowa_context_t contextP)
{
    sample_object_t *objectP;
    sample_instance_t *instanceP;
    size_t i;

    (void)objectID;

    // Retrieve our Object values
    objectP = (sample_object_t *)userData;

    switch (operation)
    {
    case IOWA_DM_CREATE:
        instanceP = (sample_instance_t *)malloc(sizeof(sample_instance_t));
        if (instanceP == NULL)
        {
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        memset(instanceP, 0, sizeof(sample_instance_t));

        instanceP->instanceValuesP = (sample_instance_values_t *)malloc(sizeof(sample_instance_values_t));
        if (instanceP->instanceValuesP == NULL)
        {
            free(instanceP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        memset(instanceP->instanceValuesP, 0, sizeof(sample_instance_values_t));

        instanceP->id = instanceID;
        objectP->instanceList = prv_addInstance(objectP->instanceList, instanceP);
        break;

    case IOWA_DM_DELETE:
        objectP->instanceList = prv_removeInstanceById(objectP->instanceList, instanceID);
        break;

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

// Add a new instance
iowa_status_t sample_object_add_instance(sample_object_t *objectP,
                                         sample_instance_values_t *instanceValuesP,
                                         uint16_t id)
{
    sample_instance_t *instanceP;

    instanceP = (sample_instance_t *)malloc(sizeof(sample_instance_t));
    if (instanceP == NULL)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    memset(instanceP, 0, sizeof(sample_instance_t));

    instanceP->instanceValuesP = (sample_instance_values_t *)malloc(sizeof(sample_instance_values_t));
    if (instanceP->instanceValuesP == NULL)
    {
        free(instanceP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    instanceP->instanceValuesP->booleanValue = instanceValuesP->booleanValue;
    instanceP->instanceValuesP->integerValue = instanceValuesP->integerValue;

    if (instanceValuesP->stringValue != NULL)
    {
        instanceP->instanceValuesP->stringValue = strdup(instanceValuesP->stringValue);
        if (instanceP->instanceValuesP->stringValue == NULL)
        {
            free(instanceP->instanceValuesP);
            free(instanceP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
    }
    else
    {
        instanceP->instanceValuesP->stringValue = NULL;
    }

    instanceP->id = id;
    objectP->instanceList = prv_addInstance(objectP->instanceList, instanceP);

    return IOWA_COAP_NO_ERROR;
}


// Free sample object
iowa_status_t sample_object_remove_object(sample_object_t *objectP)
{
    sample_instance_t *instanceP;

    instanceP = objectP->instanceList;

    while (instanceP != NULL)
    {
        sample_instance_t *tempInstance;

        tempInstance = instanceP;
        instanceP = instanceP->next;

        prv_freeInstance(tempInstance);
    }

    return IOWA_COAP_NO_ERROR;
}
