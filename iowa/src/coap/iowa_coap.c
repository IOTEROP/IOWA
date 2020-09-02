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
* Copyright (c) 2017-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_coap_internals.h"
#include <stdbool.h>

#define PRV_CONN_TYPE(S) ((S) == IOWA_CONN_DATAGRAM ? "Datagram" : \
                         ((S) == IOWA_CONN_STREAM ? "Stream" :     \
                         ((S) == IOWA_CONN_LORAWAN ? "LoRaWAN" :   \
                         ((S) == IOWA_CONN_SMS ? "SMS" :           \
                         "Unknown"))))

uint8_t coapInit(iowa_context_t contextP)
{
    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering.");

    contextP->coapContextP = (coap_context_t)iowa_system_malloc(sizeof(struct _coap_context_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if(contextP->coapContextP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _coap_context_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    memset(contextP->coapContextP, 0, sizeof(struct _coap_context_t));

    IOWA_LOG_TRACE(IOWA_PART_COAP, "CoAP init done.");

    return IOWA_COAP_NO_ERROR;
}

void coapClose(iowa_context_t contextP)
{
    iowa_coap_peer_t *peerP;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering.");

    peerP = contextP->coapContextP->peerList;
    while (peerP != NULL)
    {
        iowa_coap_peer_t *nextPeerP;

        // Keep the next peer address
        nextPeerP = peerP->base.next;

        coapPeerDelete(contextP, peerP);

        peerP = nextPeerP;
    }

    iowa_system_free(contextP->coapContextP);
    contextP->coapContextP = NULL;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "CoAP closed.");
}

uint8_t coapStep(iowa_context_t contextP)
{
    iowa_coap_peer_t *peerP;
    uint8_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Entering currentTime: %u, timeoutP: %u.", contextP->currentTime, contextP->timeout);

    result = IOWA_COAP_NO_ERROR;

    peerP = contextP->coapContextP->peerList;
    while (peerP != NULL
           && result == IOWA_COAP_NO_ERROR)
    {
        iowa_coap_peer_t *peerNextP;

        IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Peer %p.", peerP);

        // Save the next peer since the step function can delete the current peer.
        peerNextP = peerP->base.next;
        switch (peerP->base.type)
        {
#ifdef IOWA_UDP_SUPPORT
        case IOWA_CONN_DATAGRAM:
            result = transactionStep(contextP, (coap_peer_datagram_t *)peerP, contextP->currentTime, &(contextP->timeout));
            break;
#endif

        default:
            IOWA_LOG_ARG_ERROR(IOWA_PART_COAP, "Unsupported connection type: %d.", peerP->base.type);
            result = IOWA_COAP_501_NOT_IMPLEMENTED;
        }

        peerP = peerNextP;
    }

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Exiting with final timeout: %u.", contextP->timeout);

    return result;
}

uint8_t coapSend(iowa_context_t contextP,
                 iowa_coap_peer_t *peerP,
                 iowa_coap_message_t *messageP,
                 coap_message_callback_t resultCallback,
                 void *userData)
{
    // WARNING: This function is called in a critical section
    uint8_t result;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering.");

    if (coapPeerGetConnectionState(peerP) != SECURITY_STATE_CONNECTED)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Peer %p is not connected.", peerP);

        return IOWA_COAP_503_SERVICE_UNAVAILABLE;
    }

    {
        result = peerSend(contextP, peerP, messageP, resultCallback, userData);
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

void messageLog(const char *function,
                unsigned int line,
                const char *info,
                iowa_connection_type_t type,
                iowa_coap_message_t *messageP)
{
    iowa_coap_option_t *optionP;

    iowa_log_arg(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "%s CoAP message:", info);

    switch (type)
    {
    case IOWA_CONN_UNDEFINED:
    case IOWA_CONN_DATAGRAM:
    case IOWA_CONN_LORAWAN:
    case IOWA_CONN_SMS:
        iowa_log_arg(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "Type: %s, Code: %u.%02u (%s), Id: %u",
                     PRV_STR_MESSAGE_TYPE(messageP->type),
                     messageP->code >> 5, messageP->code & 0x1F,
                     PRV_STR_COAP_CODE(messageP->code),
                     messageP->id);
        break;

    case IOWA_CONN_STREAM:
        iowa_log_arg(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "Code: %u.%02u (%s)",
                     messageP->code >> 5, messageP->code & 0x1F,
                     PRV_STR_COAP_CODE(messageP->code));
        break;

    default:
        return;
    }

    iowa_log_buffer(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "Token:", messageP->token, messageP->tokenLength);

    for (optionP = messageP->optionList; optionP != NULL; optionP = optionP->next)
    {
        if (iowa_coap_option_is_integer(optionP))
        {
            iowa_log_arg(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "Option %u (%s), Integer Value: %u", optionP->number, PRV_STR_COAP_OPTION(optionP->number), optionP->value.asInteger);
        }
        else
        {
            iowa_log_arg_buffer(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "Option %u (%s), Buffer:", optionP->value.asBuffer, optionP->length, optionP->number, PRV_STR_COAP_OPTION(optionP->number));
        }
    }

    iowa_log_buffer(IOWA_PART_COAP, IOWA_LOG_LEVEL_INFO, function, line, "Payload:", messageP->payload.data, messageP->payload.length);
}


/************************************************
* Utility functions
*/

iowa_coap_option_t * iowa_coap_message_find_option(iowa_coap_message_t *messageP,
                                                   uint16_t number)
{
    iowa_coap_option_t *optionP;

    optionP = messageP->optionList;

    while (optionP != NULL
           && optionP->number < number)
    {
        optionP = optionP->next;
    }

    if (optionP != NULL
        && optionP->number == number)
    {
        return optionP;
    }

    return NULL;
}

iowa_coap_message_t * iowa_coap_message_new(uint8_t type,
                                            uint8_t code,
                                            uint8_t tokenLength,
                                            uint8_t *token)
{
    iowa_coap_message_t *messageP;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Creating new CoAP message.");

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (tokenLength != 0
        && token == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Token length is not zero but no token was provided.");
        return NULL;
    }
#endif

    messageP = (iowa_coap_message_t *)iowa_system_malloc(sizeof(iowa_coap_message_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (messageP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(iowa_coap_message_t));
        return NULL;
    }
#endif

    memset(messageP, 0, sizeof(iowa_coap_message_t));

    messageP->id = COAP_RESERVED_MID;
    messageP->type = type;
    messageP->code = code;
    if (tokenLength != 0)
    {
        messageP->tokenLength = tokenLength;
        memcpy(messageP->token, token, tokenLength);
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting on success with messageP: %p.", messageP);
    return messageP;
}

void prv_freeBufferList(iowa_linked_buffer_t *bufferP)
{
    iowa_system_free(bufferP->data);
    iowa_system_free(bufferP);
}

void iowa_coap_message_free(iowa_coap_message_t *messageP)
{
    if (messageP != NULL)
    {
        iowa_coap_option_free(messageP->optionList);

        IOWA_UTILS_LIST_FREE(messageP->userBufferList, prv_freeBufferList);
        iowa_system_free(messageP);
    }
}

iowa_coap_message_t * iowa_coap_message_prepare_response(iowa_coap_message_t *messageP,
                                                         uint8_t code)
{
    iowa_coap_message_t *responseP;

    switch (messageP->type)
    {
    case IOWA_COAP_TYPE_CONFIRMABLE:
        responseP = iowa_coap_message_new(IOWA_COAP_TYPE_ACKNOWLEDGEMENT, code, messageP->tokenLength, messageP->token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (responseP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP message.");
            break;
        }
#endif
        responseP->id = messageP->id;
        break;

    case IOWA_COAP_TYPE_NON_CONFIRMABLE:
        responseP = iowa_coap_message_new(IOWA_COAP_TYPE_NON_CONFIRMABLE, code, messageP->tokenLength, messageP->token);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (responseP == NULL)
        {
            IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP message.");
            break;
        }
#endif
        break;

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_COAP, "Message type %u must be IOWA_COAP_TYPE_CONFIRMABLE or IOWA_COAP_TYPE_NON_CONFIRMABLE", messageP->type);
        responseP = NULL;
        break;
    }

    return responseP;
}

void iowa_coap_message_add_option(iowa_coap_message_t *messageP,
                                  iowa_coap_option_t *optionP)
{
    iowa_coap_option_t *finalP;

    if (optionP == NULL)
    {
        return;
    }

    finalP = optionP;
    while (finalP->next != NULL)
    {
        finalP = finalP->next;
    }

    if (messageP->optionList == NULL)
    {
        messageP->optionList = optionP;
    }
    else
    {
        if (messageP->optionList->number > optionP->number)
        {
            finalP->next = messageP->optionList;
            messageP->optionList = optionP;
        }
        else
        {
            iowa_coap_option_t *parentP;

            parentP = messageP->optionList;
            while (parentP->next != NULL
                   && parentP->next->number <= optionP->number)
            {
                parentP = parentP->next;
            }
            finalP->next = parentP->next;
            parentP->next = optionP;
        }
    }
}

void coapSendResponse(iowa_context_t contextP,
                      iowa_coap_peer_t *peerP,
                      iowa_coap_message_t *messageP,
                      uint8_t code)
{
    // WARNING: This function is called in a critical section
    iowa_coap_message_t *responseP;


    if (coapPeerGetConnectionState(peerP) != SECURITY_STATE_CONNECTED)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Peer %p is not connected.", peerP);

        return;
    }

    responseP = iowa_coap_message_prepare_response(messageP, code);

    if (responseP != NULL)
    {
        if (responseP->type == IOWA_COAP_TYPE_ACKNOWLEDGEMENT
            && code == IOWA_COAP_CODE_EMPTY)
        {
            responseP->tokenLength = 0;
        }

        peerSend(contextP, peerP, responseP, NULL, NULL);

        iowa_coap_message_free(responseP);
    }
}

void coapSendReset(iowa_context_t contextP,
                   iowa_coap_peer_t *peerP,
                   iowa_coap_message_t *messageP)
{
    // WARNING: This function is called in a critical section
    iowa_coap_message_t *responseP;


    if (coapPeerGetConnectionState(peerP) != SECURITY_STATE_CONNECTED)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Peer %p is not connected.", peerP);

        return;
    }

    responseP = iowa_coap_message_new(IOWA_COAP_TYPE_RESET, IOWA_COAP_CODE_EMPTY, 0, NULL);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (responseP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Failed to create new CoAP message.");
        return;
    }
#endif

    responseP->id = messageP->id;

    peerSend(contextP, peerP, responseP, NULL, NULL);

    iowa_coap_message_free(responseP);
}

/************************************************
* APIs
*/

iowa_status_t iowa_coap_uri_parse(const char *uri,
                                  iowa_connection_type_t *typeP,
                                  char **hostnameP,
                                  char **portP,
                                  char **pathP,
                                  char **queryP,
                                  bool *isSecureP)
{
    iowa_connection_type_t type;
    bool isSecure;
    size_t uriLen;
    size_t hostOffset;
    size_t portOffset;
    size_t pathOffset;
    size_t queryOffset;
    char *sep;

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "uri: \"%s\", hostnameP: %p, portP: %p, pathP: %p.", uri, hostnameP, portP, pathP);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (uri == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "uri is nil.");
        return IOWA_COAP_400_BAD_REQUEST;
    }
#endif // IOWA_CONFIG_SKIP_ARGS_CHECK

    uriLen = strlen(uri);

    if (0 == strncmp(uri, IOWA_URI_SCHEME_COAP_UDP, strlen(IOWA_URI_SCHEME_COAP_UDP)))
    {
        type = IOWA_CONN_DATAGRAM;
        isSecure = false;
        hostOffset = strlen(IOWA_URI_SCHEME_COAP_UDP);
    }
    else if (0 == strncmp(uri, IOWA_URI_SCHEME_COAP_SEC_UDP, strlen(IOWA_URI_SCHEME_COAP_SEC_UDP)))
    {
        type = IOWA_CONN_DATAGRAM;
        isSecure = true;
        hostOffset = strlen(IOWA_URI_SCHEME_COAP_SEC_UDP);
    }
    else if (0 == strncmp(uri, IOWA_URI_SCHEME_COAP_TCP, strlen(IOWA_URI_SCHEME_COAP_TCP)))
    {
        type = IOWA_CONN_STREAM;
        isSecure = false;
        hostOffset = strlen(IOWA_URI_SCHEME_COAP_TCP);
    }
    else if (0 == strncmp(uri, IOWA_URI_SCHEME_COAP_SEC_TCP, strlen(IOWA_URI_SCHEME_COAP_SEC_TCP)))
    {
        type = IOWA_CONN_STREAM;
        isSecure = true;
        hostOffset = strlen(IOWA_URI_SCHEME_COAP_SEC_TCP);
    }
    else if (0 == strncmp(uri, IOWA_URI_SCHEME_COAP_SMS, strlen(IOWA_URI_SCHEME_COAP_SMS)))
    {
        type = IOWA_CONN_SMS;
        isSecure = false;
        hostOffset = strlen(IOWA_URI_SCHEME_COAP_SMS);
    }
    else if (0 == strncmp(uri, IOWA_URI_SCHEME_COAP_BINARY_SMS, strlen(IOWA_URI_SCHEME_COAP_BINARY_SMS)))
    {
        type = IOWA_CONN_SMS;
        isSecure = false;
        hostOffset = strlen(IOWA_URI_SCHEME_COAP_BINARY_SMS);
    }
    else if (0 == strncmp(uri, IOWA_URI_SCHEME_LORAWAN, strlen(IOWA_URI_SCHEME_LORAWAN)))
    {
        type = IOWA_CONN_LORAWAN;
        isSecure = false;
        hostOffset = strlen(IOWA_URI_SCHEME_LORAWAN);
    }
    else
    {
        IOWA_LOG_INFO(IOWA_PART_COAP, "Unknown URI schema.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (uri[hostOffset] != '[')
    {
        sep = strchr(uri + hostOffset, ':');
        if (sep == NULL)
        {
            portOffset = 0;
        }
        else
        {
            portOffset = sep - uri;
        }
    }
    else
    {
        size_t i;

        i = hostOffset + 1;
        while (uri[i] != 0
                && uri[i] != ']')
        {
            i++;
        }
        if (uri[i] == 0)
        {
            IOWA_LOG_INFO(IOWA_PART_COAP, "Invalid URI format: can not find closing ']'.");
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        i++;
        if (uri[i] == ':')
        {
            portOffset = i;
        }
        else
        {
            portOffset = 0;
        }
    }

    sep = strchr(uri + hostOffset, '/');
    if (sep == NULL)
    {
        pathOffset = 0;
    }
    else
    {
        pathOffset = sep - uri;
        if (pathOffset < portOffset)
        {
            IOWA_LOG_INFO(IOWA_PART_COAP, "Invalid URI format: path contain ':'.");
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
    }
    sep = strchr(uri + hostOffset, '?');
    if (sep == NULL)
    {
        queryOffset = 0;
    }
    else
    {
        queryOffset = sep - uri;
        if (queryOffset < portOffset)
        {
            IOWA_LOG_INFO(IOWA_PART_COAP, "Invalid URI format: query contain ':'.");
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
    }

    if (type == IOWA_CONN_LORAWAN)
    {
        if (portOffset != 0)
        {
            IOWA_LOG_INFO(IOWA_PART_COAP, "Invalid LoRaWAN URI: hostname present.");
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }

        portOffset = hostOffset - 1;
    }
    else
    {
        if (uri[hostOffset] == 0
            || (portOffset != 0 && portOffset <= hostOffset)
            || (pathOffset != 0 && pathOffset <= hostOffset)
            || (queryOffset != 0 && queryOffset <= hostOffset))
        {
            IOWA_LOG_INFO(IOWA_PART_COAP, "Invalid URI format: not hostname.");
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "type: %s, isSecure: %s, hostname offset: %d, port offset: %d, path offset: %d, query offset: %d.", PRV_CONN_TYPE(type), isSecure ? "true" : "false", hostOffset, portOffset, pathOffset, queryOffset);

    if (typeP != NULL)
    {
        *typeP = type;
    }
    if (isSecureP != NULL)
    {
        *isSecureP = isSecure;
    }
    if (hostnameP != NULL)
    {
        if (hostOffset == 0
            || type == IOWA_CONN_LORAWAN)
        {
            *hostnameP = NULL;
        }
        else
        {
            size_t len;

            if (portOffset != 0)
            {
                len = portOffset;
            }
            else if (pathOffset != 0)
            {
                len = pathOffset;
            }
            else if (queryOffset != 0)
            {
                len = queryOffset;
            }
            else
            {
                len = uriLen;
            }

            *hostnameP = utilsBufferToString((const uint8_t *)(uri + hostOffset), len - hostOffset);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*hostnameP == NULL)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }
    }

    if (portP != NULL)
    {
        if (portOffset == 0)
        {
            if (type == IOWA_CONN_DATAGRAM
                || type == IOWA_CONN_STREAM)
            {
                if (isSecure == true)
                {
                    *portP = utilsStrdup(COAP_DEFAULT_PORT_SEC);
                }
                else
                {
                    *portP = utilsStrdup(COAP_DEFAULT_PORT);
                }
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                if (*portP == NULL)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }
#endif
            }
            else
            {
                *portP = NULL;
            }
        }
        else
        {
            size_t len;

            portOffset += 1;

            if (pathOffset != 0)
            {
                len = pathOffset;
            }
            else if (queryOffset != 0)
            {
                len = queryOffset;
            }
            else
            {
                len = uriLen + 1;
            }

            *portP = utilsBufferToString((const uint8_t *)(uri + portOffset), len - portOffset);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*portP == NULL)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }
    }

    if (pathP != NULL)
    {
        if (pathOffset == 0)
        {
            *pathP = NULL;
        }
        else
        {
            size_t len;

            if (queryOffset != 0)
            {
                len = queryOffset;
            }
            else
            {
                len = uriLen + 1;
            }

            *pathP = utilsBufferToString((const uint8_t *)(uri + pathOffset), len - pathOffset);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*pathP == NULL)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }
    }

    if (queryP != NULL)
    {
        if (queryOffset == 0)
        {
            *queryP = NULL;
        }
        else
        {
            queryOffset += 1;

            *queryP = utilsBufferToString((const uint8_t *)(uri + queryOffset), uriLen - queryOffset);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (*queryP == NULL)
            {
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }
    }

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Exiting with no error.");

    return IOWA_COAP_NO_ERROR;
}

size_t iowa_coap_message_get_payload(iowa_coap_message_t *messageP,
                                     iowa_content_format_t *formatP,
                                     uint8_t **payloadP)
{
    if (messageP == NULL)
    {
        return 0;
    }

    if (payloadP != NULL)
    {
        *payloadP = messageP->payload.data;
    }

    if (formatP != NULL)
    {
        iowa_coap_option_t *optionP;

        optionP = iowa_coap_message_find_option(messageP, IOWA_COAP_OPTION_CONTENT_FORMAT);
        if (optionP != NULL)
        {
            *formatP = optionP->value.asInteger;
        }
        else
        {
            *formatP = IOWA_CONTENT_FORMAT_UNSET;
        }
    }

    return messageP->payload.length;
}
