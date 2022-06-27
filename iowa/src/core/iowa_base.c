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

#ifdef LWM2M_BIG_ENDIAN
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LWM2M_BIG_ENDIAN");
#endif

#ifdef LWM2M_LITTLE_ENDIAN
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LWM2M_LITTLE_ENDIAN");
#endif

#ifdef IOWA_BUFFER_SIZE
    IOWA_LOG_ARG_INFO(IOWA_PART_SYSTEM, "IOWA_BUFFER_SIZE: %d", IOWA_BUFFER_SIZE);
#endif

#ifdef IOWA_UDP_SUPPORT
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_UDP_SUPPORT");
#endif

#ifdef IOWA_WEBSOCKET_SUPPORT
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_WEBSOCKET_SUPPORT");
#endif

#if IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_NONE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SECURITY_LAYER: IOWA_SECURITY_LAYER_NONE");
#endif

#ifdef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK");
#endif

#ifdef IOWA_CONFIG_SKIP_ARGS_CHECK
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_CONFIG_SKIP_ARGS_CHECK");
#endif

#ifdef IOWA_PEER_IDENTIFIER_SIZE
    IOWA_LOG_ARG_INFO(IOWA_PART_SYSTEM, "IOWA_PEER_IDENTIFIER_SIZE: %d", IOWA_PEER_IDENTIFIER_SIZE);
#endif

#ifdef LWM2M_CLIENT_MODE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LWM2M_CLIENT_MODE");
#endif

#ifdef LWM2M_SUPPORT_TLV
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LWM2M_SUPPORT_TLV");
#endif

#ifdef LWM2M_ALTPATH_SUPPORT
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "LWM2M_ALTPATH_SUPPORT");
#endif

#ifdef IOWA_FIRMWARE_UPDATE_MAX_BLOCK_INTERVAL
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_FIRMWARE_UPDATE_MAX_BLOCK_INTERVAL");
#endif

#ifdef IOWA_DEVICE_SUPPORT_RSC_MANUFACTURER
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_MANUFACTURER");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_MODEL_NUMBER
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_MODEL_NUMBER");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_SERIAL_NUMBER
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_SERIAL_NUMBER");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_FIRMWARE_VERSION
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_FIRMWARE_VERSION");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_FACTORY_RESET
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_FACTORY_RESET");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_BATTERY
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_BATTERY");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_RESET_ERROR
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_RESET_ERROR");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_CURRENT_TIME
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_CURRENT_TIME");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_UTC_OFFSET
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_UTC_OFFSET");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_TIMEZONE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_TIMEZONE");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_DEVICE_TYPE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_DEVICE_TYPE");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_HARDWARE_VERSION
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_HARDWARE_VERSION");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_SOFTWARE_VERSION
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_SOFTWARE_VERSION");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_MEMORY_TOTAL
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_MEMORY_TOTAL");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_MEMORY_FREE
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_MEMORY_FREE");
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_EXTERNAL_INFO
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_DEVICE_SUPPORT_RSC_EXTERNAL_INFO");
#endif


#ifdef IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SERVER_SUPPORT_RSC_DISABLE_TIMEOUT");
#endif
#ifdef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS");
#endif
#ifdef IOWA_SERVER_SUPPORT_RSC_BOOTSTRAP_TRIGGER
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SERVER_SUPPORT_RSC_BOOTSTRAP_TRIGGER");
#endif
#ifdef IOWA_SERVER_SUPPORT_RSC_REGISTRATION_BEHAVIOUR
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SERVER_SUPPORT_RSC_REGISTRATION_BEHAVIOUR");
#endif
#ifdef IOWA_SERVER_SUPPORT_RSC_COMMUNICATION_ATTEMPTS
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SERVER_SUPPORT_RSC_COMMUNICATION_ATTEMPTS");
#endif
#ifdef IOWA_SERVER_SUPPORT_RSC_MUTE_SEND
    IOWA_LOG_INFO(IOWA_PART_SYSTEM, "IOWA_SERVER_SUPPORT_RSC_MUTE_SEND");
#endif

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
    CRIT_SECTION_ENTER(contextP);
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
    CRIT_SECTION_LEAVE(contextP);
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
#ifdef IOWA_DEVICE_SUPPORT_RSC_FACTORY_RESET
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

#if defined(LWM2M_CLIENT_MODE) || defined(LWM2M_SERVER_MODE) || defined(LWM2M_BOOTSTRAP_SERVER_MODE)
        status = lwm2m_step(contextP);
        if (status != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_BASE, "An error occurred during the LwM2M step routine: %u.%02u.", (status & 0xFF) >> 5, (status & 0x1F));
            CRIT_SECTION_LEAVE(contextP);
            return status;
        }
#endif

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
