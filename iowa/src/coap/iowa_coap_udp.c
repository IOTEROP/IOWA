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
* Copyright (c) 2016-2019 IoTerop.
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

#ifdef IOWA_UDP_SUPPORT

uint8_t messageSendUDP(iowa_context_t contextP,
                       iowa_coap_peer_t *peerBaseP,
                       iowa_coap_message_t *messageP,
                       coap_message_callback_t resultCallback,
                       void *userData)
{
    // WARNING: This function is called in a critical section
    coap_peer_datagram_t *peerP;
    size_t bufferLength;
    uint8_t *buffer;
    uint8_t result;
    int nbSent;

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Entering");

    peerP = (coap_peer_datagram_t *)peerBaseP;

    bufferLength = coapMessageSerializeDatagram(messageP, &buffer);
    if (bufferLength == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_COAP, "Exit on error: serialization failed.");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    nbSent = peerSendBuffer(contextP, (iowa_coap_peer_t *)peerP, buffer, bufferLength);
    if (nbSent < 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Communication error: %d.", nbSent);
        result = IOWA_COAP_503_SERVICE_UNAVAILABLE;
    }
    else if ((size_t)nbSent < bufferLength)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Need to send in blocks, %u bytes to send but connection layer returned %d.", bufferLength, nbSent);
        result = IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
    }
    else
    {
        result = IOWA_COAP_NO_ERROR;
    }

    if (result == IOWA_COAP_NO_ERROR)
    {
        result = transactionNew(contextP, peerP, messageP, buffer, bufferLength, resultCallback, userData);
        if (result == IOWA_COAP_201_CREATED)
        {
            buffer = NULL;
            result = IOWA_COAP_NO_ERROR;
        }
    }

    iowa_system_free(buffer);

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

void udpSecurityEventCb(iowa_security_session_t securityS,
                        iowa_security_event_t event,
                        void *userData,
                        iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section

    coap_peer_datagram_t *peerP;

    (void)securityS;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "PeerP: %p, event: %s.", userData, STR_SECURITY_EVENT(event));

    peerP = (coap_peer_datagram_t *)userData;

    switch (event)
    {
    case SECURITY_EVENT_CONNECTED:
        // TODO: (in case of security) send any buffered message
        // Propagate the signal to the upper layer
        PEER_CALL_EVENT_CALLBACK(contextP, peerP, COAP_EVENT_CONNECTED);
        break;

    case SECURITY_EVENT_DISCONNECTED:
        // Propagate the signal to the upper layer
        PEER_CALL_EVENT_CALLBACK(contextP, peerP, COAP_EVENT_DISCONNECTED);
        break;

    case SECURITY_EVENT_DATA_AVAILABLE:
    {
        static uint8_t buffer[IOWA_BUFFER_SIZE];
        int bufferLength;

        bufferLength = peerRecvBuffer(contextP, (iowa_coap_peer_t *)peerP, buffer, IOWA_BUFFER_SIZE);
        if (bufferLength < 0)
        {
            PEER_CALL_EVENT_CALLBACK(contextP, peerP, COAP_EVENT_DISCONNECTED);
            // Warning: do not use peerP after the callback since the peer can have been deleted
            break;
        }
        if (bufferLength > 0)
        {
            iowa_coap_message_t *messageP;
            uint8_t result;
            size_t maxPayloadSize;
            bool truncated;

            maxPayloadSize = 0;

            result = messageDatagramParse(buffer, (size_t)bufferLength, &messageP);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Message parsing failed with error %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
                // ignore message
                return;
            }

            if (bufferLength >= IOWA_BUFFER_SIZE)
            {
                IOWA_LOG_ARG_WARNING(IOWA_PART_COAP, "Received a message of %u bytes while IOWA_BUFFER_SIZE is %u. Payload was truncated.", bufferLength, IOWA_BUFFER_SIZE);

                maxPayloadSize = messageP->payload.length;
                truncated = true;
            }
            else
            {
                truncated = false;
            }

#ifndef IOWA_COAP_BLOCK_MINIMAL_SUPPORT
            if (iowa_coap_message_find_option(messageP, IOWA_COAP_OPTION_BLOCK_1) != NULL
                || iowa_coap_message_find_option(messageP, IOWA_COAP_OPTION_BLOCK_2) != NULL)
            {
                IOWA_LOG_WARNING(IOWA_PART_COAP, "Received message containing Block option but IOWA_COAP_BLOCK_MINIMAL_SUPPORT is not defined.");

                coapSendResponse(contextP, (iowa_coap_peer_t *)peerP, messageP, IOWA_COAP_402_BAD_OPTION);
                iowa_coap_message_free(messageP);
                return;
            }
#endif

            transactionHandleMessage(contextP, peerP, messageP, truncated, maxPayloadSize);

            iowa_coap_message_free(messageP);
            return;
        }
    }
    break;

    default:
        // Should not happen
        break;
    }
}

#endif // IOWA_UDP_SUPPORT
