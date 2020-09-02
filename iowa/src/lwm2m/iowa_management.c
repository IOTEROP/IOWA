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
 *    domedambrosio - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
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

void dm_handleRequest(iowa_context_t contextP,
                      iowa_lwm2m_uri_t *uriP,
                      lwm2m_server_t *serverP,
                      iowa_coap_message_t *messageP)
{
    // WARNING: This function is called in a critical section
    iowa_content_format_t responseFormat;
    iowa_content_format_t requestFormat;
    iowa_coap_option_t *optionP;
    iowa_coap_option_t *optionObserveP;
    iowa_status_t result;
    iowa_coap_message_t *responseP;
    iowa_lwm2m_data_t *dataP;
    size_t dataCount;
#if defined(LWM2M_SUPPORT_TLV) || defined(LWM2M_SUPPORT_JSON)
    uint8_t uriBufferP[PRV_URI_BUFFER_SIZE];
#endif

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Code: %u.%02u, server status: %s", messageP->code >> 5, messageP->code & 0x1F, LWM2M_SERVER_STR_STATUS(serverP->runtime.status));
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "URI: /%u/%u/%u/%u", uriP->objectId, uriP->instanceId, uriP->resourceId, uriP->resInstanceId);

    switch (uriP->objectId)
    {
    case IOWA_LWM2M_SECURITY_OBJECT_ID:
        IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Rejecting DM operation on the Object: %u", uriP->objectId);
        coapSendResponse(contextP, serverP->runtime.peerP, messageP, IOWA_COAP_401_UNAUTHORIZED);
        return;

    default:
        break;
    }

    if (serverP->runtime.status != STATE_REG_REGISTERED
        && serverP->runtime.status != STATE_REG_UPDATE_PENDING)
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Ignoring out-of-sequence DM operation.");
        return;
    }

    responseP = iowa_coap_message_prepare_response(messageP, IOWA_COAP_CODE_EMPTY);
    if (responseP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create response packet.");
        return;
    }

    dataP = NULL;
    dataCount = 0;
    optionObserveP = iowa_coap_message_find_option(messageP, IOWA_COAP_OPTION_OBSERVE);

    // Get the request message format
    requestFormat = utils_getMediaType(messageP, IOWA_COAP_OPTION_CONTENT_FORMAT);
    switch (requestFormat)
    {
    case IOWA_CONTENT_FORMAT_UNSET:
        if (messageP->payload.length != 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Unknown or missing Content-Type");
            result = IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT;
            goto error;
        }
        break;

    default:
        {
            result = dataLwm2mDeserialize(uriP, messageP->payload.data, messageP->payload.length, requestFormat, &dataP, &dataCount, object_getResourceType, contextP);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_INFO(IOWA_PART_LWM2M, "Parsing received data failed.");
                goto error;
            }
        }
    }

    switch (messageP->code)
    {
    case IOWA_COAP_CODE_GET:
    {
        if (dataCount != 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Received Read operation with a payload.");
            result = IOWA_COAP_400_BAD_REQUEST;
            goto error;
        }

        // Check the possible format for the response
        responseFormat = utils_getMediaType(messageP, IOWA_COAP_OPTION_ACCEPT);

        if (responseFormat == LWM2M_CONTENT_FORMAT_CORE_LINK)
        {
            link_t *linkP;
            size_t nbLink;

            if (optionObserveP != NULL)
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Bad request: Accept link format with Observe option.");
                result = IOWA_COAP_400_BAD_REQUEST;
                goto error;
            }

            result = object_getTree(contextP, uriP, serverP, &linkP, &nbLink);
            if (result == IOWA_COAP_NO_ERROR)
            {

                result = coreLinkSerialize(linkP, nbLink, &responseP->payload.data, &responseP->payload.length);
                coreLinkFree(linkP, nbLink);
                if (result == IOWA_COAP_NO_ERROR)
                {
                    result = IOWA_COAP_205_CONTENT;
                }
            }
        }
        else
        {
            {
                {
                    result = object_read(contextP, uriP, serverP->shortId, &dataCount, &dataP);
                }
                if (IOWA_COAP_205_CONTENT == result)
                {
                    {
                        result = dataLwm2mSerialize(uriP, dataP, dataCount, &responseFormat, &responseP->payload.data, &responseP->payload.length);
                        if (result == IOWA_COAP_NO_ERROR)
                        {
                            if (optionObserveP != NULL)
                            {
                                result = observe_handleRequest(contextP, 1, uriP, serverP, dataCount, dataP, optionObserveP, messageP, responseP, responseFormat);
                                if (result != IOWA_COAP_205_CONTENT)
                                {
                                    iowa_system_free(responseP->payload.data);
                                    responseP->payload = IOWA_BUFFER_EMPTY;
                                }
                            }
                            else
                            {
                                result = IOWA_COAP_205_CONTENT;
                            }
                        }
                        responseP->code = result;
                    }
                    if (dataP != NULL)
                    {
                        object_free(contextP, dataCount, dataP);
                        iowa_system_free(dataP);
                        dataP = NULL; // Set pointer to NULL to prevent calling dataLwm2mFree at the end of the function
                    }
                }
            }
        }

        if (result == IOWA_COAP_205_CONTENT
            && responseP->payload.length != 0)
        {
            optionP = iowa_coap_option_new(IOWA_COAP_OPTION_CONTENT_FORMAT);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (optionP == NULL)
            {
                iowa_system_free(responseP->payload.data);
                responseP->payload = IOWA_BUFFER_EMPTY;
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
                result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
            else
#endif
            {
                optionP->value.asInteger = (uint32_t)responseFormat;
                iowa_coap_message_add_option(responseP, optionP);
            }
        }
    }
    break;

    case IOWA_COAP_CODE_POST:
    {
        if (optionObserveP != NULL)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Bad request: POST command with Observe option.");
            result = IOWA_COAP_400_BAD_REQUEST;
            goto error;
        }

        if (!LWM2M_URI_IS_SET_INSTANCE(uriP))
        {
#if defined(LWM2M_SUPPORT_TLV) || defined(LWM2M_SUPPORT_JSON)
            bool addLocationPath;
#endif

            if (dataCount == 0)
            {
                IOWA_LOG_INFO(IOWA_PART_LWM2M, "Received a Create operation with no payload.");
                result = IOWA_COAP_400_BAD_REQUEST;
                break;
            }

#if defined(LWM2M_SUPPORT_TLV) || defined(LWM2M_SUPPORT_JSON)
            addLocationPath = (dataP[0].instanceID == IOWA_LWM2M_ID_ALL);
#endif // LWM2M_SUPPORT_TLV || LWM2M_SUPPORT_JSON

            result = object_create(contextP, serverP->shortId, dataCount, dataP);
            if (result == IOWA_COAP_201_CREATED)
            {
#if defined(LWM2M_SUPPORT_TLV) || defined(LWM2M_SUPPORT_JSON)
                if (addLocationPath == true)
                {
                    // Here, "uriP->instanceId" is equal to IOWA_LWM2M_ID_ALL. So, override the ID with the newly created Instance ID
                    uriP->instanceId = dataP[0].instanceID;

                    responseP->optionList = uri_encode(IOWA_COAP_OPTION_LOCATION_PATH, uriP, uriBufferP);
                    if (responseP->optionList == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to encode the URI.");
                        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                        break;
                    }
                }
#endif // LWM2M_SUPPORT_TLV || LWM2M_SUPPORT_JSON

                lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
            }
        }
        else if (!LWM2M_URI_IS_SET_RESOURCE(uriP))
        {
            result = object_checkWritePayload(contextP, dataCount, dataP);
            if (result == IOWA_COAP_NO_ERROR)
            {
                result = object_write(contextP, serverP->shortId, dataCount, dataP);
            }
        }
        else
        {
            result = object_execute(contextP, uriP, serverP->shortId, messageP->payload.data, messageP->payload.length);
        }
    }
    break;

    case IOWA_COAP_CODE_PUT:
    {
        if (optionObserveP != NULL)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Bad request: PUT command with Observe option.");
            result = IOWA_COAP_400_BAD_REQUEST;
            goto error;
        }

        optionP = iowa_coap_message_find_option(messageP, IOWA_COAP_OPTION_URI_QUERY);
        if (optionP != NULL)
        {
            if (dataCount != 0)
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Received Write-Attributes operation with a payload.");
                result = IOWA_COAP_400_BAD_REQUEST;
                goto error;
            }

            result = attributesWrite(contextP, uriP, serverP, optionP);
            if (result == IOWA_COAP_NO_ERROR)
            {
                result = observe_setParameters(contextP, uriP, serverP);
            }
        }
        else if (LWM2M_URI_IS_SET_INSTANCE(uriP))
        {
            result = object_checkWritePayload(contextP, dataCount, dataP);
            if (result == IOWA_COAP_NO_ERROR)
            {
                result = object_write(contextP, serverP->shortId, dataCount, dataP);
            }
        }
        else
        {
            result = IOWA_COAP_400_BAD_REQUEST;
        }
    }
    break;

    case IOWA_COAP_CODE_DELETE:
    {
        if (optionObserveP != NULL)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Bad request: DELETE command with Observe option.");
            result = IOWA_COAP_400_BAD_REQUEST;
            goto error;
        }

        if (dataCount != 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Received Delete operation with a payload.");
            result = IOWA_COAP_400_BAD_REQUEST;
            goto error;
        }

        if (!LWM2M_URI_IS_SET_INSTANCE(uriP) || LWM2M_URI_IS_SET_RESOURCE(uriP))
        {
            result = IOWA_COAP_400_BAD_REQUEST;
        }
        else
        {
            result = object_delete(contextP, uriP, serverP->shortId);
            if (result == IOWA_COAP_202_DELETED)
            {
                lwm2mUpdateRegistration(contextP, NULL, LWM2M_UPDATE_FLAG_OBJECTS);
            }
        }
    }
    break;

    case IOWA_COAP_CODE_FETCH:
        result = IOWA_COAP_501_NOT_IMPLEMENTED;
    break;

    case IOWA_COAP_CODE_IPATCH:
    {
        result = IOWA_COAP_501_NOT_IMPLEMENTED;
    }
    break;

    default:
        result = IOWA_COAP_400_BAD_REQUEST;
        goto error;
    }

    serverP->runtime.flags |= LWM2M_SERVER_FLAG_AVAILABLE;

error:
    if (dataP != NULL)
    {
        dataLwm2mFree(dataCount, dataP);
    }
    {
        if (result != IOWA_COAP_CODE_EMPTY
            || messageP->type == IOWA_COAP_TYPE_CONFIRMABLE)
        {
            responseP->code = result;

            if (IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE == coapSend(contextP, serverP->runtime.peerP, responseP, NULL, NULL))
            {
                // an error message was sent back to the LwM2M Server
                // if the request was an Observation, remove it.
                optionP = iowa_coap_message_find_option(messageP, IOWA_COAP_OPTION_OBSERVE);

                if (result == IOWA_COAP_205_CONTENT
                    && optionP != NULL
                    && optionP->value.asInteger == LWM2M_OBSERVE_REQUEST_NEW)
                {
                    lwm2m_observed_t *observedP;

                    // Memorize previous observe
                    observedP = serverP->runtime.observedList->next;
                    // Delete last observe
                    observe_delete(serverP->runtime.observedList);
                    serverP->runtime.observedList = observedP;
                }
            }
        }

        iowa_system_free(responseP->payload.data);
        iowa_coap_message_free(responseP);
    }
}
#endif

