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
* Copyright (c) 2018-2022 IoTerop.
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

#define MAX_LOCATION_LENGTH         10      // strlen("/rd/65534") + 1
#define PRV_LINK_URI_SEPARATOR      "/"
#define PRV_LINK_URI_SEPARATOR_SIZE 1

#define PRV_DECIMAL_POINT                  '.'
#define PRV_DEFAULT_MAJOR_OBJECT_VERSION    1
#define PRV_DEFAULT_MINOR_OBJECT_VERSION    0

#define PRV_RESULT_NO_ERROR_LIFETIME     1
#define PRV_RESULT_NO_ERROR_NO_LIFETIME  0
#define PRV_RESULT_ERROR                 -1
#define PRV_REG_INITAL_TOKEN             {0, 0, 1}

#define PRV_DEFAULT_MAX_REGISTRATION_DELAY 93

/*************************************************************************************
** Private functions
*************************************************************************************/

#ifdef LWM2M_CLIENT_MODE
static iowa_status_t prv_getRegistrationQuery(iowa_context_t contextP, lwm2m_server_t *serverP, size_t *lengthP, char **bufferP);
static iowa_status_t prv_getRegistrationPayload(iowa_context_t contextP, uint8_t **payloadP, size_t *payloadLengthP);
static int32_t prv_getUpdateDelay(lwm2m_server_t *serverP);
static void prv_serverRegistrationFailing(iowa_context_t contextP, lwm2m_server_t *serverP, bool isInternal, uint8_t code);
static void prv_handleRegistrationUpdateReply(iowa_coap_peer_t *fromPeerP, uint8_t status, iowa_coap_message_t * responseP, void * userData, iowa_context_t contextP);
static void prv_updateRegistration(iowa_context_t contextP, lwm2m_server_t *serverP);
static void prv_handleClientUpdateTimer(iowa_context_t contextP, void *userData);
static void prv_handleClientLifetimeTimer(iowa_context_t contextP, void *userData);
static void prv_handleRegistrationReply(iowa_coap_peer_t *fromPeer, uint8_t status, iowa_coap_message_t *responseP, void * userData, iowa_context_t contextP);
static iowa_status_t prv_register(iowa_context_t contextP, lwm2m_server_t *serverP);
static iowa_status_t prv_initiateServerConnection(iowa_context_t contextP, lwm2m_server_t *serverP, bool registrationFailureOnError);
#endif // LWM2M_CLIENT_MODE

#ifdef LWM2M_CLIENT_MODE
static bool prv_getServerByPeer(void *nodeP,
                                void *criteria)
{
    return (((lwm2m_server_t *)nodeP)->runtime.peerP == (iowa_coap_peer_t *)criteria);
}

iowa_status_t prv_getRegistrationQuery(iowa_context_t contextP,
                                       lwm2m_server_t *serverP,
                                       size_t *lengthP,
                                       char **bufferP)
{
    size_t length;
    size_t res;
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT) || defined(IOWA_SMS_SUPPORT)
    uint8_t *strBinding;
    size_t strBindingLen;

    // SonarQube does not raise any issue but some compilers emit warning hence the superfluous initialization of strBindingLen.
    strBindingLen = 0;
#endif

    // Calculate the query length
    switch (coapPeerGetConnectionType(serverP->runtime.peerP))
    {

    default:
    {
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT) || defined(IOWA_SMS_SUPPORT)
        iowa_lwm2m_binding_t binding;

        length = QUERY_LIFETIME_LEN + dataUtilsIntToBufferLength(serverP->lifetime, false) + 1 + QUERY_DELIMITER_LEN + QUERY_VERSION_LEN + LWM2M_VERSION_LEN;
        if (contextP->lwm2mContextP->endpointName != NULL)
        {
            length += QUERY_DELIMITER_LEN + QUERY_NAME_LEN + strlen(contextP->lwm2mContextP->endpointName);
        }

        switch (serverP->lwm2mVersion)
        {
        case IOWA_LWM2M_VERSION_1_0:
            // In LwM2M 1.0: Server transports + Server Queue mode
            binding = serverP->binding;
            break;

        default:
            goto exit_on_error;
        }

        if (binding != IOWA_LWM2M_BINDING_UDP) // UDP binding is the default binding
        {
            switch (serverP->lwm2mVersion)
            {
            case IOWA_LWM2M_VERSION_1_0:
                strBindingLen = utils_bindingToString(binding, (binding & BINDING_Q) != 0, &strBinding);
                break;

            default:
                goto exit_on_error;
            }

            length += QUERY_DELIMITER_LEN + QUERY_BINDING_LEN + strBindingLen;
        }
        else
        {
            strBindingLen = 0;
        }

#endif // IOWA_UDP_SUPPORT || IOWA_TCP_SUPPORT  || IOWA_WEBSOCKET_SUPPORT || IOWA_SMS_SUPPORT
    }
    }

    // Allocate the memory
    *bufferP = (char *)iowa_system_malloc(length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*bufferP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(length);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    // Fill the query
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
#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_TCP_SUPPORT) || defined(IOWA_WEBSOCKET_SUPPORT) || defined(IOWA_SMS_SUPPORT)
        *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, QUERY_DELIMITER QUERY_VERSION);

        switch (serverP->lwm2mVersion)
        {
        case IOWA_LWM2M_VERSION_1_0:
            *lengthP += utilsStringCopy(*bufferP + *lengthP, length - *lengthP, LWM2M_VERSION_1_0);
            break;

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

#endif // IOWA_UDP_SUPPORT || IOWA_TCP_SUPPORT  || IOWA_WEBSOCKET_SUPPORT || IOWA_SMS_SUPPORT
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

    // Get CoRE Link
    result = object_getList(contextP, IOWA_LWM2M_ID_ALL, &linkP, &nbLink);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to retrieve the object tree.");
        return result;
    }

    // Remove not needed object in registration message
    linkIndex = 0;
    while (linkIndex < nbLink)
    {
        switch (linkP[linkIndex].uri.objectId)
        {
        case IOWA_LWM2M_ID_ALL:
#ifdef LWM2M_ALTPATH_SUPPORT
            if (contextP->lwm2mContextP->altPath != NULL)
            {
                result = coreLinkAddBufferAttribute(linkP, KEY_RESOURCE_TYPE, (uint8_t *)REG_RESOURCE_TYPE, REG_RESOURCE_TYPE_LEN, false);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    coreLinkFree(linkP, nbLink);
                    return result;
                }
                linkIndex++;
            }
            else
#endif
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
#ifdef LWM2M_ALTPATH_SUPPORT
    result = coreLinkSerialize(linkP, nbLink, contextP->lwm2mContextP->altPath, payloadP, payloadLengthP);
#else
    result = coreLinkSerialize(linkP, nbLink, payloadP, payloadLengthP);
#endif
    coreLinkFree(linkP, nbLink);

    return result;
}

int32_t prv_getUpdateDelay(lwm2m_server_t *serverP)
{
    int32_t coapMaxTransmitWait;
    int32_t conservativeLifetime;

    coapMaxTransmitWait = coapPeerGetMaxTxWait(serverP->runtime.peerP);
    if (coapMaxTransmitWait < 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_BASE, "Failed to get max transmit time.");
        return -1;
    }

    conservativeLifetime = serverP->lifetime;

    if (coapMaxTransmitWait < conservativeLifetime)
    {
        conservativeLifetime -= coapMaxTransmitWait;
    }
    else
    {
        conservativeLifetime = (uint32_t)conservativeLifetime >> 1;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Conservative lifetime is %ds.", conservativeLifetime);

    return conservativeLifetime;
}

void prv_serverRegistrationFailing(iowa_context_t contextP,
                                   lwm2m_server_t *serverP,
                                   bool isInternal,
                                   uint8_t code)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Registration or Registration Update to Server %d failed.", serverP->shortId);

    lwm2m_server_close(contextP, serverP, false);
    {
        serverP->runtime.status = STATE_REG_FAILED;
    }

    contextP->timeout = 0;

    // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
    coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_FAILED, isInternal, code);
}

void prv_handleRegistrationUpdateReply(iowa_coap_peer_t *fromPeerP,
                                       uint8_t status,
                                       iowa_coap_message_t *responseP,
                                       void *userDataP,
                                       iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    lwm2m_server_t *serverP;
    uint8_t *updateFlagsP;


    (void)status;

    updateFlagsP = (uint8_t *)userDataP; // The user data contains the registration flags to remove

    // Find the Server from the CoAP peer
    serverP = (lwm2m_server_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->serverList, utilsListFindCallbackServerByPeer, fromPeerP);
    if (serverP == NULL)
    {
        iowa_system_free(updateFlagsP);
        return;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    // Set the availability of the Server
    if (responseP == NULL)
    {
        serverP->runtime.flags &= (uint16_t)(~LWM2M_SERVER_FLAG_AVAILABLE);
    }
    else
    {
        serverP->runtime.flags |= LWM2M_SERVER_FLAG_AVAILABLE;
    }

    // Handle the Registration Update reply
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
                        prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                        goto exit;
                    }
                }
                else
                {
                    if (coreTimerReset(contextP, serverP->runtime.lifetimeTimerP, serverP->lifetime) != IOWA_COAP_NO_ERROR)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to reset the timer.");
                        prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                        goto exit;
                    }
                }

                if (serverP->runtime.updateTimerP == NULL)
                {
                    serverP->runtime.updateTimerP = coreTimerNew(contextP, prv_getUpdateDelay(serverP), prv_handleClientUpdateTimer, serverP);
                    if (serverP->runtime.updateTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                        goto exit;
                    }
                }
                else
                {
                    if (coreTimerReset(contextP, serverP->runtime.updateTimerP, prv_getUpdateDelay(serverP)) != IOWA_COAP_NO_ERROR)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to reset the timer.");
                        prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                        goto exit;
                    }
                }

                serverP->runtime.status = STATE_REG_REGISTERED;

                // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
                coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_REGISTERED, false, IOWA_COAP_NO_ERROR);

                IOWA_LOG_INFO(IOWA_PART_LWM2M, "Registration update successful.");
                break;

            default:
                IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Received from the Server: %u.%02u.", (responseP->code & 0xFF) >> 5, (responseP->code & 0x1F));
                prv_serverRegistrationFailing(contextP, serverP, false, responseP->code);
            }
        }
        else
        {
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "No response received to the Registration update.");

            if (updateFlagsP != NULL)
            {
                // Set back the registration update flags since the Server didn't receive the update
                serverP->runtime.update |= *updateFlagsP;
            }
            serverP->runtime.status = STATE_REG_REGISTERED; // Fallback to registered state since the registration update mechanism is finished.

            // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
            coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UPDATE_FAILED, true, IOWA_COAP_503_SERVICE_UNAVAILABLE);
        }
        break;

    default:
        // Do nothing
        break;
    }

exit:
    iowa_system_free(updateFlagsP);
}

void prv_updateRegistration(iowa_context_t contextP,
                            lwm2m_server_t *serverP)
{
    // WARNING: This function is called in a critical section
    iowa_coap_message_t *messageP;
    iowa_coap_option_t *optionP;
    uint8_t *payload;
    char *bufferP;
    size_t length;
    iowa_status_t result;
    uint8_t token[COAP_MSG_TOKEN_MAX_LEN];
    uint8_t tokenLength;
    uint8_t *updateFlagsP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server ID: %u, with update: 0x%02X.", serverP->shortId, serverP->runtime.update);

    result = coapPeerGenerateToken(serverP->runtime.peerP, &tokenLength, token);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failure to generate a new token.");
        return;
    }

    messageP = iowa_coap_message_new(IOWA_COAP_TYPE_CONFIRMABLE, IOWA_COAP_CODE_POST, tokenLength, token);
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
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create a COAP message.");
        return;
    }
    iowa_coap_message_add_option(messageP, optionP);

    bufferP = NULL;
    length = 0;

    //Get the length of the registration update query.
    if (serverP->runtime.update & LWM2M_UPDATE_FLAG_LIFETIME)
    {
        length += QUERY_LIFETIME_LEN + QUERY_LIFETIME_MAX_LEN;
    }

    if (serverP->runtime.update & LWM2M_UPDATE_FLAG_BINDING)
    {
        if (length > 0)
        {
           length += QUERY_DELIMITER_LEN;
        }
        length += QUERY_BINDING_LEN + QUERY_BINDING_MAX_LEN;

    }

    //Set the registration update query.
    if (length > 0)
    {
        size_t index;

        length++; // for end of line.
        index = 0;

        bufferP = (char *)iowa_system_malloc(length * sizeof(char));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (bufferP == NULL)
        {
            iowa_coap_message_free(messageP);
            IOWA_LOG_ERROR_MALLOC(length * sizeof(char));
            return;
        }
#endif

        if (serverP->runtime.update & LWM2M_UPDATE_FLAG_LIFETIME)
        {
            size_t res;

            index += utilsStringCopy(bufferP, length, QUERY_LIFETIME);
            res = dataUtilsIntToBuffer(serverP->lifetime, (uint8_t *)bufferP + index, length - index, false);
            if (res == 0)
            {
                iowa_system_free(bufferP);
                iowa_coap_message_free(messageP);
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to parse lifetime");
                return;
            }
            index += res;
            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Updating lifetime to %ds.", serverP->lifetime);
        }

        if (serverP->runtime.update & LWM2M_UPDATE_FLAG_BINDING)
        {
            uint8_t *strBinding;
            size_t strBindingLen;

            if (index > 0)
            {
                index += utilsStringCopy(bufferP + index, length - index, QUERY_DELIMITER);
            }

            index += utilsStringCopy(bufferP + index, length - index, QUERY_BINDING);

            strBinding = NULL;
            strBindingLen = 0;

            switch(serverP->lwm2mVersion)
            {
            case IOWA_LWM2M_VERSION_1_0:
                strBindingLen = utils_bindingToString(serverP->binding, (serverP->binding & BINDING_Q) != 0, &strBinding);
                break;

            default:
                //Should not happen.
                break;
            }

            memcpy(bufferP + index, strBinding, strBindingLen);
            index += strBindingLen ;
            iowa_system_free(strBinding);

            IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Updating binding to %d.", serverP->binding);
        }
        bufferP[index] = '\0';

        optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_QUERY, bufferP, QUERY_SEPARATOR);
        if (optionP == NULL)
        {
            iowa_system_free(bufferP);
            iowa_coap_message_free(messageP);
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new COAP option.");
            return;
        }
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
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to get the registration payload.");
            iowa_coap_message_free(messageP);
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

        coreBufferSet(&(messageP->payload), payload, payloadLength);
    }

    // Keep the value of the registration update flags
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
        serverP->runtime.update = LWM2M_UPDATE_FLAG_NONE; // Reset the registration update flags since the message has been sent

        // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UPDATING, false, IOWA_COAP_NO_ERROR);
    }
    else
    {
        iowa_system_free(updateFlagsP);

        // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UPDATE_FAILED, true, result);
    }

    iowa_system_free(payload);
    iowa_system_free(bufferP);
    iowa_coap_message_free(messageP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
}

void prv_handleClientUpdateTimer(iowa_context_t contextP,
                                 void *userData)
{
    // WARNING: This function is called in a critical section
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
        // Do nothing
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
#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
        // Handle Disable behaviour
        if (serverP->runtime.flags & LWM2M_SERVER_FLAG_DISABLE)
        {
            serverP->runtime.flags &= (uint16_t)(~LWM2M_SERVER_FLAG_DISABLE);
            contextP->timeout = 0;
        }
#endif // IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
        break;

    default:
        prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_503_SERVICE_UNAVAILABLE);
    }
}

void prv_handleRegistrationExchangeTimer(iowa_context_t contextP,
                                         void *userData)
{
    // WARNING: This function is called in a critical section
    lwm2m_server_t *serverP;

    serverP = (lwm2m_server_t *)userData;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    serverP->runtime.updateTimerP = NULL;

    prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_503_SERVICE_UNAVAILABLE);
}

void prv_handleRegistrationReply(iowa_coap_peer_t *fromPeer,
                                 uint8_t status,
                                 iowa_coap_message_t *responseP,
                                 void *userDataP,
                                 iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    lwm2m_server_t *serverP;

    serverP = (lwm2m_server_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->serverList, prv_getServerByPeer, fromPeer);
    if (serverP == NULL)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Server has not been found.");
        goto exit;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Server state: %s.", LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    // Stop the Exchange timer
    if (serverP->runtime.updateTimerP != NULL)
    {
        coreTimerDelete(contextP, serverP->runtime.updateTimerP);
        serverP->runtime.updateTimerP = NULL;
    }

    // Handle the Registration reply
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
                    prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_406_NOT_ACCEPTABLE);
                    goto exit;
                }

                serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, serverP->lifetime, prv_handleClientLifetimeTimer, serverP);
                if (serverP->runtime.lifetimeTimerP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                    prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                    goto exit;
                }

                serverP->runtime.updateTimerP = coreTimerNew(contextP, prv_getUpdateDelay(serverP), prv_handleClientUpdateTimer, serverP);
                if (serverP->runtime.updateTimerP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                    prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                    goto exit;
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
                    prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                    goto exit;
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

                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Registration successful at \"%s\".", serverP->runtime.location);

                // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
                coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_REGISTERED, false, IOWA_COAP_NO_ERROR);
                break;
            }

            default:
                IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Received from the Server: %u.%02u.", (responseP->code & 0xFF) >> 5, (responseP->code & 0x1F));
                prv_serverRegistrationFailing(contextP, serverP, false, responseP->code);
            }
        }
        else
        {
            prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_503_SERVICE_UNAVAILABLE);
        }
        break;

    default:
        // Do nothing
        break;
    }
exit:
    iowa_system_free(userDataP);
}

// send the registration for a single server
iowa_status_t prv_register(iowa_context_t contextP,
                           lwm2m_server_t *serverP)
{
    // WARNING: This function is called in a critical section
    char *uriPath;
    char *uriQuery;
    char *query;
    size_t queryLength;
    uint8_t *payload;
    iowa_coap_message_t *messageP;
    iowa_coap_option_t *optionP;
    iowa_status_t result;
    uint8_t token[3] = PRV_REG_INITAL_TOKEN;
    uint8_t tokenLength;
    int32_t delay;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering: Server ID: %u, state: %s.", serverP->shortId, LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    uriPath = NULL;
    uriQuery = NULL;
    query = NULL;
    payload = NULL;
    messageP = NULL;

    if (iowa_coap_uri_parse(serverP->uri, NULL, NULL, NULL, &uriPath, &uriQuery, NULL) != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to parse the LwM2M Server URI.");
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }

    if (prv_getRegistrationQuery(contextP, serverP, &queryLength, &query) != IOWA_COAP_NO_ERROR)
    {
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }

    result = coapPeerGenerateToken(serverP->runtime.peerP, &tokenLength, token);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failure to generate a new token.");
        goto premature_exit;
    }

    messageP = iowa_coap_message_new(IOWA_COAP_TYPE_CONFIRMABLE, IOWA_COAP_CODE_POST, 3, token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (messageP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create new CoAP message.");
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }
#endif

    if (uriPath != NULL)
    {
        optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_PATH, uriPath, REG_PATH_DELIMITER);
        if (optionP == NULL)
        {
            result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            goto premature_exit;
        }
        iowa_coap_message_add_option(messageP, optionP);
    }

    optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_PATH, URI_REGISTRATION_SEGMENT, REG_PATH_DELIMITER);
    if (optionP == NULL)
    {
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }
    iowa_coap_message_add_option(messageP, optionP);

    if (uriQuery != NULL)
    {
        optionP = iowa_coap_path_to_option(IOWA_COAP_OPTION_URI_QUERY, uriQuery, QUERY_SEPARATOR);
        if (optionP == NULL)
        {
            result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            goto premature_exit;
        }
        iowa_coap_message_add_option(messageP, optionP);
    }

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

        coreBufferSet(&(messageP->payload), payload, payloadLength);
    }
    }

    delay = coapPeerGetExchangeLifetime(serverP->runtime.peerP);
    if ((0 == delay)
        || (INT32_MAX == delay))
    {
        delay = PRV_DEFAULT_MAX_REGISTRATION_DELAY;
    }

    if (serverP->runtime.updateTimerP != NULL)
    {
        coreTimerDelete(contextP, serverP->runtime.updateTimerP);
        serverP->runtime.updateTimerP = NULL;
    }
    serverP->runtime.updateTimerP = coreTimerNew(contextP, delay, prv_handleRegistrationExchangeTimer, serverP);
    if (serverP->runtime.updateTimerP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto premature_exit;
    }

    result = coapSend(contextP, serverP->runtime.peerP, messageP, prv_handleRegistrationReply, (void *)payload);

premature_exit:
    iowa_coap_message_free(messageP);
    iowa_system_free(uriPath);
    iowa_system_free(uriQuery);
    iowa_system_free(query);

    if (result != IOWA_COAP_NO_ERROR)
    {
        iowa_system_free(payload);
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
    // WARNING: This function is called in a critical section
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
            // The Client is not yet registered to the Server
            if (prv_register(contextP, serverP) == IOWA_COAP_NO_ERROR)
            {
                serverP->runtime.status = STATE_REG_REGISTERING;
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
            // Do nothing
            break;
        }
        break;

    case COAP_EVENT_DISCONNECTED:
        switch (serverP->runtime.status)
        {
        case STATE_WAITING_CONNECTION:
            // The Client is not yet registered to the Server
            prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_503_SERVICE_UNAVAILABLE);
            break;

        default:
            // Do nothing
            break;
        }
        break;

    default:
        break;
    }
}

iowa_status_t prv_initiateServerConnection(iowa_context_t contextP,
                                           lwm2m_server_t *serverP,
                                           bool registrationFailureOnError)
{
    iowa_status_t result;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with Short Server ID: %d.", serverP->shortId);

    {
        result = utilsConnectServer(contextP, serverP, lwm2m_client_handle_request, prv_handleCoapRegistrationEvent);
        if (result != IOWA_COAP_NO_ERROR
            && true == registrationFailureOnError)
        {
            {
                IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Failed to establish a connection with the server %d.", serverP->shortId);
                serverP->runtime.status = STATE_REG_FAILED;
                contextP->timeout = 0;
            }
        }
        else
        {
            if (STATE_REG_REGISTERED != serverP->runtime.status
                && STATE_REG_UPDATE_PENDING != serverP->runtime.status)
            {
                // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
                coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_REGISTERING, false, IOWA_COAP_NO_ERROR);
            }
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}
#endif // LWM2M_CLIENT_MODE

/*************************************************************************************
** Public functions
*************************************************************************************/

#ifdef LWM2M_CLIENT_MODE
void lwm2mUpdateRegistration(iowa_context_t contextP,
                             lwm2m_server_t *serverP,
                             uint8_t update)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "State: %s, serverP: %p, update: 0x%02X.", LWM2M_STR_STATE(contextP->lwm2mContextP->state), serverP, update);

    if (serverP == NULL)
    {
        // Do a registration update on all the Servers
        for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
        {
            // Only flag the LwM2M Server which the Client is already registered on
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
                // Do nothing
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
            // Do nothing
            break;
        }
    }
}

void registration_deregister(iowa_context_t contextP,
                             lwm2m_server_t *serverP)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "State: %s, serverP->runtime.status: %s", LWM2M_STR_STATE(contextP->lwm2mContextP->state), LWM2M_SERVER_STR_STATUS(serverP->runtime.status));

    switch (serverP->runtime.status)
    {
    case STATE_DISCONNECTED:
    case STATE_WAITING_CONNECTION:
    case STATE_REG_FAILED:
        return;

    case STATE_REG_REGISTERING:
        serverP->runtime.status = STATE_DISCONNECTED;

        // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UNREGISTERED, false, IOWA_COAP_NO_ERROR);
        break;

    default:
        if (serverP->runtime.peerP != NULL)
        {
            iowa_coap_message_t *messageP;
            iowa_coap_option_t *optionP;
            uint8_t token;
            uint8_t tokenLength;
            iowa_status_t result;

            result = coapPeerGenerateToken(serverP->runtime.peerP, &tokenLength, &token);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failure to generate a new token.");
                return;
            }

            messageP = iowa_coap_message_new(IOWA_COAP_TYPE_NON_CONFIRMABLE, IOWA_COAP_CODE_DELETE, tokenLength, &token);
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

        // After the server event callback, don't try to access 'serverP' pointer since the application callback could have removed it
        coreServerEventCallback(contextP, serverP, IOWA_EVENT_REG_UNREGISTERED, false, IOWA_COAP_NO_ERROR);
    }
}

void registration_resetServersStatus(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    lwm2m_server_t *serverP;

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        serverP->runtime.status = STATE_DISCONNECTED;
        serverP->runtime.flags &= LWM2M_SERVER_FLAG_SECURITY_DATA_ADDED; // Only keep Security Data added flag
        serverP->runtime.update = LWM2M_UPDATE_FLAG_NONE;
    }
}

iowa_status_t registration_step(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;
    lwm2m_server_t *serverP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Entering with timeout %ds and current time: %ds.", contextP->timeout, contextP->currentTime);

    result = IOWA_COAP_503_SERVICE_UNAVAILABLE;

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        iowa_status_t serverResult;

        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server ID: %u, state: %s, lifetime: %ds.", serverP->shortId, LWM2M_SERVER_STR_STATUS(serverP->runtime.status), serverP->lifetime);

        switch (serverP->runtime.status)
        {
        case STATE_DISCONNECTED:
            // Check if the server is not disabled
#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_DISABLE)
            {
                IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Server %u is disabled.", serverP->shortId);
                serverResult = IOWA_COAP_NO_ERROR;
            }
            else
#endif // IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
            {
                serverP->runtime.status = STATE_WAITING_CONNECTION;
                serverResult = prv_initiateServerConnection(contextP, serverP, true);
            }
            break;

        case STATE_REG_REGISTERED:
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_UPDATE
                && (contextP->lwm2mContextP->internalFlag & CONTEXT_FLAG_INSIDE_CALLBACK) == 0)
            {
                if (serverP->runtime.peerP == NULL
                    || coapPeerGetConnectionState(serverP->runtime.peerP) != SECURITY_STATE_CONNECTED)
                {
                    // Do not check the result here, since we are already registered on the server
                    (void)prv_initiateServerConnection(contextP, serverP, false);
                }
                else
                {
                    prv_updateRegistration(contextP, serverP);
                }

                serverResult = IOWA_COAP_NO_ERROR;
                serverP->runtime.flags &= (uint16_t)(~LWM2M_SERVER_FLAG_UPDATE);
            }
            else
#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
            if (serverP->runtime.flags & LWM2M_SERVER_FLAG_DISABLE)
            {
                serverResult = IOWA_COAP_NO_ERROR;

                lwm2m_server_close(contextP, serverP, true);

                if (serverP->disableTimeout >= 0)
                {
                    serverP->runtime.lifetimeTimerP = coreTimerNew(contextP, serverP->disableTimeout, prv_handleClientLifetimeTimer, serverP);
                    if (serverP->runtime.lifetimeTimerP == NULL)
                    {
                        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to create the timer.");
                        prv_serverRegistrationFailing(contextP, serverP, true, IOWA_COAP_500_INTERNAL_SERVER_ERROR);
                        break;
                    }
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
            {
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

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}
#endif // LWM2M_CLIENT_MODE

