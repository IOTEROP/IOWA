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


static coap_ack_t *prv_acknowledgeFind(coap_peer_datagram_t *peerP,
                                       iowa_coap_message_t *messageP)
{
    coap_ack_t *ackP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "peerP: %p, message ID: %u.", peerP, messageP->id);

    ackP = peerP->ackList;
    while (ackP != NULL)
    {
        if (ackP->mID == messageP->id)
        {
            IOWA_LOG_TRACE(IOWA_PART_COAP, "Found acknowledge.");
            return ackP;
        }
        ackP = ackP->next;
    }

    IOWA_LOG_TRACE(IOWA_PART_COAP, "Acknowledge not found.");
    return NULL;
}

static coap_transaction_t *prv_transactionFind(coap_peer_datagram_t *peerP,
                                               iowa_coap_message_t *messageP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "message ID: %u.", messageP->id);

    if (messageP->type == IOWA_COAP_TYPE_ACKNOWLEDGEMENT
        || messageP->type == IOWA_COAP_TYPE_RESET)
    {
        coap_transaction_t *transacP;

        transacP = peerP->transactionList;

        while (transacP != NULL)
        {
            if (transacP->mID == messageP->id)
            {
                IOWA_LOG_TRACE(IOWA_PART_COAP, "Transaction found.");
                return transacP;
            }
            transacP = transacP->next;
        }
    }

    IOWA_LOG_TRACE(IOWA_PART_COAP, "No transaction found.");
    return NULL;
}

void transactionFree(coap_transaction_t *transacP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Freeing transaction %p.", transacP);

    iowa_system_free(transacP->buffer);
    iowa_system_free(transacP);
}

void acknowledgeFree(coap_ack_t *ackP)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Freeing acknowledge for message ID %u.", ackP->mID);

    iowa_system_free(ackP->buffer);
    iowa_system_free(ackP);
}

uint8_t transactionNew(iowa_context_t contextP,
                       coap_peer_datagram_t *peerP,
                       iowa_coap_message_t *messageP,
                       uint8_t *buffer,
                       size_t bufferLength,
                       coap_message_callback_t resultCallback,
                       void *userData)
{

    switch (messageP->type)
    {
    case IOWA_COAP_TYPE_CONFIRMABLE:
    {
        coap_transaction_t *transacP;
        int32_t curTime;

        curTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (curTime < 0)
        {
            IOWA_LOG_ERROR_GETTIME(curTime);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        transacP = (coap_transaction_t *)iowa_system_malloc(sizeof(coap_transaction_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (transacP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(coap_transaction_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memset(transacP, 0, sizeof(coap_transaction_t));

        transacP->mID = messageP->id;
        transacP->retrans_counter = 0;
        transacP->retrans_time = curTime + peerP->ackTimeout;
        transacP->buffer_len = bufferLength;
        transacP->buffer = buffer;
        transacP->callback = resultCallback;
        transacP->userData = userData;

        peerP->transactionList = (coap_transaction_t *)IOWA_UTILS_LIST_ADD(peerP->transactionList, transacP);

        if (peerP->ackTimeout > 0
            && contextP->timeout > peerP->ackTimeout)
        {
            contextP->timeout = peerP->ackTimeout;
            CRIT_SECTION_LEAVE(contextP);
            INTERRUPT_SELECT(contextP);
            CRIT_SECTION_ENTER(contextP);
        }

        return IOWA_COAP_201_CREATED;
    }

#if defined(IOWA_UDP_SUPPORT) || defined(IOWA_SMS_SUPPORT)
    case IOWA_COAP_TYPE_ACKNOWLEDGEMENT:
    case IOWA_COAP_TYPE_RESET:
        if (peerP->ackTimeout != 0)
        {
            coap_ack_t *ackP;
            int32_t curTime;

            curTime = iowa_system_gettime();
    #ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (curTime < 0)
            {
                IOWA_LOG_ERROR_GETTIME(curTime);
                break;
            }
    #endif
            ackP = (coap_ack_t *)iowa_system_malloc(sizeof(coap_ack_t));
    #ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (ackP == NULL)
            {
                IOWA_LOG_ERROR_MALLOC(sizeof(coap_ack_t));
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
    #endif
            memset(ackP, 0, sizeof(coap_ack_t));

            ackP->buffer_len = bufferLength;
            ackP->buffer = buffer;
            ackP->mID = messageP->id;
            ackP->validity_time = curTime + peerP->transmitWait;

            peerP->ackList = (coap_ack_t *)IOWA_UTILS_LIST_ADD(peerP->ackList, ackP);

            return IOWA_COAP_201_CREATED;
        }
    break;
#endif

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

uint8_t transactionStep(iowa_context_t contextP,
                        coap_peer_datagram_t *peerP,
                        int32_t currentTime,
                        int32_t *timeoutP)
{


    coap_ack_t *ackP;
    coap_ack_t *parentAckP;
    coap_transaction_t *transacP;

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Entering peer %p, currentTime: %u, timeoutP: %u", peerP, currentTime, *timeoutP);

    ackP = peerP->ackList;
    parentAckP = NULL;
    while (ackP != NULL)
    {
        coap_ack_t *nextP;

        nextP = ackP->next;

        IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Ack ID %u validity time: %u", ackP->mID, ackP->validity_time);

        if (ackP->validity_time <= currentTime)
        {
            IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Removing cached reply for message %u.", ackP->mID);
            if (parentAckP == NULL)
            {
                peerP->ackList = ackP->next;
            }
            else
            {
                parentAckP->next = ackP->next;
            }

            acknowledgeFree(ackP);
            ackP = NULL;
        }
        else
        {
            parentAckP = ackP;
        }
        ackP = nextP;
    }

    transacP = peerP->transactionList;
    while (transacP != NULL)
    {
        coap_transaction_t *nextP;

        nextP = transacP->next;

        IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Transaction %u: retrans counter %u, retrans time %u.", transacP->mID, transacP->retrans_counter, transacP->retrans_time);

        if (transacP->retrans_time <= currentTime)
        {
            if (transacP->retrans_counter < peerP->maxRetransmit)
            {
                int32_t timeout;

                IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "Resending transaction %u.", transacP->mID);

                (void)peerSendBuffer(contextP, (iowa_coap_peer_t *)peerP, transacP->buffer, transacP->buffer_len);

                transacP->retrans_counter++;

                timeout = peerP->ackTimeout << transacP->retrans_counter;

                transacP->retrans_time = currentTime + timeout;
                if (*timeoutP > timeout)
                {
                    *timeoutP = timeout;
                }
            }
            else
            {
                peerP->transactionList = (coap_transaction_t *)IOWA_UTILS_LIST_REMOVE(peerP->transactionList, transacP);
                if (transacP->callback != NULL)
                {
                    transacP->callback((iowa_coap_peer_t *)peerP, IOWA_COAP_503_SERVICE_UNAVAILABLE, NULL, transacP->userData, contextP);
                }
                transactionFree(transacP);
            }
        }
        else
        {
            if (*timeoutP > (transacP->retrans_time - currentTime))
            {
                *timeoutP = transacP->retrans_time - currentTime;
            }
        }

        transacP = nextP;
    }

    return IOWA_COAP_NO_ERROR;
}

void transactionHandleMessage(iowa_context_t contextP,
                              coap_peer_datagram_t *peerP,
                              iowa_coap_message_t *messageP,
                              bool truncated,
                              size_t maxPayloadSize)
{

    coap_transaction_t *transacP;

    IOWA_LOG_ARG_INFO(IOWA_PART_COAP, "peerP: %p, truncated: %s, maxPayloadSize: %u.", peerP, truncated ? "true" : "false", maxPayloadSize);

    switch (messageP->type)
    {
    case IOWA_COAP_TYPE_CONFIRMABLE:
    {
        coap_ack_t *ackP;

        ackP = prv_acknowledgeFind(peerP, messageP);

        if (ackP != NULL)
        {
            (void)peerSendBuffer(contextP, (iowa_coap_peer_t *)peerP, ackP->buffer, ackP->buffer_len);
        }
        else
        {
            if (!COAP_IS_REQUEST(messageP->code))
            {
                coapSendResponse(contextP, (iowa_coap_peer_t *)peerP, messageP, IOWA_COAP_CODE_EMPTY);
            }
            peerHandleMessage(contextP, (iowa_coap_peer_t *)peerP, messageP, truncated, maxPayloadSize);
        }
    }
    break;

    case IOWA_COAP_TYPE_NON_CONFIRMABLE:
        peerHandleMessage(contextP, (iowa_coap_peer_t *)peerP, messageP, truncated, maxPayloadSize);
        break;

    case IOWA_COAP_TYPE_ACKNOWLEDGEMENT:
        transacP = prv_transactionFind(peerP, messageP);
        if (transacP != NULL)
        {
            peerP->transactionList = (coap_transaction_t *)IOWA_UTILS_LIST_REMOVE(peerP->transactionList, transacP);
            if (transacP->callback != NULL)
            {
                uint8_t code;

                if (truncated == true)
                {
                    code = IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE;
                }
                else
                {
                    code = messageP->code;
                }
                transacP->callback((iowa_coap_peer_t *)peerP, code, messageP, transacP->userData, contextP);
            }
            transactionFree(transacP);
            peerHandleMessage(contextP, (iowa_coap_peer_t *)peerP, messageP, truncated, maxPayloadSize);
        }
        break;

    case IOWA_COAP_TYPE_RESET:
        transacP = prv_transactionFind(peerP, messageP);
        if (transacP != NULL)
        {
            peerP->transactionList = (coap_transaction_t *)IOWA_UTILS_LIST_REMOVE(peerP->transactionList, transacP);
            if (transacP->callback != NULL)
            {
                transacP->callback((iowa_coap_peer_t *)peerP, messageP->code, messageP, transacP->userData, contextP);
            }
            transactionFree(transacP);
        }
        else
        {
            peerHandleMessage(contextP, (iowa_coap_peer_t *)peerP, messageP, truncated, maxPayloadSize);
        }
        break;

    default:
        break;
    }
}
