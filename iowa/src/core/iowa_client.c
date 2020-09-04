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

#include "iowa_prv_core_internals.h"
#include "iowa_prv_lwm2m_internals.h"

#ifdef LWM2M_CLIENT_MODE

/*************************************************************************************
** Private functions
*************************************************************************************/

#define SET_RESOURCE_MODE(resource, mode)                               \
{                                                                       \
    if (((mode) & IOWA_OBJECT_MODE_ASYNCHRONOUS) != 0)                  \
    {                                                                   \
        (resource).flags |= (uint8_t) IOWA_RESOURCE_FLAG_ASYNCHRONOUS;  \
    }                                                                   \
    else                                                                \
    {                                                                   \
        (resource).flags &= (uint8_t) ~IOWA_RESOURCE_FLAG_ASYNCHRONOUS; \
    }                                                                   \
}

static iowa_status_t prv_sendHearbeat(iowa_context_t contextP,
                                      lwm2m_server_t *targetP)
{

    iowa_status_t result;

    if (targetP->runtime.peerP == NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Not connected to Server with Short ID %u.", targetP->shortId);
        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    {
        lwm2mUpdateRegistration(contextP, targetP, LWM2M_UPDATE_FLAG_NONE);
        result = IOWA_COAP_NO_ERROR;

        CRIT_SECTION_LEAVE(contextP);
        INTERRUPT_SELECT(contextP);
        CRIT_SECTION_ENTER(contextP);
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}








static iowa_status_t prv_checkCoAPUri(const char *uri,
                                      iowa_security_mode_t securityMode,
                                      lwm2m_binding_t *bindingP,
                                      iowa_connection_type_t *typeP)
{
    iowa_status_t result;
    iowa_connection_type_t type;
    lwm2m_binding_t binding;
    bool isSecured;

    IOWA_LOG_ARG_TRACE(IOWA_PART_COAP, "uri: \"%s\".", uri);

    result = iowa_coap_uri_parse(uri, &type, NULL, NULL, NULL, NULL, &isSecured);
    if (result == IOWA_COAP_NO_ERROR)
    {
        if (securityMode != IOWA_SEC_NONE
            && isSecured == false)
        {
            IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "URI schema (%s) can not be secure if security is disable", uri);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        if (securityMode == IOWA_SEC_NONE
            && isSecured == true)
        {
            IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "URI schema (%s) must be secure if security is enable", uri);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }

        switch (type)
        {
#ifdef IOWA_UDP_SUPPORT
        case IOWA_CONN_DATAGRAM:
            binding = BINDING_U;
            IOWA_LOG_TRACE(IOWA_PART_COAP, "UDP binding.");
            break;
#endif

        default:
            IOWA_LOG_ARG_WARNING(IOWA_PART_LWM2M, "Incorrect URI schema: %s", uri);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }

        if (bindingP != NULL)
        {
            (*bindingP) = binding;
        }
        if (typeP != NULL)
        {
            (*typeP) = type;
        }
    }
    return result;
}

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
static iowa_status_t prv_checkSecurityModeFromParameters(iowa_security_mode_t securityMode)
{

    switch (securityMode)
    {
    case IOWA_SEC_NONE:
#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
    case IOWA_SEC_CERTIFICATE:
#endif
#ifdef IOWA_SECURITY_RAW_PUBLIC_KEY_SUPPORT
    case IOWA_SEC_RAW_PUBLIC_KEY:
#endif
        return IOWA_COAP_NO_ERROR;

    default:
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
}
#endif

/*************************************************************************************
** Internal functions
*************************************************************************************/

void clientNotificationLock(iowa_context_t contextP,
                            bool enter)
{

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Notification lock: %s", (enter==true)?"true":"false");

    if (enter == true)
    {
        contextP->lwm2mContextP->internalFlag |= CONTEXT_FLAG_INSIDE_CALLBACK;
    }
    else
    {
        contextP->lwm2mContextP->internalFlag &= (uint8_t)(~CONTEXT_FLAG_INSIDE_CALLBACK);
    }
}

iowa_status_t clientAddServer(iowa_context_t contextP,
                              lwm2m_server_t *serverP)
{

    iowa_status_t result;
    lwm2m_server_t *nodeP;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering");

    nodeP = contextP->lwm2mContextP->serverList;
    while (nodeP != NULL)
    {
        bool restart;

        restart = false;
        if (serverP->secObjInstId == nodeP->secObjInstId)
        {
            serverP->secObjInstId = (uint16_t)(serverP->secObjInstId + 1);
            restart = true;
        }
        {
            if (serverP->srvObjInstId == nodeP->srvObjInstId)
            {
                serverP->srvObjInstId = (uint16_t)(serverP->srvObjInstId + 1);
                restart = true;
            }
        }

        if (restart == true)
        {
            nodeP = contextP->lwm2mContextP->serverList;
        }
        else
        {
            nodeP = nodeP->next;
        }
    }

    result = objectSecurityCreate(contextP, serverP->secObjInstId);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to add the object Security");
        return result;
    }

    {
        result = objectServerCreate(contextP, serverP->srvObjInstId);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to add the object Server");
            return result;
        }
    }

    return result;
}

iowa_status_t clientRemoveServer(iowa_context_t contextP,
                                 lwm2m_server_t *serverP)
{

    iowa_status_t result;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Entering");

    result = objectSecurityRemove(contextP, serverP->secObjInstId);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to remove the object Security");
        return result;
    }

    {
        result = objectServerRemove(contextP, serverP->srvObjInstId);
        if (result != IOWA_COAP_NO_ERROR
            && result != IOWA_COAP_412_PRECONDITION_FAILED)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to remove the object Server");
            return result;
        }

        lwm2m_server_close(contextP, serverP, true);
    }

    return result;
}

uint32_t clientGetMaxDelayOperation(iowa_context_t contextP)
{

    lwm2m_server_t *serverP;
    uint32_t delay;

    delay = UINT32_MAX;

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Current time is %ds.", contextP->currentTime);

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        if (serverP->runtime.status == STATE_REG_REGISTERED)
        {
            int32_t regDelay;
            lwm2m_observed_t *obsP;

            IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Server %u has a lifetime of %ds.", serverP->shortId, serverP->lifetime);

            regDelay = serverP->runtime.lifetimeTimerP->executionTime - contextP->currentTime;
            if (regDelay <= 0)
            {
                return 0;
            }

            if (delay > (uint32_t)regDelay)
            {
                delay = (uint32_t)regDelay;
                IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Delay set to %ds.", delay);
            }

            for (obsP = serverP->runtime.observedList; obsP != NULL; obsP = obsP->next)
            {
                if (obsP->timeAttrP != NULL
                    && (obsP->timeAttrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0)
                {
                    uint32_t obsDelay;

                    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Observation has a pmax of %ds.", obsP->timeAttrP->maxPeriod);

                    if (obsP->lastTime + obsP->timeAttrP->maxPeriod <= contextP->currentTime)
                    {
                        return 0;
                    }

                    obsDelay = (uint32_t)obsP->lastTime + obsP->timeAttrP->maxPeriod - contextP->currentTime;
                    if (delay > obsDelay)
                    {
                        delay = obsDelay;
                        IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Delay set to %ds.", delay);
                    }
                }
            }
        }
    }

    return delay;
}

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_status_t iowa_client_configure(iowa_context_t contextP,
                                    const char *identity,
                                    iowa_device_info_t *infoP,
                                    iowa_event_callback_t eventCb)
{
    iowa_status_t result;
    const char *msisdn;

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Configuring client.");

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (identity == NULL
        || identity[0] == '\0')
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Identity is nil.");
        return IOWA_COAP_400_BAD_REQUEST;
    }
#endif

    CRIT_SECTION_ENTER(contextP);

    result = objectDeviceInit(contextP, infoP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to initialize the Device Object.");
        return result;
    }

    result = objectServerInit(contextP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to initialize the Server Object.");
        return result;
    }

    result = objectSecurityInit(contextP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to initialize the Security Object.");
        return result;
    }

    if (infoP != NULL)
    {
        msisdn = infoP->msisdn;
        if (msisdn != NULL)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Cannot set the MSISDN without SMS transport enabled.");
            return IOWA_COAP_400_BAD_REQUEST;
        }
    }
    else
    {
        msisdn = NULL;
    }

    result = lwm2m_configure(contextP, identity, msisdn);

    contextP->eventCb = eventCb;
    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Client configured.");

    return result;
}

iowa_status_t iowa_client_add_server(iowa_context_t contextP,
                                     uint16_t shortId,
                                     const char *uri,
                                     uint32_t lifetime,
                                     uint16_t configFlags,
                                     iowa_security_mode_t securityMode)
{
    iowa_status_t result;
    lwm2m_server_t *targetP;
    lwm2m_binding_t binding;
    iowa_connection_type_t type;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Adding LwM2M Server to the client. short ID: %u, URI: \"%s\", lifetime: %us, flags: %X, security mode: %d.", shortId, uri, lifetime, configFlags, securityMode);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID
        || shortId == IOWA_LWM2M_ID_ALL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID must not be equal to 0 nor 65535.");
        return IOWA_COAP_403_FORBIDDEN;
    }
    if (prv_checkSecurityModeFromParameters(securityMode) != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Requested security mode is not supported.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
#endif

    binding = 0;
    result = prv_checkCoAPUri(uri, securityMode, &binding, &type);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "URI has not the right format.");
        return result;
    }
    if ((configFlags & IOWA_LWM2M_QUEUE_MODE) != 0)
    {
        binding |= BINDING_Q;
    }

    CRIT_SECTION_ENTER(contextP);
    for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
    {
        if (targetP->shortId == shortId)
        {
            break;
        }
    }
    if (targetP != NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Short ID %u is already used.", shortId);
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_403_FORBIDDEN;
    }

    targetP = (lwm2m_server_t *)iowa_system_malloc(sizeof(lwm2m_server_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (targetP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR_MALLOC(sizeof(lwm2m_server_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(targetP, 0, sizeof(lwm2m_server_t));

    targetP->uri = utilsStrdup(uri);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (targetP->uri == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to copy the server URI.");

        CRIT_SECTION_LEAVE(contextP);
        iowa_system_free(targetP);

        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    targetP->shortId = shortId;
    if (lifetime == 0)
    {
        {
            targetP->lifetime = LWM2M_DEFAULT_LIFETIME;
        }
    }
    else if (lifetime > INT32_MAX)
    {
        targetP->lifetime = INT32_MAX;
    }
    else
    {
        targetP->lifetime = lifetime;
    }

    targetP->securityMode = securityMode;
    targetP->binding = binding;

    targetP->notifStoring = IOWA_SERVER_RSC_STORING_DEFAULT_VALUE;
#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
    targetP->disableTimeout = IOWA_SERVER_RSC_DISABLE_TIMEOUT_DEFAULT_VALUE;
#endif
#ifndef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
    targetP->defaultPmax = PMAX_UNSET_VALUE;
#endif

#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
    targetP->registrationProcedure.priorityOrder = IOWA_SERVER_ACCOUNT_DEFAULT_PRIORITY_VALUE;
    targetP->registrationProcedure.initialDelayTimer = IOWA_SERVER_ACCOUNT_DEFAULT_INITIAL_DELAY_VALUE;
    targetP->registrationProcedure.blockOnFailure = IOWA_SERVER_ACCOUNT_DEFAULT_REG_FAIL_BLOCK_VALUE;
    targetP->registrationProcedure.bootstrapOnFailure = IOWA_SERVER_ACCOUNT_DEFAULT_BOOTSTRAP_REG_FAIL_VALUE;
#endif
#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
    targetP->registrationProcedure.retryCount = IOWA_SERVER_ACCOUNT_DEFAULT_COMM_RETRY_COUNT_VALUE;
    targetP->registrationProcedure.retryDelayTimer = IOWA_SERVER_ACCOUNT_DEFAULT_COMM_RETRY_TIMER_VALUE;
    targetP->registrationProcedure.sequenceDelayTimer = IOWA_SERVER_ACCOUNT_DEFAULT_COMM_SEQUENCE_DELAY_VALUE;
    targetP->registrationProcedure.sequenceRetryCount = IOWA_SERVER_ACCOUNT_DEFAULT_COMM_SEQUENCE_COUNT_VALUE;
#endif

    targetP->lwm2mVersion = IOWA_LWM2M_VERSION_1_0;

    result = clientAddServer(contextP, targetP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Failed to add the server.");

        CRIT_SECTION_LEAVE(contextP);
        iowa_system_free(targetP->uri);
        iowa_system_free(targetP);

        return result;
    }

    targetP->next = contextP->lwm2mContextP->serverList;
    contextP->lwm2mContextP->serverList = targetP;
    contextP->lwm2mContextP->state = STATE_INITIAL;

    CRIT_SECTION_LEAVE(contextP);
    INTERRUPT_SELECT(contextP);

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Server added to the client.");

    return result;
}

#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
iowa_status_t iowa_client_set_server_registration_behaviour(iowa_context_t contextP,
                                                            uint16_t shortId,
                                                            uint16_t priorityOrder,
                                                            int32_t initialDelayTimer,
                                                            bool blockOnFailure,
                                                            bool bootstrapOnFailure)
{
    lwm2m_server_t *serverP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Setting the registration behaviour for Server ID %u. priorityOrder: %u, initialDelayTimer: %d, blockOnFailure: %d, bootstrapOnFailure: %d.", shortId, priorityOrder, initialDelayTimer, blockOnFailure, bootstrapOnFailure);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID
        || shortId == IOWA_LWM2M_ID_ALL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID must not be equal to 0 nor 65535.");
        return IOWA_COAP_403_FORBIDDEN;
    }
    if (initialDelayTimer < 0)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Initial delay timer cannot be negative: %d.", initialDelayTimer);
        return IOWA_COAP_400_BAD_REQUEST;
    }
    if (bootstrapOnFailure == true)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Bootstrap on failure cannot be true if Bootstrap is not supported.");
        return IOWA_COAP_400_BAD_REQUEST;
    }
#endif

    CRIT_SECTION_ENTER(contextP);

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        if (serverP->shortId == shortId)
        {
            break;
        }
    }
    if (serverP == NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Short ID %u was not found.", shortId);
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_404_NOT_FOUND;
    }

    serverP->registrationProcedure.priorityOrder = priorityOrder;
    serverP->registrationProcedure.initialDelayTimer = initialDelayTimer;
    serverP->registrationProcedure.blockOnFailure = blockOnFailure;
    serverP->registrationProcedure.bootstrapOnFailure = bootstrapOnFailure;

    CRIT_SECTION_LEAVE(contextP);
    INTERRUPT_SELECT(contextP);

    return IOWA_COAP_NO_ERROR;
}
#endif

#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
iowa_status_t iowa_client_set_server_communication_attempts(iowa_context_t contextP,
                                                            uint16_t shortId,
                                                            uint8_t retryCount,
                                                            int32_t retryDelayTimer,
                                                            uint8_t sequenceRetryCount,
                                                            int32_t sequenceDelayTimer)
{
    lwm2m_server_t *serverP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Setting the communication attempts for Server ID %u. retryCount: %u, retryDelayTimer: %u, sequenceRetryCount: %u, sequenceDelayTimer: %u.", shortId, retryCount, retryDelayTimer, sequenceRetryCount, sequenceDelayTimer);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID
        || shortId == IOWA_LWM2M_ID_ALL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID must not be equal to 0 nor 65535.");
        return IOWA_COAP_403_FORBIDDEN;
    }
    if (retryCount > 32)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Retry count is not in the range [0; 32]: %u.", retryCount);
        return IOWA_COAP_400_BAD_REQUEST;
    }
    if (retryDelayTimer < 0)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Retry delay timer cannot be negative: %d.", retryDelayTimer);
        return IOWA_COAP_400_BAD_REQUEST;
    }
    if (sequenceRetryCount > 32)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Sequence retry count is not in the range [0; 32]: %u.", sequenceRetryCount);
        return IOWA_COAP_400_BAD_REQUEST;
    }
    if (sequenceDelayTimer < 0)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Sequence delay timer cannot be negative: %d.", sequenceDelayTimer);
        return IOWA_COAP_400_BAD_REQUEST;
    }
#endif

    CRIT_SECTION_ENTER(contextP);

    for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
    {
        if (serverP->shortId == shortId)
        {
            break;
        }
    }
    if (serverP == NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Short ID %u was not found.", shortId);
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_404_NOT_FOUND;
    }

    serverP->registrationProcedure.retryCount = retryCount;
    serverP->registrationProcedure.retryDelayTimer = retryDelayTimer;
    serverP->registrationProcedure.sequenceRetryCount = sequenceRetryCount;
    serverP->registrationProcedure.sequenceDelayTimer = sequenceDelayTimer;

    CRIT_SECTION_LEAVE(contextP);
    INTERRUPT_SELECT(contextP);

    return IOWA_COAP_NO_ERROR;
}
#endif

iowa_status_t iowa_client_remove_server(iowa_context_t contextP,
                                        uint16_t shortId)
{
    iowa_status_t result;
    lwm2m_server_t *targetP;
    lwm2m_server_t *previousTargetP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Removing server with ID %u from the client.", shortId);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID must not be equal to 0.");
        return IOWA_COAP_403_FORBIDDEN;
    }
#endif

    CRIT_SECTION_ENTER(contextP);
    if (shortId != IOWA_LWM2M_ID_ALL)
    {
        previousTargetP = NULL;
        targetP = contextP->lwm2mContextP->serverList;
        while (targetP != NULL
            && targetP->shortId != shortId)
        {
            previousTargetP = targetP;
            targetP = targetP->next;
        }
        if (targetP == NULL)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Server with Short ID %u not found.", shortId);
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_404_NOT_FOUND;
        }
        if (previousTargetP == NULL)
        {
            contextP->lwm2mContextP->serverList = targetP->next;
        }
        else
        {
            previousTargetP->next = targetP->next;
        }

        result = clientRemoveServer(contextP, targetP);
        utilsFreeServer(contextP, targetP);

    }
    else
    {
        lwm2m_server_t *bootstrapServerP;
        lwm2m_server_t *nextServerP;

        result = IOWA_COAP_NO_ERROR;
        bootstrapServerP = NULL;

        targetP = contextP->lwm2mContextP->serverList;
        while (targetP != NULL)
        {
            nextServerP = targetP->next;
            {
                result = clientRemoveServer(contextP, targetP);
                utilsFreeServer(contextP, targetP);

            }
            targetP = nextServerP;
        }
        contextP->lwm2mContextP->serverList = bootstrapServerP;
    }

    CRIT_SECTION_LEAVE(contextP);
    INTERRUPT_SELECT(contextP);

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Server removed from the client.");

    return result;
}

iowa_status_t iowa_client_set_notification_default_periods(iowa_context_t contextP,
                                                           uint16_t shortId,
                                                           uint32_t minPeriod,
                                                           uint32_t maxPeriod)
{
    lwm2m_server_t *targetP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Server short ID %u, minPeriod: %us, maxPeriod: %us.", shortId, minPeriod, maxPeriod);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID zero is reserved.");
        return IOWA_COAP_403_FORBIDDEN;
    }
#endif

#ifndef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
    CRIT_SECTION_ENTER(contextP);

    if (shortId == IOWA_LWM2M_ID_ALL)
    {
        for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
        {
            targetP->defaultPmin = minPeriod;
            if (maxPeriod < minPeriod)
            {
                targetP->defaultPmax = PMAX_UNSET_VALUE;
            }
            else
            {
                targetP->defaultPmax = maxPeriod;
            }
        }
    }
    else
    {
        for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
        {
            if (targetP->shortId == shortId)
            {
                break;
            }
        }
        if (targetP == NULL)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Server with Short ID %u was not found.", shortId);
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_404_NOT_FOUND;
        }

        targetP->defaultPmin = minPeriod;
        if (maxPeriod < minPeriod)
        {
            targetP->defaultPmax = PMAX_UNSET_VALUE;
        }
        else
        {
            targetP->defaultPmax = maxPeriod;
        }
    }

    CRIT_SECTION_LEAVE(contextP);
#endif

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t iowa_client_use_reliable_notifications(iowa_context_t contextP,
                                                     uint16_t shortId,
                                                     bool enable)
{
    iowa_status_t result;
    lwm2m_server_t *targetP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Server Short ID %u, enable: %s", shortId, enable?"true":"false");

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID must not be equal to zero.");
        return IOWA_COAP_403_FORBIDDEN;
    }
#endif

    result = IOWA_COAP_NO_ERROR;

    CRIT_SECTION_ENTER(contextP);
    if (shortId == IOWA_LWM2M_ID_ALL)
    {
        for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
        {

            targetP->notifStoring = enable;
        }
    }
    else
    {
        for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
        {
            if (targetP->shortId == shortId)
            {
                break;
            }
        }
        if (targetP == NULL)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Server with Short ID %u was not found.", shortId);
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_404_NOT_FOUND;
        }

        targetP->notifStoring = enable;
    }

    CRIT_SECTION_LEAVE(contextP);

    return result;
}

iowa_status_t iowa_client_add_custom_object(iowa_context_t contextP,
                                            uint16_t objectID,
                                            uint16_t instanceCount,
                                            uint16_t * instanceIDs,
                                            uint16_t resourceCount,
                                            iowa_lwm2m_resource_desc_t * resourceArray,
                                            iowa_RWE_callback_t dataCallback,
                                            iowa_CD_callback_t instanceCallback,
                                            iowa_RI_callback_t resInstanceCallback,
                                            void * userData)
{
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    uint16_t i;
#endif
    lwm2m_object_type_t type;
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Object ID : %u, instanceCount : %u, resourceCount : %u", objectID, instanceCount, resourceCount);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    switch (objectID)
    {
    case IOWA_LWM2M_SECURITY_OBJECT_ID:
    case IOWA_LWM2M_SERVER_OBJECT_ID:
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object ID %u is reserved.", objectID);
        return IOWA_COAP_403_FORBIDDEN;
    case IOWA_LWM2M_ID_ALL:
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Object ID 65535 is not acceptable.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    default:
        break;
    }

    if (resourceCount == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Object requires at least one resource.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
    else if (resourceArray == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Empty list of resources.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (dataCallback == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "dataCallback is required.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    if (instanceCount != 0 && instanceIDs == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Empty list of instance IDs.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }

    for (i = 0; i < resourceCount; i++)
    {
        uint16_t j;

        if (resourceArray[i].id == IOWA_LWM2M_ID_ALL)
        {
            IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Resource id 65535 is not acceptable.");
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        if ((resourceArray[i].operations & PRV_RWE_OP_MASK) == 0)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Resource %u has no operation defined.", resourceArray[i].id);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }

        if (IS_RSC_STREAMABLE(resourceArray[i])
            && IS_RSC_ASYNCHRONOUS(resourceArray[i]))
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "A resource cannot be both Asynchronous and Streamable which is the case for resource %u.", resourceArray[i].id);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }

        if (resourceArray[i].type == IOWA_LWM2M_TYPE_UNDEFINED
            && (resourceArray[i].operations & PRV_RWE_OP_MASK) != IOWA_OPERATION_EXECUTE)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Resource %u requires a type or only Execute operation.", resourceArray[i].id);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        if (IS_RSC_MULTIPLE(resourceArray[i])
            && resInstanceCallback == NULL)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Resource %u is defined as multiple but resInstanceCallback is nil.", resourceArray[i].id);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }

        for (j = (uint16_t)(i + 1); j < resourceCount; j++)
        {
            if (resourceArray[i].id == resourceArray[j].id)
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Resource %u is not unique.", resourceArray[i].id);
                return IOWA_COAP_406_NOT_ACCEPTABLE;
            }
        }
    }
#endif

    if (instanceCount == 0
        && instanceCallback == NULL)
    {
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "This is a single instance Object.");
            type = OBJECT_SINGLE;
    }
    else
    {
        type = OBJECT_MULTIPLE;
    }

    CRIT_SECTION_ENTER(contextP);
    result = customObjectAdd(contextP,
                             objectID,
                             type,
                             instanceCount,
                             instanceIDs,
                             resourceCount,
                             resourceArray,
                             dataCallback,
                             instanceCallback,
                             resInstanceCallback,
                             userData);
    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t iowa_client_remove_custom_object(iowa_context_t contextP,
                                               uint16_t objectID)
{
    iowa_status_t result;

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Entering");

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    switch (objectID)
    {
    case IOWA_LWM2M_SECURITY_OBJECT_ID:
    case IOWA_LWM2M_SERVER_OBJECT_ID:
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object ID %u is reserved.", objectID);
        return IOWA_COAP_403_FORBIDDEN;

    case IOWA_LWM2M_ID_ALL:
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Object ID 65535 is not acceptable.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;

    default:
        break;
    }
#endif

    CRIT_SECTION_ENTER(contextP);
    result = customObjectRemove(contextP, objectID);
    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t iowa_client_object_resource_changed(iowa_context_t contextP,
                                                  uint16_t objectID,
                                                  uint16_t instanceID,
                                                  uint16_t resourceID)
{
    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "objectID: %u, instanceID: %u, resourceID: %u", objectID, instanceID, resourceID);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    switch (objectID)
    {
    case IOWA_LWM2M_SECURITY_OBJECT_ID:
    case IOWA_LWM2M_SERVER_OBJECT_ID:
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object ID %u is reserved.", objectID);
        return IOWA_COAP_403_FORBIDDEN;

    case IOWA_LWM2M_ID_ALL:
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Object ID 65535 is not acceptable.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;

    default:
        break;
    }
#endif

    CRIT_SECTION_ENTER(contextP);
    customObjectResourceChanged(contextP, objectID, instanceID, resourceID);
    CRIT_SECTION_LEAVE(contextP);

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t iowa_client_object_instance_changed(iowa_context_t contextP,
                                                  uint16_t objectID,
                                                  uint16_t instanceID,
                                                  iowa_dm_operation_t operation)
{
    iowa_status_t result;

    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Entering");

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    switch (objectID)
    {
    case IOWA_LWM2M_SECURITY_OBJECT_ID:
    case IOWA_LWM2M_SERVER_OBJECT_ID:
    case IOWA_LWM2M_DEVICE_OBJECT_ID:
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Object ID %u is reserved.", objectID);
        return IOWA_COAP_403_FORBIDDEN;

    case IOWA_LWM2M_ID_ALL:
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Object ID 65535 is not acceptable.");
        return IOWA_COAP_406_NOT_ACCEPTABLE;

    default:
        break;
    }
#endif

    CRIT_SECTION_ENTER(contextP);

    switch (operation)
    {
    case IOWA_DM_CREATE:
        result = objectAddInstance(contextP, objectID, instanceID, 0, NULL);
        break;

    case IOWA_DM_DELETE:
        result = objectRemoveInstance(contextP, objectID, instanceID);
        break;

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Invalid operation %d.", operation);
        result = IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

void iowa_client_notification_lock(iowa_context_t contextP,
                                   bool enter)
{
    IOWA_LOG_INFO(IOWA_PART_LWM2M, "Entering");

    CRIT_SECTION_ENTER(contextP);
    clientNotificationLock(contextP, enter);
    CRIT_SECTION_LEAVE(contextP);
}

iowa_status_t iowa_client_send_heartbeat(iowa_context_t contextP,
                                         uint16_t shortId)
{
    iowa_status_t result;
    lwm2m_server_t *targetP;

    IOWA_LOG_ARG_INFO(IOWA_PART_LWM2M, "Sending heartbeat to Server Short ID %u.", shortId);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (shortId == LWM2M_RESERVED_FIRST_ID)
    {
        IOWA_LOG_ERROR(IOWA_PART_LWM2M, "Short ID must not be equal to zero.");
        return IOWA_COAP_403_FORBIDDEN;
    }
#endif

    CRIT_SECTION_ENTER(contextP);

    if (shortId == IOWA_LWM2M_ID_ALL)
    {
        result = IOWA_COAP_412_PRECONDITION_FAILED;

        for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
        {
            {
                iowa_status_t status;

                status = prv_sendHearbeat(contextP, targetP);
                if (result != IOWA_COAP_NO_ERROR)
                {
                    result = status;
                }
            }
        }
    }
    else
    {
        for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
        {
            if (targetP->shortId == shortId)
            {
                break;
            }
        }
        if (targetP == NULL)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ARG_ERROR(IOWA_PART_LWM2M, "Server with Short ID %u not found.", shortId);
            return IOWA_COAP_404_NOT_FOUND;
        }

        result = prv_sendHearbeat(contextP, targetP);
    }

    CRIT_SECTION_LEAVE(contextP);

    return result;
}

#endif
