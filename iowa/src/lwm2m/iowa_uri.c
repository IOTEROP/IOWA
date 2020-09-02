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

static int prv_parseNumber(uint8_t * uriString,
                            size_t uriLength,
                            size_t * headP)
{
    int result = 0;

    if (uriString[*headP] == '/')
    {
        // empty Object Instance ID with resource ID is not allowed
        return -1;
    }
    while (*headP < uriLength && uriString[*headP] != '/')
    {
        if ('0' <= uriString[*headP] && uriString[*headP] <= '9')
        {
            result += uriString[*headP] - '0';
            result *= 10;
        }
        else
        {
            return -1;
        }
        *headP += 1;
    }

    result /= 10;
    return result;
}


int uri_getNumber(uint8_t * uriString,
                  size_t uriLength)
{
    size_t index = 0;

    if (uriString == NULL
        || uriLength == 0)
    {
        return -1;
    }

    return prv_parseNumber(uriString, uriLength, &index);
}

lwm2m_uri_type_t uri_decode(iowa_coap_message_t *messageP,
                            uint16_t number,
                            iowa_lwm2m_uri_t *uriP)
{
    iowa_coap_option_t *optionP;

    assert(messageP != NULL);
    assert(uriP != NULL);

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "number: %u.", number);

    LWM2M_URI_RESET(uriP);

    optionP = iowa_coap_message_find_option(messageP, number);
    if (optionP == NULL)
    {
        return LWM2M_URI_TYPE_DM;
    }

    if (URI_REGISTRATION_SEGMENT_LEN == optionP->length
       && 0 == strncmp(URI_REGISTRATION_SEGMENT, (char *)optionP->value.asBuffer, optionP->length))
    {
        return LWM2M_URI_TYPE_REGISTRATION;
    }

    if (URI_BOOTSTRAP_SEGMENT_LEN == optionP->length
        && 0 == strncmp(URI_BOOTSTRAP_SEGMENT, (char *)optionP->value.asBuffer, optionP->length))
    {
        return LWM2M_URI_TYPE_BOOTSTRAP;
    }

    if (optionP->length == 0)
    {
        optionP = optionP->next;
        if (optionP != NULL
            && optionP->number == number)
        {
            return LWM2M_URI_TYPE_UNKNOWN;
        }
    }
    else
    {
        int readNum;

        // Read object ID
        readNum = uri_getNumber(optionP->value.asBuffer, optionP->length);
        if (readNum < 0
            || readNum >= IOWA_LWM2M_ID_ALL)
        {
            return LWM2M_URI_TYPE_UNKNOWN;
        }
        uriP->objectId = (uint16_t)readNum;

        optionP = optionP->next;
        if (optionP != NULL
            && optionP->number == number)
        {
            // Read object instance ID
            readNum = uri_getNumber(optionP->value.asBuffer, optionP->length);
            if (readNum < 0
                || readNum >= IOWA_LWM2M_ID_ALL)
            {
                return LWM2M_URI_TYPE_UNKNOWN;
            }
            uriP->instanceId = (uint16_t)readNum;

            optionP = optionP->next;
            if (optionP != NULL
                && optionP->number == number)
            {
                // Read resource ID
                readNum = uri_getNumber(optionP->value.asBuffer, optionP->length);
                if (readNum < 0
                    || readNum >= IOWA_LWM2M_ID_ALL)
                {
                    return LWM2M_URI_TYPE_UNKNOWN;
                }
                uriP->resourceId = (uint16_t)readNum;

                optionP = optionP->next;
                if (optionP != NULL
                    && optionP->number == number)
                {
                    return LWM2M_URI_TYPE_UNKNOWN;
                }
            }
        }
    }
    return LWM2M_URI_TYPE_DM;
}

iowa_coap_option_t * uri_encode(uint16_t number,
                                iowa_lwm2m_uri_t *uriP,
                                uint8_t buffer[PRV_URI_BUFFER_SIZE])
{
    iowa_coap_option_t *resultP;
    iowa_coap_option_t *optionP;
    uint16_t index;

    resultP = iowa_coap_option_new(number);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (resultP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
        return NULL;
    }
#endif

    if (uriP == NULL)
    {
        return resultP;
    }

    optionP = resultP;

    // length of a text version of a LwM2M ID is always less than an uint16_t max value
    if(uriP->objectId == IOWA_LWM2M_ID_ALL)
    {
        optionP->length = 0;
    }
    else
    {
        optionP->length = (uint16_t)dataUtilsIntToBuffer(uriP->objectId, buffer, PRV_URI_BUFFER_SIZE, false);
    }

    if (optionP->length == 0
        && uriP->objectId != IOWA_LWM2M_ID_ALL)
    {
        iowa_coap_option_free(resultP);
        return NULL;
    }
    optionP->value.asBuffer = buffer;
    index = optionP->length;

    if (LWM2M_URI_IS_SET_INSTANCE(uriP))
    {
        optionP->next = iowa_coap_option_new(number);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (optionP->next == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
            iowa_coap_option_free(resultP);
            return NULL;
        }
#endif
        optionP = optionP->next;

        // length of a text version of a LwM2M ID is always less than an uint16_t max value
        optionP->length = (uint16_t)dataUtilsIntToBuffer(uriP->instanceId, buffer + index, PRV_URI_BUFFER_SIZE - index, false);
        if (optionP->length == 0)
        {
            iowa_coap_option_free(resultP);
            return NULL;
        }
        optionP->value.asBuffer = buffer + index;
        index = (uint16_t)(index + optionP->length);

        if (LWM2M_URI_IS_SET_RESOURCE(uriP))
        {
            optionP->next = iowa_coap_option_new(number);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (optionP->next == NULL)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
                iowa_coap_option_free(resultP);
                return NULL;
            }
#endif
            optionP = optionP->next;

            // length of a text version of a LwM2M ID is always less than an uint16_t max value
            optionP->length = (uint16_t)dataUtilsIntToBuffer(uriP->resourceId, buffer + index, PRV_URI_BUFFER_SIZE - index, false);
            if (optionP->length == 0)
            {
                iowa_coap_option_free(resultP);
                return NULL;
            }
            optionP->value.asBuffer = buffer + index;
        }
    }

    return resultP;
}

void lwm2m_uri_set(iowa_lwm2m_uri_t *uriP,
                   uint16_t objectId,
                   uint16_t instanceId,
                   uint16_t resourceId,
                   uint16_t resInstanceId)
{
    uriP->objectId = objectId;
    uriP->instanceId = instanceId;
    uriP->resourceId = resourceId;
    uriP->resInstanceId = resInstanceId;
}
