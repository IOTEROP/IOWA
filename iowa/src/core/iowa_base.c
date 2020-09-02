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
*
**********************************************/

#include "iowa_prv_core_internals.h"
#include "iowa_prv_lwm2m_internals.h"

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_context_t iowa_init(void *userData)
{
    iowa_context_t contextP;

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "---------- IOWA configuration ----------");
    IOWA_LOG_ARG_INFO(IOWA_PART_SYSTEM, "IOWA %s, built %s (%s).", IOWA_VERSION, __DATE__, __TIME__);
    IOWA_LOG_ARG_INFO(IOWA_PART_SYSTEM, "LwM2M version: %d.%d.%d", LWM2M_MAJOR_VERSION, LWM2M_MINOR_VERSION, LWM2M_REVISION_VERSION);

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LwM2M Bootstrap Server mode: No.");

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LwM2M Server mode: No.");
#ifdef LWM2M_CLIENT_MODE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LwM2M Client mode: Yes.");
#else
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LwM2M Client mode: No.");
#endif

    IOWA_LOG_ARG_INFO(IOWA_PART_SYSTEM, "Buffer size: %d", IOWA_BUFFER_SIZE);

#ifdef LWM2M_BIG_ENDIAN
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Endianness: Big endian.");
#else
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Endianness: Little endian.");
#endif

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Security enabled: No.");

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Storage queue support: No.");

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Thread support: No.");

#ifdef LWM2M_CLIENT_MODE

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Client Bootstrap support: No.");

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Client incoming connection support: No.");

#endif

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Data push support: No.");

#ifdef IOWA_UDP_SUPPORT
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "UDP transport support: Yes.");
#else
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "UDP transport support: No.");
#endif
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "TCP transport support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LoRaWAN transport support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "SMS transport support: No.");

#ifdef LWM2M_SUPPORT_TLV
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "TLV data format support: Yes.");
#else
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "TLV data format support: No.");
#endif
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "JSON data format support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "CBOR data format support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "SenML CBOR data format support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "SenML JSON data format support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Read composite support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Observe composite support: No.");
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "Write composite support: No.");

    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "----------------------------------------");


    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "userData: %p.", userData);

    contextP = (iowa_context_t)iowa_system_malloc(sizeof(struct _iowa_context_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (contextP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(struct _iowa_context_t));
        return NULL;
    }
#endif
    memset(contextP, 0, sizeof(struct _iowa_context_t));

    contextP->userData = userData;

    if (IOWA_COAP_NO_ERROR != commInit(contextP))
    {
        IOWA_LOG_ERROR(IOWA_PART_BASE, "Comm layer initialization failed.");
        goto error;
    }

    if (IOWA_COAP_NO_ERROR != securityInit(contextP))
    {
        IOWA_LOG_ERROR(IOWA_PART_BASE, "Security layer initialization failed.");
        goto error;
    }

    if (IOWA_COAP_NO_ERROR != coapInit(contextP))
    {
        IOWA_LOG_ERROR(IOWA_PART_BASE, "CoAP layer initialization failed.");
        goto error;
    }

#if defined(LWM2M_CLIENT_MODE) || defined(LWM2M_SERVER_MODE) || defined(LWM2M_BOOTSTRAP_SERVER_MODE)
    if (IOWA_COAP_NO_ERROR != lwm2m_init(contextP))
    {
        IOWA_LOG_ERROR(IOWA_PART_BASE, "LwM2M layer init failed");
        goto error;
    }
#endif

    IOWA_LOG_INFO(IOWA_PART_BASE, "IOWA init done");

    return contextP;

error:
    if (contextP->commContextP != NULL)
    {
        commClose(contextP);
    }
    if (contextP->securityContextP != NULL)
    {
        securityClose(contextP);
    }
    if (contextP->coapContextP != NULL)
    {
        coapClose(contextP);
    }
    iowa_system_free(contextP);

    return NULL;
}

void iowa_close(iowa_context_t contextP)
{
    IOWA_LOG_INFO(IOWA_PART_BASE, "Closing IOWA");

    CRIT_SECTION_ENTER(contextP);

#ifdef LWM2M_CLIENT_MODE
    // Prevent the LwM2M stack from updating the registration
    contextP->lwm2mContextP->state = STATE_INITIAL;

    objectDeviceClose(contextP);
    objectSecurityClose(contextP);
    objectServerClose(contextP);
#endif // LWM2M_CLIENT_MODE

#if defined(LWM2M_CLIENT_MODE) || defined(LWM2M_SERVER_MODE) || defined(LWM2M_BOOTSTRAP_SERVER_MODE)
    lwm2m_close(contextP);
#endif

    if (contextP->coapContextP != NULL)
    {
        coapClose(contextP);
    }
    if (contextP->securityContextP != NULL)
    {
        securityClose(contextP);
    }
    if (contextP->commContextP != NULL)
    {
        commClose(contextP);
    }

    coreTimerClose(contextP);

    CRIT_SECTION_LEAVE(contextP);

    iowa_system_free(contextP);

    IOWA_LOG_INFO(IOWA_PART_BASE, "IOWA closed");
}

iowa_status_t iowa_step(iowa_context_t contextP,
                        int32_t timeout)
{
    iowa_status_t status;
    int32_t startTime;
    int32_t remainingTime;

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "timeout: %d.", timeout);

    if (timeout > 0)
    {
        startTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (startTime < 0)
        {
            IOWA_LOG_ERROR_GETTIME(startTime);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
    }
    else
    {
        startTime = 0;
    }

    do
    {
        int32_t currentTime;

        CRIT_SECTION_ENTER(contextP);
        if (timeout < 0)
        {
            IOWA_LOG_WARNING(IOWA_PART_BASE, "WARNING: IOWA_THREAD_SUPPORT is not defined and an \"infinite\" timeout is set.");
            contextP->timeout = INT32_MAX;
        }
        else
        {
            contextP->timeout = timeout;
        }

        if ((contextP->action & ACTION_EXIT) == ACTION_EXIT)
        {
            contextP->action &= (uint16_t)~ACTION_EXIT;
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_NO_ERROR;
        }
#ifdef LWM2M_CLIENT_MODE
        if ((contextP->action & ACTION_REBOOT) == ACTION_REBOOT)
        {
            contextP->action &= (uint16_t)~ACTION_REBOOT;
            CRIT_SECTION_LEAVE(contextP);
            iowa_system_reboot(contextP->userData);
            CRIT_SECTION_ENTER(contextP);
        }
#ifndef IOWA_DEVICE_RSC_FACTORY_RESET_REMOVE
        if ((contextP->action & ACTION_FACTORY_RESET) == ACTION_FACTORY_RESET)
        {
            contextP->action &= (uint16_t)~ACTION_FACTORY_RESET;
            objectDeviceFactoryReset(contextP);
        }
#endif

#endif // LWM2M_CLIENT_MODE

        CRIT_SECTION_LEAVE(contextP);

        currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (currentTime < 0 || currentTime < startTime)
        {
            IOWA_LOG_ERROR_GETTIME(currentTime);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        CRIT_SECTION_ENTER(contextP);

        contextP->currentTime = currentTime;

        status = securityStep(contextP);
        if (status != IOWA_COAP_NO_ERROR)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "An error occurred during the Security step routine: %u.%02u.", (status & 0xFF) >> 5, (status & 0x1F));
            return status;
        }

        status = coapStep(contextP);
        if (status != IOWA_COAP_NO_ERROR)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "An error occurred during the CoAP step routine: %u.%02u.", (status & 0xFF) >> 5, (status & 0x1F));
            return status;
        }

        status = lwm2m_step(contextP);
        if (status != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "An error occurred during the LwM2M step routine: %u.%02u.", (status & 0xFF) >> 5, (status & 0x1F));
            CRIT_SECTION_LEAVE(contextP);
            return status;
        }

        coreTimerStep(contextP);

        status = commSelect(contextP);
        CRIT_SECTION_LEAVE(contextP);

        if (status != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "An error occurred during the Comm Select routine: %u.%02u.", (status & 0xFF) >> 5, (status & 0x1F));
            return status;
        }

        if (timeout > 0)
        {
            currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (currentTime < 0 || currentTime < startTime)
            {
                IOWA_LOG_ERROR_GETTIME(currentTime);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
            remainingTime = timeout - (currentTime - startTime);
        }
        else if (timeout == 0)
        {
            // immediate
            remainingTime = 0;
        }
        else
        {
            // infinite
            remainingTime = 1;
        }
    } while (remainingTime > 0);

    IOWA_LOG_TRACE(IOWA_PART_BASE, "Exiting");

    return status;
}

void iowa_stop(iowa_context_t contextP)
{
    IOWA_LOG_INFO(IOWA_PART_BASE, "Stopping IOWA");

    CRIT_SECTION_ENTER(contextP);
    contextP->action |= ACTION_EXIT;
    CRIT_SECTION_LEAVE(contextP);

    INTERRUPT_SELECT(contextP);

    IOWA_LOG_INFO(IOWA_PART_BASE, "IOWA stopped");
}

iowa_status_t iowa_flush_before_pause(iowa_context_t contextP,
                                      int32_t duration,
                                      uint32_t *delayP)
{
    iowa_status_t result;
    int32_t currentTime;
#ifdef LWM2M_CLIENT_MODE
    lwm2m_server_t *targetP;
#endif

    IOWA_LOG_ARG_INFO(IOWA_PART_BASE, "Flushing IOWA with announced duration: %ds.", duration);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (duration < 0)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "Duration is negative: %d.", duration);
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
#endif

#if defined(LWM2M_CLIENT_MODE)
    if (contextP->lwm2mContextP->serverList == NULL)
    {
        IOWA_LOG_INFO(IOWA_PART_BASE, "No server configured");
        return IOWA_COAP_NO_ERROR;
    }
#endif

    // Add some time for resuming
    duration += PAUSE_TIME_BUFFER;
    if (duration < PAUSE_TIME_BUFFER)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "Integer overflow for duration. Using %d.", INT32_MAX);
        duration = INT32_MAX;
    }

    CRIT_SECTION_ENTER(contextP);

#ifdef LWM2M_CLIENT_MODE
    // Check the duration is inferior to all registered servers lifetime
    for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
    {
        if (targetP->runtime.status == STATE_REG_REGISTERED)
        {
            int32_t maxTxWait;

            maxTxWait = coapPeerGetMaxTxWait(targetP->runtime.peerP);

            if (targetP->lifetime <= maxTxWait
                || duration >= targetP->lifetime - maxTxWait)
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "Server %u has a lifetime of %ds.", targetP->shortId, targetP->lifetime);
                CRIT_SECTION_LEAVE(contextP);
                return IOWA_COAP_406_NOT_ACCEPTABLE;
            }
        }
    }
#endif

    CRIT_SECTION_LEAVE(contextP);
    currentTime = iowa_system_gettime();
    CRIT_SECTION_ENTER(contextP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (currentTime < 0)
    {
        IOWA_LOG_ERROR_GETTIME(currentTime);
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    contextP->currentTime = currentTime;

#ifdef LWM2M_CLIENT_MODE
    // Check if we need to send a registration update before pausing
    for (targetP = contextP->lwm2mContextP->serverList; targetP != NULL; targetP = targetP->next)
    {
        if (targetP->runtime.status == STATE_REG_REGISTERED)
        {
            if (duration >= targetP->runtime.lifetimeTimerP->executionTime - contextP->currentTime - coapPeerGetMaxTxWait(targetP->runtime.peerP))
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "Server %u requires a registration update.", targetP->shortId);
                lwm2mUpdateRegistration(contextP, targetP, LWM2M_UPDATE_FLAG_NONE);
            }
        }
    }
#endif

    do
    {
        CRIT_SECTION_LEAVE(contextP);
        currentTime = iowa_system_gettime();
        CRIT_SECTION_ENTER(contextP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (currentTime < 0)
        {
            IOWA_LOG_ERROR_GETTIME(currentTime);
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif

        contextP->timeout = duration;
        contextP->currentTime = currentTime;

        result = coapStep(contextP);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_BASE, "coap_step() failed.");
            CRIT_SECTION_LEAVE(contextP);
            return result;
        }
        IOWA_LOG_ARG_INFO(IOWA_PART_BASE, "CoAP timeout: %u.", contextP->timeout);
        if (contextP->timeout < duration)
        {
            CRIT_SECTION_LEAVE(contextP);
            result = iowa_step(contextP, 3);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "iowa_step() returned %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
                return result;
            }
            CRIT_SECTION_ENTER(contextP);
        }
#ifdef LWM2M_CLIENT_MODE
        else
        {
            if (contextP->lwm2mContextP->state != STATE_DEVICE_MANAGEMENT)
            {
                IOWA_LOG_ERROR(IOWA_PART_BASE, "LwM2M State is not STATE_DEVICE_MANAGEMENT.");
                CRIT_SECTION_LEAVE(contextP);
                return IOWA_COAP_503_SERVICE_UNAVAILABLE;
            }
        }
#endif
    } while (contextP->timeout < duration);

    if (delayP != NULL)
    {
#ifdef LWM2M_CLIENT_MODE
        *delayP = clientGetMaxDelayOperation(contextP);
#else
        *delayP = UINT32_MAX;
#endif
    }

    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_INFO(IOWA_PART_BASE, "IOWA flushed");

    return result;
}

void iowa_connection_closed(iowa_context_t contextP,
                            void *connP)
{
    comm_channel_t *channelP;

    IOWA_LOG_ARG_INFO(IOWA_PART_BASE, "ConnP: %p.", connP);

    CRIT_SECTION_ENTER(contextP);

    channelP = commChannelFind(contextP, connP);
    if (channelP != NULL)
    {
        if (channelP->eventCallback != NULL)
        {
            channelP->eventCallback(channelP, COMM_EVENT_DISCONNECTED, channelP->userData, contextP);
        }
        commChannelDelete(contextP, channelP);
    }

    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_INFO(IOWA_PART_BASE, "Exiting.");
}
