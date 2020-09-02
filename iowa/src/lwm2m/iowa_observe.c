
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
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
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

#if defined(LWM2M_CLIENT_MODE)

// T: threshold, N: new value, P: previous value
#define TEST_THRESHOLD(T,N,P) (((N) < (T) && (P) >= (T)) || ((N) > (T) && (P) <= (T)))

// Check if message received match with the observe set.
// Returned value: true if message matched with observe or false if not.
// Parameters:
// - OBS: observe's information, lwm2m_observed_t.
// - MSG: message received, iowa_coap_message_t.
// Note: used it only as condition (if, while ...)
#define PRV_OBSERVE_MATCH_TOKEN(OBS,MSG) ((MSG->tokenLength == OBS->tokenLen) && (memcmp(MSG->token, OBS->token, OBS->tokenLen) == 0))

static void prv_addMID(lwm2m_observed_t * observedP,
                       uint16_t mId)
{
#if LWM2M_OBSERVATION_MID_ARRAY_SIZE > 1
    memmove(observedP->lastMid + 1, observedP->lastMid, (LWM2M_OBSERVATION_MID_ARRAY_SIZE - 1) * sizeof(uint16_t));
#endif
    observedP->lastMid[0] = mId;
}

// Check if message received match with one of the last notification.
// Returned value: true if message matched with notification or false if not.
// Parameters:
// - observedP: observe's information.
// - messageP: message received.
static bool prv_notificationMatch(lwm2m_observed_t * observedP,
                                  iowa_coap_message_t * messageP)
{
    uint8_t i;

    for (i = 0; i < LWM2M_OBSERVATION_MID_ARRAY_SIZE; i++)
    {
        if (observedP->lastMid[i] == messageP->id)
        {
            return true;
        }
    }
    return false;
}

void observe_delete(lwm2m_observed_t *observedP)
{
    size_t ind;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Delete observe.");

    for (ind = 0; ind < observedP->uriCount; ind++)
    {
        iowa_system_free(observedP->uriInfoP[ind].uriAttrP);
    }
    iowa_system_free(observedP->uriInfoP);
    iowa_system_free(observedP->timeAttrP);
    iowa_system_free(observedP);
}

void observeRemoveFromServer(lwm2m_server_t *serverP)
{
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Clearing observe list.");

    while (serverP->runtime.observedList != NULL)
    {
        lwm2m_observed_t *observedP;

        observedP = serverP->runtime.observedList->next;
        observe_delete(serverP->runtime.observedList);
        serverP->runtime.observedList = observedP;
    }
}

// Update observe according with its attributes.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: LwM2M context
// - serverP: server's information.
// - observedP: observe's information.
iowa_status_t observe_updateObserve(iowa_context_t contextP,
                                    lwm2m_server_t *serverP,
                                    lwm2m_observed_t *observedP)
{
    attributes_t attr;
    uint32_t minPeriod;
    uint32_t maxPeriod;
    size_t ind;

    minPeriod = 0;
    maxPeriod = UINT32_MAX;

    for (ind = 0; ind < observedP->uriCount; ind++)
    {
        // Get the attributes for the current observation
        if (attributesGet(serverP, &observedP->uriInfoP[ind].uri, &attr, true, true) == true)
        {
            if ((attr.flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0
                && minPeriod < attr.minPeriod)
            {
                minPeriod = attr.minPeriod;
            }

            if ((attr.flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0
                && maxPeriod > attr.maxPeriod)
            {
                maxPeriod = attr.maxPeriod;
            }

            if ((attr.flags & (ATTR_FLAG_NUMERIC | LWM2M_ATTR_FLAG_MIN_EVAL_PERIOD | LWM2M_ATTR_FLAG_MAX_EVAL_PERIOD)) != 0)
            {
                if (observedP->uriInfoP[ind].uriAttrP == NULL)
                {
                    observedP->uriInfoP[ind].uriAttrP = (lwm2m_uri_attributes_t *)iowa_system_malloc(sizeof(lwm2m_uri_attributes_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                    if (observedP->uriInfoP[ind].uriAttrP == NULL)
                    {
                        IOWA_LOG_ERROR_MALLOC(sizeof(lwm2m_uri_attributes_t));
                        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    }
#endif
                    memset(observedP->uriInfoP[ind].uriAttrP, 0, sizeof(lwm2m_uri_attributes_t));
                }

                observedP->uriInfoP[ind].uriAttrP->flags = attr.flags;
                if (attr.flags & LWM2M_ATTR_FLAG_GREATER_THAN)
                {
                    observedP->uriInfoP[ind].uriAttrP->greaterThan = attr.greaterThan;
                }
                if (attr.flags & LWM2M_ATTR_FLAG_LESS_THAN)
                {
                    observedP->uriInfoP[ind].uriAttrP->lessThan = attr.lessThan;
                }
                if (attr.flags & LWM2M_ATTR_FLAG_STEP)
                {
                    observedP->uriInfoP[ind].uriAttrP->step = attr.step;
                }

                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Update observe numeric attributes (/%u/%u/%u/%u); toSet: %08X, greaterThan: %f, lessThan: %f, step: %f.",
                                  observedP->uriInfoP[ind].uri.objectId,
                                  observedP->uriInfoP[ind].uri.instanceId,
                                  observedP->uriInfoP[ind].uri.resourceId,
                                  observedP->uriInfoP[ind].uri.resInstanceId,
                                  observedP->uriInfoP[ind].uriAttrP->flags,
                                  observedP->uriInfoP[ind].uriAttrP->greaterThan,
                                  observedP->uriInfoP[ind].uriAttrP->lessThan,
                                  observedP->uriInfoP[ind].uriAttrP->step);
            }
            else
            {
                if (observedP->uriInfoP[ind].uriAttrP != NULL)
                {
                    iowa_system_free(observedP->uriInfoP[ind].uriAttrP);
                    observedP->uriInfoP[ind].uriAttrP = NULL;
                    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Remove observation numeric attributes.");
                }
            }
        }
        else if (observedP->uriInfoP[ind].uriAttrP != NULL)
        {
            iowa_system_free(observedP->uriInfoP[ind].uriAttrP);
            observedP->uriInfoP[ind].uriAttrP = NULL;
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Remove observation numeric attributes.");
        }
    }

    if (minPeriod > 0
        || maxPeriod < UINT32_MAX)
    {
        if (minPeriod > maxPeriod)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Time attributes are inconsistent: minPeriod: %d, maxPeriod: %d.", minPeriod, maxPeriod);
            return IOWA_COAP_400_BAD_REQUEST;
        }

        if (observedP->timeAttrP == NULL)
        {
            observedP->timeAttrP = (lwm2m_time_attributes_t *)iowa_system_malloc(sizeof(lwm2m_time_attributes_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (observedP->timeAttrP == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(sizeof(lwm2m_time_attributes_t));
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }

        memset(observedP->timeAttrP, 0, sizeof(lwm2m_time_attributes_t));
        if (minPeriod > 0)
        {
            observedP->timeAttrP->flags |= LWM2M_ATTR_FLAG_MIN_PERIOD;
            observedP->timeAttrP->minPeriod = minPeriod;
        }
        if (maxPeriod < UINT32_MAX)
        {
            observedP->timeAttrP->flags |= LWM2M_ATTR_FLAG_MAX_PERIOD;
            observedP->timeAttrP->maxPeriod = maxPeriod;
        }
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Time attributes toSet: %08X, minPeriod: %d, maxPeriod: %d.",
                          observedP->timeAttrP->flags,
                          observedP->timeAttrP->minPeriod,
                          observedP->timeAttrP->maxPeriod);
    }
    else
    {
        if (observedP->timeAttrP != NULL)
        {
            iowa_system_free(observedP->timeAttrP);
            observedP->timeAttrP = NULL;
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Remove observation time attributes.");
        }
    }

    coreObservationEventCallback(contextP, observedP, IOWA_EVENT_OBSERVATION_STARTED);

    return IOWA_COAP_NO_ERROR;
}

static void prv_observeRemove(iowa_context_t contextP,
                              lwm2m_server_t *serverP,
                              lwm2m_observed_t *observedP)
{
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    coreObservationEventCallback(contextP, observedP, IOWA_EVENT_OBSERVATION_CANCELED);

    if (observedP == serverP->runtime.observedList)
    {
        serverP->runtime.observedList = observedP->next;
    }
    else
    {
        lwm2m_observed_t * parentP;

        parentP = serverP->runtime.observedList;
        while (parentP != NULL
                && parentP->next != observedP)
        {
            parentP = parentP->next;
        }
        if (parentP != NULL)
        {
            parentP->next = observedP->next;
        }
    }
    observe_delete(observedP);
}

iowa_status_t observe_handleRequest(iowa_context_t contextP,
                                    size_t uriCount,
                                    iowa_lwm2m_uri_t *uriP,
                                    lwm2m_server_t *serverP,
                                    size_t dataCount,
                                    iowa_lwm2m_data_t *dataP,
                                    iowa_coap_option_t *obsOptionP,
                                    iowa_coap_message_t *requestP,
                                    iowa_coap_message_t *responseP,
                                    iowa_content_format_t format)
{
    lwm2m_observed_t *observedP;
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Observe: %u.", obsOptionP->value.asInteger);

    (void)dataCount;

    switch (obsOptionP->value.asInteger)
    {
    case LWM2M_OBSERVE_REQUEST_NEW:
    {
        iowa_status_t result;
        iowa_coap_option_t *optionP;
        size_t ind;

        if (!LWM2M_URI_IS_SET_INSTANCE(uriP)
            && LWM2M_URI_IS_SET_RESOURCE(uriP))
        {
            return IOWA_COAP_400_BAD_REQUEST;
        }
        if (requestP->tokenLength == 0)
        {
            return IOWA_COAP_400_BAD_REQUEST;
        }

        observedP = (lwm2m_observed_t *)iowa_system_malloc(sizeof(lwm2m_observed_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (observedP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(lwm2m_observed_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memset(observedP, 0, sizeof(lwm2m_observed_t));

        observedP->uriInfoP = (lwm2m_observed_uri_info_t *)iowa_system_malloc(uriCount * sizeof(lwm2m_observed_uri_info_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (observedP->uriInfoP == NULL)
        {
            iowa_system_free(observedP);
            IOWA_LOG_ERROR_MALLOC(uriCount * sizeof(lwm2m_observed_uri_info_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memset(observedP->uriInfoP, 0, uriCount * sizeof(lwm2m_observed_uri_info_t));

        observedP->uriCount = uriCount;
        for (ind = 0; ind < uriCount; ind++)
        {
            memcpy(&observedP->uriInfoP[ind].uri, &uriP[ind], sizeof(iowa_lwm2m_uri_t));
        }

        observedP->tokenLen = requestP->tokenLength;
        memcpy(observedP->token, requestP->token, requestP->tokenLength);
        observedP->counter = 0;
        observedP->lastTime = contextP->currentTime;
        observedP->format = format;

        for (ind = 0; ind < uriCount; ind++)
        {
            if (LWM2M_URI_IS_SET_RESOURCE(&observedP->uriInfoP[ind].uri)
                && !LWM2M_URI_IS_SET_RESOURCE_INSTANCE(&observedP->uriInfoP[ind].uri))
            {
                size_t dataIndex;
                        dataIndex = 0;
                        switch (dataP[dataIndex].type)
                        {
                        case IOWA_LWM2M_TYPE_INTEGER:
                        case IOWA_LWM2M_TYPE_TIME:
                        case IOWA_LWM2M_TYPE_UNSIGNED_INTEGER:
                            observedP->uriInfoP[ind].lastValue.asInteger = dataP[dataIndex].value.asInteger;
                            observedP->uriInfoP[ind].flags |= LWM2M_OBSERVE_FLAG_INTEGER;
                            break;

                        case IOWA_LWM2M_TYPE_FLOAT:
                            observedP->uriInfoP[ind].lastValue.asFloat = dataP[dataIndex].value.asFloat;
                            observedP->uriInfoP[ind].flags |= LWM2M_OBSERVE_FLAG_FLOAT;
                            break;

                        default:
                            break;
                        }
            }
        }

        result = observe_updateObserve(contextP, serverP, observedP);
        if (result != IOWA_COAP_NO_ERROR)
        {
            observe_delete(observedP);
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to retrieve the observe attributes.");
            return result;
        }

        optionP = iowa_coap_option_new(IOWA_COAP_OPTION_OBSERVE);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (optionP == NULL)
        {
            observe_delete(observedP);
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        // Add the new observation to the list
        observedP->next = serverP->runtime.observedList;
        serverP->runtime.observedList = observedP;

        optionP->value.asInteger = LWM2M_OBSERVE_REQUEST_NEW;
        observedP->counter++;
        iowa_coap_message_add_option(responseP, optionP);
        break;
    }

    case LWM2M_OBSERVE_REQUEST_CANCEL:
        observe_cancel(contextP, serverP, requestP);
        break;

    default:
        return IOWA_COAP_400_BAD_REQUEST;
    }

    return IOWA_COAP_205_CONTENT;
}

void observe_cancel(iowa_context_t contextP,
                    lwm2m_server_t *serverP,
                    iowa_coap_message_t *messageP)
{
    lwm2m_observed_t * observedP;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    observedP = serverP->runtime.observedList;
    while (observedP != NULL)
    {
        if ((messageP->type == IOWA_COAP_TYPE_RESET
            && prv_notificationMatch(observedP, messageP) == true)
            || PRV_OBSERVE_MATCH_TOKEN(observedP, messageP) == true)
        {
            IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Matching Observation found. Removing.");
            prv_observeRemove(contextP, serverP, observedP);
            break;
        }
        observedP = observedP->next;
    }

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Exiting.");
}

void observe_clear(iowa_context_t contextP,
                   iowa_lwm2m_uri_t * uriP)
{
    lwm2m_server_t * serverP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Clear observation on: /%u/%u/%u/%u.", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId);

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        lwm2m_observed_t * observedP;
        lwm2m_observed_t * parentP;

        parentP = NULL;
        observedP = serverP->runtime.observedList;
        while (observedP != NULL)
        {
            if (observedP->uriInfoP[0].uri.objectId == uriP->objectId
                && (LWM2M_URI_IS_SET_INSTANCE(uriP) == false
                    || observedP->uriInfoP[0].uri.instanceId == uriP->instanceId))
            {
                lwm2m_observed_t * nextP;

                nextP = observedP->next;

                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Delete observation: %p.", observedP);
                observe_delete(observedP);
                if (parentP == NULL)
                {
                    serverP->runtime.observedList = nextP;
                }
                else
                {
                    parentP->next = nextP;
                }

                observedP = nextP;
            }
            else
            {
                parentP = observedP;
                observedP = observedP->next;
            }
        }
    }
}

iowa_status_t observe_setParameters(iowa_context_t contextP,
                                    iowa_lwm2m_uri_t *uriP,
                                    lwm2m_server_t *serverP)
{
    iowa_status_t result;
    lwm2m_observed_t *observedP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    result = IOWA_COAP_NO_ERROR;
    observedP = serverP->runtime.observedList;
    while (observedP != NULL)
    {
        size_t ind;

        for (ind = 0; ind < observedP->uriCount; ind++)
        {
            if (LWM2M_URI_IS_SET_RESOURCE(uriP))
            {
                // Parameters have been set at resource level
                if (observedP->uriInfoP[ind].uri.resourceId == uriP->resourceId
                    && observedP->uriInfoP[ind].uri.instanceId == uriP->instanceId
                    && observedP->uriInfoP[ind].uri.objectId == uriP->objectId)
                {
                    result = observe_updateObserve(contextP, serverP, observedP);
                }
            }
            else if (LWM2M_URI_IS_SET_INSTANCE(uriP))
            {
                // Parameters have been set at instance level
                if (observedP->uriInfoP[ind].uri.instanceId == uriP->instanceId
                    && observedP->uriInfoP[ind].uri.objectId == uriP->objectId)
                {
                    result = observe_updateObserve(contextP, serverP, observedP);
                }
            }
            else
            {
                // Parameters have been set at object level
                if (observedP->uriInfoP[ind].uri.objectId == uriP->objectId)
                {
                    result = observe_updateObserve(contextP, serverP, observedP);
                }
            }

            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to update the observe attributes.");
                return result;
            }
        }
        observedP = observedP->next;
    }

    return IOWA_COAP_204_CHANGED;
}

void lwm2m_resource_value_changed(iowa_context_t contextP,
                                  iowa_lwm2m_uri_t *uriP)
{
    lwm2m_server_t *serverP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "URI: /%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId);

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        lwm2m_observed_t *observedP;

        for (observedP = serverP->runtime.observedList; observedP != NULL; observedP = observedP->next)
        {
            size_t ind;

            for (ind = 0; ind < observedP->uriCount; ind++)
            {
                if (observedP->uriInfoP[ind].uri.objectId == uriP->objectId)
                {
                    if (uriP->instanceId == IOWA_LWM2M_ID_ALL
                        || observedP->uriInfoP[ind].uri.instanceId == IOWA_LWM2M_ID_ALL
                        || uriP->instanceId == observedP->uriInfoP[ind].uri.instanceId)
                    {
                        if (uriP->resourceId == IOWA_LWM2M_ID_ALL
                            || observedP->uriInfoP[ind].uri.resourceId == IOWA_LWM2M_ID_ALL
                            || uriP->resourceId == observedP->uriInfoP[ind].uri.resourceId)
                        {
                            observedP->uriInfoP[ind].flags |= LWM2M_OBSERVE_FLAG_UPDATE;
                            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Tagging the observation.");
                            observedP->flags |= LWM2M_OBSERVE_FLAG_UPDATE;
                        }
                    }
                }
            }
        }

    }
}

// Update observe according with its attributes.
// Parameters:
// - contextP: iowa context.
// - serverP: server's information.
// - observedP: observe's information.
// - dataP: data to send.
// - dataCount: number of data.
static void prv_checkAndSendNotification(iowa_context_t contextP,
                                         lwm2m_server_t * serverP,
                                         lwm2m_observed_t * observedP,
                                         iowa_lwm2m_data_t * dataP,
                                         size_t dataCount)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;
    size_t ind;
    uint8_t *bufferP;
    size_t bufferLength;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    // Update each lastValue when URI is resource and numeric
    for (ind = 0; ind < observedP->uriCount; ind++)
    {
        //Check if it's a resource with no multiple instance && a numeric resource
        if (LWM2M_URI_IS_SET_RESOURCE(&observedP->uriInfoP[ind].uri)
            && !LWM2M_URI_IS_SET_RESOURCE_INSTANCE(&observedP->uriInfoP[ind].uri)
            && LWM2M_OBSERVE_IS_NUMERIC(&observedP->uriInfoP[ind]))
        {
            size_t indData;
            for (indData = 0; indData < dataCount; indData++)
            {
                iowa_lwm2m_uri_t tempUri;

                tempUri.objectId = dataP[indData].objectID;
                tempUri.instanceId = dataP[indData].instanceID;
                tempUri.resourceId = dataP[indData].resourceID;
                tempUri.resInstanceId = dataP[indData].resInstanceID;

                if (LWM2M_URI_ARE_EQUAL(&observedP->uriInfoP[ind].uri, &tempUri))
                {
                    if ((observedP->uriInfoP[ind].flags & LWM2M_OBSERVE_FLAG_INTEGER) != 0)
                    {
                        observedP->uriInfoP[ind].lastValue.asInteger = dataP[indData].value.asInteger;
                    }
                    else
                    {
                        observedP->uriInfoP[ind].lastValue.asFloat = dataP[indData].value.asFloat;
                    }
                    break;
                }
            }
        }
    }

    result = dataLwm2mSerialize(&observedP->uriInfoP[0].uri, dataP, dataCount, &(observedP->format), &bufferP, &bufferLength);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "dataLwm2mSerialize() failed with code %d.", result);
        return;
    }

    observedP->lastTime = contextP->currentTime;

    {
        if (serverP->runtime.status == STATE_REG_REGISTERED
            || serverP->runtime.status == STATE_REG_UPDATE_PENDING)
        {
            iowa_coap_message_t *messageP;
            iowa_coap_option_t *optionP;
            uint8_t messageType;

            if (serverP->notifStoring == true)
            {
                messageType = IOWA_COAP_TYPE_CONFIRMABLE;
            }
            else
            {
                messageType = IOWA_COAP_TYPE_NON_CONFIRMABLE;
            }

            messageP = iowa_coap_message_new(messageType, IOWA_COAP_205_CONTENT, observedP->tokenLen, observedP->token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (messageP == NULL)
            {
                iowa_system_free(bufferP);
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP message.");
                return;
            }
#endif

            optionP = iowa_coap_option_new(IOWA_COAP_OPTION_CONTENT_FORMAT);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (optionP == NULL)
            {
                iowa_coap_message_free(messageP);
                iowa_system_free(bufferP);
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
                return;
            }
#endif
            optionP->value.asInteger = observedP->format;
            iowa_coap_message_add_option(messageP, optionP);

            optionP = iowa_coap_option_new(IOWA_COAP_OPTION_OBSERVE);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (optionP == NULL)
            {
                iowa_coap_message_free(messageP);
                iowa_system_free(bufferP);
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
                return;
            }
#endif
            optionP->value.asInteger = observedP->counter;
            iowa_coap_message_add_option(messageP, optionP);

            messageP->payload.data = bufferP;
            messageP->payload.length = bufferLength;

            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Send notification number %d.", observedP->counter);
            (void)coapSend(contextP, serverP->runtime.peerP, messageP, NULL, NULL);

            prv_addMID(observedP, messageP->id);

            iowa_coap_message_free(messageP);
            iowa_system_free(bufferP);
        }
    }

    observedP->counter++;
    observedP->flags &= (uint8_t)~(LWM2M_OBSERVE_FLAG_UPDATE);
}

void observe_step(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    lwm2m_server_t *serverP;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        lwm2m_observed_t *observedP;
        iowa_status_t result;
        iowa_lwm2m_data_t *dataP;
        size_t dataCount;
        size_t ind;
        dataP = NULL;
        dataCount = 0;

        for (observedP = serverP->runtime.observedList; observedP != NULL; observedP = observedP->next)
        {
            bool sendNotif;
            bool nextObs;

            sendNotif = false;
            nextObs = false;
            // if tag true
            if ((observedP->flags & LWM2M_OBSERVE_FLAG_UPDATE) != 0
                && (contextP->lwm2mContextP->internalFlag & CONTEXT_FLAG_INSIDE_CALLBACK) == 0)
            {
                //Check if there is timeAttribute
                if (observedP->timeAttrP != NULL)
                {
                    if ((observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0)
                    {
                        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Checking minimum period (%d s).", observedP->timeAttrP->minPeriod);
                        if (observedP->lastTime + observedP->timeAttrP->minPeriod > contextP->currentTime)
                        {
                            nextObs = true;
                        }
                    }
                }
                if (nextObs == false)
                {
                    for (ind = 0; ind < observedP->uriCount; ind++)
                    {
                        //Get value to send
                        result = object_read(contextP, &observedP->uriInfoP[ind].uri, serverP->shortId, &dataCount, &dataP);
                        {
                            if (result != IOWA_COAP_205_CONTENT)
                            {
                                IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Getting value to send failed with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
                                return;
                            }
                        }
                        //Check if it's a resource with no multiple instance && a numeric resource && if there is a ST, LT, GT set
                        if (LWM2M_URI_IS_SET_RESOURCE(&observedP->uriInfoP[ind].uri)
                            && !LWM2M_URI_IS_SET_RESOURCE_INSTANCE(&observedP->uriInfoP[ind].uri)
                            && observedP->uriInfoP[ind].uriAttrP != NULL
                            && (observedP->uriInfoP[ind].uriAttrP->flags & ATTR_FLAG_NUMERIC) != 0
                            && LWM2M_OBSERVE_IS_NUMERIC(&observedP->uriInfoP[ind]))
                        {
                            if ((observedP->uriInfoP[ind].flags & LWM2M_OBSERVE_FLAG_INTEGER) != 0)
                            {
                                int64_t integerValue;
                                integerValue = dataP->value.asInteger;

                                if ((observedP->uriInfoP[ind].uriAttrP->flags & LWM2M_ATTR_FLAG_LESS_THAN) != 0
                                    && TEST_THRESHOLD(observedP->uriInfoP[ind].uriAttrP->lessThan, integerValue, observedP->uriInfoP[ind].lastValue.asInteger))
                                {
                                    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Notify on lower threshold crossing.");
                                    sendNotif = true;
                                }

                                if ((observedP->uriInfoP[ind].uriAttrP->flags & LWM2M_ATTR_FLAG_GREATER_THAN) != 0
                                    && TEST_THRESHOLD(observedP->uriInfoP[ind].uriAttrP->greaterThan, integerValue, observedP->uriInfoP[ind].lastValue.asInteger))
                                {
                                    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Notify on lower upper crossing.");
                                    sendNotif = true;
                                }

                                if ((observedP->uriInfoP[ind].uriAttrP->flags & LWM2M_ATTR_FLAG_STEP) != 0)
                                {
                                    int64_t diff;

                                    diff = integerValue - observedP->uriInfoP[ind].lastValue.asInteger;
                                    if (diff < 0 )
                                    {
                                        diff = 0 - diff;
                                    }
                                    if (diff >= observedP->uriInfoP[ind].uriAttrP->step)
                                    {
                                        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Notify on step condition.");
                                        sendNotif = true;
                                    }
                                }
                            }
                            else
                            {
                                double floatValue;
                                floatValue = dataP->value.asFloat;

                                if ((observedP->uriInfoP[ind].uriAttrP->flags & LWM2M_ATTR_FLAG_LESS_THAN) != 0
                                    && TEST_THRESHOLD(observedP->uriInfoP[ind].uriAttrP->lessThan, floatValue, observedP->uriInfoP[ind].lastValue.asFloat))
                                {
                                    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Notify on lower threshold crossing.");
                                    sendNotif = true;
                                }
                                if ((observedP->uriInfoP[ind].uriAttrP->flags & LWM2M_ATTR_FLAG_GREATER_THAN) != 0
                                    && TEST_THRESHOLD(observedP->uriInfoP[ind].uriAttrP->greaterThan, floatValue, observedP->uriInfoP[ind].lastValue.asFloat))
                                {
                                    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Notify on lower upper crossing.");
                                    sendNotif = true;
                                }
                                if ((observedP->uriInfoP[ind].uriAttrP->flags & LWM2M_ATTR_FLAG_STEP) != 0)
                                {
                                    double diff;

                                    diff = floatValue - observedP->uriInfoP[ind].lastValue.asFloat;
                                    if (diff < 0 )
                                    {
                                        diff = 0 - diff;
                                    }
                                    if (diff >= observedP->uriInfoP[ind].uriAttrP->step) //Todo : check FLT_EPSILON
                                    {
                                        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Notify on step condition.");
                                        sendNotif = true;
                                    }
                                }
                            }
                        }
                        else
                        {
                            sendNotif = true;
                        }
                    }
                    if (sendNotif == true)
                    {
                        prv_checkAndSendNotification(contextP, serverP, observedP, dataP, dataCount);
                    }
                    object_free(contextP, dataCount, dataP);
                    iowa_system_free(dataP);
                    observedP->flags &= (uint8_t)~(LWM2M_OBSERVE_FLAG_UPDATE);
                }
            }
            else // Check pmax
            {
                //Check if there is timeAttribute
                if (observedP->timeAttrP != NULL)
                {
                    if ((observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0)
                    {
                        // Ignore pmax if lesser than pmin
                        if (((observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0
                            && observedP->timeAttrP->maxPeriod >= observedP->timeAttrP->minPeriod)
                            || ((observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) == 0))
                        {
                            IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Checking maximum period (%d s).", observedP->timeAttrP->maxPeriod);
                            if (observedP->lastTime + observedP->timeAttrP->maxPeriod <= contextP->currentTime)
                            {
                                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Notify on elapsed maximal period (%d s).", observedP->timeAttrP->maxPeriod);

                                for (ind = 0; ind < observedP->uriCount; ind++)
                                {
                                    //Get value to send
                                    result = object_read(contextP, &observedP->uriInfoP[ind].uri, serverP->shortId, &dataCount, &dataP);
                                    {
                                        if (result != IOWA_COAP_205_CONTENT)
                                        {
                                            IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Getting value to send failed with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
                                            return;
                                        }
                                    }
                                }
                                prv_checkAndSendNotification(contextP, serverP, observedP, dataP, dataCount);
                                object_free(contextP, dataCount, dataP);
                                iowa_system_free(dataP);
                            }
                        }
                    }
                }
            }
            if (observedP->timeAttrP != NULL
                && (observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0)
            {
                // Ignore pmax if lesser than pmin
                if (((observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0
                    && observedP->timeAttrP->maxPeriod >= observedP->timeAttrP->minPeriod)
                    || ((observedP->timeAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) == 0))
                {
                    int32_t interval;

                    interval = observedP->lastTime + observedP->timeAttrP->maxPeriod - contextP->currentTime;
                    if (contextP->timeout > interval)
                    {
                        contextP->timeout = interval;
                    }
                }
            }
        }

    }
    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with timeoutP: %ds.", contextP->timeout);
}
#endif // LWM2M_CLIENT_MODE

