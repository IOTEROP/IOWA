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
**********************************************/

/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http:
 * The Eclipse Distribution License is available at
 *    http:
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Fabien Fleutot - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Benjamin Cabé - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Scott Bertin - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>

*/
#include "iowa_prv_lwm2m_internals.h"

#ifdef LWM2M_CLIENT_MODE

#include "iowa_access_control_list.h"

static iowa_status_t prv_callDataCb(iowa_context_t contextP,
                                    iowa_dm_operation_t op,
                                    lwm2m_object_t *objectP,
                                    size_t dataCount,
                                    iowa_lwm2m_data_t *dataP)
{
    iowa_status_t result;
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling dataCb() for %s on %u resources.", STR_DM_OPERATION(op), dataCount);
    CRIT_SECTION_LEAVE(contextP);
    result = objectP->dataCb(op, dataP, dataCount, objectP->userData, contextP);
    CRIT_SECTION_ENTER(contextP);
    return result;
}

static iowa_status_t prv_getResourceDimension(iowa_context_t contextP,
                                              lwm2m_object_t *objectP,
                                              uint16_t instanceIndex,
                                              uint16_t resourceIndex,
                                              uint16_t *nbResInstanceP,
                                              uint16_t **resInstanceArrayP)
{
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling resInstanceCb(%u, %u, %u).", objectP->objID, objectP->instanceArray[instanceIndex].id, objectP->resourceArray[resourceIndex].id);

    CRIT_SECTION_LEAVE(contextP);

    if (resInstanceArrayP != NULL)
    {
        result = objectP->resInstanceCb(objectP->objID, objectP->instanceArray[instanceIndex].id, objectP->resourceArray[resourceIndex].id, nbResInstanceP, resInstanceArrayP, objectP->userData, contextP);
    }
    else
    {
        uint16_t *resInstanceArray;

        resInstanceArray = NULL;
        result = objectP->resInstanceCb(objectP->objID, objectP->instanceArray[instanceIndex].id, objectP->resourceArray[resourceIndex].id, nbResInstanceP, &resInstanceArray, objectP->userData, contextP);
        iowa_system_free(resInstanceArray);
    }

    CRIT_SECTION_ENTER(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

static iowa_status_t prv_addInstance(lwm2m_object_t *objectP,
                                     uint16_t id,
                                     uint16_t resourceCount,
                                     uint16_t *resourceArray)
{
    uint16_t instIndex;
    lwm2m_instance_details_t *newArray;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Adding instance %u with %u resources to Object %u.", id, resourceCount, objectP->objID);

    instIndex = object_getInstanceIndex(objectP, id);
    if (instIndex != objectP->instanceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u already exists in Object %u.", id, objectP->objID);
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
    newArray = (lwm2m_instance_details_t *)iowa_system_malloc((size_t)(objectP->instanceCount + 1) * sizeof(lwm2m_instance_details_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (newArray == NULL)
    {
        IOWA_LOG_ERROR_MALLOC((size_t)(objectP->instanceCount + 1) * sizeof(lwm2m_instance_details_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    if (objectP->instanceCount != 0)
    {
        memcpy(newArray, objectP->instanceArray, objectP->instanceCount * sizeof(lwm2m_instance_details_t));
    }
    newArray[objectP->instanceCount].id = id;
    newArray[objectP->instanceCount].resCount = resourceCount;
    if (resourceCount != 0)
    {
        newArray[objectP->instanceCount].resArray = (uint16_t *)iowa_system_malloc(resourceCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (newArray[objectP->instanceCount].resArray == NULL)
        {
            iowa_system_free(newArray);
            IOWA_LOG_ERROR_MALLOC(resourceCount * sizeof(uint16_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memcpy(newArray[objectP->instanceCount].resArray, resourceArray, resourceCount * sizeof(uint16_t));
    }
    else
    {
        newArray[objectP->instanceCount].resArray = NULL;
    }

    iowa_system_free(objectP->instanceArray);
    objectP->instanceArray = newArray;
    objectP->instanceCount++;

    return IOWA_COAP_NO_ERROR;
}

static uint16_t prv_getNewInstanceId(lwm2m_object_t *objectP)
{
    uint16_t instanceId;
    size_t i;

    instanceId = 0;
    i = 0;
    while (i < objectP->instanceCount)
    {
        for (i = 0; i < objectP->instanceCount; i++)
        {
            if (objectP->instanceArray[i].id == instanceId)
            {
                instanceId++;
                break;
            }
        }
    }

    return instanceId;
}

static iowa_status_t prv_removeInstance(lwm2m_object_t *objectP,
                                        uint16_t id)
{
    uint16_t instIndex;

   IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Removing instance %u from Object %u.", id, objectP->objID);

    if (id == IOWA_LWM2M_ID_ALL)
    {
        for (instIndex = 0; instIndex < objectP->instanceCount; instIndex++)
        {
            iowa_system_free(objectP->instanceArray[instIndex].resArray);
        }
        iowa_system_free(objectP->instanceArray);
        objectP->instanceArray = NULL;

        objectP->instanceCount = 0;
    }
    else
    {
        lwm2m_instance_details_t *newArray;

        instIndex = object_getInstanceIndex(objectP, id);
        if (instIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u does not exist in Object %u.", id, objectP->objID);
            return IOWA_COAP_404_NOT_FOUND;
        }

        if (objectP->instanceCount > 1)
        {
            newArray = (lwm2m_instance_details_t *)iowa_system_malloc((size_t)(objectP->instanceCount - 1) * sizeof(lwm2m_instance_details_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (newArray == NULL)
            {
                IOWA_LOG_ERROR_MALLOC((size_t)(objectP->instanceCount - 1) * sizeof(lwm2m_instance_details_t));
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            if (instIndex != 0)
            {
                memcpy(newArray, objectP->instanceArray, instIndex * sizeof(lwm2m_instance_details_t));
            }
            if (instIndex != objectP->instanceCount - 1)
            {
                memcpy(newArray + instIndex, objectP->instanceArray + instIndex + 1, (size_t)(objectP->instanceCount - instIndex - 1) * sizeof(lwm2m_instance_details_t));
            }
        }
        else
        {
            newArray = NULL;
        }
        iowa_system_free(objectP->instanceArray[instIndex].resArray);
        iowa_system_free(objectP->instanceArray);

        objectP->instanceArray = newArray;
        objectP->instanceCount--;
    }

    return IOWA_COAP_NO_ERROR;
}

static uint16_t prv_getResourceIndex(lwm2m_object_t *objectP,
                                     uint16_t instIndex,
                                     uint16_t id)
{
    uint16_t i;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Looking for resource %u in Object %u, instance index: %u.", id, objectP->objID, instIndex);

    if (instIndex < objectP->instanceCount
        && objectP->instanceArray[instIndex].resArray != NULL)
    {
        for (i = 0; i < objectP->instanceArray[instIndex].resCount; i++)
        {
            if (objectP->instanceArray[instIndex].resArray[i] == id)
            {
                break;
            }
        }
        if (i == objectP->instanceArray[instIndex].resCount)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u not found in Object %u, instance index: %u.", id, objectP->objID, instIndex);
            return objectP->resourceCount;
        }
    }

    for (i = 0; i < objectP->resourceCount; i++)
    {
        if (objectP->resourceArray[i].id == id)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u found at index %u in Object %u.", id, i, objectP->objID);
            return i;
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u not found in Object %u.", id, objectP->objID);

    return objectP->resourceCount;
}

static iowa_status_t prv_extendDataArray(size_t *arraySizeP,
                                         iowa_lwm2m_data_t **arrayP,
                                         size_t index,
                                         size_t newSize)
{
    iowa_lwm2m_data_t *newArray;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Array size: %u, array: %p, index: %u, new size: %u.", *arraySizeP, *arrayP, index, newSize);

    newArray = (iowa_lwm2m_data_t *)iowa_system_malloc(newSize * sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (newArray == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(newSize * sizeof(iowa_lwm2m_data_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    if (*arrayP != NULL)
    {
        if (index != 0)
        {
            memcpy(newArray, *arrayP, index * sizeof(iowa_lwm2m_data_t));
        }

        iowa_system_free(*arrayP);
    }

    *arrayP = newArray;
    *arraySizeP = newSize;

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_addResourceToDataArray(iowa_context_t contextP,
                                                lwm2m_object_t *objectP,
                                                uint16_t instIndex,
                                                uint16_t resIndex,
                                                uint16_t resInstanceId,
                                                size_t *arraySizeP,
                                                iowa_lwm2m_data_t **arrayP,
                                                size_t *indexP)
{
    uint16_t instanceId;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Adding resource /%u/n/%u/%u (instIndex: %u, resIndex: %u) at index %u in array %p (size: %u).", objectP->objID, objectP->resourceArray[resIndex].id, resInstanceId, instIndex, resIndex, *indexP, *arrayP, *arraySizeP);

    if (objectP->instanceArray[instIndex].resArray != NULL)
    {
        uint16_t i;

        for (i = 0; i < objectP->instanceArray[instIndex].resCount; i++)
        {
            if (objectP->instanceArray[instIndex].resArray[i] == objectP->resourceArray[resIndex].id)
            {
                break;
            }
        }
        if (i == objectP->instanceArray[instIndex].resCount)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u not found in Object %u, instance %u.", objectP->resourceArray[resIndex].id, objectP->objID, objectP->instanceArray[instIndex].id);
            return IOWA_COAP_404_NOT_FOUND;
        }

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u is present in Object %u, instance %u.", objectP->resourceArray[resIndex].id, objectP->objID, objectP->instanceArray[instIndex].id);

    }

    instanceId = objectP->instanceArray[instIndex].id;

    if (IS_RSC_MULTIPLE(objectP->resourceArray[resIndex]))
    {
        iowa_status_t result;
        uint16_t nbResInstance;
        uint16_t *resInstanceArray;

        result = prv_getResourceDimension(contextP, objectP, instIndex, resIndex, &nbResInstance, &resInstanceArray);
        if (result != IOWA_COAP_NO_ERROR)
        {
            return result;
        }

        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource /%u/%u/%u has %u resource instances.", objectP->objID, objectP->instanceArray[instIndex].id, objectP->resourceArray[resIndex].id, nbResInstance);

        if (nbResInstance != 0)
        {
            uint16_t i;
            size_t dataSizeNeeded;
            {
                dataSizeNeeded = nbResInstance;
            }

            if (*indexP + dataSizeNeeded > *arraySizeP)
            {
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                result = prv_extendDataArray(arraySizeP, arrayP, *indexP, *indexP + nbResInstance);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Error extending array.");
                    return result;
                }
#else
                (void)prv_extendDataArray(arraySizeP, arrayP, *indexP, *indexP + nbResInstance);
#endif
            }

            for (i = 0; i < nbResInstance; i++)
            {
                {
                    (*arrayP)[*indexP].objectID = objectP->objID;
                    (*arrayP)[*indexP].instanceID = instanceId;
                    (*arrayP)[*indexP].resourceID = objectP->resourceArray[resIndex].id;
                    (*arrayP)[*indexP].resInstanceID = resInstanceArray[i];
                    (*arrayP)[*indexP].type = objectP->resourceArray[resIndex].type;
                    (*indexP)++;
                }
            }
            iowa_system_free(resInstanceArray);
        }
    }
    else
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource /%u/n/%u is a single one.", objectP->objID, objectP->resourceArray[resIndex].id);

        if (*indexP + 1 > *arraySizeP)
        {
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            iowa_status_t result;

            result = prv_extendDataArray(arraySizeP, arrayP, *indexP, *indexP + 1);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Error extending array.");
                return result;
            }
#else
            (void)prv_extendDataArray(arraySizeP, arrayP, *indexP, *indexP + 1);
#endif
        }

        (*arrayP)[*indexP].objectID = objectP->objID;
        (*arrayP)[*indexP].instanceID = instanceId;
        (*arrayP)[*indexP].resourceID = objectP->resourceArray[resIndex].id;
        (*arrayP)[*indexP].resInstanceID = IOWA_LWM2M_ID_ALL;
        (*arrayP)[*indexP].type = objectP->resourceArray[resIndex].type;

        *indexP += 1;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with array size: %u, array: %p, next index: %u.", *arraySizeP, *arrayP, *indexP);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_getResourceReadableArray(iowa_context_t contextP,
                                                  lwm2m_object_t *objectP,
                                                  iowa_lwm2m_uri_t *uriP,
                                                  uint16_t serverShortId,
                                                  bool resNone,
                                                  size_t *dataCountP,
                                                  iowa_lwm2m_data_t **dataArrayP)
{
    iowa_status_t result;
    uint16_t instIndex;
    uint16_t resIndex;
    size_t iowaDataCount;
    iowa_lwm2m_data_t *iowaDataArray;
    size_t iowaDataArraySize;
    (void)serverShortId;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Object: %u, instanceId: %u, resourceId: %u, resInstanceId: %u, serverShortId: %u, resNone: %s.", objectP->objID, uriP->instanceId, uriP->resourceId, uriP->resInstanceId, serverShortId, resNone?"true":"false");

    iowaDataCount = 0;
    iowaDataArray = NULL;
    iowaDataArraySize = 0;
    result = IOWA_COAP_NO_ERROR;

    if (uriP->instanceId != IOWA_LWM2M_ID_ALL)
    {
        instIndex = object_getInstanceIndex(objectP, uriP->instanceId);
        if (instIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);

            return IOWA_COAP_404_NOT_FOUND;
        }

        if (uriP->resourceId != IOWA_LWM2M_ID_ALL)
        {
            resIndex = prv_getResourceIndex(objectP, instIndex, uriP->resourceId);
            if (resIndex == objectP->resourceCount)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u not found.", uriP->resourceId);

                return IOWA_COAP_404_NOT_FOUND;
            }

            if ((resNone == true && (IS_RSC_USEABLE(objectP->resourceArray[resIndex]) && !IS_RSC_READABLE(objectP->resourceArray[resIndex])))
                || (resNone == false && !IS_RSC_READABLE(objectP->resourceArray[resIndex])))
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u/%u/%u is not readable.", objectP->objID, uriP->instanceId, uriP->resourceId);

                return IOWA_COAP_405_METHOD_NOT_ALLOWED;
            }

            result = prv_addResourceToDataArray(contextP, objectP, instIndex, resIndex, uriP->resInstanceId, &iowaDataArraySize, &iowaDataArray, &iowaDataCount);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_TRACE(IOWA_PART_LWM2M, "prv_addResourceToDataArray() failed.");
                iowa_system_free(iowaDataArray);
                return result;
            }
        }
        else
        {
            iowaDataArraySize = objectP->resourceCount;
            iowaDataArray = (iowa_lwm2m_data_t *)iowa_system_malloc(iowaDataArraySize * sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (iowaDataArray == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(iowaDataArraySize * sizeof(iowa_lwm2m_data_t));
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            memset(iowaDataArray, 0, iowaDataCount * sizeof(iowa_lwm2m_data_t));

            for (resIndex = 0; resIndex < objectP->resourceCount; resIndex++)
            {
                if ((resNone == true && (IS_RSC_USEABLE(objectP->resourceArray[resIndex]) && !IS_RSC_READABLE(objectP->resourceArray[resIndex])))
                    || (resNone == false && !IS_RSC_READABLE(objectP->resourceArray[resIndex])))
                {
                    continue;
                }

                result = prv_addResourceToDataArray(contextP, objectP, instIndex, resIndex, IOWA_LWM2M_ID_ALL, &iowaDataArraySize, &iowaDataArray, &iowaDataCount);
                if (result == IOWA_COAP_404_NOT_FOUND)
                {
                    result = IOWA_COAP_NO_ERROR;
                }
                if (result != IOWA_COAP_NO_ERROR)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "prv_addResourceToDataArray() failed.");
                    iowa_system_free(iowaDataArray);
                    return result;
                }
            }
        }
    }
    else
    {
        iowaDataArraySize = (size_t)(objectP->instanceCount * objectP->resourceCount);
        iowaDataArray = (iowa_lwm2m_data_t *)iowa_system_malloc(iowaDataArraySize * sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (iowaDataArray == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(iowaDataArraySize * sizeof(iowa_lwm2m_data_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memset(iowaDataArray, 0, iowaDataArraySize * sizeof(iowa_lwm2m_data_t));

        for (instIndex = 0; instIndex < objectP->instanceCount; instIndex++)
        {

            for (resIndex = 0; resIndex < objectP->resourceCount; resIndex++)
            {
                if ((resNone == true && (IS_RSC_USEABLE(objectP->resourceArray[resIndex]) && !IS_RSC_READABLE(objectP->resourceArray[resIndex])))
                    || (resNone == false && !IS_RSC_READABLE(objectP->resourceArray[resIndex])))
                {
                    continue;
                }

                result = prv_addResourceToDataArray(contextP, objectP, instIndex, resIndex, IOWA_LWM2M_ID_ALL, &iowaDataArraySize, &iowaDataArray, &iowaDataCount);
                if (result == IOWA_COAP_404_NOT_FOUND)
                {
                    result = IOWA_COAP_NO_ERROR;
                }
                if (result != IOWA_COAP_NO_ERROR)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "prv_addResourceToDataArray() failed.");
                    iowa_system_free(iowaDataArray);
                    return result;
                }
            }
        }

    }

    *dataCountP = iowaDataCount;

    if (*dataCountP == 0)
    {
        iowa_system_free(iowaDataArray);
        *dataArrayP = NULL;
    }
    else
    {
        *dataArrayP = iowaDataArray;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with %u data in array: %p.", *dataCountP, *dataArrayP);

    return result;
}

static iowa_status_t prv_convertAttributes(attributes_t *paramP,
                                           attribute_t **attrP)
{
    if (paramP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD)
    {
        attribute_t *attrNodeP;

        attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (attrNodeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        attrNodeP->nextP = NULL;
        attrNodeP->key = KEY_PERIOD_MINIMUM;
        attrNodeP->value.asInteger = paramP->minPeriod;

        *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);
    }
    if (paramP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD)
    {
        attribute_t *attrNodeP;

        attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (attrNodeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        attrNodeP->nextP = NULL;
        attrNodeP->key = KEY_PERIOD_MAXIMUM;
        attrNodeP->value.asInteger = paramP->maxPeriod;

        *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);
    }
    if (paramP->flags & LWM2M_ATTR_FLAG_GREATER_THAN)
    {
        attribute_t *attrNodeP;

        attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (attrNodeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        attrNodeP->nextP = NULL;
        attrNodeP->key = KEY_GREATER_THAN;
        attrNodeP->value.asFloat = paramP->greaterThan;

        *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);
    }
    if (paramP->flags & LWM2M_ATTR_FLAG_LESS_THAN)
    {
        attribute_t *attrNodeP;

        attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (attrNodeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        attrNodeP->nextP = NULL;
        attrNodeP->key = KEY_LESS_THAN;
        attrNodeP->value.asFloat = paramP->lessThan;

        *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);
    }
    if (paramP->flags & LWM2M_ATTR_FLAG_STEP)
    {
        attribute_t *attrNodeP;

        attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (attrNodeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        attrNodeP->nextP = NULL;
        attrNodeP->key = KEY_STEP;
        attrNodeP->value.asFloat = paramP->step;

        *attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(*attrP, attrNodeP);
    }

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_checkDataConsistency(iowa_lwm2m_data_t *dataP,
                                              lwm2m_object_t *objectP,
                                              uint16_t resourceIndex)
{
    iowa_lwm2m_data_type_t type;

    type = dataP->type;

    if (type != objectP->resourceArray[resourceIndex].type)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Data type mismatch.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (dataP->type == IOWA_LWM2M_TYPE_UNSIGNED_INTEGER
        && dataP->value.asInteger < 0)
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "The unsigned integer value is negative.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
    if (dataP->timestamp != 0)
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Timestamp is not allowed on data.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (dataP->resInstanceID != IOWA_LWM2M_ID_ALL
        && !IS_RSC_MULTIPLE(objectP->resourceArray[resourceIndex]))
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u in Object %u is Single Instance.", dataP->resourceID, objectP->objID);
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_adaptCreatePayload(lwm2m_object_t *objectP,
                                            size_t *dataCountP,
                                            iowa_lwm2m_data_t *dataArray,
                                            bool bootstapWrite)
{
    size_t dataIndex;
    uint16_t resourceIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Checking %u resources. bootstapWrite: %s.", *dataCountP, bootstapWrite ? "true" : "false");

    for (dataIndex = 1; dataIndex < *dataCountP; dataIndex++)
    {
        if (dataArray[dataIndex].objectID != dataArray[0].objectID
            || (bootstapWrite == false
                && dataArray[dataIndex].instanceID != dataArray[0].instanceID))
        {
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Payload contains several Objects or Object Instances.");
            return IOWA_COAP_400_BAD_REQUEST;
        }
    }

    dataIndex = 0;
    while (dataIndex < *dataCountP)
    {
        resourceIndex = prv_getResourceIndex(objectP, objectP->instanceCount, dataArray[dataIndex].resourceID);
        if (resourceIndex == objectP->resourceCount)
        {
            iowa_lwm2m_data_t data;

            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Remove the Resource %u from the list.", dataArray[dataIndex].resourceID);

            data = dataArray[dataIndex];
            dataArray[dataIndex] = dataArray[*dataCountP-1];
            dataArray[*dataCountP-1] = data;
            *dataCountP -= 1;
        }
        else
        {
            iowa_status_t result;

            result = prv_checkDataConsistency(&(dataArray[dataIndex]), objectP, resourceIndex);
            if (result != IOWA_COAP_NO_ERROR)
            {
                return result;
            }

            dataIndex++;
        }
    }

    if (bootstapWrite == false)
    {
        for (resourceIndex = 0; resourceIndex < objectP->resourceCount; resourceIndex++)
        {
            if (IS_RSC_MANDATORY(objectP->resourceArray[resourceIndex])
                && !IS_RSC_EXECUTABLE(objectP->resourceArray[resourceIndex]))
            {
                dataIndex = 0;
                while (dataIndex < *dataCountP
                       && dataArray[dataIndex].resourceID != objectP->resourceArray[resourceIndex].id)
                {
                    dataIndex++;
                }
                if (dataIndex == *dataCountP)
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Mandatory Resource %u is missing.", objectP->resourceArray[resourceIndex].id);
                    return IOWA_COAP_400_BAD_REQUEST;
                }
            }
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting on success. Number of resources: %u.", *dataCountP);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_writeData(iowa_context_t contextP,
                                   lwm2m_object_t *objectP,
                                   uint16_t serverShortId,
                                   size_t dataCount,
                                   iowa_lwm2m_data_t *dataArrayP)
{
    iowa_status_t result;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object ID: %u, dataCount: %u.\nCalling dataCallback(IOWA_DM_WRITE) on:", objectP->objID, dataCount);

    {
        size_t index;

        for (index = 0; index < dataCount; index++)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "\t/%u/%u/%u with type %s", objectP->objID, dataArrayP[index].instanceID, dataArrayP[index].resourceID, STR_LWM2M_TYPE(dataArrayP[index].type));
        }
    }

    result = prv_callDataCb(contextP, IOWA_DM_WRITE, objectP, dataCount, dataArrayP);
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "dataCallback() returned code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    if (result == IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_204_CHANGED;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u", (result & 0xFF) >> 5, (result & 0x1F));
    return result;
}

static iowa_status_t prv_create(iowa_context_t contextP,
                                lwm2m_object_t *objectP,
                                size_t dataCount,
                                iowa_lwm2m_data_t *dataP)
{
    iowa_status_t result;
    uint16_t instanceId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object ID: %u.", dataP[0].objectID);

    if (objectP->instanceCb == NULL)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u has no create callback.", dataP[0].objectID);
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    instanceId = dataP[0].instanceID;

    if (instanceId != IOWA_LWM2M_ID_ALL)
    {
        uint16_t instanceIndex;

        instanceIndex = object_getInstanceIndex(objectP, instanceId);
        if (instanceIndex != objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u already exists in Object %u.", instanceId, dataP[0].objectID);
            return IOWA_COAP_400_BAD_REQUEST;
        }
    }
    else
    {
        size_t i;

        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "No instance id specified.");

        instanceId = prv_getNewInstanceId(objectP);

        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Using new instance id %u.", instanceId);

        for (i = 0; i < dataCount; i++)
        {
            dataP[i].instanceID = instanceId;
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling instanceCb(IOWA_DM_CREATE) on Object %u for instance %u.", objectP->objID, instanceId);
    CRIT_SECTION_LEAVE(contextP);
    result = objectP->instanceCb(IOWA_DM_CREATE, objectP->objID, instanceId, objectP->userData, contextP);
    CRIT_SECTION_ENTER(contextP);
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "instanceCb(IOWA_DM_CREATE) returned code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    if (result != IOWA_COAP_NO_ERROR)
    {
        return result;
    }

    result = prv_writeData(contextP, objectP, IOWA_LWM2M_ID_ALL, dataCount, dataP);
    if (result == IOWA_COAP_NO_ERROR
        || result == IOWA_COAP_204_CHANGED)
    {
        result = IOWA_COAP_201_CREATED;
    }

    if (result != IOWA_COAP_201_CREATED)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling instanceCb(IOWA_DM_DELETE) on Object %u for instance %u.", objectP->objID, dataP[0].instanceID);
        CRIT_SECTION_LEAVE(contextP);
        (void)objectP->instanceCb(IOWA_DM_DELETE, objectP->objID, dataP[0].instanceID, objectP->userData, contextP);
        CRIT_SECTION_ENTER(contextP);
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "instanceCb(IOWA_DM_DELETE) returned code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    }
    else
    {
        if (objectP->type == OBJECT_MULTIPLE_ADVANCED)
        {
            uint16_t resCount;
            uint16_t *resArray;
            size_t i;

            resArray = (uint16_t *)iowa_system_malloc(dataCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (resArray == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(dataCount * sizeof(uint16_t));
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            resCount = 0;
            for (i = 0; i < dataCount; i++)
            {
                if (dataP[i].type != IOWA_LWM2M_TYPE_UNDEFINED)
                {
                    if (resCount == 0
                        || resArray[resCount - 1] != dataP[i].resourceID)
                    {
                        resArray[resCount] = dataP[i].resourceID;
                        resCount++;
                    }
                }
            }
            result = prv_addInstance(objectP, dataP[0].instanceID, resCount, resArray);
            iowa_system_free(resArray);
        }
        else
        {
            result = prv_addInstance(objectP, dataP[0].instanceID, 0, NULL);
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

static iowa_status_t prv_getResourceAttributes(iowa_context_t contextP,
                                               lwm2m_object_t *objectP,
                                               lwm2m_server_t *serverP,
                                               link_t *linkP,
                                               uint16_t instanceIndex,
                                               uint16_t resourceIndex,
                                               bool useInheritance)
{
    iowa_status_t result;
    attributes_t attr;

    if (LWM2M_URI_IS_SET_RESOURCE_INSTANCE(&linkP->uri) == false
        && IS_RSC_MULTIPLE(objectP->resourceArray[resourceIndex]))
    {
        attribute_t *attrNodeP;
        uint16_t nbResInstance;

        result = prv_getResourceDimension(contextP, objectP, instanceIndex, resourceIndex, &nbResInstance, NULL);
        if (result != IOWA_COAP_NO_ERROR)
        {
            return result;
        }

        attrNodeP = (attribute_t *)iowa_system_malloc(sizeof(attribute_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (attrNodeP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attribute_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        attrNodeP->nextP = NULL;
        attrNodeP->key = KEY_DIMENSION;
        attrNodeP->value.asInteger = nbResInstance;

        linkP->attrP = (attribute_t *)IOWA_UTILS_LIST_ADD(linkP->attrP, attrNodeP);
    }

    if (attributesGet(serverP, &linkP->uri, &attr, useInheritance, false) == true)
    {
        result = prv_convertAttributes(&attr, &linkP->attrP);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert the attributes.");
            return result;
        }
    }

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_deleteObjectInstance(iowa_context_t contextP,
                                              lwm2m_object_t *objectP,
                                              iowa_lwm2m_uri_t *uriP,
                                              uint16_t serverShortId)
{
    iowa_status_t result;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling instanceCallback(IOWA_DM_DELETE) on /%u/%u.", objectP->objID, uriP->instanceId);

    CRIT_SECTION_LEAVE(contextP);
    result = objectP->instanceCb(IOWA_DM_DELETE, objectP->objID, uriP->instanceId, objectP->userData, contextP);
    CRIT_SECTION_ENTER(contextP);
    if (result != IOWA_COAP_NO_ERROR
        && result != IOWA_COAP_202_DELETED)
    {
        return result;
    }

    (void)prv_removeInstance(objectP, uriP->instanceId);
    observe_clear(contextP, uriP);

    return IOWA_COAP_202_DELETED;
}

static void prv_instanceEventCallback(iowa_context_t contextP,
                                      iowa_lwm2m_uri_t * uriP,
                                      iowa_event_type_t eventType)
{
    iowa_event_t event;

    if (contextP->eventCb != NULL)
    {
        event.eventType = eventType;
        event.details.objectInstance.uriP = uriP;

        CRIT_SECTION_LEAVE(contextP);
        contextP->eventCb(&event, contextP->userData, contextP);
        CRIT_SECTION_ENTER(contextP);
    }
}

uint16_t object_getInstanceIndex(lwm2m_object_t *objectP,
                                 uint16_t id)
{
    uint16_t i;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Looking for instance %u in Object %u.", id, objectP->objID);

    if (id == IOWA_LWM2M_ID_ALL)
    {
        return objectP->instanceCount;
    }

    switch (objectP->type)
    {
    case OBJECT_SINGLE:
    case OBJECT_SINGLE_ADVANCED:
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "This is a single-instance Object.");
        if (id == LWM2M_SINGLE_OBJECT_INSTANCE_ID)
        {
            return 0;
        }
        else
        {
            return 1;
        }

    case OBJECT_MULTIPLE:
    case OBJECT_MULTIPLE_ADVANCED:
        for (i = 0; i < objectP->instanceCount; i++)
        {
            if (objectP->instanceArray[i].id == id)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Instance %u found at index %u in Object %u.", id, i, objectP->objID);
                break;
            }
        }
        return i;

    default:
        return objectP->instanceCount;
    }
}

iowa_status_t object_checkReadable(iowa_context_t contextP,
                                   iowa_lwm2m_uri_t * uriP)
{
    lwm2m_object_t * targetP;
    uint16_t instIndex;
    uint16_t index;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    targetP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &uriP->objectId);
    if (NULL == targetP)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (!LWM2M_URI_IS_SET_INSTANCE(uriP))
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u is readable.", uriP->objectId);
        return IOWA_COAP_205_CONTENT;
    }

    instIndex = object_getInstanceIndex(targetP, uriP->instanceId);
    if (instIndex == targetP->instanceCount)
    {
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (!LWM2M_URI_IS_SET_RESOURCE(uriP))
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u/%u is readable.", uriP->objectId, uriP->instanceId);
        return IOWA_COAP_205_CONTENT;
    }

    index = prv_getResourceIndex(targetP, instIndex, uriP->resourceId);
    if (index == targetP->resourceCount)
    {
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (!IS_RSC_READABLE(targetP->resourceArray[index]))
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u/%u/%u is not readable.", uriP->objectId, uriP->instanceId, uriP->resourceId);

        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u/%u/%u is readable.", uriP->objectId, uriP->instanceId, uriP->resourceId);

    return IOWA_COAP_205_CONTENT;
}

void object_sendReadEvent(iowa_context_t contextP,
                          uint16_t objectId,
                          uint16_t instanceId)
{
    if (contextP->eventCb != NULL)
    {
        iowa_event_t event;

        memset(&event, 0, sizeof(iowa_event_t));

        event.eventType = IOWA_EVENT_READ;
        event.details.sensor.sensorId = OBJECT_INSTANCE_ID_TO_SENSOR(objectId, instanceId);

        CRIT_SECTION_LEAVE(contextP);
        contextP->eventCb(&event, contextP->userData, contextP);
        CRIT_SECTION_ENTER(contextP);
    }
}

bool object_isAsynchronous(iowa_context_t contextP,
                           iowa_lwm2m_uri_t *uriP)
{
    lwm2m_object_t *objectP;
    uint16_t index;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &(uriP->objectId));
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return false;
    }

    for (index = 0; index < objectP->resourceCount; index++)
    {
        if ((!LWM2M_URI_IS_SET_RESOURCE(uriP)
             || uriP->resourceId == objectP->resourceArray[index].id)
            && IS_RSC_ASYNCHRONOUS(objectP->resourceArray[index]))
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource /%u/I/%u is asynchronous.", uriP->objectId, objectP->resourceArray[index].id);

            return true;
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "No asynchronous resource in Object %u.", uriP->objectId);

    return false;
}

iowa_lwm2m_data_type_t object_getResourceType(uint16_t objectId,
                                              uint16_t resourceId,
                                              void *userDataP)
{
    iowa_context_t contextP;
    lwm2m_object_t *objectP;
    size_t index;

    contextP = (iowa_context_t)userDataP;

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectId);
        return IOWA_LWM2M_TYPE_UNDEFINED;
    }

    for (index = 0; index < objectP->resourceCount; index++)
    {
        if (objectP->resourceArray[index].id == resourceId)
        {
            return objectP->resourceArray[index].type;
        }
    }

    return IOWA_LWM2M_TYPE_UNDEFINED;
}

iowa_status_t object_read(iowa_context_t contextP,
                          iowa_lwm2m_uri_t *uriP,
                          uint16_t serverShortId,
                          size_t *dataCountP,
                          iowa_lwm2m_data_t **dataArrayP)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &uriP->objectId);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_getResourceReadableArray(contextP, objectP, uriP, serverShortId, false, dataCountP, dataArrayP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to retrieve resource list.");
        return result;
    }

    if (*dataCountP == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "No values to read.");
        return IOWA_COAP_205_CONTENT;
    }

    result = prv_callDataCb(contextP, IOWA_DM_READ, objectP, *dataCountP, *dataArrayP);

    if (result == IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_205_CONTENT;
    }

    if (result != IOWA_COAP_205_CONTENT
        && (*dataArrayP) != NULL)
    {
        object_free(contextP, *dataCountP, *dataArrayP);
        iowa_system_free(*dataArrayP);
        *dataArrayP = NULL;
        *dataCountP = 0;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    return result;
}

iowa_status_t object_readBlock(iowa_context_t contextP,
                               iowa_lwm2m_uri_t *uriP,
                               uint32_t blockInfo,
                               size_t *dataCountP,
                               iowa_lwm2m_data_t **dataArrayP)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;
    uint16_t instIndex;
    uint16_t resIndex;
    iowa_lwm2m_data_type_t blockType;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u/%u, blockInfo: 0x%08X.", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId, blockInfo);

    if (!LWM2M_URI_IS_SET_RESOURCE(uriP))
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Cannot read by blocks several resources.");
        return IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE;
    }

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId , &(uriP->objectId));
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    instIndex = object_getInstanceIndex(objectP, uriP->instanceId);
    if (instIndex == objectP->instanceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);

        return IOWA_COAP_404_NOT_FOUND;
    }

    resIndex = prv_getResourceIndex(objectP, instIndex, uriP->resourceId);
    if (resIndex == objectP->resourceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u not found.", uriP->resourceId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (!IS_RSC_READABLE(objectP->resourceArray[resIndex]))
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "/%u/%u/%u is not readable.", objectP->objID, uriP->instanceId, uriP->resourceId);
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    switch (objectP->resourceArray[resIndex].type)
    {
    case IOWA_LWM2M_TYPE_STRING:
        blockType = IOWA_LWM2M_TYPE_STRING_BLOCK;
        break;

    case IOWA_LWM2M_TYPE_OPAQUE:
        blockType = IOWA_LWM2M_TYPE_OPAQUE_BLOCK;
        break;

    case IOWA_LWM2M_TYPE_CORE_LINK:
        blockType = IOWA_LWM2M_TYPE_CORE_LINK_BLOCK;
        break;

    default:
        IOWA_LOG_ARG_INFO(IOWA_PART_DATA, "/%u/%u/%u type is not String, Opaque, or CoRE Link. Exiting with 4.08 Request Entity Incomplete.", objectP->objID, uriP->instanceId, uriP->resourceId);
        return IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE;
    }

    if (IS_RSC_MULTIPLE(objectP->resourceArray[resIndex])
        && !LWM2M_URI_IS_SET_RESOURCE_INSTANCE(uriP))
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Cannot read by blocks several resource instances.");
        return IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE;
    }

    if (!IS_RSC_STREAMABLE(objectP->resourceArray[resIndex]))
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Resource /%u/%u/%u does not support read by blocks.", objectP->objID, uriP->instanceId, uriP->resourceId);
        return IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE;
    }

    *dataArrayP = (iowa_lwm2m_data_t *)iowa_system_malloc(sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*dataArrayP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_lwm2m_data_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*dataArrayP, 0, sizeof(iowa_lwm2m_data_t));
    *dataCountP = 1;

    (*dataArrayP)[0].objectID = uriP->objectId;
    (*dataArrayP)[0].instanceID = uriP->instanceId;
    (*dataArrayP)[0].resourceID = uriP->resourceId;
    (*dataArrayP)[0].resInstanceID = uriP->resInstanceId;
    (*dataArrayP)[0].type = blockType;
    (*dataArrayP)[0].value.asBlock.details = blockInfo;

    result = prv_callDataCb(contextP, IOWA_DM_READ, objectP, *dataCountP, *dataArrayP);

    if (result == IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_205_CONTENT;
    }

    if (result != IOWA_COAP_205_CONTENT
        && (*dataArrayP) != NULL)
    {
        object_free(contextP, *dataCountP, *dataArrayP);
        iowa_system_free(*dataArrayP);
        *dataArrayP = NULL;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    return result;
}

iowa_status_t object_bootstrapRead(iowa_context_t contextP,
                                   iowa_lwm2m_uri_t *uriP,
                                   size_t *dataCountP,
                                   iowa_lwm2m_data_t **dataArrayP)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &uriP->objectId);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_getResourceReadableArray(contextP, objectP, uriP, IOWA_LWM2M_ID_ALL, true, dataCountP, dataArrayP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to retrieve resource list.");
        return result;
    }

    if (*dataCountP == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "No values to read.");
        return IOWA_COAP_205_CONTENT;
    }

    result = prv_callDataCb(contextP, IOWA_DM_READ, objectP, *dataCountP, *dataArrayP);
    if (result == IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_205_CONTENT;
    }

    if (result != IOWA_COAP_205_CONTENT
        && (*dataArrayP) != NULL)
    {
        object_free(contextP, *dataCountP, *dataArrayP);
        iowa_system_free(*dataArrayP);
        *dataArrayP = NULL;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

void object_free(iowa_context_t contextP,
                 size_t dataCount,
                 iowa_lwm2m_data_t *dataP)
{
    lwm2m_object_t *objectP;
    uint16_t objectId;
    size_t ind;
    size_t startInd;

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Entering.");

    if (dataCount == 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "No data to free.");
        return;
    }

    objectId = dataP[0].objectID;
    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
    if (objectP == NULL)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectId);
        return;
    }
    startInd = 0;
    for (ind = 1; ind < dataCount; ind++)
    {
        if (dataP[ind].objectID != objectId)
        {
            (void)prv_callDataCb(contextP, IOWA_DM_FREE, objectP, ind - startInd, &dataP[startInd]);
            startInd = ind;

            objectId = dataP[ind].objectID;
            objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
            if (objectP == NULL)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectId);
                break;
            }
        }
    }
    if (objectP != NULL)
    {
        (void)prv_callDataCb(contextP, IOWA_DM_FREE, objectP, ind - startInd, &dataP[startInd]);
    }
}

iowa_status_t object_readRequest(iowa_context_t contextP,
                                 iowa_lwm2m_uri_t *uriP,
                                 uint16_t serverShortId)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;
    size_t dataCount;
    iowa_lwm2m_data_t *dataArray;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &(uriP->objectId));
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_getResourceReadableArray(contextP, objectP, uriP, serverShortId, false, &dataCount, &dataArray);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to retrieve resource list.");
        return result;
    }

    if (dataCount == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "No values to read.");
        return IOWA_COAP_205_CONTENT;
    }

    result = prv_callDataCb(contextP, IOWA_DM_READ_REQUEST, objectP, dataCount, dataArray);

    iowa_system_free(dataArray);

    return result;
}

iowa_status_t object_checkWritePayload(iowa_context_t contextP,
                                       size_t dataCount,
                                       iowa_lwm2m_data_t *dataArray)
{
    size_t dataIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Checking %u resources", dataCount);

    if (dataCount == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "No data to write.");
        return IOWA_COAP_400_BAD_REQUEST;
    }

    dataIndex = 0;

    while (dataIndex < dataCount)
    {
        lwm2m_object_t *objectP;
        size_t objLen;
        size_t resIndex;
        uint16_t instanceIndex;
        uint16_t resourceIndex;
        size_t instIndex;

        objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &dataArray[dataIndex].objectID);
        if (NULL == objectP)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", dataArray[dataIndex].objectID);
            return IOWA_COAP_404_NOT_FOUND;
        }

        objLen = 1;
        while (dataIndex + objLen < dataCount
               && dataArray[dataIndex].objectID == dataArray[dataIndex + objLen].objectID)
        {
            objLen++;
        }

        instanceIndex = object_getInstanceIndex(objectP, dataArray[dataIndex].instanceID);

        if (instanceIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found in Object %u.", dataArray[dataIndex].instanceID, dataArray[dataIndex].objectID);
            return IOWA_COAP_404_NOT_FOUND;
        }

        instIndex = dataIndex;
        while (instIndex < objLen)
        {
            size_t instLen;

            instLen = 1;
            while (instIndex + instLen < objLen
                   && dataArray[instIndex].instanceID == dataArray[instIndex + instLen].instanceID)
            {
                instLen++;
            }

            for (resIndex = instIndex; resIndex - instIndex < instLen; resIndex++)
            {
                iowa_status_t result;

                resourceIndex = prv_getResourceIndex(objectP, instanceIndex, dataArray[resIndex].resourceID);

                if (resourceIndex == objectP->resourceCount)
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource /%u/%u/%u not found.", dataArray[resIndex].objectID, dataArray[resIndex].instanceID, dataArray[resIndex].resourceID);
                    return IOWA_COAP_404_NOT_FOUND;
                }
                if (!IS_RSC_WRITABLE(objectP->resourceArray[resourceIndex]))
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource /%u/%u/%u is not writable.", dataArray[resIndex].objectID, dataArray[resIndex].instanceID, dataArray[resIndex].resourceID);
                    return IOWA_COAP_405_METHOD_NOT_ALLOWED;
                }
                result = prv_checkDataConsistency(&(dataArray[resIndex]), objectP, resourceIndex);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    return result;
                }
            }

            instIndex += instLen;
        }

        dataIndex += objLen;
    }

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Exiting on success.");

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t object_bootstrapWrite(iowa_context_t contextP,
                                    size_t dataCount,
                                    iowa_lwm2m_data_t *dataP)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &dataP[0].objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", dataP[0].objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_adaptCreatePayload(objectP, &dataCount, dataP, true);
    if (result == IOWA_COAP_NO_ERROR)
    {
        if (dataCount == 0)
        {
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Payload contains only unsupported resources.");
            result = IOWA_COAP_NO_ERROR;
        }
        else
        {
            size_t i;

            i = 0;
            while (i < dataCount
                   && result == IOWA_COAP_NO_ERROR)
            {
                size_t n;
                uint16_t instanceIndex;

                if (dataP[0].instanceID == IOWA_LWM2M_ID_ALL)
                {
                    n = dataCount;
                }
                else
                {
                    n = 1;
                    while (i + n < dataCount
                           && dataP[i].instanceID == dataP[i + n].instanceID)
                    {
                        n++;
                    }
                }

                instanceIndex = object_getInstanceIndex(objectP, dataP[i].instanceID);
                if (instanceIndex == objectP->instanceCount)
                {
                    result = prv_create(contextP, objectP, n, dataP + i);
                }
                else
                {
                    result = prv_writeData(contextP, objectP, IOWA_LWM2M_ID_ALL, n, dataP + i);
                }

                i += n;
            }
        }
    }

    if (result == IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_204_CHANGED;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t object_write(iowa_context_t contextP,
                           uint16_t serverShortId,
                           size_t dataCount,
                           iowa_lwm2m_data_t *dataP)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;
    size_t i;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "dataCount: %u.", dataCount);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (dataCount == 0)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "No data to write.");
        return IOWA_COAP_400_BAD_REQUEST;
    }
#endif

    i = 0;
    objectP = NULL;
    result = IOWA_COAP_204_CHANGED;

    while (i < dataCount
           && result == IOWA_COAP_204_CHANGED)
    {
        size_t j;

        if (objectP == NULL
            || objectP->objID != dataP[i].objectID)
        {
            objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &dataP[i].objectID);
            if (NULL == objectP)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", dataP[i].objectID);
                return IOWA_COAP_404_NOT_FOUND;
            }
        }

        j = i + 1;
        while (j < dataCount
               && dataP[i].objectID == dataP[j].objectID)
        {
            j++;
        }

        result = prv_writeData(contextP, objectP, serverShortId, j - i, dataP + i);

        i = j;
    }

    if (result == IOWA_COAP_204_CHANGED)
    {
        clientNotificationLock(contextP, true);
        for (i = 0; i < dataCount; i++)
        {
            customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, dataP[i].resourceID);
        }
        clientNotificationLock(contextP, false);
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t object_execute(iowa_context_t contextP,
                             iowa_lwm2m_uri_t *uriP,
                             uint16_t serverShortId,
                             uint8_t *buffer,
                             size_t length)
{
    iowa_status_t result;
    lwm2m_object_t * objectP;
    uint16_t instIndex;
    uint16_t resIndex;
    iowa_lwm2m_data_t data;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &uriP->objectId);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    instIndex = object_getInstanceIndex(objectP, uriP->instanceId);
    if (instIndex == objectP->instanceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);

        return IOWA_COAP_404_NOT_FOUND;
    }

    resIndex = prv_getResourceIndex(objectP, instIndex, uriP->resourceId);
    if (resIndex == objectP->resourceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u not found.", uriP->resourceId);

        return IOWA_COAP_404_NOT_FOUND;
    }

    if (!IS_RSC_EXECUTABLE(objectP->resourceArray[resIndex]))
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u is not executable.", uriP->resourceId);

        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    data.objectID = uriP->objectId;
    data.instanceID = uriP->instanceId;
    data.resourceID = uriP->resourceId;
    data.resInstanceID = IOWA_LWM2M_ID_ALL;
    data.value.asBuffer.length = length;
    data.value.asBuffer.buffer = buffer;
    if (length != 0)
    {
        data.type = IOWA_LWM2M_TYPE_STRING;
    }
    else
    {
        data.type = IOWA_LWM2M_TYPE_UNDEFINED;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling dataCallback(IOWA_DM_EXECUTE) on /%u/%u/%u.", data.objectID, data.instanceID, data.resourceID);
    result = prv_callDataCb(contextP, IOWA_DM_EXECUTE, objectP, 1, &data);

    if (result == IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_204_CHANGED;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t object_create(iowa_context_t contextP,
                            uint16_t serverShortId,
                            size_t dataCount,
                            iowa_lwm2m_data_t *dataP)
{
    iowa_status_t result;
    lwm2m_object_t *objectP;
    iowa_lwm2m_uri_t uri;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object ID: %u.", dataP[0].objectID);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &dataP[0].objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", dataP[0].objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_adaptCreatePayload(objectP, &dataCount, dataP, false);
    if (result != IOWA_COAP_NO_ERROR)
    {
        return result;
    }

    result = prv_create(contextP, objectP, dataCount, dataP);
    if (result == IOWA_COAP_NO_ERROR)
    {
        {
            lwm2m_uri_set(&uri, dataP[0].objectID, dataP[0].instanceID, IOWA_LWM2M_ID_ALL, IOWA_LWM2M_ID_ALL);
            prv_instanceEventCallback(contextP, &uri, IOWA_EVENT_OBJECT_INSTANCE_CREATED);

            result = IOWA_COAP_201_CREATED;
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t object_delete(iowa_context_t contextP,
                            iowa_lwm2m_uri_t *uriP,
                            uint16_t serverShortId)
{
    lwm2m_object_t *objectP;
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &uriP->objectId);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (NULL == objectP->instanceCb)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u has no delete callback.", uriP->objectId);
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering");

    if (LWM2M_URI_IS_SET_INSTANCE(uriP))
    {
        uint16_t instIndex;

        instIndex = object_getInstanceIndex(objectP, uriP->instanceId);
        if (instIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);

            return IOWA_COAP_404_NOT_FOUND;
        }

        result = prv_deleteObjectInstance(contextP, objectP, uriP, serverShortId);
    }
    else
    {
        result = IOWA_COAP_202_DELETED;

        while (objectP->instanceCount != 0
               && result == IOWA_COAP_202_DELETED)
        {
            uriP->instanceId = objectP->instanceArray[0].id;

            result = prv_deleteObjectInstance(contextP, objectP, uriP, serverShortId);
        }
    }

    if (result == IOWA_COAP_202_DELETED
        && contextP->lwm2mContextP->state == STATE_DEVICE_MANAGEMENT)
    {
        prv_instanceEventCallback(contextP, uriP, IOWA_EVENT_OBJECT_INSTANCE_DELETED);
        lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
        CRIT_SECTION_LEAVE(contextP);
        INTERRUPT_SELECT(contextP);
        CRIT_SECTION_ENTER(contextP);
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t object_getList(iowa_context_t contextP,
                             uint16_t objectId,
                             link_t **linkP,
                             size_t *nbLinkP)
{
    size_t linkIndex;
    lwm2m_object_t *objectP;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    *nbLinkP = 0;

    switch (objectId)
    {
    case IOWA_LWM2M_ID_ALL:
        (*nbLinkP)++;

        for (objectP = contextP->lwm2mContextP->objectList; objectP != NULL; objectP = objectP->next)
        {
            if (objectP->instanceCount > 0)
            {
                *nbLinkP += objectP->instanceCount;
            }
            else
            {
                (*nbLinkP)++;
            }
        }
        break;

    default:
        objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
        if (objectP == NULL)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectId);
            return IOWA_COAP_404_NOT_FOUND;
        }

        if (objectP->instanceCount > 0)
        {
            *nbLinkP += objectP->instanceCount;
        }
        else
        {
            (*nbLinkP)++;
        }
    }

    *linkP = (link_t *)iowa_system_malloc(*nbLinkP * sizeof(link_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*linkP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*nbLinkP * sizeof(link_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*linkP, 0, *nbLinkP * sizeof(link_t));

    linkIndex = 0;
    switch (objectId)
    {
    case IOWA_LWM2M_ID_ALL:
        LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
        linkIndex++;

        for (objectP = contextP->lwm2mContextP->objectList; objectP != NULL; objectP = objectP->next)
        {
            if (objectP->instanceCount > 0)
            {
                uint16_t instanceIndex;

                for (instanceIndex = 0; instanceIndex < objectP->instanceCount; instanceIndex++)
                {
                    LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                    (*linkP)[linkIndex].uri.objectId = objectP->objID;
                    (*linkP)[linkIndex].uri.instanceId = objectP->instanceArray[instanceIndex].id;
                    linkIndex++;
                }
            }
            else
            {
                LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                (*linkP)[linkIndex].uri.objectId = objectP->objID;
                linkIndex++;
            }
        }
        break;

    default:
        if (objectP->instanceCount > 0)
        {
            uint16_t instanceIndex;

            for (instanceIndex = 0; instanceIndex < objectP->instanceCount; instanceIndex++)
            {
                LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                (*linkP)[linkIndex].uri.objectId = objectP->objID;
                (*linkP)[linkIndex].uri.instanceId = objectP->instanceArray[instanceIndex].id;
                linkIndex++;
            }
        }
        else
        {
            LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
            (*linkP)[linkIndex].uri.objectId = objectP->objID;
        }
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t object_getTree(iowa_context_t contextP,
                             iowa_lwm2m_uri_t *uriP,
                             lwm2m_server_t *serverP,
                             link_t **linkP,
                             size_t *nbLinkP)
{
    uint16_t instanceIndex;
    uint16_t resourceIndex;
    size_t linkIndex;
    lwm2m_object_t *objectP;
    lwm2m_uri_depth_t uriDepth;
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    uriDepth = dataUtilsGetUriDepth(uriP);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_ROOT:
    case LWM2M_URI_DEPTH_RESOURCE_INSTANCE:
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Discover on root or resource instance.");
        return IOWA_COAP_402_BAD_OPTION;

    default:
        break;
    }
#endif

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &uriP->objectId);
    if (objectP == NULL)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", uriP->objectId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    instanceIndex = 0;
    resourceIndex = 0;

    *nbLinkP = 1;
    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_OBJECT:
        *nbLinkP += objectP->instanceCount;

        switch (objectP->type)
        {
        case OBJECT_MULTIPLE_ADVANCED:
            for (instanceIndex = 0; instanceIndex < objectP->instanceCount; instanceIndex++)
            {
                *nbLinkP += objectP->instanceArray[instanceIndex].resCount;
            }
            break;

        default:
            *nbLinkP += (size_t)(objectP->instanceCount * objectP->resourceCount);
        }
        break;

    case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
        switch (objectP->type)
        {
        case OBJECT_MULTIPLE_ADVANCED:
            instanceIndex = object_getInstanceIndex(objectP, uriP->instanceId);
            if (instanceIndex == objectP->instanceCount)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);
                return IOWA_COAP_404_NOT_FOUND;
            }

            *nbLinkP += objectP->instanceArray[instanceIndex].resCount;
            break;

        case OBJECT_SINGLE_ADVANCED:
            instanceIndex = object_getInstanceIndex(objectP, uriP->instanceId);
            if (instanceIndex == objectP->instanceCount)
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);
                return IOWA_COAP_404_NOT_FOUND;
            }

            *nbLinkP += objectP->resourceCount;
            break;

        default:
            *nbLinkP += objectP->resourceCount;
        }
        break;

    case LWM2M_URI_DEPTH_RESOURCE:
        instanceIndex = object_getInstanceIndex(objectP, uriP->instanceId);
        if (instanceIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);
            return IOWA_COAP_404_NOT_FOUND;
        }

        resourceIndex = prv_getResourceIndex(objectP, instanceIndex, uriP->resourceId);
        if (resourceIndex == objectP->resourceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u not found.", uriP->resourceId);
            return IOWA_COAP_404_NOT_FOUND;
        }
        break;

    default:
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Uri depth not allowed.");
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    *linkP = (link_t *)iowa_system_malloc(*nbLinkP * sizeof(link_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*linkP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*nbLinkP * sizeof(link_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*linkP, 0, *nbLinkP * sizeof(link_t));

    linkIndex = 0;
    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_OBJECT:
    {
        attributes_t attr;

        LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
        (*linkP)[linkIndex].uri.objectId = uriP->objectId;

        if (attributesGet(serverP, &(*linkP)[linkIndex].uri, &attr, false, false) == true)
        {
            iowa_status_t result;

            result = prv_convertAttributes(&attr, &(*linkP)[linkIndex].attrP);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert the attributes.");
                coreLinkFree(*linkP, *nbLinkP);
                return result;
            }
        }

        linkIndex++;

        for (instanceIndex = 0; instanceIndex < objectP->instanceCount; instanceIndex++)
        {
            LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
            (*linkP)[linkIndex].uri.objectId = uriP->objectId;
            (*linkP)[linkIndex].uri.instanceId = objectP->instanceArray[instanceIndex].id;
            linkIndex++;

            switch (objectP->type)
            {
            case OBJECT_MULTIPLE_ADVANCED:
                for (resourceIndex = 0; resourceIndex < objectP->instanceArray[instanceIndex].resCount; resourceIndex++)
                {
                    LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                    (*linkP)[linkIndex].uri.objectId = uriP->objectId;
                    (*linkP)[linkIndex].uri.instanceId = objectP->instanceArray[instanceIndex].id;
                    (*linkP)[linkIndex].uri.resourceId = objectP->instanceArray[instanceIndex].resArray[resourceIndex];
                    linkIndex++;
                }
                break;

            default:
                for (resourceIndex = 0; resourceIndex < objectP->resourceCount; resourceIndex++)
                {
                    LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                    (*linkP)[linkIndex].uri.objectId = uriP->objectId;
                    (*linkP)[linkIndex].uri.instanceId = objectP->instanceArray[instanceIndex].id;
                    (*linkP)[linkIndex].uri.resourceId = objectP->resourceArray[resourceIndex].id;
                    linkIndex++;
                }
            }
        }
        break;
    }

    case LWM2M_URI_DEPTH_OBJECT_INSTANCE:
    {
        attributes_t attr;
        iowa_status_t result;

        instanceIndex = object_getInstanceIndex(objectP, uriP->instanceId);
        if (instanceIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);
            iowa_system_free(*linkP);
            return IOWA_COAP_404_NOT_FOUND;
        }

        LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
        (*linkP)[linkIndex].uri.objectId = uriP->objectId;
        (*linkP)[linkIndex].uri.instanceId = uriP->instanceId;

        if (attributesGet(serverP, &(*linkP)[linkIndex].uri, &attr, true, false) == true)
        {
            result = prv_convertAttributes(&attr, &(*linkP)[linkIndex].attrP);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert the attributes.");
                coreLinkFree(*linkP, *nbLinkP);
                return result;
            }
        }
        linkIndex++;

        switch (objectP->type)
        {
        case OBJECT_MULTIPLE_ADVANCED:
            for (resourceIndex = 0; resourceIndex < objectP->instanceArray[instanceIndex].resCount; resourceIndex++)
            {
                uint16_t resIndex;

                LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                (*linkP)[linkIndex].uri.objectId = uriP->objectId;
                (*linkP)[linkIndex].uri.instanceId = uriP->instanceId;
                (*linkP)[linkIndex].uri.resourceId = objectP->instanceArray[instanceIndex].resArray[resourceIndex];

                resIndex = prv_getResourceIndex(objectP, instanceIndex, objectP->instanceArray[instanceIndex].resArray[resourceIndex]);
                result = prv_getResourceAttributes(contextP, objectP, serverP, &(*linkP)[linkIndex], instanceIndex, resIndex, false);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert the attributes.");
                    coreLinkFree(*linkP, *nbLinkP);
                    return result;
                }

                linkIndex++;
            }
            break;

        default:
            for (resourceIndex = 0; resourceIndex < objectP->resourceCount; resourceIndex++)
            {
                LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
                (*linkP)[linkIndex].uri.objectId = uriP->objectId;
                (*linkP)[linkIndex].uri.instanceId = uriP->instanceId;
                (*linkP)[linkIndex].uri.resourceId = objectP->resourceArray[resourceIndex].id;

                result = prv_getResourceAttributes(contextP, objectP, serverP, &(*linkP)[linkIndex], instanceIndex, resourceIndex, false);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert the attributes.");
                    coreLinkFree(*linkP, *nbLinkP);
                    return result;
                }

                linkIndex++;
            }
        }

        break;
    }

    case LWM2M_URI_DEPTH_RESOURCE:
    {
        iowa_status_t result;

        LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);
        (*linkP)[linkIndex].uri.objectId = uriP->objectId;
        (*linkP)[linkIndex].uri.instanceId = uriP->instanceId;
        (*linkP)[linkIndex].uri.resourceId = uriP->resourceId;

        result = prv_getResourceAttributes(contextP, objectP, serverP, &(*linkP)[linkIndex], instanceIndex, resourceIndex, true);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert the attributes.");
            coreLinkFree(*linkP, *nbLinkP);
            return result;
        }
        break;
    }

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t customObjectAdd(iowa_context_t contextP,
                              uint16_t objectID,
                              lwm2m_object_type_t type,
                              uint16_t instanceCount,
                              void *instanceIDs,
                              uint16_t resourceCount,
                              iowa_lwm2m_resource_desc_t *resourceArray,
                              iowa_RWE_callback_t dataCallback,
                              iowa_CD_callback_t instanceCallback,
                              iowa_RI_callback_t resInstanceCallback,
                              void *userData)
{
    lwm2m_object_t *objectP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Adding new custom object with ID: %u, instanceCount: %u and resourceCount: %u", objectID, instanceCount, resourceCount);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID);
    if (objectP != NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object %u already exists.", objectID);
        return IOWA_COAP_409_CONFLICT;
    }

    objectP = (lwm2m_object_t *)iowa_system_malloc(sizeof(lwm2m_object_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (objectP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(lwm2m_object_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(objectP, 0, sizeof(lwm2m_object_t));

    objectP->objID = objectID;

    objectP->resourceArray = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resourceCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (objectP->resourceArray == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(resourceCount * sizeof(iowa_lwm2m_resource_desc_t));
        iowa_system_free(objectP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    if (type == OBJECT_SINGLE)
    {
        iowa_status_t result;

        IOWA_LOG_INFO(IOWA_PART_LWM2M, "This is a single instance Object.");
        result = prv_addInstance(objectP, LWM2M_SINGLE_OBJECT_INSTANCE_ID, 0, NULL);

        if (result != IOWA_COAP_NO_ERROR)
        {
            iowa_system_free(objectP->resourceArray);
            iowa_system_free(objectP);
            return result;
        }
    }
    else
    {
        if (instanceCount != 0)
        {
            uint16_t i;

            objectP->instanceArray = (lwm2m_instance_details_t *)iowa_system_malloc(instanceCount * sizeof(lwm2m_instance_details_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (objectP->instanceArray == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(instanceCount * sizeof(lwm2m_instance_details_t));
                iowa_system_free(objectP->resourceArray);
                iowa_system_free(objectP);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            memset(objectP->instanceArray, 0, instanceCount * sizeof(lwm2m_instance_details_t));
            for (i = 0; i < instanceCount; i++)
            {
                switch (type)
                {
                case OBJECT_MULTIPLE_ADVANCED:
                    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Adding instance %u with %u resources to Object %u.", ((lwm2m_instance_details_t *)instanceIDs)[i].id, ((lwm2m_instance_details_t *)instanceIDs)[i].resCount, objectID);
                    objectP->instanceArray[i].id = ((lwm2m_instance_details_t *)instanceIDs)[i].id;
                    objectP->instanceArray[i].resCount = ((lwm2m_instance_details_t *)instanceIDs)[i].resCount;
                    if (objectP->instanceArray[i].resCount != 0)
                    {
                        objectP->instanceArray[i].resArray = (uint16_t *)iowa_system_malloc(objectP->instanceArray[i].resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                        if (objectP->instanceArray[i].resArray == NULL)
                        {
                            IOWA_LOG_ERROR_MALLOC(objectP->instanceArray[i].resCount * sizeof(uint16_t));
                            iowa_system_free(objectP->instanceArray);
                            iowa_system_free(objectP->resourceArray);
                            iowa_system_free(objectP);
                            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                        }
#endif
                        memcpy(objectP->instanceArray[i].resArray, ((lwm2m_instance_details_t *)instanceIDs)[i].resArray, objectP->instanceArray[i].resCount * sizeof(uint16_t));
                    }
                    else
                    {
                        objectP->instanceArray[i].resArray = NULL;
                    }
                    break;

                default:
                    objectP->instanceArray[i].id = ((uint16_t *)instanceIDs)[i];
                    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Adding instance %u to Object %u.", ((uint16_t *)instanceIDs)[i], objectID);
                }
            }
            objectP->instanceCount = instanceCount;
        }
    }

    objectP->type = type;
    objectP->resourceCount = resourceCount;
    objectP->dataCb = dataCallback;
    objectP->instanceCb = instanceCallback;
    objectP->resInstanceCb = resInstanceCallback;
    objectP->userData = userData;

    memcpy(objectP->resourceArray, resourceArray, resourceCount * sizeof(iowa_lwm2m_resource_desc_t));

    contextP->lwm2mContextP->objectList = (lwm2m_object_t *)IOWA_UTILS_LIST_ADD(contextP->lwm2mContextP->objectList, objectP);

    if (contextP->lwm2mContextP->state == STATE_DEVICE_MANAGEMENT)
    {
        lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
        CRIT_SECTION_LEAVE(contextP);
        INTERRUPT_SELECT(contextP);
        CRIT_SECTION_ENTER(contextP);
    }

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Custom object added.");

    return IOWA_COAP_NO_ERROR;
}

void customObjectDelete(lwm2m_object_t *objectP)
{
    uint16_t i;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Deleting custom object with ID: %u.", objectP->objID);

    iowa_system_free(objectP->resourceArray);

    for (i = 0; i < objectP->instanceCount; i++)
    {
        iowa_system_free(objectP->instanceArray[i].resArray);
    }
    iowa_system_free(objectP->instanceArray);

    iowa_system_free(objectP);

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Custom object deleted.");
}

iowa_status_t customObjectRemove(iowa_context_t contextP,
                                 uint16_t objectID)
{
    lwm2m_object_t *objectP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Removing custom object with ID: %u", objectID);

    contextP->lwm2mContextP->objectList = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND_AND_REMOVE(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID, &objectP);
    if (objectP == NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object ID %u not found.", objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    customObjectDelete(objectP);

    if (contextP->lwm2mContextP->state == STATE_DEVICE_MANAGEMENT)
    {
        lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
        CRIT_SECTION_LEAVE(contextP);
        INTERRUPT_SELECT(contextP);
        CRIT_SECTION_ENTER(contextP);
    }

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Custom object removed");

    return IOWA_COAP_NO_ERROR;
}

void customObjectResourceChanged(iowa_context_t contextP,
                                 uint16_t objectID,
                                 uint16_t instanceID,
                                 uint16_t resourceID)
{
    iowa_lwm2m_uri_t uri;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Ressource %u on object /%u/%u changed", resourceID, objectID, instanceID);

    uri.objectId = objectID;
    uri.instanceId = instanceID;
    uri.resourceId = resourceID;
    uri.resInstanceId = IOWA_LWM2M_ID_ALL;

    if (object_checkReadable(contextP, &uri) == IOWA_COAP_205_CONTENT)
    {
        lwm2m_resource_value_changed(contextP, &uri);
        CRIT_SECTION_LEAVE(contextP);
        INTERRUPT_SELECT(contextP);
        CRIT_SECTION_ENTER(contextP);
    }
}

iowa_status_t objectAddInstance(iowa_context_t contextP,
                                uint16_t objectID,
                                uint16_t instanceID,
                                uint16_t resourceCount,
                                uint16_t *resourceArray)
{
    lwm2m_object_t * objectP;
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Adding instance %u to Object %u. ", instanceID, objectID);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    switch (objectP->type)
    {
    case OBJECT_SINGLE:
        result = IOWA_COAP_405_METHOD_NOT_ALLOWED;
        break;

    case OBJECT_SINGLE_ADVANCED:
        if (instanceID == 0)
        {
            result = prv_addInstance(objectP, instanceID, 0, NULL);
        }
        else
        {
            result = IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        break;

    case OBJECT_MULTIPLE:
        result = prv_addInstance(objectP, instanceID, 0, NULL);
        break;

    case OBJECT_MULTIPLE_ADVANCED:
        if (resourceCount != 0
            && resourceArray != NULL)
        {
            result = prv_addInstance(objectP, instanceID, resourceCount, resourceArray);
        }
        else
        {
            result = IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        break;

    default:
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (result == IOWA_COAP_NO_ERROR
        && contextP->lwm2mContextP->state == STATE_DEVICE_MANAGEMENT)
    {
        lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
        CRIT_SECTION_LEAVE(contextP);
        INTERRUPT_SELECT(contextP);
        CRIT_SECTION_ENTER(contextP);
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectRemoveInstance(iowa_context_t contextP,
                                   uint16_t objectID,
                                   uint16_t instanceID)
{
    lwm2m_object_t * objectP;
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Removing instance %u of Object %u. ", instanceID, objectID);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_removeInstance(objectP, instanceID);

    if (result == IOWA_COAP_NO_ERROR
        && contextP->lwm2mContextP->state == STATE_DEVICE_MANAGEMENT)
    {
        switch (objectID)
        {
        case IOWA_LWM2M_SECURITY_OBJECT_ID:
            break;

        default:
            lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
            CRIT_SECTION_LEAVE(contextP);
            INTERRUPT_SELECT(contextP);
            CRIT_SECTION_ENTER(contextP);
        }
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

bool customObjectHasResource(iowa_context_t contextP,
                             uint16_t objectID,
                             uint16_t instanceID,
                             uint16_t resourceID)
{
    lwm2m_object_t *objectP;
    uint16_t instIndex;
    uint16_t resIndex;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Looking for resource %u in instance %u of Object %u. ", resourceID, instanceID, objectID);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectID);
        return false;
    }

    if (instanceID != IOWA_LWM2M_ID_ALL)
    {

        instIndex = object_getInstanceIndex(objectP, instanceID);
        if (instIndex == objectP->instanceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", instanceID);

            return false;
        }
    }
    else
    {
        instIndex = objectP->instanceCount;
    }

    resIndex = prv_getResourceIndex(objectP, instIndex, resourceID);

    if (resIndex == objectP->resourceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u not found.", resourceID);
        return false;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u found.", resourceID);
    return true;
}

#endif
