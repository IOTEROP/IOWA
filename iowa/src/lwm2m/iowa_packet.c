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
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
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

/*
Contains code snippets which are:

 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.

*/


#include "iowa_prv_lwm2m_internals.h"

#ifdef LWM2M_CLIENT_MODE
void lwm2m_client_handle_out_of_bound_request(iowa_coap_peer_t *fromPeer,
                                              uint8_t code,
                                              iowa_coap_message_t *requestP,
                                              void *userData,
                                              iowa_context_t contextP)
{
    lwm2m_uri_type_t type;
    iowa_lwm2m_uri_t uri;

    (void)code;
    (void)userData;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering");

    if (requestP->type == IOWA_COAP_TYPE_RESET)
    {
        IOWA_LOG_INFO(IOWA_PART_LWM2M, "Received a CoAP reset");
        goto premature_exit;
    }
    if (requestP->code == IOWA_COAP_CODE_EMPTY)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Received an empty CoAP packet");
        goto premature_exit;
    }

    type = uri_decode(requestP, IOWA_COAP_OPTION_URI_PATH, &uri);
    if (type != LWM2M_URI_TYPE_DM)
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Unhandled URI.");
        goto premature_exit;
    }

    switch (requestP->code)
    {
    case IOWA_COAP_CODE_POST:
        if (uri.objectId == IOWA_LWM2M_SERVER_OBJECT_ID
            && uri.instanceId != IOWA_LWM2M_ID_ALL
            && uri.resourceId == IOWA_LWM2M_SERVER_ID_UPDATE
            && uri.resInstanceId == IOWA_LWM2M_ID_ALL)
        {
            object_execute(contextP, &uri, IOWA_LWM2M_ID_ALL, requestP->payload.data, requestP->payload.length);
        }
        break;

    default:
        break;
    }

premature_exit:
    coapPeerDelete(contextP, fromPeer);
}

void lwm2m_client_handle_request(iowa_coap_peer_t *fromPeer,
                                 uint8_t code,
                                 iowa_coap_message_t *requestP,
                                 void *userData,
                                 iowa_context_t contextP)
{
    lwm2m_server_t *serverP;
    iowa_lwm2m_uri_t uri;
    lwm2m_uri_type_t type;

    (void)code;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering.");

    serverP = (lwm2m_server_t *)userData;

    if (requestP->type == IOWA_COAP_TYPE_RESET)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Received a reset CoAP packet.");
        observe_cancel(contextP, serverP, requestP);
        return;
    }

    if (requestP->code == IOWA_COAP_CODE_EMPTY)
    {
        IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Received an empty CoAP packet.");
        return;
    }

    type = uri_decode(requestP, IOWA_COAP_OPTION_URI_PATH, &uri);

    switch (type)
    {
    case LWM2M_URI_TYPE_DM:
        {
            dm_handleRequest(contextP, &uri, serverP, requestP);
        }
        break;

    default:
        coapSendResponse(contextP, fromPeer, requestP, IOWA_COAP_400_BAD_REQUEST);
    }
}
#endif

