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

#include "iowa_client.h"

/*************************************************************************************
** Private functions
*************************************************************************************/
uint8_t prv_getNumberOfBit(uint8_t arg)
{
    uint8_t number;

    number = 0;

    while (arg > 0)
    {
        number++;
        arg &= (uint8_t)(arg-1);
    }

    return number;
}

uint8_t prv_getErrorValue(uint8_t errorCodesFlag, uint16_t resInstanceId)
{
    uint8_t n;
    uint8_t res;

    n = errorCodesFlag;
    res = 0;

    if (errorCodesFlag > 0)
    {
        size_t i;

        res = 1;

        for (i = 0 ; i < resInstanceId ; i++)
        {
            n &= (uint8_t)(n - 1);
        }
        while ((n & 0x01) != 0x01)
        {
            res++;
            n >>= 1;
        }
    }

    return res;
}

static iowa_status_t prv_deviceObjectCallback(iowa_dm_operation_t operation,
                                              iowa_lwm2m_data_t *dataP,
                                              size_t numData,
                                              void *userData,
                                              iowa_context_t contextP)
{
    device_object_t *deviceDataP;
    size_t i;
    iowa_status_t result;

    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)userData;

    result = IOWA_COAP_NO_ERROR;

    switch (operation)
    {

    case IOWA_DM_READ:
        for (i = 0 ; i < numData ; i++)
        {
            switch (dataP[i].resourceID)
            {
#ifndef IOWA_DEVICE_RSC_MANUFACTURER_REMOVE
            case IOWA_LWM2M_DEVICE_ID_MANUFACTURER:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->manufacturer);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->manufacturer;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_MODEL_NUMBER_REMOVE
            case IOWA_LWM2M_DEVICE_ID_MODEL_NUMBER:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->modelNumber);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->modelNumber;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_SERIAL_NUMBER_REMOVE
            case IOWA_LWM2M_DEVICE_ID_SERIAL_NUMBER:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->serialNumber);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->serialNumber;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_FIRMWARE_VERSION_REMOVE
            case IOWA_LWM2M_DEVICE_ID_FIRMWARE_VERSION:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->firmwareVersion);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->firmwareVersion;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
            case IOWA_LWM2M_DEVICE_ID_AVAILABLE_POWER_SRC:
            {
                device_power_source_t *powerSourceP;

                // Find corresponding resource instance
                powerSourceP = deviceDataP->powerSourceListP;
                while (powerSourceP != NULL)
                {
                    // If resource's instance found, memorize its type
                    if (dataP[i].resInstanceID == powerSourceP->resInstanceId)
                    {
                        dataP[i].value.asInteger = (int)powerSourceP->type;
                        break;
                    }
                    powerSourceP = powerSourceP->nextP;
                }
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
                if (powerSourceP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Power source ID not found.");
                    result = IOWA_COAP_404_NOT_FOUND;
                }
#endif
            }
            break;

            case IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE:
            {
                device_power_source_t *powerSourceP;

                // Find corresponding resource instance
                powerSourceP = deviceDataP->powerSourceListP;
                while (powerSourceP != NULL)
                {
                    // If resource's instance found, memorize its type
                    if (dataP[i].resInstanceID == powerSourceP->resInstanceId)
                    {
                        dataP[i].value.asInteger = (int64_t)powerSourceP->voltageValue;
                        break;
                    }
                    powerSourceP = powerSourceP->nextP;
                }
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
                if (powerSourceP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Power source ID not found.");
                    result = IOWA_COAP_404_NOT_FOUND;
                }
#endif
            }
            break;

            case IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT:
            {
                device_power_source_t *powerSourceP;

                // Find corresponding resource instance
                powerSourceP = deviceDataP->powerSourceListP;
                while (powerSourceP != NULL)
                {
                    // If resource's instance found, memorize its type
                    if (dataP[i].resInstanceID == powerSourceP->resInstanceId)
                    {
                        dataP[i].value.asInteger = (int64_t)powerSourceP->currentValue;
                        break;
                    }
                    powerSourceP = powerSourceP->nextP;
                }
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
                if (powerSourceP == NULL)
                {
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Power source ID not found.");
                    result = IOWA_COAP_404_NOT_FOUND;
                }
#endif
            }
            break;
#endif

#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
            case IOWA_LWM2M_DEVICE_ID_BATTERY_LEVEL:
                dataP[i].value.asInteger = deviceDataP->batteryLevel;
                break;
#endif

            case IOWA_LWM2M_DEVICE_ID_ERROR_CODE:
            {
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
                if (dataP[i].resInstanceID != 0 && (dataP[i].resInstanceID >= prv_getNumberOfBit(deviceDataP->errorCodesFlag)))
                {
                    IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Error code ID not found.");
                    result = IOWA_COAP_404_NOT_FOUND;
                }
#endif
                dataP[i].value.asInteger = (int64_t)prv_getErrorValue(deviceDataP->errorCodesFlag, dataP[i].resInstanceID);
            }
                break;

#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
            case IOWA_LWM2M_DEVICE_ID_CURRENT_TIME:
                dataP[i].type = IOWA_LWM2M_TYPE_TIME;
                dataP[i].value.asInteger = deviceDataP->currentTime;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
            case IOWA_LWM2M_DEVICE_ID_UTC_OFFSET:
                dataP[i].type = IOWA_LWM2M_TYPE_STRING;
                dataP[i].value.asBuffer.length = utilsStrlen(deviceDataP->utcOffsetP);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->utcOffsetP;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
            case IOWA_LWM2M_DEVICE_ID_TIME_ZONE:
                dataP[i].type = IOWA_LWM2M_TYPE_STRING;
                dataP[i].value.asBuffer.length = utilsStrlen(deviceDataP->timezoneP);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->timezoneP;
                break;
#endif

            case IOWA_LWM2M_DEVICE_ID_BINDING:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->binding);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->binding;
                break;

#ifndef IOWA_DEVICE_RSC_DEVICE_TYPE_REMOVE
            case IOWA_LWM2M_DEVICE_ID_TYPE:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->deviceType);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->deviceType;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_HARDWARE_VERSION_REMOVE
            case IOWA_LWM2M_DEVICE_ID_HARDWARE_VERSION:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->hardwareVersion);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->hardwareVersion;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_SOFTWARE_VERSION_REMOVE
            case IOWA_LWM2M_DEVICE_ID_SOFTWARE_VERSION:
                dataP[i].value.asBuffer.length = strlen(deviceDataP->softwareVersion);
                dataP[i].value.asBuffer.buffer = (uint8_t*)deviceDataP->softwareVersion;
                break;
#endif

#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
            case IOWA_LWM2M_DEVICE_ID_BATTERY_STATUS:
                dataP[i].value.asInteger = deviceDataP->batteryStatus;
                break;
#endif

            default:
                // Should not happen
                break;
            }
        }
        break;

    case IOWA_DM_WRITE:
        {
            iowa_device_time_info_t info;
            info.flags = 0;
            for (i = 0 ; i < numData ; i++)
            {
                switch (dataP[i].resourceID)
                {
#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
                case IOWA_LWM2M_DEVICE_ID_CURRENT_TIME:
                    if (dataP[i].value.asInteger < 0
                        || dataP[i].value.asInteger > UINT32_MAX)
                    {
                        IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "Current time value is outside the range [0; %d].", UINT32_MAX);
                        result = IOWA_COAP_400_BAD_REQUEST;
                        break;
                    }

                    if (deviceDataP->currentTime != dataP[i].value.asInteger)
                    {
                        deviceDataP->currentTime = dataP[i].value.asInteger;
                        info.flags |= IOWA_DEVICE_RSC_CURRENT_TIME;
                        info.currentTime = deviceDataP->currentTime;
                    }
                    break;
#endif

#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
                case IOWA_LWM2M_DEVICE_ID_UTC_OFFSET:
                    if (utilsCmpBufferWithString(dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length, deviceDataP->utcOffsetP) == false)
                    {
                        iowa_system_free(deviceDataP->utcOffsetP);
                        deviceDataP->utcOffsetP = utilsBufferToString(dataP[i].value.asBuffer.buffer,dataP[i].value.asBuffer.length);
                        info.flags |= IOWA_DEVICE_RSC_UTC_OFFSET;
                        info.utcOffsetP = deviceDataP->utcOffsetP;
                    }
                    break;
#endif

#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
                case IOWA_LWM2M_DEVICE_ID_TIME_ZONE:
                    if (utilsCmpBufferWithString(dataP[i].value.asBuffer.buffer, dataP[i].value.asBuffer.length, deviceDataP->timezoneP) == false)
                    {
                        iowa_system_free(deviceDataP->timezoneP);
                        deviceDataP->timezoneP = utilsBufferToString(dataP[i].value.asBuffer.buffer,dataP[i].value.asBuffer.length);
                        info.flags |= IOWA_DEVICE_RSC_TIMEZONE;
                        info.timezoneP = deviceDataP->timezoneP;
                    }
                    break;
#endif

                default:
                    // Should not happen
                    break;
                }
            }

            if (deviceDataP->dataTimeUpdateCallback != NULL)
            {
                deviceDataP->dataTimeUpdateCallback(&info, deviceDataP->callbackUserDataP, contextP);
            }
        }
        break;

    case IOWA_DM_EXECUTE:
        for (i = 0; i < numData; i++)
        {
            if (dataP[i].type != IOWA_LWM2M_TYPE_UNDEFINED)
            {
                // dataP[i].type can only be IOWA_LWM2M_TYPE_UNDEFINED or IOWA_LWM2M_TYPE_STRING with an Execute operation
                IOWA_LOG_ARG_WARNING(IOWA_PART_OBJECT, "No argument should be provided. Found: \"%.*s\".", dataP[i].value.asBuffer.length, dataP[i].value.asBuffer.buffer);
                result = IOWA_COAP_400_BAD_REQUEST;
                break;
            }

            switch (dataP[i].resourceID)
            {
            case IOWA_LWM2M_DEVICE_ID_REBOOT:
                contextP->action |= ACTION_REBOOT;
                break;

#ifndef IOWA_DEVICE_RSC_FACTORY_RESET_REMOVE
            case IOWA_LWM2M_DEVICE_ID_FACTORY_RESET:
                contextP->action |= ACTION_FACTORY_RESET;
                break;
#endif

            default:
                // Should not happen
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

static iowa_status_t prv_deviceResInstanceCallback(uint16_t objectID,
                                                   uint16_t instanceID,
                                                   uint16_t resourceID,
                                                   uint16_t *nbResInstanceP,
                                                   uint16_t **resInstanceArrayP,
                                                   void *userData,
                                                   iowa_context_t contextP)
{
    device_object_t *deviceDataP;

    (void)objectID;
    (void)instanceID;
    (void)contextP;

    deviceDataP = (device_object_t *)userData;

    CRIT_SECTION_ENTER(contextP);

    // Get number of ressource instance
    switch (resourceID)
    {
    case IOWA_LWM2M_DEVICE_ID_ERROR_CODE:
    {
        *nbResInstanceP = prv_getNumberOfBit(deviceDataP->errorCodesFlag);
        if (*nbResInstanceP == 0)
        {
            *nbResInstanceP = 1;// there is always a res instance id for no error code
        }
    }
        break;

#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
    case IOWA_LWM2M_DEVICE_ID_AVAILABLE_POWER_SRC:
    case IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE:
    case IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT:
    {
        device_power_source_t *powerSourceP;
        uint16_t i;

        i = 0;
        for (powerSourceP = deviceDataP->powerSourceListP; powerSourceP != NULL; powerSourceP = powerSourceP->nextP)
        {
            i++;
        }

        *nbResInstanceP = i;
        break;
    }
#endif

    default:
        // Should not happen
        break;
    }

    // Don't allocate an array with a length of zero
    if (*nbResInstanceP == 0)
    {
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_NO_ERROR;
    }

    // Allocate the array
    *resInstanceArrayP = (uint16_t *)iowa_system_malloc(*nbResInstanceP * sizeof(uint16_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (*resInstanceArrayP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(*nbResInstanceP * sizeof(uint16_t));
        CRIT_SECTION_LEAVE(contextP);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    // Retrieve the ressource instance ID
    switch (resourceID)
    {
    case IOWA_LWM2M_DEVICE_ID_ERROR_CODE:
    {
        uint16_t i;
        for (i = 0 ; i < *nbResInstanceP ; i++)
        {
            (*resInstanceArrayP)[i] = i;
        }
    }
        break;

#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
    case IOWA_LWM2M_DEVICE_ID_AVAILABLE_POWER_SRC:
    case IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE:
    case IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT:
    {
        device_power_source_t *powerSourceP;
        uint16_t i;

        i = 0;
        for (powerSourceP = deviceDataP->powerSourceListP; powerSourceP != NULL; powerSourceP = powerSourceP->nextP)
        {
            (*resInstanceArrayP)[i] = powerSourceP->resInstanceId;
            i++;
        }
        break;
    }
#endif

    default:
        // Should not happen
        break;
    }

    CRIT_SECTION_LEAVE(contextP);

    return IOWA_COAP_NO_ERROR;
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t objectDeviceInit(iowa_context_t contextP,
                               iowa_device_info_t *deviceInfo)
{
    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Adding new device object.");

    iowa_status_t result;
    iowa_lwm2m_resource_desc_t *resources;
    uint16_t resourcesNb;
    int currentPt;
    device_object_t *deviceDataP;

    if (objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID) != NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "A device object already exists.");

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    // Get the resources list
    resourcesNb = 3;

    if (deviceInfo != NULL)
    {
#ifndef IOWA_DEVICE_RSC_DEVICE_TYPE_REMOVE
        if (deviceInfo->deviceType != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Device Type resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_FIRMWARE_VERSION_REMOVE
        if (deviceInfo->firmwareVersion != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Firmware Version resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_HARDWARE_VERSION_REMOVE
        if (deviceInfo->hardwareVersion != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Hardware Version resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_MANUFACTURER_REMOVE
        if (deviceInfo->manufacturer != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Manufacturer resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_MODEL_NUMBER_REMOVE
        if (deviceInfo->modelNumber != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Model Number resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_SERIAL_NUMBER_REMOVE
        if (deviceInfo->serialNumber != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Serial Number resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_SOFTWARE_VERSION_REMOVE
        if (deviceInfo->softwareVersion != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Software Version resource is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_BATTERY) != 0)
        {
            resourcesNb = (uint16_t)(resourcesNb + DEVICE_OBJECT_OPT_RES_BATTERY_COUNT);
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_BATTERY is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_POWER_SOURCE) != 0)
        {
            resourcesNb = (uint16_t)(resourcesNb + DEVICE_OBJECT_OPT_RES_POWER_SOURCE_COUNT);
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_POWER_SOURCE is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_CURRENT_TIME) != 0)
        {
            resourcesNb ++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_CURRENT_TIME is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_UTC_OFFSET) != 0)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_UTC_OFFSET is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_TIMEZONE) != 0)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_TIMEZONE is not supported.");
#endif
#ifndef IOWA_DEVICE_RSC_FACTORY_RESET_REMOVE
        if (deviceInfo->factoryResetCallback != NULL)
        {
            resourcesNb++;
        }
#else
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_FACTORY_RESET is not supported.");
#endif
    }

    resources = (iowa_lwm2m_resource_desc_t *)iowa_system_malloc(resourcesNb*sizeof(iowa_lwm2m_resource_desc_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (resources == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(resourcesNb * sizeof(iowa_lwm2m_resource_desc_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    currentPt = 0;
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, REBOOT, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, ERROR_CODE, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, BINDING, resources, currentPt);

    if (deviceInfo != NULL)
    {
#ifndef IOWA_DEVICE_RSC_DEVICE_TYPE_REMOVE
        if (deviceInfo->deviceType != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, TYPE, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_FIRMWARE_VERSION_REMOVE
        if (deviceInfo->firmwareVersion != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, FIRMWARE_VERSION, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_HARDWARE_VERSION_REMOVE
        if (deviceInfo->hardwareVersion != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, HARDWARE_VERSION, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_MANUFACTURER_REMOVE
        if (deviceInfo->manufacturer != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, MANUFACTURER, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_MODEL_NUMBER_REMOVE
        if (deviceInfo->modelNumber != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, MODEL_NUMBER, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_SERIAL_NUMBER_REMOVE
        if (deviceInfo->serialNumber != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, SERIAL_NUMBER, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_SOFTWARE_VERSION_REMOVE
        if (deviceInfo->softwareVersion != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, SOFTWARE_VERSION, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_BATTERY) != 0)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, BATTERY_LEVEL, resources, currentPt);
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, BATTERY_STATUS, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_POWER_SOURCE) != 0)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, AVAILABLE_POWER_SRC, resources, currentPt);
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, POWER_SRC_VOLTAGE, resources, currentPt);
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, POWER_SRC_CURRENT, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_CURRENT_TIME) != 0)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, CURRENT_TIME, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_UTC_OFFSET) != 0)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, UTC_OFFSET, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_TIMEZONE) != 0)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, TIME_ZONE, resources, currentPt);
        }
#endif
#ifndef IOWA_DEVICE_RSC_FACTORY_RESET_REMOVE
        if (deviceInfo->factoryResetCallback != NULL)
        {
            SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_DEVICE, FACTORY_RESET, resources, currentPt);
        }
#endif
    }

    deviceDataP = (device_object_t *)iowa_system_malloc(sizeof(device_object_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(device_object_t));
        iowa_system_free(resources);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif
    memset(deviceDataP, 0, sizeof(device_object_t));

    if (deviceInfo != NULL)
    {
#ifndef IOWA_DEVICE_RSC_DEVICE_TYPE_REMOVE
        deviceDataP->deviceType = deviceInfo->deviceType;
#endif
#ifndef IOWA_DEVICE_RSC_FIRMWARE_VERSION_REMOVE
        deviceDataP->firmwareVersion = deviceInfo->firmwareVersion;
#endif
#ifndef IOWA_DEVICE_RSC_HARDWARE_VERSION_REMOVE
        deviceDataP->hardwareVersion = deviceInfo->hardwareVersion;
#endif
#ifndef IOWA_DEVICE_RSC_MANUFACTURER_REMOVE
        deviceDataP->manufacturer = deviceInfo->manufacturer;
#endif
#ifndef IOWA_DEVICE_RSC_MODEL_NUMBER_REMOVE
        deviceDataP->modelNumber = deviceInfo->modelNumber;
#endif
#ifndef IOWA_DEVICE_RSC_SERIAL_NUMBER_REMOVE
        deviceDataP->serialNumber = deviceInfo->serialNumber;
#endif
#ifndef IOWA_DEVICE_RSC_SOFTWARE_VERSION_REMOVE
        deviceDataP->softwareVersion = deviceInfo->softwareVersion;
#endif
        deviceDataP->optFlags = deviceInfo->optFlags;
#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
        deviceDataP->batteryStatus = IOWA_DEVICE_BATTERY_STATUS_UNKNOWN;
#endif
#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
        deviceDataP->currentTime = 0;
#endif
#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_UTC_OFFSET) != 0)
        {
            deviceDataP->utcOffsetP = utilsStrdup(deviceInfo->utcOffsetP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (deviceDataP->utcOffsetP == NULL
                && deviceInfo->utcOffsetP != NULL)
            {
                iowa_system_free(resources);
                iowa_system_free(deviceDataP);

                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }
#endif
#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
        if ((deviceInfo->optFlags & IOWA_DEVICE_RSC_TIMEZONE) != 0)
        {
            deviceDataP->timezoneP = utilsStrdup(deviceInfo->timezoneP);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
            if (deviceDataP->timezoneP == NULL
                && deviceInfo->timezoneP != NULL)
            {
                iowa_system_free(resources);
#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
                iowa_system_free(deviceDataP->utcOffsetP);
#endif
                iowa_system_free(deviceDataP);

                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
#endif
        }
#endif
        deviceDataP->dataTimeUpdateCallback = deviceInfo->dataTimeUpdateCallback;
#ifndef IOWA_DEVICE_RSC_FACTORY_RESET_REMOVE
        deviceDataP->factoryResetCallback = deviceInfo->factoryResetCallback;
#endif
        deviceDataP->callbackUserDataP = deviceInfo->callbackUserDataP;
    }
#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
    deviceDataP->powerSourceListP = NULL;
#endif

    // Set error code flag to no error
    deviceDataP->errorCodesFlag = 0x00;

    // Retrieve the binding
    deviceDataP->binding = ""
    // In LwM2M 1.0: list of transports supported + Queue mode
#if !defined(IOWA_UDP_SUPPORT) && !defined(IOWA_TCP_SUPPORT) && !defined(IOWA_LORAWAN_SUPPORT)
                           "SQ"
#else
#ifdef IOWA_UDP_SUPPORT
                           "UQ"
#endif
#endif // !defined(IOWA_UDP_SUPPORT) && !defined(IOWA_TCP_SUPPORT) && !defined(IOWA_LORAWAN_SUPPORT)
                             ;

    // Inform the stack
    result = customObjectAdd(contextP,
                             IOWA_LWM2M_DEVICE_OBJECT_ID,
                             OBJECT_SINGLE,
                             0, NULL,
                             resourcesNb, resources,
                             prv_deviceObjectCallback, NULL, prv_deviceResInstanceCallback,
                             deviceDataP);

    iowa_system_free(resources);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectDeviceClose(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;
    device_object_t *deviceDataP;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Removing device object.");

    // Remove the instance
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        // No error should occur if the object is not created
        return IOWA_COAP_NO_ERROR;
    }

#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
    iowa_system_free(deviceDataP->utcOffsetP);
#endif
#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
    iowa_system_free(deviceDataP->timezoneP);
#endif
    iowa_system_free(deviceDataP);

    // Inform the stack
    result = customObjectRemove(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

void objectDeviceFactoryReset(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    device_object_t *deviceDataP;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Initiating Factory Reset.");

    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_WARNING(IOWA_PART_OBJECT, "No device object has been created.");
        return;
    }

    CRIT_SECTION_LEAVE(contextP);
    deviceDataP->factoryResetCallback(deviceDataP->callbackUserDataP, contextP);
    CRIT_SECTION_ENTER(contextP);

    IOWA_LOG_TRACE(IOWA_PART_OBJECT, "Exiting.");
}

/*************************************************************************************
** Public functions
*************************************************************************************/

#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
iowa_status_t iowa_client_device_update_battery(iowa_context_t contextP,
                                                uint8_t batteryLevel,
                                                iowa_device_battery_status_t batteryStatus)
{
    device_object_t *deviceDataP;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Updating device battery.");

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    // Check arguments
    if (batteryLevel > 100)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "Battery level %u is outside the range [0, 100].", batteryLevel);
        return IOWA_COAP_406_NOT_ACCEPTABLE;
    }
#endif

    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    if (!(deviceDataP->optFlags & IOWA_DEVICE_RSC_BATTERY))
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The battery resource does not exist.");
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    clientNotificationLock(contextP, true);

    if (deviceDataP->batteryLevel != batteryLevel)
    {
        deviceDataP->batteryLevel = batteryLevel;
        customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_BATTERY_LEVEL);
    }

    if (deviceDataP->batteryStatus != batteryStatus)
    {
        deviceDataP->batteryStatus = batteryStatus;
        customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_BATTERY_STATUS);
    }

    clientNotificationLock(contextP, false);
    CRIT_SECTION_LEAVE(contextP);

    return IOWA_COAP_NO_ERROR;
}
#endif

iowa_status_t iowa_client_set_device_error_code(iowa_context_t contextP,
                                                uint8_t errorCode)
{
    device_object_t *deviceDataP;

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (errorCode > 8)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Error code must be part of [0:8].");
        return IOWA_COAP_402_BAD_OPTION;
    }
#endif

    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    if (errorCode == IOWA_ERROR_CODE_NO_ERROR)
    {
        if (deviceDataP->errorCodesFlag == 0x00)
        {
            IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Device doesn't get any error.");
            CRIT_SECTION_LEAVE(contextP);

            return IOWA_COAP_404_NOT_FOUND;
        }
        deviceDataP->errorCodesFlag = 0x00;
    }
    else
    {
        uint8_t errorCodeFlag;

        errorCodeFlag = (uint8_t)0x01 << (errorCode - 1);
        if ((errorCodeFlag & deviceDataP->errorCodesFlag) > 0)
        {
            IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "The device already gets error %d.", errorCode);
            CRIT_SECTION_LEAVE(contextP);

            return IOWA_COAP_409_CONFLICT;
        }
        deviceDataP->errorCodesFlag |= errorCodeFlag;
    }

    clientNotificationLock(contextP, true);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_ERROR_CODE);
    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);
    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Set error code succeed (%d).", errorCode);

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t iowa_client_clear_device_error_code(iowa_context_t contextP,
                                                   uint8_t errorCode)
{
    device_object_t *deviceDataP;
    uint8_t errorCodeFlag;

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (errorCode < 1 || errorCode > 8)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Error code must be part of [1:8]. Cannot clear IOWA_ERROR_CODE_NO_ERROR error.");
        return IOWA_COAP_402_BAD_OPTION;
    }
#endif

    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    errorCodeFlag = (uint8_t)0x01 << (errorCode - 1);
    if ((errorCodeFlag & deviceDataP->errorCodesFlag) == 0)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_OBJECT, "Device doesn't get error %d.", errorCode);
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_404_NOT_FOUND;
    }

    deviceDataP->errorCodesFlag ^= errorCodeFlag;

    clientNotificationLock(contextP, true);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_ERROR_CODE);
    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);
    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Clear error code %d succeed.", errorCode);

    return IOWA_COAP_NO_ERROR;
}

#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
iowa_status_t iowa_client_add_device_power_source(iowa_context_t contextP,
                                                  iowa_power_source_type_t type,
                                                  int voltageValue,
                                                  int currentValue,
                                                  iowa_sensor_t *idP)
{
    device_object_t *deviceDataP;
    uint16_t resInstanceId;
    device_power_source_t *powerSourceP;

    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    if ((deviceDataP->optFlags & IOWA_DEVICE_RSC_POWER_SOURCE) == 0)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "IOWA_DEVICE_RSC_POWER_SOURCE is not set in iowa_client_configure().");
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    // Find resource instance non used
    resInstanceId = 0;
    powerSourceP = deviceDataP->powerSourceListP;

    while (powerSourceP != NULL)
    {
        if (resInstanceId == powerSourceP->resInstanceId)
        {
            powerSourceP = deviceDataP->powerSourceListP;
            resInstanceId++;
        }
        else
        {
            powerSourceP = powerSourceP->nextP;
        }
    }

    // Create new power source
    powerSourceP = (device_power_source_t *)iowa_system_malloc(sizeof(device_power_source_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (powerSourceP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR_MALLOC(sizeof(device_power_source_t));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    // Add new power source values
    powerSourceP->nextP = NULL;
    powerSourceP->resInstanceId = resInstanceId;
    powerSourceP->type = type;
    powerSourceP->voltageValue = voltageValue;
    powerSourceP->currentValue = currentValue;

    // Add new power source to list
    deviceDataP->powerSourceListP = (device_power_source_t *)IOWA_UTILS_LIST_ADD(deviceDataP->powerSourceListP, powerSourceP);

    (*idP) = OBJECT_INSTANCE_ID_TO_SENSOR(IOWA_LWM2M_DEVICE_OBJECT_ID, resInstanceId);

    clientNotificationLock(contextP, true);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_AVAILABLE_POWER_SRC);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT);
    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);
    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Add new power source on resource instance %d succeed (%d mV, %d mA).", GET_INSTANCE_ID_FROM_SENSOR(*idP), powerSourceP->voltageValue, powerSourceP->currentValue);

    return IOWA_COAP_NO_ERROR;
}
#endif

#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
iowa_status_t iowa_client_remove_device_power_source(iowa_context_t contextP,
                                                     iowa_sensor_t id)
{
    device_object_t *deviceDataP;
    device_power_source_t *powerSourceP;

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    // Check Object Id
    if (GET_OBJECT_ID_FROM_SENSOR(id) != IOWA_LWM2M_DEVICE_OBJECT_ID)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "id is wrong.");
        return IOWA_COAP_404_NOT_FOUND;
    }
#endif

    // Check Object and Object instance
    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    if ((deviceDataP->optFlags & IOWA_DEVICE_RSC_POWER_SOURCE) == 0)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "IOWA_DEVICE_RSC_POWER_SOURCE is not set in iowa_client_configure().");
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    // Find corresponding resource instance
    powerSourceP = deviceDataP->powerSourceListP;
    while (powerSourceP != NULL
           && powerSourceP->resInstanceId != GET_INSTANCE_ID_FROM_SENSOR(id))
    {
        powerSourceP = powerSourceP->nextP;
    }
    if (powerSourceP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Power source ID not found.");
        return IOWA_COAP_404_NOT_FOUND;
    }

    // Remove power source from list
    deviceDataP->powerSourceListP = (device_power_source_t *)IOWA_UTILS_LIST_REMOVE(deviceDataP->powerSourceListP, powerSourceP);
    iowa_system_free(powerSourceP);

    clientNotificationLock(contextP, true);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_AVAILABLE_POWER_SRC);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE);
    customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT);
    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);
    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Remove power source on resource instance %d succeed.",GET_INSTANCE_ID_FROM_SENSOR(id));

    return IOWA_COAP_NO_ERROR;
}
#endif

#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
iowa_status_t iowa_client_update_device_power_source(iowa_context_t contextP,
                                                     iowa_sensor_t id,
                                                     int voltageValue,
                                                     int currentValue)
{

    device_object_t *deviceDataP;
    device_power_source_t *powerSourceP;

#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    // Check Object Id
    if (GET_OBJECT_ID_FROM_SENSOR(id) != IOWA_LWM2M_DEVICE_OBJECT_ID)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "id is wrong.");
        return IOWA_COAP_404_NOT_FOUND;
    }
#endif

    // Check Object and Object instance
    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");
        CRIT_SECTION_LEAVE(contextP);

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    if ((deviceDataP->optFlags & IOWA_DEVICE_RSC_POWER_SOURCE) == 0)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "IOWA_DEVICE_RSC_POWER_SOURCE is not set in iowa_client_configure().");
        return IOWA_COAP_405_METHOD_NOT_ALLOWED;
    }

    // Find corresponding resource instance
    powerSourceP = deviceDataP->powerSourceListP;
    while (powerSourceP != NULL
           && powerSourceP->resInstanceId != GET_INSTANCE_ID_FROM_SENSOR(id))
    {
        powerSourceP = powerSourceP->nextP;
    }
#ifndef IOWA_CONFIG_SKIP_ARGS_CHECK
    if (powerSourceP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "Power source ID not found.");
        return IOWA_COAP_404_NOT_FOUND;
    }
#endif

    clientNotificationLock(contextP, true);

    if (powerSourceP->voltageValue != voltageValue)
    {
        powerSourceP->voltageValue = voltageValue;
        customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_POWER_SRC_VOLTAGE);
    }
    if (powerSourceP->currentValue != currentValue)
    {
        powerSourceP->currentValue = currentValue;
        customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_POWER_SRC_CURRENT);
    }
    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);
    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Update power source on resource instance %d succeed (%d mV, %d mA).",GET_INSTANCE_ID_FROM_SENSOR(id), powerSourceP->voltageValue, powerSourceP->currentValue);

    return IOWA_COAP_NO_ERROR;
}
#endif

iowa_status_t iowa_client_update_device_time_information(iowa_context_t contextP,
                                                         iowa_device_time_info_t *timeInfoP)
{
    device_object_t *deviceDataP;

    // Check Object and Object instance
    CRIT_SECTION_ENTER(contextP);
    deviceDataP = (device_object_t *)objectGetData(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID);
    if (deviceDataP == NULL)
    {
        CRIT_SECTION_LEAVE(contextP);
        IOWA_LOG_ERROR(IOWA_PART_OBJECT, "The device is not instanciated. Call first iowa_client_configure().");

        return IOWA_COAP_412_PRECONDITION_FAILED;
    }

    clientNotificationLock(contextP, true);
#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
    if ((deviceDataP->optFlags & IOWA_DEVICE_RSC_CURRENT_TIME) != 0
        && (timeInfoP->flags & IOWA_DEVICE_RSC_CURRENT_TIME) != 0)
    {
        if (deviceDataP->currentTime != timeInfoP->currentTime)
        {
            deviceDataP->currentTime = timeInfoP->currentTime;
            customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_CURRENT_TIME);
        }
    }
#else
    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_CURRENT_TIME is not supported.");
#endif

#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
    if ((deviceDataP->optFlags & IOWA_DEVICE_RSC_UTC_OFFSET) != 0
        && (timeInfoP->flags & IOWA_DEVICE_RSC_UTC_OFFSET) != 0)
    {
        iowa_system_free(deviceDataP->utcOffsetP);
        deviceDataP->utcOffsetP = utilsStrdup(timeInfoP->utcOffsetP);
        customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_UTC_OFFSET);
    }
#else
    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_UTC_OFFSET is not supported.");
#endif

#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
    if ((deviceDataP->optFlags & IOWA_DEVICE_RSC_TIMEZONE) != 0
        && (timeInfoP->flags & IOWA_DEVICE_RSC_TIMEZONE) != 0)
    {
        iowa_system_free(deviceDataP->timezoneP);
        deviceDataP->timezoneP = utilsStrdup(timeInfoP->timezoneP);
        customObjectResourceChanged(contextP, IOWA_LWM2M_DEVICE_OBJECT_ID, DEVICE_OBJECT_DEFAULT_INSTANCE_ID, IOWA_LWM2M_DEVICE_ID_TIME_ZONE);
    }
#else
    IOWA_LOG_WARNING(IOWA_PART_OBJECT, "Resource IOWA_DEVICE_RSC_TIMEZONE is not supported.");
#endif

    clientNotificationLock(contextP, false);

    CRIT_SECTION_LEAVE(contextP);
    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Update time information succeed.");

    return IOWA_COAP_NO_ERROR;
}

#endif // LWM2M_CLIENT_MODE
