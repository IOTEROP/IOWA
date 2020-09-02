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

    securityS = NULL;

    CRIT_SECTION_ENTER(contextP);

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        if (serverP->shortId == shortServerId)
        {
            securityS = coapPeerGetSecuritySession(serverP->runtime.peerP);
            break;
        }
    }

    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with securityS: %p.", securityS);

    return securityS;
}
#endif // LWM2M_CLIENT_MODE

