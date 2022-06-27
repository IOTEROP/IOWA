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
* Copyright (c) 2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_security_internals.h"

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_security_state_t iowa_security_session_get_state(iowa_security_session_t securityS)
{
    return securityS->state;
}

#ifdef LWM2M_CLIENT_MODE
iowa_security_session_t iowa_security_get_server_session(iowa_context_t contextP,
                                                         uint16_t shortServerId)
{
    iowa_security_session_t securityS;
    lwm2m_server_t *serverP;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    CRIT_SECTION_ENTER(contextP);

    serverP = (lwm2m_server_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->serverList, listFindCallbackBy16bitsId, &shortServerId);
    if (serverP == NULL)
    {
        securityS = NULL;
    }
    else
    {
        securityS = coapPeerGetSecuritySession(serverP->runtime.peerP);
    }

    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with securityS: %p.", securityS);

    return securityS;
}

iowa_security_mode_t iowa_security_session_get_security_mode(iowa_security_session_t securityS)
{
    return securityS->securityMode;
}

#endif // LWM2M_CLIENT_MODE

#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER

void *iowa_security_session_get_user_internals(iowa_security_session_t securityS)
{
    return securityS->userInternalsP;
}

void iowa_security_session_set_user_internals(iowa_security_session_t securityS,
                                              void *internalsP)
{
    securityS->userInternalsP = internalsP;
}

void iowa_security_session_set_state(iowa_security_session_t securityS,
                                     iowa_security_state_t state)
{
    securityS->state = state;
}

void iowa_security_session_generate_event(iowa_security_session_t securityS,
                                          iowa_security_event_t event)
{
    SESSION_CALL_EVENT_CALLBACK(securityS, event);
}

iowa_connection_type_t iowa_security_session_get_connection_type(iowa_security_session_t securityS)
{
    return securityS->type;
}

void * iowa_security_session_get_connection(iowa_security_session_t securityS)
{
    if (securityS->channelP != NULL)
    {
        return securityS->channelP->connP;
    }

    return NULL;
}

const char *iowa_security_session_get_uri(iowa_security_session_t securityS)
{
    return securityS->uri;
}

void iowa_security_session_set_step_delay(iowa_security_session_t securityS,
                                          int32_t delay)
{
    if (delay >= 0
        && delay < securityS->contextP->timeout)
    {
        securityS->contextP->timeout = delay;
    }
}

iowa_context_t iowa_security_session_get_context(iowa_security_session_t securityS)
{
    return securityS->contextP;
}

void *iowa_security_session_get_context_user_data(iowa_security_session_t securityS)
{
    return securityS->contextP->userData;
}

int iowa_security_connection_send(iowa_security_session_t securityS,
                                  uint8_t *buffer,
                                  size_t length)
{
    if (NULL == securityS->channelP)
    {
        return -1;
    }

    return commSend(securityS->contextP, securityS->channelP, buffer, length);
}

int iowa_security_connection_recv(iowa_security_session_t securityS,
                                  uint8_t *buffer,
                                  size_t length)
{
    if (NULL == securityS->channelP)
    {
        return -1;
    }

    return commRecv(securityS->contextP, securityS->channelP, buffer, length);
}

#endif // IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_USER
