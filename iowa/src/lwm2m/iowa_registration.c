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
 *    domedambrosio - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Manuel Sangoi - Please refer to git log
 *    Julien Vermillard - Please refer to git log
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
#include "iowa_prv_coap_internals.h"

#define MAX_LOCATION_LENGTH         10
#define PRV_LINK_URI_SEPARATOR      "/"
#define PRV_LINK_URI_SEPARATOR_SIZE 1

#define PRV_RESULT_NO_ERROR_LIFETIME 1
#define PRV_RESULT_NO_ERROR_NO_LIFETIME 0
#define PRV_RESULT_ERROR    -1

/*************************************************************************************
** Private functions
*************************************************************************************/

#ifdef LWM2M_CLIENT_MODE
static iowa_status_t prv_getRegistrationQuery(iowa_context_t contextP, lwm2m_server_t *serverP, size_t *lengthP, char **bufferP);
static iowa_status_t prv_getRegistrationPayload(iowa_context_t contextP, uint8_t **payloadP, size_t *payloadLengthP);
static int32_t prv_getUpdateDelay(lwm2m_server_t *serverP);
static void prv_serverRegistrationFailing(iowa_context_t contextP, lwm2m_server_t *serverP);
static void prv_handleRegistrationUpdateReply(iowa_coap_peer_t *fromPeerP, uint8_t status, iowa_coap_message_t * responseP, void * userData, iowa_context_t contextP);
static void prv_updateRegistration(iowa_context_t contextP, lwm2m_server_t *serverP);
static void prv_handleClientUpdateTimer(iowa_context_t contextP, void *userData);
static void prv_handleClientLifetimeTimer(iowa_context_t contextP, void *userData);
static void prv_handleRegistrationReply(iowa_coap_peer_t *fromPeer, uint8_t status, iowa_coap_message_t *responseP, void * userData, iowa_context_t contextP);
static iowa_status_t prv_register(iowa_context_t contextP, lwm2m_server_t *serverP);
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
static bool prv_checkStartServerRegistration(iowa_context_t contextP, lwm2m_server_t *serverP);
#endif
static iowa_status_t prv_initiateServerConnection(iowa_context_t contextP, lwm2m_server_t *serverP);
#endif

#ifdef LWM2M_CLIENT_MODE
iowa_status_t prv_getRegistrationQuery(iowa_context_t contextP,
                                       lwm2m_server_t *serverP,
                                       size_t *lengthP,
                                       char **bufferP)
{
    size_t length;
    size_t res;
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_TCP_SUPPORT) || defined(IOWA_SMS_SUPPORT)
    uint8_t *strBinding;
    size_t strBindingLen;
#endif


    switch (coapPeerGetConnectionType(serverP->runtime.peerP))
    {

    default:
    {
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_TCP_SUPPORT) || defined(IOWA_SMS_SUPPORT)
        lwm2m_binding_t binding;

        length = QUERY_LIFETIME_LEN + dataUtilsIntToBufferLength(serverP->lifetime, false) + 1 + QUERY_DELIMITER_LEN + QUERY_VERSION_LEN + LWM2M_VERSION_LEN;
        if (contextP->lwm2mContextP->endpointName != NULL)
        {
            length += QUERY_DELIMITER_LEN + QUERY_NAME_LEN + strlen(contextP->lwm2mContextP->endpointName);
        }

        switch (serverP->lwm2mVersion)
        {
#ifndef LWM2M_VERSION_1_0_REMOVE
        case IOWA_LWM2M_VERSION_1_0:

            binding = serverP->binding;
            break;
#endif

        default:
            goto exit_on_error;
        }

        if (binding != BINDING_U)
        {
            switch (serverP->lwm2mVersion)
            {
#ifndef LWM2M_VERSION_1_0_REMOVE
            case IOWA_LWM2M_VERSION_1_0:
                strBindingLen = utils_bindingToString(binding, (binding & BINDING_Q) != 0, &strBinding);
                break;
#endif

            default:
                goto exit_on_error;
            }

            length += QUERY_DELIMITER_LEN + QUERY_BINDING_LEN + strBindingLen;
        }
        else
        {
            strBindingLen = 0;
        }

#endif
    }
    }


    *bufferP = (char *)iowa_system_malloc(length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*bufferP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(length);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif


    *lengthP = utilsStringCopy(*bufferP, length, QUERY_LIFETIME);

    res = dataUtilsIntToBuffer(serverP->lifetime, (uint8_t*)*bufferP + *lengthP, length - *lengthP, false);
    if (res == 0)
    {
        goto exit_on_error;
    }
    *lengthP += res;

    switch (coapPeerGetConnectionType(serverP->runtime.peerP))
    {

    default:
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_TCP_SUPPORT) || defined(IOWA_SMS_SUPPORT)
        *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, QUERY_DELIMITER QUERY_VERSION);

        switch (serverP->lwm2mVersion)
        {
#ifndef LWM2M_VERSION_1_0_REMOVE
        case IOWA_LWM2M_VERSION_1_0:
            *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, LWM2M_VERSION_1_0);
            break;
#endif

        default:
            goto exit_on_error;
        }

        if (contextP->lwm2mContextP->endpointName != NULL)
        {
            *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, QUERY_DELIMITER QUERY_NAME);
            *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, contextP->lwm2mContextP->endpointName);
        }

        if (strBindingLen != 0)
        {
            *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, QUERY_DELIMITER QUERY_BINDING);

            if (length - *lengthP < strBindingLen)
            {
                goto exit_on_error;
            }
            memcpy(*bufferP + *lengthP, strBinding, strBindingLen);
            *lengthP += strBindingLen;

            iowa_system_free(strBinding);
        }

#endif
        break;
    }

    (*bufferP)[*lengthP] = '\0';
    (*lengthP)++;

    return IOWA_COAP_NO_ERROR;

exit_on_error:
    iowa_system_free(*bufferP);
    *bufferP = NULL;

    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}

iowa_status_t prv_getRegistrationPayload(iowa_context_t contextP,
                                         uint8_t **payloadP,
                                         size_t *payloadLengthP)
{
    iowa_status_t result;
    link_t *linkP;
    size_t nbLink;
    size_t linkIndex;


    result = object_getList(contextP, IOWA_LWM2M_ID_ALL, &linkP, &nbLink);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to retrieve the object tree.");
        return result;
    }


    linkIndex = 0;
    while (linkIndex < nbLink)
    {
        switch (linkP[linkIndex].uri.objectId)
        {
        case IOWA_LWM2M_ID_ALL:
            {
                nbLink--;
                memmove(linkP + linkIndex, linkP + linkIndex + 1, (nbLink - linkIndex) * sizeof(link_t));
            }
            break;

        case IOWA_LWM2M_SECURITY_OBJECT_ID:
            nbLink--;
            memmove(linkP + linkIndex, linkP + linkIndex + 1, (nbLink - linkIndex) * sizeof(link_t));
            break;

        default:
            linkIndex++;
        }
    }
    result = coreLinkSerialize(linkP, nbLink, payloadP, payloadLengthP);
    coreLinkFree(linkP, nbLink);

    return result;
}

int32_t prv_getUpdateDelay(lwm2m_server_t *serverP)
{
    int32_t coapMaxTransmitWait;
    int32_t conservativeLifetime;

    coapMaxTransmitWait = coapPeerGetMaxTxWait(serverP->runtime.peerP);

    conservativeLifetime = serverP->lifetime;

    if (coapMaxTransmitWait < conservativeLifetime)
    {
        conservativeLifetime -= coapMaxTransmitWait;
    }
    else
    {
        conservativeLifetime = conservativeLifetime >> 1;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Conservative lifetime is %ds.", conservativeLifetime);

    return conservativeLifetime;
}

void prv_serverRegistrationFailing(iowa_context_t contextP,
                                   lwm2m_server_t *serverP)
{

    IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Registration or Registration Update to Server %d failed.", serverP->shortId);

    lwm2m_server_close(contextP, serverP, false);
    {
        serverP->runtime.status = STATE_REG_FAILED;
    }

    contextP->timeout = 0;


    coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_FAILED);
}

void prv_handleRegistrationUpdateReply(iowa_coap_peer_t *fromPeerP,
                                       uint8_t status,
                                       iowa_coap_message_t *responseP,
                                       void *userDataP,
                                       iowa_context_t contextP)
{

    lwm2m_server_t *serverP;
    uint8_t *updateFlagsP;


    (void)status;

    updateFlagsP = (uint8_t *)userDataP;


    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        if (serverP->runtime.peerP == fromPeerP)
        {
            break;
        }
    }
    if (serverP == NULL)
    {
        iowa_system_free(updateFlagsP);
        return;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));


    if (responseP == NULL)
    {
        serverP->runtime.flags &= ~LWM2M_SERVER_FLAG_AVAILABLE;
    }
    else
    {
        serverP->runtime.flags |= LWM2M_SERVER_FLAG_AVAILABLE;
    }


    switch (serverP->runtime.status)
    {
    case STATE_REG_UPDATE_PENDING:
        if (responseP != NULL)
        {
            switch (responseP->code)
            {
            case IOWA_COAP_204_CHANGED:
                if (serverP->runtime.lifetimeTimerP == NULL)
                {
                    serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, serverP->lifetime, prv_handleClientLifetimeTimer, serverP);
                    if (serverP->runtime.lifetimeTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        prv_serverRegistrationFailing(contextP, serverP);
                        goto exit;
                    }
                }
                else
                {
                    if (coreTimerReset(contextP, serverP->runtime.lifetimeTimerP, serverP->lifetime) != IOWA_COAP_NO_ERROR)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to reset the timer.");
                        prv_serverRegistrationFailing(contextP, serverP);
                        goto exit;
                    }
                }

                if (serverP->runtime.updateTimerP == NULL)
                {
                    serverP->runtime.updateTimerP = coreTimerNew(contextP, prv_getUpdateDelay(serverP), prv_handleClientUpdateTimer, serverP);
                    if (serverP->runtime.updateTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        prv_serverRegistrationFailing(contextP, serverP);
                        goto exit;
                    }
                }
                else
                {
                    if (coreTimerReset(contextP, serverP->runtime.updateTimerP, prv_getUpdateDelay(serverP)) != IOWA_COAP_NO_ERROR)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to reset the timer.");
                        prv_serverRegistrationFailing(contextP, serverP);
                        goto exit;
                    }
                }

                serverP->runtime.status = STATE_REG_REGISTERED;


                coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_REGISTERED);

                IOWA_LOG_INFO(IOWA_PART_LWM2M, "Registration update successful.");
                break;

            default:
                IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Received from the Server: %u.%02u.", (responseP->code & 0xFF) >> 5, (responseP->code & 0x1F));
                prv_serverRegistrationFailing(contextP, serverP);
            }
        }
        else
        {
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "No response received to the Registration update.");

            if (updateFlagsP != NULL)
            {

                serverP->runtime.update |= *updateFlagsP;
            }
            serverP->runtime.status = STATE_REG_REGISTERED;


            coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UPDATE_FAILED);
        }
        break;

    default:

        break;
    }

exit:
    iowa_system_free(updateFlagsP);
}

void prv_updateRegistration(iowa_context_t contextP,
                            lwm2m_server_t *serverP)
{

    iowa_coap_message_t *messageP;
    iowa_coap_option_t *optionP;
    uint8_t *payload;
    uint8_t lifetimeBuffer[QUERY_LIFETIME_LEN + QUERY_LIFETIME_MAX_LEN];
    iowa_status_t result;
    uint8_t token;
    uint8_t *updateFlagsP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server ID: %u, with update: 0x%02X.", serverP->shortId, serverP->runtime.update);

    coapPeerGenerateToken(serverP->runtime.peerP, 1, &token);
    messageP = iowa_coap_message_new(IOWA_COAP_TYPE_CONFIRMABLE, IOWA_COAP_CODE_POST, 1, &token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (messageP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP message.");
        return;
    }
#endif

    optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_PATH, serverP->runtime.location, REG_PATH_DELIMITER);
    if (optionP == NULL)
    {
        iowa_coap_message_free(messageP);
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Exiting.");
        return;
    }
    iowa_coap_message_add_option(messageP, optionP);

    if (serverP->runtime.update & LWM2M_UPDATE_FLAG_LIFETIME)
    {
        size_t length;

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Updating lifetime to %ds.", serverP->lifetime);

        memcpy(lifetimeBuffer, QUERY_LIFETIME, QUERY_LIFETIME_LEN);
        length = dataUtilsIntToBuffer(serverP->lifetime, lifetimeBuffer + QUERY_LIFETIME_LEN, QUERY_LIFETIME_MAX_LEN, false);
        if (length == 0)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Exiting.");
            return;
        }
        length += QUERY_LIFETIME_LEN;

        optionP = iowa_coap_option_new(IOWA_COAP_OPTION_URI_QUERY);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (optionP == NULL)
        {
            iowa_coap_message_free(messageP);
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
            return;
        }
#endif
        optionP->length = (uint16_t)length;
        optionP->value.asBuffer = lifetimeBuffer;
        iowa_coap_message_add_option(messageP, optionP);
    }

    payload = NULL;

    if (serverP->runtime.update & LWM2M_UPDATE_FLAG_OBJECTS)
    {
        size_t payloadLength;

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Updating Object list.", serverP->lifetime);

        result = prv_getRegistrationPayload(contextP, &payload, &payloadLength);
        if (result != IOWA_COAP_NO_ERROR)
        {
            return;
        }

        optionP = iowa_coap_option_new(IOWA_COAP_OPTION_CONTENT_FORMAT);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (optionP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");
            iowa_system_free(payload);
            iowa_coap_message_free(messageP);
            return;
        }
#endif
        optionP->value.asInteger = IOWA_COAP_FORMAT_LINK_FORMAT;
        iowa_coap_message_add_option(messageP, optionP);

        messageP->payload.length = payloadLength;
        messageP->payload.data = payload;
    }


    if (serverP->runtime.update != LWM2M_UPDATE_FLAG_NONE)
    {
        updateFlagsP = (uint8_t *)iowa_system_malloc(sizeof(uint8_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (updateFlagsP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(uint8_t));
            iowa_system_free(payload);
            iowa_coap_message_free(messageP);
            return;
        }
#endif
        *updateFlagsP = serverP->runtime.update;
    }
    else
    {
        updateFlagsP = NULL;
    }

    result = coapSend(contextP, serverP->runtime.peerP, messageP, prv_handleRegistrationUpdateReply, updateFlagsP);
    if (result == IOWA_COAP_NO_ERROR)
    {
        serverP->runtime.status = STATE_REG_UPDATE_PENDING;
        serverP->runtime.update = LWM2M_UPDATE_FLAG_NONE;


        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UPDATING);
    }
    else
    {
        iowa_system_free(updateFlagsP);


        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UPDATE_FAILED);
    }

    iowa_system_free(payload);
    iowa_coap_message_free(messageP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
}

void prv_handleClientUpdateTimer(iowa_context_t contextP,
                                 void *userData)
{

    lwm2m_server_t *serverP;

    serverP = (lwm2m_server_t *)userData;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    serverP->runtime.updateTimerP = NULL;

    switch (serverP->runtime.status)
    {
    case STATE_REG_REGISTERED:
        prv_updateRegistration(contextP, serverP);
        break;

    default:

        break;
    }
}

void prv_handleClientLifetimeTimer(iowa_context_t contextP,
                                   void *userData)
{
    lwm2m_server_t *serverP;

    serverP = (lwm2m_server_t *)userData;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    serverP->runtime.lifetimeTimerP = NULL;

    switch (serverP->runtime.status)
    {
    case STATE_DISCONNECTED:
#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE

        if (serverP->runtime.flags & LWM2M_SERVER_FLAG_DISABLE)
        {
            serverP->runtime.flags &= ~LWM2M_SERVER_FLAG_DISABLE;
        }
#endif
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
        if (serverP->runtime.flags & LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT)
        {
            serverP->runtime.flags &= ~LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT;

            serverP->runtime.status = STATE_WAITING_CONNECTION;
            if (prv_initiateServerConnection(contextP, serverP) != IOWA_COAP_NO_ERROR)
            {
                serverP->runtime.status = STATE_REG_FAILED;
                contextP->timeout = 0;
            }
        }
#endif
        break;

    default:
        prv_serverRegistrationFailing(contextP, serverP);
    }
}

void prv_handleRegistrationReply(iowa_coap_peer_t *fromPeer,
                                 uint8_t status,
                                 iowa_coap_message_t *responseP,
                                 void *userDataP,
                                 iowa_context_t contextP)
{

    lwm2m_server_t *serverP;

    (void)fromPeer;

    serverP = (lwm2m_server_t *)userDataP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));


    switch (serverP->runtime.status)
    {
    case STATE_REG_REGISTERING:
        if (responseP != NULL)
        {
            switch (status)
            {
            case IOWA_COAP_201_CREATED:
            {
                iowa_coap_option_t *optionP;
                iowa_coap_option_t *startP;
                size_t length;

                optionP = iowa_coap_message_find_option(responseP, IOWA_COAP_OPTION_LOCATION_PATH);
                if (optionP == NULL)
                {
                    IOWA_LOG_WARNING(IOWA_PART_LWM2M, "No Location-Path option found.");
                    return;
                }

                serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, serverP->lifetime, prv_handleClientLifetimeTimer, serverP);
                if (serverP->runtime.lifetimeTimerP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                    prv_serverRegistrationFailing(contextP, serverP);
                    return;
                }

                serverP->runtime.updateTimerP = coreTimerNew(contextP, prv_getUpdateDelay(serverP), prv_handleClientUpdateTimer, serverP);
                if (serverP->runtime.updateTimerP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                    prv_serverRegistrationFailing(contextP, serverP);
                    return;
                }

                startP = optionP;
                length = 0;
                while (optionP != NULL
                       && optionP->number == IOWA_COAP_OPTION_LOCATION_PATH)
                {
                    length += (size_t)(optionP->length + PRV_LINK_URI_SEPARATOR_SIZE);
                    optionP = optionP->next;
                }

                iowa_system_free(serverP->runtime.location);
                serverP->runtime.location = (char *)iowa_system_malloc(length + 1);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                if (serverP->runtime.location == NULL)
                {
                    IOWA_LOG_ERROR_MALLOC(length + 1);
                    return;
                }
#endif

                optionP = startP;
                length = 0;
                while (optionP != NULL
                       && optionP->number == IOWA_COAP_OPTION_LOCATION_PATH)
                {
                    memcpy(serverP->runtime.location + length, PRV_LINK_URI_SEPARATOR, PRV_LINK_URI_SEPARATOR_SIZE);
                    length += PRV_LINK_URI_SEPARATOR_SIZE;
                    memcpy(serverP->runtime.location + length, (char *)optionP->value.asBuffer, optionP->length);
                    length += optionP->length;
                    optionP = optionP->next;
                }
                serverP->runtime.location[length] = 0;

                serverP->runtime.status = STATE_REG_REGISTERED;

#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
                serverP->runtime.retryCount = 0;
                serverP->runtime.sequenceRetryCount = 0;
#endif

                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Registration successful at \"%s\".", serverP->runtime.location);


                coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_REGISTERED);
                break;
            }

            default:
                IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Received from the Server: %u.%02u.", (responseP->code & 0xFF) >> 5, (responseP->code & 0x1F));
                prv_serverRegistrationFailing(contextP, serverP);
            }
        }
        else
        {
            prv_serverRegistrationFailing(contextP, serverP);
        }
        break;

    default:

        break;
    }
}


iowa_status_t prv_register(iowa_context_t contextP,
                           lwm2m_server_t *serverP)
{

    char *query;
    size_t queryLength;
    uint8_t *payload;
    iowa_coap_message_t *messageP;
    iowa_coap_option_t *optionP;
    iowa_status_t result;
    uint8_t token;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering: Server ID: %u, state: %s.", serverP->shortId, LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    query = NULL;
    payload = NULL;
    messageP = NULL;

    if (prv_getRegistrationQuery(contextP, serverP, &queryLength, &query) != IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }

    coapPeerGenerateToken(serverP->runtime.peerP, 1, &token);
    messageP = iowa_coap_message_new(IOWA_COAP_TYPE_CONFIRMABLE, IOWA_COAP_CODE_POST, 1, &token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (messageP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP message.");
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }
#endif

    optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_PATH, URI_REGISTRATION_SEGMENT, REG_PATH_DELIMITER);
    if (optionP == NULL)
    {
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }
    iowa_coap_message_add_option(messageP, optionP);

    if (query != NULL)
    {
        optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_QUERY, query, QUERY_SEPARATOR);
        if (optionP == NULL)
        {
            result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            goto premature_exit;
        }
        iowa_coap_message_add_option(messageP, optionP);
    }

    switch (coapPeerGetConnectionType(serverP->runtime.peerP))
    {
    default:
    {
        size_t payloadLength;

        result = prv_getRegistrationPayload(contextP, &payload, &payloadLength);
        if (result != IOWA_COAP_NO_ERROR)
        {
            goto premature_exit;
        }

        optionP = iowa_coap_option_new(IOWA_COAP_OPTION_CONTENT_FORMAT);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (optionP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP option.");

            result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            goto premature_exit;
        }
#endif
        optionP->value.asInteger = IOWA_COAP_FORMAT_LINK_FORMAT;
        iowa_coap_message_add_option(messageP, optionP);

        messageP->payload.length = payloadLength;
        messageP->payload.data = payload;
    }
    }

    result = coapSend(contextP, serverP->runtime.peerP, messageP, prv_handleRegistrationReply, (void *)serverP);

premature_exit:
    iowa_coap_message_free(messageP);
    iowa_system_free(payload);
    iowa_system_free(query);

    if (result != IOWA_COAP_NO_ERROR)
    {
        coapPeerDelete(contextP, serverP->runtime.peerP);
        serverP->runtime.peerP = NULL;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with status: %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

static void prv_handleCoapRegistrationEvent(iowa_coap_peer_t *fromPeer,
                                            iowa_coap_peer_event_t event,
                                            void *userData,
                                            iowa_context_t contextP)
{

    lwm2m_server_t *serverP;

    (void)fromPeer;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "ServerP: %p, event: %s.", userData, STR_COAP_EVENT(event));

    serverP = (lwm2m_server_t *)userData;

    switch (event)
    {
    case COAP_EVENT_CONNECTED:
        switch (serverP->runtime.status)
        {
        case STATE_WAITING_CONNECTION:

            if (prv_register(contextP, serverP) == IOWA_COAP_NO_ERROR)
            {
                serverP->runtime.status = STATE_REG_REGISTERING;


                coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_REGISTERING);
            }
            else
            {
                serverP->runtime.status = STATE_REG_FAILED;
                contextP->timeout = 0;
            }
            break;

        case STATE_REG_REGISTERED:
            prv_updateRegistration(contextP, serverP);
            break;

        default:

            break;
        }
        break;

    case COAP_EVENT_DISCONNECTED:
        switch (serverP->runtime.status)
        {
        case STATE_WAITING_CONNECTION:

            serverP->runtime.status = STATE_REG_FAILED;
            contextP->timeout = 0;

            coapPeerDelete(contextP, serverP->runtime.peerP);
            serverP->runtime.peerP = NULL;
            break;

        case STATE_REG_REGISTERING:
            prv_serverRegistrationFailing(contextP, serverP);
            break;

        case STATE_REG_UPDATE_PENDING:

            serverP->runtime.status = STATE_REG_REGISTERED;
            break;

        default:

            break;
        }
        break;

    default:
        break;
    }
}

#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
bool prv_checkStartServerRegistration(iowa_context_t contextP,
                                      lwm2m_server_t *serverP)
{
    uint16_t currentLevelPriority;
    lwm2m_server_t *serverListP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Short Server ID: %d and Server priority order: %u.", serverP->shortId, serverP->registrationProcedure.priorityOrder);

    currentLevelPriority = UINT16_MAX;

    for (serverListP = contextP->lwm2mContextP->serverList; serverListP != NULL; serverListP = serverListP->next)
    {

        if (serverListP->registrationProcedure.priorityOrder < currentLevelPriority)
        {
            switch (serverListP->runtime.status)
            {
            case STATE_REG_FAILED:
                if (serverListP->registrationProcedure.blockOnFailure == false
#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
                    && serverListP->runtime.sequenceRetryCount == serverListP->registrationProcedure.sequenceRetryCount
                    && serverListP->runtime.retryCount == serverListP->registrationProcedure.retryCount
#endif
                    )
                {
                    break;
                }

            case STATE_DISCONNECTED:
            case STATE_REG_REGISTERING:
                currentLevelPriority = serverListP->registrationProcedure.priorityOrder;
                break;

            default:

                break;
            }
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Current level priority order: %u.", currentLevelPriority);

    return serverP->registrationProcedure.priorityOrder == currentLevelPriority;
}
#endif

iowa_status_t prv_initiateServerConnection(iowa_context_t contextP,
                                           lwm2m_server_t *serverP)
{
    iowa_status_t result;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Short Server ID: %d.", serverP->shortId);

    {
        result = utilsConnectServer(contextP, serverP, lwm2m_client_handle_request, prv_handleCoapRegistrationEvent);
        if (result != IOWA_COAP_NO_ERROR)
        {
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Failed to establish a connection with the server %d.", serverP->shortId);
                serverP->runtime.status = STATE_REG_FAILED;
                contextP->timeout = 0;
            }
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}
#endif

/*************************************************************************************
** Public functions
*************************************************************************************/

#ifdef LWM2M_CLIENT_MODE
void lwm2mUpdateRegistration(iowa_context_t contextP,
                             lwm2m_server_t *serverP,
                             uint8_t update)
{

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "State: %s, serverP: %p, update: 0x%02X.", LWM2M_STR_STATE(contextP->lwm2mContextP->state), serverP, update);

    if (serverP == NULL)
    {

        for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
        {

            switch (serverP->runtime.status)
            {
            case STATE_REG_REGISTERING:
            case STATE_REG_REGISTERED:
            case STATE_REG_UPDATE_PENDING:
                {
                    serverP->runtime.flags |= LWM2M_SERVER_FLAG_UPDATE;
                    serverP->runtime.update |= update;
                }
                break;

            default:

                break;
            }
        }
    }
    else
    {
        switch (serverP->runtime.status)
        {
        case STATE_REG_REGISTERING:
        case STATE_REG_REGISTERED:
        case STATE_REG_UPDATE_PENDING:
            {
                serverP->runtime.flags |= LWM2M_SERVER_FLAG_UPDATE;
                serverP->runtime.update |= update;
            }
            break;

        default:

            break;
        }
    }
}

void registration_deregister(iowa_context_t contextP,
                             lwm2m_server_t *serverP)
{

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "State: %s, serverP->runtime.status: %s", LWM2M_STR_STATE(contextP->lwm2mContextP->state), LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    switch (serverP->runtime.status)
    {
    case STATE_DISCONNECTED:
    case STATE_REG_FAILED:
        return;

    case STATE_REG_REGISTERING:
        serverP->runtime.status = STATE_DISCONNECTED;


        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UNREGISTERED);
        break;

    default:
        if (serverP->runtime.peerP != NULL)
        {
            iowa_coap_message_t *messageP;
            iowa_coap_option_t *optionP;
            uint8_t token;

            coapPeerGenerateToken(serverP->runtime.peerP, 1, &token);
            messageP = iowa_coap_message_new(IOWA_COAP_TYPE_NON_CONFIRMABLE, IOWA_COAP_CODE_DELETE, 1, &token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (messageP == NULL)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP message.");
                return;
            }
#endif

            optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_PATH, serverP->runtime.location, REG_PATH_DELIMITER);
            if (optionP != NULL)
            {
                iowa_coap_message_add_option(messageP, optionP);

                (void)coapSend(contextP, serverP->runtime.peerP, messageP, NULL, NULL);

                iowa_coap_message_free(messageP);
            }
        }

        serverP->runtime.status = STATE_DISCONNECTED;


        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UNREGISTERED);
    }
}

void registration_resetServersStatus(iowa_context_t contextP)
{
    lwm2m_server_t *serverP;

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        serverP->runtime.status = STATE_DISCONNECTED;
        serverP->runtime.flags &= LWM2M_SERVER_FLAG_SECURITY_DATA_ADDED;
        serverP->runtime.update = LWM2M_UPDATE_FLAG_NONE;
    }
}

iowa_status_t registration_step(iowa_context_t contextP)
{

    iowa_status_t result;
    lwm2m_server_t *serverP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with timeout %ds and current time: %ds.", contextP->timeout, contextP->currentTime);

    result = IOWA_COAP_503_SERVICE_UNAVAILABLE;

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        iowa_status_t serverResult;

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server ID: %u, state: %s, lifetime: %ds.", serverP->shortId, LWM2M_SERVER_STR_STATUS(serverP->runtime.status), serverP->lifetime);

        switch (serverP->runtime.status)
        {
        case STATE_DISCONNECTED:

#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_DISABLE)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server %u is disabled.", serverP->shortId);
                serverResult = IOWA_COAP_NO_ERROR;
            }
            else
#endif
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Waiting before registering to Server %u.", serverP->shortId);
                serverResult = IOWA_COAP_NO_ERROR;
            }
            else
#endif
            {
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE

                if (prv_checkStartServerRegistration(contextP, serverP) == false)
                {
                    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "The registration to the Server %u can not start.", serverP->shortId);
                    serverResult = IOWA_COAP_NO_ERROR;
                    break;
                }

                if (serverP->registrationProcedure.initialDelayTimer != 0)
                {
                    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Starting a timer to wait before starting the registration to Server %u.", serverP->shortId);

                    serverResult = IOWA_COAP_NO_ERROR;


                    serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, serverP->registrationProcedure.initialDelayTimer, prv_handleClientLifetimeTimer, serverP);
                    if (serverP->runtime.lifetimeTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        serverP->runtime.status = STATE_REG_FAILED;
                        contextP->timeout = 0;
                        break;
                    }

                    serverP->runtime.flags |= LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT;
                    break;
                }
#endif

                serverP->runtime.status = STATE_WAITING_CONNECTION;
                serverResult = prv_initiateServerConnection(contextP, serverP);
            }
            break;

        case STATE_REG_REGISTERED:
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_UPDATE)
            {
                if (serverP->runtime.peerP == NULL)
                {
                    serverResult = prv_initiateServerConnection(contextP, serverP);
                }
                else
                {
                    serverResult = IOWA_COAP_NO_ERROR;
                    prv_updateRegistration(contextP, serverP);
                }

                serverP->runtime.flags &= ~LWM2M_SERVER_FLAG_UPDATE;
            }
            else
#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_DISABLE)
            {
                serverResult = IOWA_COAP_NO_ERROR;

                lwm2m_server_close(contextP, serverP, true);

                serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, serverP->disableTimeout, prv_handleClientLifetimeTimer, serverP);
                if (serverP->runtime.lifetimeTimerP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                    prv_serverRegistrationFailing(contextP, serverP);
                    break;
                }

                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server %u disabled.", serverP->shortId);
            }
            else
#endif
            {
                serverResult = IOWA_COAP_NO_ERROR;
            }
            break;

        case STATE_REG_FAILED:
#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
            if (serverP->runtime.retryCount < serverP->registrationProcedure.retryCount)
            {
                if (serverP->registrationProcedure.retryDelayTimer != 0)
                {
                    int32_t nextTimer;

                    nextTimer = serverP->registrationProcedure.retryDelayTimer * (1 << serverP->runtime.retryCount);
                    if ((nextTimer / serverP->registrationProcedure.retryDelayTimer) != (1 << serverP->runtime.retryCount))
                    {
                        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Integer overflow.");
                        serverResult = IOWA_COAP_503_SERVICE_UNAVAILABLE;
                        break;
                    }


                    serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, nextTimer, prv_handleClientLifetimeTimer, serverP);
                    if (serverP->runtime.lifetimeTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        serverResult = IOWA_COAP_503_SERVICE_UNAVAILABLE;
                        break;
                    }

                    serverP->runtime.flags |= LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT;
                }

                serverP->runtime.retryCount++;
                serverP->runtime.status = STATE_DISCONNECTED;
                serverP->runtime.update = LWM2M_UPDATE_FLAG_NONE;

                serverResult = IOWA_COAP_NO_ERROR;

                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server retry count %u/%u.", serverP->runtime.retryCount, serverP->registrationProcedure.retryCount);
            }
            else if (serverP->runtime.sequenceRetryCount < serverP->registrationProcedure.sequenceRetryCount)
            {
                if (serverP->registrationProcedure.sequenceDelayTimer != 0)
                {
                    int32_t nextTimer;

                    nextTimer = serverP->registrationProcedure.sequenceDelayTimer * (1 << serverP->runtime.sequenceRetryCount);
                    if ((nextTimer / serverP->registrationProcedure.sequenceDelayTimer) != (1 << serverP->runtime.sequenceRetryCount))
                    {
                        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Integer overflow.");
                        serverResult = IOWA_COAP_503_SERVICE_UNAVAILABLE;
                        break;
                    }


                    serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, nextTimer, prv_handleClientLifetimeTimer, serverP);
                    if (serverP->runtime.lifetimeTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        serverResult = IOWA_COAP_503_SERVICE_UNAVAILABLE;
                        break;
                    }

                    serverP->runtime.flags |= LWM2M_SERVER_FLAG_INITIAL_TIMER_WAIT;
                }

                serverP->runtime.retryCount = 0;
                serverP->runtime.sequenceRetryCount++;
                serverP->runtime.status = STATE_DISCONNECTED;
                serverP->runtime.update = LWM2M_UPDATE_FLAG_NONE;

                serverResult = IOWA_COAP_NO_ERROR;

                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server sequence retry count %u/%u.", serverP->runtime.sequenceRetryCount, serverP->registrationProcedure.sequenceRetryCount);
            }
            else
#endif
            {
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE

                if (serverP->registrationProcedure.bootstrapOnFailure == true)
                {

                    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Registration sequence ended.");
                    return IOWA_COAP_503_SERVICE_UNAVAILABLE;
                }
#endif

                IOWA_LOG_INFO(IOWA_PART_LWM2M, "Registration failed.");
                serverResult = IOWA_COAP_503_SERVICE_UNAVAILABLE;
            }
            break;

        default:
            serverResult = IOWA_COAP_NO_ERROR;
        }

        if (serverResult == IOWA_COAP_NO_ERROR
            && result != IOWA_COAP_NO_ERROR)
        {
            result = IOWA_COAP_NO_ERROR;
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}
#endif

