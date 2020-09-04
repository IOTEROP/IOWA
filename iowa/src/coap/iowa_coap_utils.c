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

void coapUtilsfreeApplicationData(application_coap_data_t *dataP)
{
    if (dataP != NULL)
    {
        if (dataP->messageP != NULL)
        {
            iowa_coap_message_free(dataP->messageP);
        }
        iowa_system_free(dataP);
    }
}

void coapMessageCallback(iowa_coap_peer_t *fromPeer,
                         uint8_t code,
                         iowa_coap_message_t *messageP,
                         void *userData,
                         iowa_context_t contextP)
{
    application_coap_data_t *dataP;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Entering (fromPeer: %p, code: %u.%02u (%s), messageP: %p).", fromPeer, code >> 5, code & 0x1F, PRV_STR_COAP_CODE(code), messageP);

    dataP = (application_coap_data_t *)userData;

    CRIT_SECTION_LEAVE(contextP);

    dataP->messageCallback(fromPeer, code, messageP, dataP->callbackUserData, contextP);

    CRIT_SECTION_ENTER(contextP);

    coapUtilsfreeApplicationData(dataP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "Exiting (fromPeer: %p, code: %u.%02u (%s), messageP: %p).", fromPeer, code >> 5, code & 0x1F, PRV_STR_COAP_CODE(code), messageP);
}
