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
* Copyright (c) 2017-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_OBJECT_INTERNALS_
#define _IOWA_PRV_OBJECT_INTERNALS_

#include "iowa_config.h"

#include "iowa_prv_core.h"
#include "iowa_prv_lwm2m_internals.h"

#include "iowa_ipso.h"

/**************************************************************
 * Accelerometer: Data Structures and Constants
 **************************************************************/

typedef struct _accelerometer_instance_t
{
    struct _accelerometer_instance_t *next;
    uint16_t  id;
    int       optFlags;
    float     xValue;
    float     yValue;
    float     zValue;
    char     *sensorUnits;
    float     minRangeValue;
    float     maxRangeValue;
} accelerometer_instance_t;

typedef struct
{
    accelerometer_instance_t *instanceList;
} accelerometer_object_t;

/**************************************************************
 * Device: Data Structures and Constants
 **************************************************************/
#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
typedef struct _device_power_source_t
{
    struct _device_power_source_t *nextP;
    uint16_t                 resInstanceId;
    iowa_power_source_type_t type;
    int                      voltageValue;
    int                      currentValue;
} device_power_source_t;
#endif

typedef struct
{
#ifndef IOWA_DEVICE_RSC_MANUFACTURER_REMOVE
    const char                   *manufacturer;
#endif
    uint8_t                       errorCodesFlag;
#ifndef IOWA_DEVICE_RSC_MODEL_NUMBER_REMOVE
    const char                   *modelNumber;
#endif
#ifndef IOWA_DEVICE_RSC_SERIAL_NUMBER_REMOVE
    const char                   *serialNumber;
#endif
#ifndef IOWA_DEVICE_RSC_FIRMWARE_VERSION_REMOVE
    const char                   *firmwareVersion;
#endif
    const char                   *binding;
#ifndef IOWA_DEVICE_RSC_DEVICE_TYPE_REMOVE
    const char                   *deviceType;
#endif
#ifndef IOWA_DEVICE_RSC_HARDWARE_VERSION_REMOVE
    const char                   *hardwareVersion;
#endif
#ifndef IOWA_DEVICE_RSC_SOFTWARE_VERSION_REMOVE
    const char                   *softwareVersion;
#endif
    uint16_t                      optFlags;
#ifndef IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE
    device_power_source_t        *powerSourceListP;
#endif
#ifndef IOWA_DEVICE_RSC_BATTERY_REMOVE
    uint8_t                       batteryLevel;
    iowa_device_battery_status_t  batteryStatus;
#endif
#ifndef IOWA_DEVICE_RSC_CURRENT_TIME_REMOVE
    int                           currentTime;
#endif
#ifndef IOWA_DEVICE_RSC_UTC_OFFSET_REMOVE
    char                         *utcOffsetP;
#endif
#ifndef IOWA_DEVICE_RSC_TIMEZONE_REMOVE
    char                         *timezoneP;
#endif
    iowa_client_time_update_callback_t    dataTimeUpdateCallback;
#ifndef IOWA_DEVICE_RSC_FACTORY_RESET_REMOVE
    iowa_client_factory_reset_callback_t  factoryResetCallback;
#endif
    void                                 *callbackUserDataP;
} device_object_t;

/**************************************************************
 * IPSO: Data Structures and Constants
 **************************************************************/

typedef struct _ipso_instance_t
{
    struct _ipso_instance_t *next;
    uint16_t  id;
    float     value;
    int       timePowerOn;
    int       onTime;
    int       timePowerOff;
    int       offTime;
    float     min;
    float     max;
    char     *unit;
    char     *appType;
    float     rangeMin;
    float     rangeMax;

} ipso_instance_t;

typedef struct
{
    ipso_instance_t *instanceList;
} ipso_object_t;

/**************************************************************
 * Location: Data Structures and Constants
 **************************************************************/

#define PRV_LOCATION_RSC_MASK 0x000F

typedef struct _location_instance_t
{
    struct _location_instance_t *next;
    uint16_t  id;
    uint16_t  optFlags;
    float     latitude;
    float     longitude;
    float     altitude;
    float     radius;
    size_t    velocityLength;
    uint8_t  *velocity;
    int32_t   timestamp;
    float     speed;
} location_instance_t;

typedef struct
{
    location_instance_t *instanceList;
} location_object_t;

/**************************************************************
* Common Constants
**************************************************************/

#define DEVICE_OBJECT_DEFAULT_INSTANCE_ID 0

#define DEVICE_OBJECT_OPT_RES_BATTERY_COUNT 2
#define DEVICE_OBJECT_OPT_RES_POWER_SOURCE_COUNT 3

/**************************************************************
* Common API
**************************************************************/

// This is a macro to fill an iowa_lwm2m_desc_t with the values of LWM2M Reusable Resources
#define SET_LWM2M_DESC_T_TO_OBJECT_RSC(OBJECT, RES, desc, pt) \
{                                                             \
    objectSetRscDesc(desc,                                    \
                     &pt,                                     \
                     OBJECT##_ID_##RES,                       \
                     OBJECT##_TYPE_##RES,                     \
                     OBJECT##_OP_##RES,                       \
                     OBJECT##_FLAGS_##RES);                   \
}

#define SET_LWM2M_DESC_T_TO_IPSO_RSC(desc, pt, RES) SET_LWM2M_DESC_T_TO_OBJECT_RSC(IPSO_RSC, RES, desc, pt)

#define OBJECT_LWM2M_RESOURCE_DESC(OBJECT, RES) { IOWA_LWM2M_##OBJECT##_ID_##RES, IOWA_LWM2M_##OBJECT##_TYPE_##RES, IOWA_LWM2M_##OBJECT##_OP_##RES, IOWA_LWM2M_##OBJECT##_FLAGS_##RES }

// Add resource object information to buffer and increment its index
// Parameters:
// OBJECT: resource Object Name
// RSC: resource Name
// DEST: buffer of iowa_lwm2m_resource_desc_t
// IND: buffer's index
// Note: work like SET_LWM2M_DESC_T_TO_OBJECT_RSC but to configure object with multiple advanced type
#define SET_LWM2M_RSC_FOR_NEW_OBJ_TO_DEST(OBJECT,RSC,DEST,IND)              \
{                                                                           \
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_##OBJECT, RSC, DEST, IND);    \
}

// Add resource ID to buffer and increment its index
// Parameters:
// OBJECT: resource Object Name
// RSC: resource Name
// DEST: buffer of uint16_t
// IND: buffer's index
// Note: work like SET_LWM2M_DESC_T_TO_OBJECT_RSC but to add new instance of object with multiple advanced type
#define SET_LWM2M_RSC_FOR_NEW_INST_OF_MULTI_OBJ_TO_DEST(OBJECT,RSC,DEST,IND)    \
{                                                                               \
    DEST[IND] = IOWA_LWM2M_##OBJECT##_ID_##RSC;                                 \
    IND+=1;                                                                     \
}

typedef struct
{
    list_16_bits_id_t *instanceList;
} object_data_t;

void * objectGetData(iowa_context_t contextP,
                     uint16_t objectId);
void * objectGetInstanceData(iowa_context_t contextP,
                             uint16_t objectId,
                             uint16_t instanceId);
void objectSetRscDesc(iowa_lwm2m_resource_desc_t *rscDescP,
                      int *ptP,
                      uint16_t resId,
                      iowa_lwm2m_data_type_t resType,
                      uint8_t resOp,
                      uint8_t flags);

#endif // _IOWA_PRV_OBJECT_INTERNALS_
