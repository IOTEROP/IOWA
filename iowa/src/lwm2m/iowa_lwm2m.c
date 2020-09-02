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
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
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

iowa_status_t lwm2m_init(iowa_context_t contextP)
{
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering");

    contextP->lwm2mContextP = (lwm2m_context_t *)iowa_system_malloc(sizeof(lwm2m_context_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (contextP->lwm2mContextP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(lwm2m_context_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(contextP->lwm2mContextP, 0, sizeof(lwm2m_context_t));

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "LwM2M init done");

    return IOWA_COAP_NO_ERROR;
}

void lwm2m_close(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering");

#ifdef LWM2M_CLIENT_MODE
    // Remove all the LwM2M servers
    while (contextP->lwm2mContextP->serverList != NULL)
    {
        lwm2m_server_t *serverP;

        serverP = contextP->lwm2mContextP->serverList;
        {
            lwm2m_server_close(contextP, serverP, false);
        }

        contextP->lwm2mContextP->serverList = serverP->next;
        utilsFreeServer(contextP, serverP);
    }

    while (contextP->lwm2mContextP->objectList != NULL)
    {
        lwm2m_object_t *objectP;

        objectP = contextP->lwm2mContextP->objectList;
        contextP->lwm2mContextP->objectList = contextP->lwm2mContextP->objectList->next;

        customObjectDelete(objectP);
    }

    iowa_system_free(contextP->lwm2mContextP->endpointName);
#endif // LWM2M_CLIENT_MODE

    iowa_system_free(contextP->lwm2mContextP);
    contextP->lwm2mContextP = NULL;

    IOWA_LOG_TRACE(IOWA_PART_BASE, "LwM2M closed");
}

#ifdef LWM2M_CLIENT_MODE

iowa_status_t lwm2m_configure(iowa_context_t contextP,
                              const char *endpointName,
                              const char *msisdn)
{
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "endpointName: \"%s\", msisdn: \"%s\"",
                      endpointName?endpointName:"NULL",
                      msisdn?msisdn:"NULL");

    contextP->lwm2mContextP->endpointName = utilsStrdup(endpointName);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (contextP->lwm2mContextP->endpointName == NULL
        && endpointName != NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Endpoint name copy failed");
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    return IOWA_COAP_NO_ERROR;
}

void lwm2m_server_close(iowa_context_t contextP,
                        lwm2m_server_t *serverP,
                        bool sendDeregistration)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Close the LwM2M Server %d.", serverP->shortId);

    if (sendDeregistration == true)
    {
        registration_deregister(contextP, serverP);
    }

    if (serverP->runtime.updateTimerP != NULL)
    {
        coreTimerDelete(contextP, serverP->runtime.updateTimerP);
        serverP->runtime.updateTimerP = NULL;
    }

    if (serverP->runtime.lifetimeTimerP != NULL)
    {
        coreTimerDelete(contextP, serverP->runtime.lifetimeTimerP);
        serverP->runtime.lifetimeTimerP = NULL;
    }

    utilsDisconnectServer(contextP, serverP);
    attributesRemoveFromServer(serverP);
    observeRemoveFromServer(serverP);
}
#endif // LWM2M_CLIENT_MODE

iowa_status_t lwm2m_step(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with timeout: %u.", contextP->timeout);

    result = IOWA_COAP_NO_ERROR;

#ifdef LWM2M_CLIENT_MODE
    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Initial state: %s.", LWM2M_STR_STATE(contextP->lwm2mContextP->state));

    switch (contextP->lwm2mContextP->state)
    {
    case STATE_INITIAL:
    {

        if (contextP->lwm2mContextP->serverList == NULL)
        {
            // Do nothing else, since there is no server
            break;
        }

        {
            contextP->lwm2mContextP->state = STATE_DEVICE_MANAGEMENT;
        }

        contextP->timeout = 0;
        break;
    }

    case STATE_DEVICE_MANAGEMENT:
        result = registration_step(contextP);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "No longer registered.");
            contextP->lwm2mContextP->state = STATE_INITIAL; // Reset LwM2M FSM

            registration_resetServersStatus(contextP);
            contextP->timeout = 0;
            break;
        }

        observe_step(contextP);
        break;

    default:
        // Should not happen
        break;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Final state: %s.", LWM2M_STR_STATE(contextP->lwm2mContextP->state));
#endif

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with result: %u.%02u, timeout: %u.", (result & 0xFF) >> 5, (result & 0x1F), contextP->timeout);

    return result;
}
