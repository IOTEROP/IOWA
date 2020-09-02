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

#include "iowa_prv_objects_internals.h"

#ifdef LWM2M_CLIENT_MODE

#define PRV_RSC_NUMBER 19

/*************************************************************************************
** Private functions
*************************************************************************************/

static iowa_status_t prv_serverObjectCallback(iowa_dm_operation_t operation,
                                              iowa_lwm2m_data_t *dataP,
                                              size_t numData,
                                              void *userData,
                                              iowa_context_t contextP)
{
    lwm2m_server_t *targetP;
    size_t i;
    iowa_status_t result;

    (void)userData;

    CRIT_SECTION_ENTER(contextP);

    result = IOWA_COAP_NO_ERROR;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0; i < numData && result == IOWA_COAP_NO_ERROR; i++)
        {
            if (i == 0
                || dataP[i].instanceID != dataP[i - 1].instanceID)
            {
                {
                    targetP = contextP->lwm2mContextP->serverList;
                }

                while (targetP != NULL
                       && targetP->srvObjInstId != dataP[i].instanceID)
                {
                    targetP = targetP->next;
                }
                if (targetP == NULL)
                {
                    CRIT_SECTION_LEAVE(contextP);
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Target server not found.");
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_SERVER_ID_SHORT_ID:
                dataP[i].value.asInteger = targetP->shortId;
                break;

            case IOWA_LWM2M_SERVER_ID_LIFETIME:
                dataP[i].value.asInteger = targetP->lifetime;
                break;

#ifndef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
            case IOWA_LWM2M_SERVER_ID_MIN_PERIOD:
                dataP[i].value.asInteger = targetP->defaultPmin;
                break;

            case IOWA_LWM2M_SERVER_ID_MAX_PERIOD:
                dataP[i].value.asInteger = targetP->defaultPmax;
                break;
#endif

#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
            case IOWA_LWM2M_SERVER_ID_TIMEOUT:
                dataP[i].value.asInteger = targetP->disableTimeout;
                break;
#endif

            case IOWA_LWM2M_SERVER_ID_STORING:
                dataP[i].value.asBoolean = targetP->notifStoring;
                break;

            case IOWA_LWM2M_SERVER_ID_BINDING:

                dataP[i].value.asBuffer.length = utils_bindingToString(targetP->binding, (targetP->binding & BINDING_Q) != 0, &dataP[i].value.asBuffer.buffer);
                break;

            default:

                break;
            }
        }
        break;

    case IOWA_DM_FREE:
        for (i = 0 ; i < numData ; i++)
        {
            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_SERVER_ID_BINDING:
                iowa_system_free(dataP[i].value.asBuffer.buffer);
                break;

            default:

                break;
            }
        }
        break;

    case IOWA_DM_WRITE:
        for (i = 0; i < numData && result == IOWA_COAP_NO_ERROR; i++)
        {
            if (i == 0
                || dataP[i].instanceID != dataP[i - 1].instanceID)
            {
                {
                    targetP = contextP->lwm2mContextP->serverList;
                }

                while (targetP != NULL
                       && targetP->srvObjInstId != dataP[i].instanceID)
                {
                    targetP = targetP->next;
                }
                if (targetP == NULL)
                {
                    CRIT_SECTION_LEAVE(contextP);
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Target server not found.");
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            switch (dataP[i].resourceID)
            {

            case IOWA_LWM2M_SERVER_ID_LIFETIME:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > INT_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Lifetime value is outside the range [0; %d].", INT_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }

                if (targetP->lifetime != dataP[i].value.asInteger)
                {
                    targetP->lifetime = dataP[i].value.asInteger;

                    {
                        lwm2mUpdateRegistration(contextP, targetP, LWM2M_UPDATE_FLAG_LIFETIME);

                        CRIT_SECTION_LEAVE(contextP);
                        INTERRUPT_SELECT(contextP);
                        CRIT_SECTION_ENTER(contextP);
                    }
                }
                break;
#ifndef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
            case IOWA_LWM2M_SERVER_ID_MIN_PERIOD:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > UINT32_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Minimum period value is outside the range [0; %d].", UINT32_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }
                targetP->defaultPmin = dataP[i].value.asInteger;
                break;

            case IOWA_LWM2M_SERVER_ID_MAX_PERIOD:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > UINT32_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Maximum period value is outside the range [0; %d].", UINT32_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }
                targetP->defaultPmax = dataP[i].value.asInteger;
                break;
#endif

#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
            case IOWA_LWM2M_SERVER_ID_TIMEOUT:
                if (dataP[i].value.asInteger < 0
                    || dataP[i].value.asInteger > INT32_MAX)
                {
                    IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Timeout value is outside the range [0; %d].", INT32_MAX);
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }
                targetP->disableTimeout = dataP[i].value.asInteger;
                break;
#endif

            case IOWA_LWM2M_SERVER_ID_STORING:
                {
                }

                targetP->notifStoring = dataP[i].value.asBoolean;
                break;

            case IOWA_LWM2M_SERVER_ID_BINDING:
                IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Cannot write the resource Binding.");
                result = IOWA_COAP_400_BAD_REQUEST;
                break;

            default:

                break;
            }
        }
        break;

    case IOWA_DM_EXECUTE:
        for (i = 0; i < numData && result == IOWA_COAP_NO_ERROR; i++)
        {
            if (dataP[i].type != IOWA_LWM2M_TYPE_UNDEFINED)
            {

                IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "No argument should be provided. Found: \"%.*s\".", dataP[i].value.asBuffer.length, dataP[i].value.asBuffer.buffer);
                result = IOWA_COAP_400_BAD_REQUEST;
                break;
            }

            if (i == 0
                || dataP[i].instanceID != dataP[i - 1].instanceID)
            {
                {
                    targetP = contextP->lwm2mContextP->serverList;
                }

                while (targetP != NULL
                       && targetP->srvObjInstId != dataP[i].instanceID)
                {
                    targetP = targetP->next;
                }
                if (targetP == NULL)
                {
                    CRIT_SECTION_LEAVE(contextP);
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Target server not found.");
                    return IOWA_COAP_404_NOT_FOUND;
                }
            }

            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_SERVER_ID_UPDATE:
                lwm2mUpdateRegistration(contextP, targetP, LWM2M_UPDATE_FLAG_NONE);
                result = IOWA_COAP_NO_ERROR;
                CRIT_SECTION_LEAVE(contextP);
                INTERRUPT_SELECT(contextP);
                CRIT_SECTION_ENTER(contextP);
                break;

#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
            case IOWA_LWM2M_SERVER_ID_DISABLE:
                if (targetP->runtime.status == STATE_DISCONNECTED
                    || targetP->runtime.status == STATE_REG_REGISTERING
                    || targetP->runtime.status == STATE_REG_FAILED)
                {
                    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Try to disable a server which is not yet registered.");
                    result = IOWA_COAP_400_BAD_REQUEST;
                    break;
                }

                targetP->runtime.flags |= LWM2M_SERVER_FLAG_DISABLE;
                break;
#endif

            default:

                break;
            }
        }
        break;

    default:
        break;
    }

    CRIT_SECTION_LEAVE(contextP);

    return result;
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t objectServerInit(iowa_context_t contextP)
{
    iowa_status_t result;
    iowa_lwm2m_resource_desc_t resources[PRV_RSC_NUMBER];
    int currentPt;
    const uint16_t nbrRes = PRV_RSC_NUMBER
#ifdef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
                            - 2
#endif
#ifdef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
                            - 2
#endif
                            - 1
#ifdef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
                            - 4
#endif
#ifdef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
                            - 4
#endif
                            - 1
                            ;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Entering");


    currentPt = 0;

    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, SHORT_ID, resources, currentPt);
#ifndef IOWA_SERVER_RSC_DEFAULT_PERIODS_REMOVE
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, MIN_PERIOD, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, MAX_PERIOD, resources, currentPt);
#endif
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, LIFETIME, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, STORING, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, BINDING, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, UPDATE, resources, currentPt);
#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_REMOVE
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, DISABLE, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, TIMEOUT, resources, currentPt);
#endif
#ifndef IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, PRIORITY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, INITIAL_DELAY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, REG_FAIL_BLOCK, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, BOOTSTRAP_REG_FAIL, resources, currentPt);
#endif
#ifndef IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, COMM_RETRY_COUNT, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, COMM_RETRY_TIMER, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, COMM_SEQUENCE_DELAY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SERVER, COMM_SEQUENCE_COUNT, resources, currentPt);
#endif


    result = customObjectAdd(contextP,
                             IOWA_LWM2M_SERVER_OBJECT_ID,
                             OBJECT_MULTIPLE,
                             0, NULL,
                             nbrRes, resources,
                             prv_serverObjectCallback,
                             NULL,
                             NULL,
                             NULL);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectServerCreate(iowa_context_t contextP,
                                 uint16_t id)
{
    iowa_status_t result;

    assert(id != IOWA_LWM2M_ID_ALL);

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Adding new server object");

    result = objectAddInstance(contextP,
                               IOWA_LWM2M_SERVER_OBJECT_ID,
                               id,
                               0, NULL);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectServerRemove(iowa_context_t contextP,
                                 uint16_t id)
{
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Removing server object (instance: %d)", id);


    result = objectRemoveInstance(contextP,
                                  IOWA_LWM2M_SERVER_OBJECT_ID,
                                  id);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectServerClose(iowa_context_t contextP)
{

    iowa_status_t result;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Closing server object");

    result = customObjectRemove(contextP, IOWA_LWM2M_SERVER_OBJECT_ID);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

#endif
