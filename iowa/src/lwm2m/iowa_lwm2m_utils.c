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
 *    http:
 * The Eclipse Distribution License is available at
 *    http:
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Toby Jaffey - Please refer to git log
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

#include <float.h>

/*************************************************************************************
** Public functions
*************************************************************************************/

size_t utils_stringToBinding(uint8_t *strBinding,
                             size_t strBindingLen,
                             lwm2m_binding_t *bindingP)
{
    size_t index;

    assert(bindingP != NULL);

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with strBinding: %.*s.", strBindingLen, strBinding);

    for (index = 0; index < strBindingLen; index++)
    {
        switch (strBinding[index])
        {
        case QUERY_BINDING_UDP:
            *bindingP |= BINDING_U;
            break;

        case QUERY_BINDING_TCP:
            *bindingP |= BINDING_T;
            break;

        case QUERY_BINDING_SMS:
            *bindingP |= BINDING_S;
            break;

        case QUERY_BINDING_NON_IP:
            *bindingP |= BINDING_N;
            break;

        case QUERY_BINDING_QUEUE_MODE:
            *bindingP |= BINDING_Q;
            break;

        default:
            return 0;
        }
    }

    return index;
}

size_t utils_bindingToString(lwm2m_binding_t binding,
                             bool queueMode,
                             uint8_t **strBindingP)
{
    size_t index;
    size_t cursor;

    assert(strBindingP != NULL);
    assert((binding & (BINDING_U | BINDING_T | BINDING_S | BINDING_N)) != 0);

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with binding: %d, queueMode: %s.", binding, queueMode?"true":"false");

    if (queueMode == true)
    {
        index = 1;
    }
    else
    {
        index = 0;
    }

    for (cursor = BINDING_U; cursor != BINDING_Q; cursor <<= 1)
    {
        if ((cursor & binding) != 0)
        {
            index++;
        }
    }

    *strBindingP = (uint8_t *)iowa_system_malloc(index);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*strBindingP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(index));
        return 0;
    }
#endif

    index = 0;

#ifdef IOWA_UDP_SUPPORT
    if ((binding & BINDING_U) != 0)
    {
        (*strBindingP)[index] = QUERY_BINDING_UDP;
        index++;
    }
#endif
    if (queueMode == true)
    {
        (*strBindingP)[index] = QUERY_BINDING_QUEUE_MODE;
        index++;
    }

    return index;
}

iowa_content_format_t utils_getMediaType(iowa_coap_message_t *messageP,
                                         uint16_t number)
{
    iowa_coap_option_t *optionP;
    iowa_content_format_t format;

    assert(messageP != NULL);

    optionP = iowa_coap_message_find_option(messageP, number);
    if (optionP == NULL)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Option number %u not found.", number);
        switch (number)
        {
        case IOWA_COAP_OPTION_CONTENT_FORMAT:
            return IOWA_CONTENT_FORMAT_UNSET;

        default:
            return LWM2M_DEFAULT_CONTENT_FORMAT;
        }
    }

    if (optionP->next != NULL
        && optionP->next->number == number)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Multiple option number %u found.", number);
        return IOWA_CONTENT_FORMAT_UNSET;
    }

    format = (uint16_t)(optionP->value.asInteger);
    switch (format)
    {
    case IOWA_CONTENT_FORMAT_TEXT:
    case LWM2M_CONTENT_FORMAT_CORE_LINK:
    case IOWA_CONTENT_FORMAT_OPAQUE:
#ifdef LWM2M_SUPPORT_TLV
    case IOWA_CONTENT_FORMAT_TLV_OLD:
    case IOWA_CONTENT_FORMAT_TLV:
#endif
        break;

    default:
        format = IOWA_CONTENT_FORMAT_UNSET;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Format: %s", STR_MEDIA_TYPE(format));

    return format;
}

#if defined(LWM2M_CLIENT_MODE)
iowa_status_t utilsConnectServer(iowa_context_t contextP,
                                 lwm2m_server_t *serverP,
                                 coap_message_callback_t messageCb,
                                 coap_event_callback_t eventCb)
{
    iowa_status_t result;

    if (serverP->runtime.peerP == NULL)
    {
        serverP->runtime.peerP = coapPeerCreate(contextP,
                                                serverP->uri,
                                                serverP->securityMode,
                                                messageCb,
                                                eventCb,
                                                (void *)serverP);
        if (serverP->runtime.peerP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Cannot create the CoAP peer.");
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    result = coapPeerConnect(contextP, serverP->runtime.peerP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        coapPeerDelete(contextP, serverP->runtime.peerP);
        serverP->runtime.peerP = NULL;
    }

    return result;
}

void utilsDisconnectServer(iowa_context_t contextP,
                           lwm2m_server_t *serverP)
{
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Disconnect server.");
    if (serverP->runtime.peerP != NULL)
    {
        coapPeerDelete(contextP, serverP->runtime.peerP);
        serverP->runtime.peerP = NULL;
    }
}

void utilsFreeServer(iowa_context_t contextP,
                     lwm2m_server_t *serverP)
{
    iowa_system_free(serverP->runtime.location);
    iowa_system_free(serverP->uri);
    iowa_system_free(serverP);
}

iowa_status_t lwm2mMergeData(iowa_lwm2m_data_t **dataP,
                             size_t *dataCountP,
                             lwm2m_data_array_t *dataArrayP,
                             size_t dataArrayCount)
{
    *dataP = NULL;
    *dataCountP = 0;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    if (dataArrayCount > 0)
    {
        size_t ind;

        for (ind = 0; ind < dataArrayCount; ind++)
        {
            *dataCountP += dataArrayP[ind].dataCount;
        }

        if (*dataCountP == 0)
        {
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Empty data.");
            *dataP = NULL;
            return IOWA_COAP_205_CONTENT;
        }

        *dataP = (iowa_lwm2m_data_t *)iowa_system_malloc(*dataCountP * sizeof(iowa_lwm2m_data_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*dataP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(*dataCountP * sizeof(iowa_lwm2m_data_t));
            *dataCountP = 0;
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
#endif
        {
            *dataCountP = 0;
            for (ind = 0; ind < dataArrayCount; ind++)
            {
                memcpy(&(*dataP)[*dataCountP], dataArrayP[ind].dataP, dataArrayP[ind].dataCount * sizeof(iowa_lwm2m_data_t));
                iowa_system_free(dataArrayP[ind].dataP);

                *dataCountP += dataArrayP[ind].dataCount;
            }
        }
    }
    return IOWA_COAP_205_CONTENT;
}

#endif

