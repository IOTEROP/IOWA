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
#include <float.h>

#if defined(LWM2M_CLIENT_MODE)

/*************************************************************************************
** Private functions
*************************************************************************************/
#define PRV_ADD_RES_ID_TO_ARRAY(array, pt, RES) (array)[(pt)] = (IPSO_RSC_ID_##RES); (pt) += 1





static void prv_freeInstance(ipso_instance_t *instanceP)
{
    if (instanceP != NULL)
    {
        iowa_system_free(instanceP->appType);
        iowa_system_free(instanceP->unit);
        iowa_system_free(instanceP);
    }
}

static uint16_t prv_getResourceDesc(iowa_IPSO_ID_t type,
                                    iowa_lwm2m_resource_desc_t **resArrayP)
{
    int currentPt = 0;
    uint16_t resCount = 0;
    bool isCTO;

    isCTO = false;
    *resArrayP = NULL;

    switch (type)
    {
    case IOWA_IPSO_ILLUMINANCE:
    case IOWA_IPSO_TEMPERATURE:
    case IOWA_IPSO_HUMIDITY:
    case IOWA_IPSO_BAROMETER:
        isCTO = false;
        break;

    case IOWA_IPSO_GENERIC:
    case IOWA_IPSO_VOLTAGE:
    case IOWA_IPSO_CURRENT:
    case IOWA_IPSO_FREQUENCY:
    case IOWA_IPSO_DEPTH:
    case IOWA_IPSO_PERCENTAGE:
    case IOWA_IPSO_ALTITUDE:
    case IOWA_IPSO_LOAD:
    case IOWA_IPSO_PRESSURE:
    case IOWA_IPSO_LOUDNESS:
    case IOWA_IPSO_CONCENTRATION:
    case IOWA_IPSO_ACIDITY:
    case IOWA_IPSO_CONDUCTIVITY:
    case IOWA_IPSO_POWER:
    case IOWA_IPSO_POWER_FACTOR:
    case IOWA_IPSO_RATE:
    case IOWA_IPSO_DISTANCE:
        isCTO = true;
        break;

    case IOWA_IPSO_ANALOG_INPUT:
        resCount = 7;

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif

        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, ANALOG_INPUT_CURRENT_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MIN_MEASURED_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MAX_MEASURED_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, RESET_MIN_AND_MAX_MEASURED_VALUES);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MIN_RANGE_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MAX_RANGE_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, APPLICATION_TYPE);
        break;

    case IOWA_IPSO_ENERGY:
        resCount = 4;

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, SENSOR_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, RESET_CUMULATIVE_ENERGY);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, SENSOR_UNITS);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, APPLICATION_TYPE);
        break;

    case IOWA_IPSO_DIRECTION:
        resCount = 5;

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, COMPASS_DIRECTION);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, APPLICATION_TYPE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MIN_MEASURED_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MAX_MEASURED_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, RESET_MIN_AND_MAX_MEASURED_VALUES);
        break;

    case IOWA_IPSO_LEVEL_CONTROL:
        resCount = 4;

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, LEVEL);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, APPLICATION_TYPE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, ON_TIME);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, OFF_TIME);
        break;

    case IOWA_IPSO_DIGITAL_INPUT:
    case IOWA_IPSO_PUSH_BUTTON:
    case IOWA_IPSO_ON_OFF_SWITCH:
        resCount = 3;

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, DIGITAL_INPUT_STATE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, DIGITAL_INPUT_COUNTER);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, APPLICATION_TYPE);
        break;

    case IOWA_IPSO_PRESENCE:
        resCount = 4;

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, DIGITAL_INPUT_STATE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, DIGITAL_INPUT_COUNTER);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, SENSOR_TYPE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, DIGITAL_INPUT_COUNTER_RESET);
        break;

    default:
        IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Unsupported type %u.", type);
        return 0;
    }

    if (*resArrayP == NULL)
    {
        resCount = 7;

        if (isCTO == true)
        {
            resCount = (uint16_t)(resCount + 1);
        }

        *resArrayP = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resCount * sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(iowa_lwm2m_resource_desc_t));
            return 0;
        }
#endif
        memset(*resArrayP, 0, resCount * sizeof(iowa_lwm2m_resource_desc_t));

        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, SENSOR_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MIN_MEASURED_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MAX_MEASURED_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, RESET_MIN_AND_MAX_MEASURED_VALUES);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, SENSOR_UNITS);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MIN_RANGE_VALUE);
        SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, MAX_RANGE_VALUE);

        if (isCTO == true)
        {
            SET_LWM2M_DESC_T_TO_IPSO_RSC((*resArrayP), currentPt, APPLICATION_TYPE);
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_OBJECT, "Exiting with %u resources.", resCount);
    return resCount;
}

static uint16_t prv_getResourceList(iowa_IPSO_ID_t type,
                                    uint16_t **resArrayP,
                                    bool withRange,
                                    bool withUnit,
                                    bool withAppType)
{
    int currentPt = 0;
    uint16_t resCount = 0;
    bool isCTO;

    isCTO = false;
    *resArrayP = NULL;

    switch (type)
    {
    case IOWA_IPSO_ILLUMINANCE:
    case IOWA_IPSO_TEMPERATURE:
    case IOWA_IPSO_HUMIDITY:
    case IOWA_IPSO_BAROMETER:
        isCTO = false;
        break;

    case IOWA_IPSO_GENERIC:
    case IOWA_IPSO_VOLTAGE:
    case IOWA_IPSO_CURRENT:
    case IOWA_IPSO_FREQUENCY:
    case IOWA_IPSO_DEPTH:
    case IOWA_IPSO_PERCENTAGE:
    case IOWA_IPSO_ALTITUDE:
    case IOWA_IPSO_LOAD:
    case IOWA_IPSO_PRESSURE:
    case IOWA_IPSO_LOUDNESS:
    case IOWA_IPSO_CONCENTRATION:
    case IOWA_IPSO_ACIDITY:
    case IOWA_IPSO_CONDUCTIVITY:
    case IOWA_IPSO_POWER:
    case IOWA_IPSO_POWER_FACTOR:
    case IOWA_IPSO_RATE:
    case IOWA_IPSO_DISTANCE:
        isCTO = true;
        break;

    case IOWA_IPSO_ANALOG_INPUT:
        resCount = 4;
        if (withAppType == true)
        {
            resCount++;
        }
        if (withRange == true)
        {
            resCount = (uint16_t)(resCount + 2);
        }
        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif

        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, ANALOG_INPUT_CURRENT_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MIN_MEASURED_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MAX_MEASURED_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, RESET_MIN_AND_MAX_MEASURED_VALUES);

        if (withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, APPLICATION_TYPE);
        }

        if (withRange == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MIN_RANGE_VALUE);
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MAX_RANGE_VALUE);
        }

        break;

    case IOWA_IPSO_ENERGY:
        resCount = 2;
        if (withUnit == true)
        {
            resCount++;
        }
        if (withAppType == true)
        {
            resCount++;
        }

        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, SENSOR_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, RESET_CUMULATIVE_ENERGY);
        if (withUnit == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, SENSOR_UNITS);
        }
        if (withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, APPLICATION_TYPE);
        }
        break;

    case IOWA_IPSO_DIRECTION:
        resCount = 4;
        if (withAppType == true)
        {
            resCount++;
        }

        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, COMPASS_DIRECTION);
        if (withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, APPLICATION_TYPE);
        }
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MIN_MEASURED_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MAX_MEASURED_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, RESET_MIN_AND_MAX_MEASURED_VALUES);
        break;

    case IOWA_IPSO_LEVEL_CONTROL:

        resCount = 3;
        if (withAppType == true)
        {
            resCount++;
        }

        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, LEVEL);
        if (withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, APPLICATION_TYPE);
        }
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, ON_TIME);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, OFF_TIME);
        break;

    case IOWA_IPSO_DIGITAL_INPUT:
    case IOWA_IPSO_PUSH_BUTTON:
    case IOWA_IPSO_ON_OFF_SWITCH:
        resCount = 2;
        if (withAppType == true)
        {
            resCount++;
        }

        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, DIGITAL_INPUT_STATE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, DIGITAL_INPUT_COUNTER);
        if (withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, APPLICATION_TYPE);
        }
        break;

    case IOWA_IPSO_PRESENCE:
        resCount = 3;
        if (withAppType == true)
        {
            resCount++;
        }

        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, DIGITAL_INPUT_STATE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, DIGITAL_INPUT_COUNTER);
        if (withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, SENSOR_TYPE);
        }
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, DIGITAL_INPUT_COUNTER_RESET);
        break;

    default:
        IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Unsupported type %u.", type);
        return 0;
    }

    if (*resArrayP == NULL)
    {
        resCount = 4;
        if (withUnit == true)
        {
            resCount++;
        }
        if (isCTO == true && withAppType == true)
        {
            resCount++;
        }
        if (withRange == true)
        {
            resCount = (uint16_t)(resCount + 2);
        }
        *resArrayP = (uint16_t *)iowa_system_malloc(resCount * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (*resArrayP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(resCount * sizeof(uint16_t));
            return 0;
        }
#endif

        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, SENSOR_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MIN_MEASURED_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MAX_MEASURED_VALUE);
        PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, RESET_MIN_AND_MAX_MEASURED_VALUES);

        if (withUnit == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, SENSOR_UNITS);
        }

        if (isCTO == true && withAppType == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, APPLICATION_TYPE);
        }

        if (withRange == true)
        {
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MIN_RANGE_VALUE);
            PRV_ADD_RES_ID_TO_ARRAY(*resArrayP, currentPt, MAX_RANGE_VALUE);
        }
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_OBJECT, "Exiting with %u resources.", resCount);
    return resCount;
}

static iowa_status_t prv_ipsoObjectCallback(iowa_dm_operation_t operation,
                                            iowa_lwm2m_data_t *dataP,
                                            size_t numData,
                                            void *userData,
                                            iowa_context_t contextP)
{
    ipso_object_t *objDataP;
    ipso_instance_t *instanceP;
    size_t i;
    iowa_status_t result;

    CRIT_SECTION_ENTER(contextP);
    objDataP = (ipso_object_t *)userData;

    result = IOWA_COAP_NO_ERROR;

    for (i = 0; i < numData; i++)
    {
        if (i == 0
            || dataP[i].instanceID != dataP[i - 1].instanceID)
        {

            instanceP = (ipso_instance_t *)IOWA_UTILS_LIST_FIND(objDataP->instanceList, listFindCallbackBy16bitsId, &dataP[i].instanceID);
        }

        switch (operation)
        {

        case IOWA_DM_READ:
            switch (dataP[i].resourceID)
            {
            case IPSO_RSC_ID_ON_TIME:
                dataP[i].value.asInteger = instanceP->onTime;
                if (instanceP->value > (0.f + FLT_EPSILON))
                {
                    int32_t currentTime;

                    currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                    if (currentTime < 0 || currentTime < instanceP->timePowerOn)
                    {
                        CRIT_SECTION_LEAVE(contextP);
                        IOWA_LOG_ERROR_GETTIME(currentTime);
                        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    }
#endif

                    dataP[i].value.asInteger += currentTime - instanceP->timePowerOn;
                }
                break;
            case IPSO_RSC_ID_OFF_TIME:
                dataP[i].value.asInteger = instanceP->offTime;
                if (instanceP->value <= (0.f + FLT_EPSILON))
                {
                    int32_t currentTime;

                    currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                    if (currentTime < 0 || currentTime < instanceP->timePowerOff)
                    {
                        CRIT_SECTION_LEAVE(contextP);
                        IOWA_LOG_ERROR_GETTIME(currentTime);
                        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    }
#endif

                    dataP[i].value.asInteger += currentTime - instanceP->timePowerOff;
                }
                break;
            case IPSO_RSC_ID_SENSOR_VALUE:
            case IPSO_RSC_ID_ANALOG_INPUT_CURRENT_VALUE:
            case IPSO_RSC_ID_CUMULATIVE_ACTIVE_POWER:
            case IPSO_RSC_ID_COMPASS_DIRECTION:
            case IPSO_RSC_ID_LEVEL:
            {
                dataP[i].value.asFloat = instanceP->value;
                break;
            }

            case IPSO_RSC_ID_DIGITAL_INPUT_STATE:
            {
                dataP[i].value.asBoolean = (instanceP->value != 0);
                break;
            }

            case IPSO_RSC_ID_DIGITAL_INPUT_COUNTER:
                dataP[i].value.asInteger = (int32_t)instanceP->max;
                break;

            case IPSO_RSC_ID_SENSOR_UNITS:

                dataP[i].value.asBuffer.buffer = (uint8_t*)instanceP->unit;
                dataP[i].value.asBuffer.length = strlen(instanceP->unit);
                break;

            case IPSO_RSC_ID_MIN_MEASURED_VALUE:
                dataP[i].value.asFloat = instanceP->min;
                break;

            case IPSO_RSC_ID_MAX_MEASURED_VALUE:
                dataP[i].value.asFloat = instanceP->max;
                break;

            case IPSO_RSC_ID_MIN_RANGE_VALUE:
                dataP[i].value.asFloat = instanceP->rangeMin;
                break;

            case IPSO_RSC_ID_MAX_RANGE_VALUE:
                dataP[i].value.asFloat = instanceP->rangeMax;
                break;

            case IPSO_RSC_ID_APPLICATION_TYPE:
            case IPSO_RSC_ID_SENSOR_TYPE:
                dataP[i].value.asBuffer.buffer = (uint8_t*)instanceP->appType;
                dataP[i].value.asBuffer.length = strlen(instanceP->appType);
                break;

            default:
                break;
            }
            break;

        case IOWA_DM_WRITE:
            switch (dataP[i].resourceID)
            {
            case IPSO_RSC_ID_APPLICATION_TYPE:
            {
                char *tempBuffer;

                tempBuffer = utilsBufferToString(dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                if (dataP[i].value.asBuffer.buffer != NULL
                    && tempBuffer == NULL)
                {
                    result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    break;
                }
#endif
                iowa_system_free(instanceP->appType);
                instanceP->appType = tempBuffer;
            }
            break;

            case IPSO_RSC_ID_ON_TIME:
            {
                if (dataP[i].value.asInteger != 0)
                {
                    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Can only write 0 to the 'On Time' resource");
                    result = IOWA_COAP_406_NOT_ACCEPTABLE;
                }
                else
                {
                    instanceP->onTime = 0;
                    if (instanceP->value > (0.f + FLT_EPSILON))
                    {
                        int32_t currentTime;

                        currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                        if (currentTime < 0 || currentTime < instanceP->timePowerOn)
                        {
                            IOWA_LOG_ERROR_GETTIME(currentTime);
                            result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                            break;
                        }
#endif
                        instanceP->timePowerOn = currentTime;
                    }
                    customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, IPSO_RSC_ID_ON_TIME);
                }
            }
            break;

            case IPSO_RSC_ID_OFF_TIME:
            {
                if (dataP[i].value.asInteger != 0)
                {
                    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Can only write 0 to the 'Off Time' resource");
                    result = IOWA_COAP_406_NOT_ACCEPTABLE;
                }
                else
                {
                    instanceP->offTime = 0;
                    if (instanceP->value <= (0.f + FLT_EPSILON))
                    {
                        int32_t currentTime;

                        currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
                        if (currentTime < 0 || currentTime < instanceP->timePowerOff)
                        {
                            IOWA_LOG_ERROR_GETTIME(currentTime);
                            result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                            break;
                        }
#endif
                        instanceP->timePowerOff = currentTime;
                    }
                    customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, IPSO_RSC_ID_OFF_TIME);
                }
            }
            break;

            default:
                result = IOWA_COAP_405_METHOD_NOT_ALLOWED;
                break;
            }
            break;

        case IOWA_DM_EXECUTE:
        {
            iowa_status_t resultExecute;

            if (dataP[i].type != IOWA_LWM2M_TYPE_UNDEFINED)
            {

                IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "No argument should be provided. Found: \"%.*s\".", dataP[i].value.asBuffer.length, dataP[i].value.asBuffer.buffer);
                result = IOWA_COAP_400_BAD_REQUEST;
                break;
            }

            resultExecute = IOWA_COAP_NO_ERROR;

            switch (dataP[i].resourceID)
            {
            case IPSO_RSC_ID_RESET_MIN_AND_MAX_MEASURED_VALUES:
                instanceP->min = instanceP->value;
                instanceP->max = instanceP->value;
                customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, IPSO_RSC_ID_MIN_MEASURED_VALUE);
                customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, IPSO_RSC_ID_MAX_MEASURED_VALUE);
                break;

            case IPSO_RSC_ID_RESET_CUMULATIVE_ENERGY:
                instanceP->value = 0.0f;
                customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, IPSO_RSC_ID_CUMULATIVE_ACTIVE_POWER);
                break;

            case IPSO_RSC_ID_DIGITAL_INPUT_COUNTER_RESET:
                instanceP->max = 0.0f;
                customObjectResourceChanged(contextP, dataP[i].objectID, dataP[i].instanceID, IPSO_RSC_ID_DIGITAL_INPUT_COUNTER);
                break;

            default:
                resultExecute = IOWA_COAP_405_METHOD_NOT_ALLOWED;
                break;
            }

            if (i == 0 || resultExecute == IOWA_COAP_NO_ERROR)
            {
                result = resultExecute;
            }
        }
        break;

        default:
            break;
        }
    }

    CRIT_SECTION_LEAVE(contextP);

    return result;
}

#if !defined(IOWA_CONFIG_SKIP_ARGS_CHECK) || defined(LWM2M_SUPPORT_TIMESTAMP)
static iowa_status_t prv_checkResourceValue(iowa_IPSO_ID_t type,
                                            float value)
{
    switch (type)
    {

    case IOWA_IPSO_LEVEL_CONTROL:
        if (value < 0.f || value > 100.f)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "Percentage value %0.3f is outside the range [0,100].", (double)value);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        break;


    case IOWA_IPSO_DIGITAL_INPUT:
    case IOWA_IPSO_PRESENCE:
    case IOWA_IPSO_ON_OFF_SWITCH:
    case IOWA_IPSO_PUSH_BUTTON:

        if (dataUtilsCompareFloatingPointNumbers((double)value, 0., FLT_EPSILON) == false
            && dataUtilsCompareFloatingPointNumbers((double)value, 1., FLT_EPSILON) == false)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "Value %0.3f is neither 0 nor 1.", (double)value);
            return IOWA_COAP_406_NOT_ACCEPTABLE;
        }
        break;

    default:
        break;
    }
    return IOWA_COAP_NO_ERROR;
}
#endif

static void prv_updateResourceValues(iowa_context_t contextP,
                                     iowa_sensor_t id,
                                     ipso_instance_t *instanceP,
                                     size_t valueCount,
                                     iowa_ipso_timed_value_t *valueArray)
{
    size_t index;
    float prevValue;

    prevValue = instanceP->value;

    if (customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_DIGITAL_INPUT_STATE) == true)
    {
        bool hasCounter;

        hasCounter = customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_DIGITAL_INPUT_COUNTER);

        for (index = 0; index < valueCount; index++)
        {

            if ((prevValue > valueArray[index].value && prevValue - valueArray[index].value > FLT_EPSILON)
                || (valueArray[index].value > prevValue && valueArray[index].value - prevValue > FLT_EPSILON))
            {
                customObjectResourceChanged(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_DIGITAL_INPUT_STATE);

                if (hasCounter == true
                    && (valueArray[index].value > prevValue && valueArray[index].value - prevValue > FLT_EPSILON))
                {
                    instanceP->max += 1.f;
                    customObjectResourceChanged(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_DIGITAL_INPUT_COUNTER);
                }
            }
            prevValue = valueArray[index].value;
        }
    }
    else
    {
        uint16_t valueId;
        bool hasMin;
        bool hasMax;

        if (customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_LEVEL) == true)
        {
            valueId = IPSO_RSC_ID_LEVEL;
        }
        else if(customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_COMPASS_DIRECTION) == true)
        {
            valueId = IPSO_RSC_ID_COMPASS_DIRECTION;
        }
        else if(customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_ANALOG_INPUT_CURRENT_VALUE) == true)
        {
            valueId = IPSO_RSC_ID_ANALOG_INPUT_CURRENT_VALUE;
        }
        else
        {
            valueId = IPSO_RSC_ID_SENSOR_VALUE;
        }

        hasMin = customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_MIN_MEASURED_VALUE);
        hasMax = customObjectHasResource(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_MAX_MEASURED_VALUE);

        for (index = 0; index < valueCount; index++)
        {
            if (dataUtilsCompareFloatingPointNumbers(valueArray[index].value, prevValue, FLT_EPSILON) == false)
            {
                customObjectResourceChanged(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), valueId);

                if (hasMin == true
                    && valueArray[index].value < instanceP->min)
                {
                    instanceP->min = valueArray[index].value;
                    customObjectResourceChanged(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_MIN_MEASURED_VALUE);
                }
                else if (hasMax == true
                         && valueArray[index].value > instanceP->max)
                {
                    instanceP->max = valueArray[index].value;
                    customObjectResourceChanged(contextP, GET_OBJECT_ID_FROM_SENSOR(id), GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_MAX_MEASURED_VALUE);
                }
            }
            prevValue = valueArray[index].value;
        }
    }
}

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_status_t iowa_client_IPSO_add_sensor(iowa_context_t contextP,
                                          iowa_IPSO_ID_t type,
                                          float value,
                                          const char *unit,
                                          const char *appType,
                                          float rangeMin,
                                          float rangeMax,
                                          iowa_sensor_t *idP)
{
    ipso_instance_t *instanceP;
    ipso_object_t *objDataP;
    iowa_status_t result;
    uint16_t resourceCount;
    uint16_t *resourceArray;
    int32_t currentTime;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Adding IPSO sensor with type: %u, value: %f, unit: \"%s\", appType: \"%s\", rangeMin: %f, rangeMax: %f.", type, (double)value, unit, appType, (double)rangeMin, (double)rangeMax);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK

    result = prv_checkResourceValue(type, value);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Resources value check failed.");
        return result;
    }
#endif

    instanceP = (ipso_instance_t *)iowa_system_malloc(sizeof(ipso_instance_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (instanceP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(ipso_instance_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(instanceP, 0, sizeof(ipso_instance_t));

    instanceP->appType = utilsStrdup(appType);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (instanceP->appType == NULL
        && appType != NULL)
    {
        iowa_system_free(instanceP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    instanceP->unit = utilsStrdup(unit);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (instanceP->unit == NULL
        && unit != NULL)
    {
        iowa_system_free(instanceP->appType);
        iowa_system_free(instanceP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    instanceP->value = value;
    instanceP->onTime = 0;
    instanceP->offTime = 0;

    currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (currentTime < 0)
    {
        IOWA_LOG_ERROR_GETTIME(currentTime);
        prv_freeInstance(instanceP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    if (value > (0.f + FLT_EPSILON))
    {
        instanceP->timePowerOn = currentTime;
    }
    else
    {
        instanceP->timePowerOff = currentTime;
    }
    instanceP->min = value;
    instanceP->max = value;
    instanceP->rangeMin = rangeMin;
    instanceP->rangeMax = rangeMax;

    CRIT_SECTION_ENTER(contextP);
    objDataP = (ipso_object_t *)objectGetData(contextP, type);
    if (objDataP == NULL)
    {
        iowa_lwm2m_resource_desc_t *resDescArray;

        IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Creating new ipso_object_t for type %u.", type);

        resourceCount = prv_getResourceDesc(type, &resDescArray);
        if (resourceCount == 0)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Unsupported type %u.", type);
            prv_freeInstance(instanceP);
            return IOWA_COAP_402_BAD_OPTION;
        }
        objDataP = (ipso_object_t *)iowa_system_malloc(sizeof(ipso_object_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (objDataP == NULL)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ERROR_MALLOC(sizeof(ipso_object_t));
            prv_freeInstance(instanceP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        memset(objDataP, 0, sizeof(ipso_object_t));

        result = customObjectAdd(contextP,
                                 type,
                                 OBJECT_MULTIPLE_ADVANCED,
                                 0, NULL,
                                 resourceCount, resDescArray,
                                 prv_ipsoObjectCallback, NULL, NULL,
                                 objDataP);

        iowa_system_free(resDescArray);

        if (result != IOWA_COAP_NO_ERROR)
        {
            CRIT_SECTION_LEAVE(contextP);
            IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "Failed to add new Object %u with result: %u.%02u.", type, (result & 0xFF) >> 5, (result & 0x1F));
            prv_freeInstance(instanceP);
            iowa_system_free(objDataP);

            return result;
        }
    }

    resourceCount = prv_getResourceList(type, &resourceArray,
                                        ((dataUtilsCompareFloatingPointNumbers(rangeMin, 0, FLT_EPSILON) == false) || (dataUtilsCompareFloatingPointNumbers(rangeMax, 0, FLT_EPSILON) == false)),
                                        instanceP->unit != NULL,
                                        instanceP->appType != NULL);
    if (resourceCount == 0)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Unsupported type %u.", type);
        prv_freeInstance(instanceP);
        return IOWA_COAP_402_BAD_OPTION;
    }

    instanceP->id = LIST_NEW_16_BITS_ID(objDataP->instanceList);

    result = objectAddInstance(contextP,
                               type, instanceP->id,
                               resourceCount, resourceArray);
    iowa_system_free(resourceArray);
    if (result != IOWA_COAP_NO_ERROR)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "Failed to add new Instance to Object %u with result: %u.%02u.", type, (result & 0xFF) >> 5, (result & 0x1F));
        prv_freeInstance(instanceP);

        return result;
    }

    objDataP->instanceList = (ipso_instance_t *)IOWA_UTILS_LIST_ADD(objDataP->instanceList, instanceP);

    CRIT_SECTION_LEAVE(contextP);

    *idP = type << 16 | instanceP->id;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    return result;
}

iowa_status_t iowa_client_IPSO_remove_sensor(iowa_context_t contextP,
                                             iowa_sensor_t id)
{
    ipso_object_t *objDataP;
    ipso_instance_t *instanceP;
    iowa_status_t result;
    uint16_t instanceId;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Removing IPSO sensor /%d/%d.", (uint16_t)(id >> 16), id & 0xFFFF);

    CRIT_SECTION_ENTER(contextP);

    objDataP = (ipso_object_t *)objectGetData(contextP, GET_OBJECT_ID_FROM_SENSOR(id));
    if (objDataP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "IPSO sensor with Object ID %d has not been found.", GET_OBJECT_ID_FROM_SENSOR(id));
        return IOWA_COAP_404_NOT_FOUND;
    }

    instanceId = GET_INSTANCE_ID_FROM_SENSOR(id);
    objDataP->instanceList = (ipso_instance_t *)IOWA_UTILS_LIST_FIND_AND_REMOVE(objDataP->instanceList, listFindCallbackBy16bitsId, &instanceId, &instanceP);
    if (instanceP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "IPSO sensor with Object Instance ID %d has not been found.", instanceId);
        return IOWA_COAP_404_NOT_FOUND;
    }

    prv_freeInstance(instanceP);

    if (objDataP->instanceList != NULL)
    {
        result = objectRemoveInstance(contextP,
                                      GET_OBJECT_ID_FROM_SENSOR(id),
                                      instanceId);
    }
    else
    {
        result = customObjectRemove(contextP, GET_OBJECT_ID_FROM_SENSOR(id));
        iowa_system_free(objDataP);
    }

    CRIT_SECTION_LEAVE(contextP);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));
    return result;
}

iowa_status_t iowa_client_IPSO_update_value(iowa_context_t contextP,
                                            iowa_sensor_t id,
                                            float value)
{
    ipso_instance_t *instanceP;
    lwm2m_object_t *dataP;
    iowa_ipso_timed_value_t valueToUpdate;
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    iowa_status_t result;
#endif
    uint16_t objectId;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Updating IPSO object /%d/%d. New value: %f.", (uint16_t)(id >> 16), id & 0xFFFF, (double)value);


    objectId = GET_OBJECT_ID_FROM_SENSOR(id);

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK

    result = prv_checkResourceValue((iowa_IPSO_ID_t)objectId, value);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Resources value check failed.");
        return result;
    }
#endif

    CRIT_SECTION_ENTER(contextP);

    dataP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
    if (dataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The structure 'lwm2m_object_t' associated with the IPSO sensor has not been found.");
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_404_NOT_FOUND;
    }

    instanceP = (ipso_instance_t *)objectGetInstanceData(contextP, objectId, GET_INSTANCE_ID_FROM_SENSOR(id));
    if (instanceP == NULL)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "IPSO sensor with Object ID %d and Object Instance ID %d has not been found.", &objectId, GET_INSTANCE_ID_FROM_SENSOR(id));
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_404_NOT_FOUND;
    }

    clientNotificationLock(contextP, true);


    valueToUpdate.value = value;
    valueToUpdate.timestamp = 0;
    prv_updateResourceValues(contextP, id, instanceP, 1, &valueToUpdate);

    if ((instanceP->value <= (0.f + FLT_EPSILON)) && (value > (0.f + FLT_EPSILON)))
    {
        int32_t currentTime;

        currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (currentTime < 0)
        {
            IOWA_LOG_ERROR_GETTIME(currentTime);
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        instanceP->timePowerOn = currentTime;
        instanceP->offTime += currentTime - instanceP->timePowerOff;
        customObjectResourceChanged(contextP, objectId, GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_OFF_TIME);
    }
    else if ((instanceP->value > (0.f + FLT_EPSILON)) && (value <= (0.f + FLT_EPSILON)))
    {
        int32_t currentTime;

        currentTime = iowa_system_gettime();
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (currentTime < 0 || currentTime < instanceP->timePowerOn)
        {
            IOWA_LOG_ERROR_GETTIME(currentTime);
            CRIT_SECTION_LEAVE(contextP);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        instanceP->onTime += currentTime - instanceP->timePowerOn;
        instanceP->timePowerOff = currentTime;
        customObjectResourceChanged(contextP, objectId, GET_INSTANCE_ID_FROM_SENSOR(id), IPSO_RSC_ID_ON_TIME);
    }

    instanceP->value = value;

    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);

    return IOWA_COAP_NO_ERROR;
}

#endif
