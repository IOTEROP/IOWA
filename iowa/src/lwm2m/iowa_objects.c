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
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
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

#define PRV_DEFAULT_MANDATORY_OBJECT_MINOR_VERSION    1
#define PRV_DEFAULT_MAJOR_OBJECT_VERSION              1
#define PRV_DEFAULT_MINOR_OBJECT_VERSION              0
#define PRV_MAX_VERSION_LENGTH                        7  // 255(3) + .(1) + 255(3) = 7

// Call object's data callback by getting out critical section.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - op: The operation to perform on the resource among **IOWA_DM_READ**, **IOWA_DM_FREE**, **IOWA_DM_WRITE** and **IOWA_DM_EXECUTE**.
// - objectP: object's information
// - dataCount, dataP: data concerning by the operation
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
    // WARNING: This function is called in a critical section
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
    lwm2m_instance_details_t *newArray;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Adding instance %u with %u resources to Object %u.", id, resourceCount, objectP->objID);

    if (IOWA_COAP_404_NOT_FOUND != object_getInstanceIndex(objectP, id, NULL))
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

        if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, id, &instIndex))
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
    uint16_t index;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Looking for resource %u in Object %u, instance index: %u.", id, objectP->objID, instIndex);

    for (index = 0; index < objectP->resourceCount; index++)
    {
        if (objectP->resourceArray[index].id == id)
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u found at index %u in Object %u.", id, index, objectP->objID);
            break;
        }
    }
    if (index == objectP->resourceCount)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u does not exist in Object %u.", id, objectP->objID);
        return objectP->resourceCount;
    }

    if (instIndex < objectP->instanceCount
        && objectP->instanceArray[instIndex].resArray != NULL)
    {
        uint16_t i;

        // check if resource exists in this instance
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
            index = objectP->resourceCount;
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource %u not found in Object %u.", id, objectP->objID);

    return index;
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
    memset(newArray, 0, newSize * sizeof(iowa_lwm2m_data_t));

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
        // multiple resource
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
            if (resInstanceId != IOWA_LWM2M_ID_ALL)
            {
                dataSizeNeeded = 1;
            }
            else
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
                if (resInstanceId == IOWA_LWM2M_ID_ALL
                    || resInstanceArray[i] == resInstanceId)
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
        if (resInstanceId != IOWA_LWM2M_ID_ALL
            && *indexP == 0)
        {
            return IOWA_COAP_404_NOT_FOUND;
        }
    }
    else
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource /%u/n/%u is a single one.", objectP->objID, objectP->resourceArray[resIndex].id);

        if (resInstanceId != IOWA_LWM2M_ID_ALL)
        {
            return IOWA_COAP_404_NOT_FOUND;
        }

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
        // single instance read

        result = object_getInstanceIndex(objectP, uriP->instanceId, &instIndex);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);

            return result;
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
            // multiple resource read

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
        // multiple object instances read
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

// TODO: Temporary function
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
    size_t dataIndex;          // index in dataArray
    uint16_t resourceIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Checking %u resources. bootstapWrite: %s.", *dataCountP, bootstapWrite ? "true" : "false");

    // check that we have only one Object and, if not allowed, one Object Instance
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

            // Move the resource not found at the end of the list and decrement the data count
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
        // check that all mandatory resources are present
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
    // WARNING: This function is called in a critical section
    iowa_status_t result;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object ID: %u, dataCount: %u.\nCalling dataCallback(IOWA_DM_WRITE) on:", objectP->objID, dataCount);

#if (IOWA_LOG_LEVEL >= IOWA_LOG_LEVEL_INFO)
    {
        size_t index;

        for (index = 0; index < dataCount; index++)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "\t/%u/%u/%u with type %s", objectP->objID, dataArrayP[index].instanceID, dataArrayP[index].resourceID, STR_LWM2M_TYPE(dataArrayP[index].type));
        }
    }
#endif

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
    // WARNING: This function is called in a critical section
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
        if (IOWA_COAP_404_NOT_FOUND != object_getInstanceIndex(objectP, instanceId, NULL))
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

static iowa_status_t prv_deleteObjectResInstance(iowa_context_t contextP,
                                                 lwm2m_object_t *objectP,
                                                 iowa_lwm2m_uri_t *uriP,
                                                 uint16_t serverShortId)
{
    iowa_status_t result;
    iowa_lwm2m_data_t data;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Calling dataCallback(IOWA_DM_DELETE) on /%u/%u/%u.", objectP->objID, uriP->instanceId, uriP->resourceId);

    memset(&data, 0, sizeof(iowa_lwm2m_data_t));

    data.objectID = uriP->objectId;
    data.instanceID = uriP->instanceId;
    data.resourceID = uriP->resourceId;
    data.resInstanceID = IOWA_LWM2M_ID_ALL;

    CRIT_SECTION_LEAVE(contextP);
    result = objectP->dataCb(IOWA_DM_DELETE, &data, 1, objectP->userData, contextP);
    CRIT_SECTION_ENTER(contextP);

    return result;
}

static void prv_instanceEventCallback(iowa_context_t contextP,
                                      uint16_t serverShortId,
                                      iowa_lwm2m_uri_t * uriP,
                                      iowa_event_type_t eventType)
{
    iowa_event_t event;

    if (contextP->eventCb != NULL)
    {
        event.eventType = eventType;
        event.details.objectInstance.uriP = uriP;
        event.serverShortId = serverShortId;

        CRIT_SECTION_LEAVE(contextP);
        contextP->eventCb(&event, contextP->userData, contextP);
        CRIT_SECTION_ENTER(contextP);
    }
}

iowa_status_t object_getInstanceIndex(lwm2m_object_t *objectP,
                                      uint16_t id,
                                      uint16_t *indexP)
{
    uint16_t index;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Looking for instance %u in Object %u.", id, objectP->objID);

    if (id == IOWA_LWM2M_ID_ALL)
    {
        return IOWA_COAP_404_NOT_FOUND;
    }

    index = objectP->instanceCount;

    if (OBJECT_SINGLE == objectP->type)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "This is a single-instance Object.");
        if (id == LWM2M_SINGLE_OBJECT_INSTANCE_ID)
        {
            index = 0;
        }
    }
    else
    {
        for (index = 0; index < objectP->instanceCount; index++)
        {
            if (objectP->instanceArray[index].id == id)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Instance %u found at index %u in Object %u.", id, index, objectP->objID);
                break;
            }
        }
    }

    if (indexP != NULL)
    {
       *indexP = index;
    }

    if (index == objectP->instanceCount)
    {
        return IOWA_COAP_404_NOT_FOUND;
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t object_find(iowa_context_t contextP,
                          uint16_t objectID,
                          uint16_t instanceID,
                          uint16_t resourceID,
                          lwm2m_object_t **objectPP,
                          uint16_t *instanceIndexP,
                          uint16_t *resourceIndexP)
{
    lwm2m_object_t *objectP;
    uint16_t instIndex;
    uint16_t resIndex;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Looking for /%u/%u/%u.", objectID, instanceID, resourceID);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object %u not found.", objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (instanceID != IOWA_LWM2M_ID_ALL)
    {
        if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, instanceID, &instIndex))
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance /%u/%u not found.", objectID, instanceID);

            return IOWA_COAP_404_NOT_FOUND;
        }
    }
    else
    {
        instIndex = objectP->instanceCount;
    }

    if (resourceID != IOWA_LWM2M_ID_ALL)
    {
        resIndex = prv_getResourceIndex(objectP, instIndex, resourceID);
        if (resIndex == objectP->resourceCount)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Resource %u not found.", resourceID);
            return IOWA_COAP_404_NOT_FOUND;
        }
    }
    else
    {
        resIndex = objectP->resourceCount;
    }

    if (objectPP != NULL)
    {
        *objectPP = objectP;
    }
    if (instanceIndexP != NULL)
    {
        *instanceIndexP = instIndex;
    }
    if (resourceIndexP != NULL)
    {
        *resourceIndexP = resIndex;
    }

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_addObjectVersion(lwm2m_object_t *objectP,
                                          link_t *linkP,
                                          size_t *linkIndex)
{
    iowa_status_t result;

    result = IOWA_COAP_NO_ERROR;

    if (objectP->version.major != PRV_DEFAULT_MAJOR_OBJECT_VERSION || objectP->version.minor != PRV_DEFAULT_MINOR_OBJECT_VERSION)
    {
        uint8_t version[PRV_MAX_VERSION_LENGTH];
        size_t bufferLength;
        size_t convertLength;
        size_t versionLength;

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Adding version attribute to Object %d.", objectP->objID);

        bufferLength = PRV_MAX_VERSION_LENGTH;

        // major version
        convertLength = dataUtilsIntToBuffer(objectP->version.major, version, bufferLength, false);
        if (convertLength == 0)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert Object major version to string.");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        // dot
        version[convertLength] = '.';
        versionLength = convertLength + 1;
        bufferLength -= convertLength + 1;
        // minor version
        convertLength = dataUtilsIntToBuffer(objectP->version.minor, version + versionLength, bufferLength, false);
        if (convertLength == 0)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to convert Object minor version to string.");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        versionLength += convertLength;

        result = coreLinkAddBufferAttribute(linkP + *linkIndex, KEY_OBJECT_VERSION, (uint8_t *)version, versionLength, false);
    }

    return result;
}

iowa_status_t object_getTargets(iowa_context_t contextP,
                                uint16_t objectId,
                                lwm2m_object_t **startPP,
                                lwm2m_object_t **endPP)
{
    if (IOWA_LWM2M_ID_ALL == objectId)
    {
        *startPP = contextP->lwm2mContextP->objectList;
        *endPP = NULL;
    }
    else
    {
        *startPP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
        if (*startPP == NULL)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object with ID %u not found.", objectId);
            return IOWA_COAP_404_NOT_FOUND;
        }
        *endPP = (*startPP)->next;
    }

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t object_checkReadable(iowa_context_t contextP,
                                   uint16_t serverShortId,
                                   iowa_lwm2m_uri_t * uriP)
{
    iowa_status_t result;
    lwm2m_object_t * targetP;
    uint16_t instIndex;
    uint16_t resIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "SSID: %u, URI: /%u/%u/%u", serverShortId, uriP->objectId, uriP->instanceId, uriP->resourceId);

    result = object_find(contextP, uriP->objectId, uriP->instanceId, uriP->resourceId, &targetP, &instIndex, &resIndex);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "URI not found.");
        return IOWA_COAP_404_NOT_FOUND;
    }

    if (LWM2M_URI_IS_SET_RESOURCE(uriP))
    {
        if (!IS_RSC_READABLE(targetP->resourceArray[resIndex]))
        {
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u/%u/%u is not readable.", uriP->objectId, uriP->instanceId, uriP->resourceId);

            return IOWA_COAP_405_METHOD_NOT_ALLOWED;
        }

    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "/%u/%u/%u is readable.", uriP->objectId, uriP->instanceId, uriP->resourceId);

    return IOWA_COAP_205_CONTENT;
}

bool object_checkResourceFlag(iowa_context_t contextP,
                              iowa_lwm2m_uri_t *uriP,
                              uint8_t flag)
{
    lwm2m_object_t *objectP;
    uint16_t resIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Checking flag 0x%02X of URI: /%u/%u/%u", flag, uriP->objectId, uriP->instanceId, uriP->resourceId);

    if (object_find(contextP, uriP->objectId, uriP->instanceId, uriP->resourceId, &objectP, NULL, &resIndex) != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "URI not found.");
        return false;
    }

    if ((objectP->resourceArray[resIndex].flags & flag) == flag)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Resource /%u/I/%u is multiple.", uriP->objectId, objectP->resourceArray[resIndex].id);
        return true;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "No multiple resource in Object %u.", uriP->objectId);

    return false;
}

iowa_lwm2m_data_type_t object_getResourceType(uint16_t objectId,
                                              uint16_t resourceId,
                                              void *userDataP)
{
    iowa_context_t contextP;
    lwm2m_object_t *objectP;
    uint16_t resIndex;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "objectId: %u, resourceId: %u", objectId, resourceId);

    contextP = (iowa_context_t)userDataP;

    if (object_find(contextP, objectId, IOWA_LWM2M_ID_ALL, resourceId, &objectP, NULL, &resIndex) != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "URI not found.");
        return IOWA_LWM2M_TYPE_UNDEFINED;
    }

    return objectP->resourceArray[resIndex].type;
}

iowa_status_t object_read(iowa_context_t contextP,
                          iowa_lwm2m_uri_t *uriP,
                          uint16_t serverShortId,
                          size_t *dataCountP,
                          iowa_lwm2m_data_t **dataArrayP)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;
    lwm2m_object_t *objectP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId);

    result = object_find(contextP, uriP->objectId, uriP->instanceId, uriP->resourceId, &objectP, NULL, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "URI not found.");
        return result;
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

iowa_status_t object_checkWritePayload(iowa_context_t contextP,
                                       size_t dataCount,
                                       iowa_lwm2m_data_t *dataArray)
{
    size_t dataIndex;          // index in dataArray

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
        size_t objLen;           // number of data_t matching an Object
        size_t resIndex;         // index of a data_t matching a Resource of an Object or Object Instance in dataArray
        uint16_t resourceIndex;  // index of a Resource inside a lwm2m_object_t
        size_t instIndex;        // index of the first data_t matching the beginning of an Object Instance in dataArray

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

        instIndex = dataIndex;
        while (instIndex < objLen)
        {
            size_t instLen;   // number of data_t matching an Object Instance
            uint16_t instanceIndex;  // index of an Instance inside a lwm2m_object_t

            if (instIndex == 0
                || dataArray[instIndex].instanceID != dataArray[instIndex-1].instanceID)
            {
                // check that the targeted instance exist
                if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, dataArray[instIndex].instanceID, &instanceIndex))
                {
                    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found in Object %u.", dataArray[instIndex].instanceID, dataArray[instIndex].objectID);
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            instLen = 1;
            while (instIndex + instLen < objLen
                   && dataArray[instIndex].instanceID == dataArray[instIndex + instLen].instanceID)
            {
                instLen++;
            }

            for (resIndex = instIndex; resIndex - instIndex < instLen; resIndex++)
            {
                iowa_status_t result;

                // check the resource exists in the Object Instance
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

iowa_status_t object_write(iowa_context_t contextP,
                           uint16_t serverShortId,
                           size_t dataCount,
                           iowa_lwm2m_data_t *dataP,
                           bool isPartial)
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

        if (dataP[i].resInstanceID != IOWA_LWM2M_ID_ALL
            && isPartial == false)
        {
            iowa_lwm2m_uri_t uri;

            uri.objectId = dataP[i].objectID;
            uri.instanceId = dataP[i].instanceID;
            uri.resourceId = dataP[i].resourceID;
            uri.resInstanceId = IOWA_LWM2M_ID_ALL;

            prv_deleteObjectResInstance(contextP, objectP, &uri, serverShortId);
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
    // WARNING: This function is called in a critical section
    iowa_status_t result;
    lwm2m_object_t * objectP;
    uint16_t instIndex;
    uint16_t resIndex;
    iowa_lwm2m_data_t data;

    (void)serverShortId;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    result = object_find(contextP, uriP->objectId, uriP->instanceId, uriP->resourceId, &objectP, &instIndex, &resIndex);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "URI not found.");
        return result;
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
            prv_instanceEventCallback(contextP, serverShortId, &uri, IOWA_EVENT_OBJECT_INSTANCE_CREATED);

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
    // WARNING: This function is called in a critical section
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
        if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, uriP->instanceId, NULL))
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);

            return IOWA_COAP_404_NOT_FOUND;
        }

        result = prv_deleteObjectInstance(contextP, objectP, uriP, serverShortId);
    }
    else
    {
        // Loop on all Object Instances
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
        prv_instanceEventCallback(contextP, serverShortId, uriP, IOWA_EVENT_OBJECT_INSTANCE_DELETED);
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
    iowa_status_t result;
    size_t linkIndex;
    lwm2m_object_t *objectP;
    lwm2m_object_t *startP;
    lwm2m_object_t *endP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering. objectId: %u.", objectId);

    result = object_getTargets(contextP, objectId, &startP, &endP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        return result;
    }

    if (IOWA_LWM2M_ID_ALL == objectId)
    {
        *nbLinkP = 1;
    }
    else
    {
        *nbLinkP = 0;
    }

    for (objectP = startP; objectP != endP && IOWA_COAP_NO_ERROR == result; objectP = objectP->next)
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

    // Allocate the links.
    *linkP = (link_t *)iowa_system_malloc(*nbLinkP * sizeof(link_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*linkP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*nbLinkP * sizeof(link_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*linkP, 0, *nbLinkP * sizeof(link_t));

    // Set the links URI
    if (IOWA_LWM2M_ID_ALL == objectId)
    {
        // Add Root URI
        LWM2M_URI_RESET(&(*linkP)[0].uri);
        linkIndex = 1;
    }
    else
    {
        linkIndex = 0;
    }

    // Add Objects URI
    for (objectP = contextP->lwm2mContextP->objectList; objectP != NULL; objectP = objectP->next)
    {
        if (objectP->instanceCount > 0)
        {
            uint16_t instanceIndex;
            iowa_status_t result;

            if (IOWA_LWM2M_ID_ALL == objectId)
            {
                result = prv_addObjectVersion(objectP, *linkP, &linkIndex);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    iowa_system_free(*linkP);
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to add object version to the payload");
                    return result;
                }
            }

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

    // Check arguments
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

    // Count the number of link.
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
            if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, uriP->instanceId, &instanceIndex))
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Instance %u not found.", uriP->instanceId);
                return IOWA_COAP_404_NOT_FOUND;
            }

            *nbLinkP += objectP->instanceArray[instanceIndex].resCount;
            break;

        default:
            *nbLinkP += objectP->resourceCount;
        }
        break;

    case LWM2M_URI_DEPTH_RESOURCE:
        // Handle resource cases.
        if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, uriP->instanceId, &instanceIndex))
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

    // Allocate the links.
    *linkP = (link_t *)iowa_system_malloc(*nbLinkP * sizeof(link_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*linkP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*nbLinkP * sizeof(link_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(*linkP, 0, *nbLinkP * sizeof(link_t));

    // Set the links URI and their attributes
    linkIndex = 0;
    switch (uriDepth)
    {
    case LWM2M_URI_DEPTH_OBJECT:
    {
        attributes_t attr;
        iowa_status_t result;

        LWM2M_URI_RESET(&(*linkP)[linkIndex].uri);

        result = prv_addObjectVersion(objectP, *linkP, &linkIndex);
        if (result != IOWA_COAP_NO_ERROR)
        {
            iowa_system_free(*linkP);
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to add object version to the payload");
            return result;
        }
        (*linkP)[linkIndex].uri.objectId = uriP->objectId;

        if (attributesGet(serverP, &(*linkP)[linkIndex].uri, &attr, false, false) == true)
        {
            // No inheritance
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

        if (IOWA_COAP_NO_ERROR != object_getInstanceIndex(objectP, uriP->instanceId, &instanceIndex))
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

                // No inheritance
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

                // No inheritance
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

        // Inheritance from object and object instance level
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
    // WARNING: This function is called in a critical section
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

    objectP->version.major = PRV_DEFAULT_MAJOR_OBJECT_VERSION;
    objectP->type = type;
    objectP->resourceCount = resourceCount;
    objectP->dataCb = dataCallback;
    objectP->instanceCb = instanceCallback;
    objectP->resInstanceCb = resInstanceCallback;
    objectP->userData = userData;

    memcpy(objectP->resourceArray, resourceArray, resourceCount * sizeof(iowa_lwm2m_resource_desc_t));

    switch (objectID)
    {
    case IOWA_LWM2M_SERVER_OBJECT_ID:
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
    default:
        objectP->version.minor = PRV_DEFAULT_MINOR_OBJECT_VERSION;
        break;
    }
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
    // WARNING: This function is called in a critical section
    lwm2m_object_t *objectP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Removing custom object with ID: %u", objectID);

    contextP->lwm2mContextP->objectList = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND_AND_REMOVE(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID, &objectP);
    if (objectP == NULL)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Object ID %u not found.", objectID);
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
    // WARNING: This function is called in a critical section
    iowa_lwm2m_uri_t uri;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Ressource %u on object /%u/%u changed", resourceID, objectID, instanceID);

    uri.objectId = objectID;
    uri.instanceId = instanceID;
    uri.resourceId = resourceID;
    uri.resInstanceId = IOWA_LWM2M_ID_ALL;

    if (object_checkReadable(contextP, IOWA_LWM2M_ID_ALL, &uri) == IOWA_COAP_205_CONTENT)
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
    // WARNING: This function is called in a critical section
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
    // WARNING: This function is called in a critical section
    lwm2m_object_t * objectP;
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Removing instance %u of Object %u. ", instanceID, objectID);

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectID);
    if (NULL == objectP)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Object %u not found.", objectID);
        return IOWA_COAP_404_NOT_FOUND;
    }

    result = prv_removeInstance(objectP, instanceID);

    if (result == IOWA_COAP_NO_ERROR
        && contextP->lwm2mContextP->state == STATE_DEVICE_MANAGEMENT)
    {
        switch (objectID)
        {
        case IOWA_LWM2M_SECURITY_OBJECT_ID:
            // No need to update registration for objects not exposed to the Server
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

bool object_hasResource(lwm2m_object_t *objectP,
                        uint16_t instIndex,
                        uint16_t resourceID)
{
    uint16_t resIndex;

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
