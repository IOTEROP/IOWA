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
* Copyright (c) 2017-2020 IoTerop.
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

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_config.h"

#include "iowa_prv_core.h"
#include "iowa_prv_lwm2m_internals.h"

#include "iowa_at_command.h"
#include "iowa_cellular_connectivity.h"
#include "iowa_connectivity_monitoring.h"
#include "iowa_connectivity_stats.h"
#include "iowa_bearer_selection.h"
#include "iowa_digital_output.h"
#include "iowa_dimmer.h"
#include "iowa_ipso.h"
#include "iowa_light_control.h"
#include "iowa_firmware_update.h"
#include "iowa_mqtt_objects.h"
#include "iowa_software_component.h"

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
 * Bearer Selection: Data Structures and Constants
 **************************************************************/

typedef struct
{
    uint16_t                                         optFlags;
    iowa_bearer_selection_info_t                     info;
#ifdef IOWA_BEARER_SELECTION_SUPPORT_RSC_PREFERRED_COMM_BEARER
    uint16_t                                        *preferredCommBearerIdList;
#endif
    void                                            *userDataCallback;
    iowa_bearer_selection_update_state_callback_t    updateStateCallback;
} bearer_selection_object_t;

/**************************************************************
 * Cellular Connectivity: Data Structures and Constants
 **************************************************************/

typedef struct
{
    uint16_t                                             optFlags;
    iowa_cellular_connectivity_info_t                    info;
    void                                                *userDataCallback;
    iowa_cellular_connectivity_update_state_callback_t   updateStateCallback;
} cellular_connectivity_object_t;

/**************************************************************
 * Connectivity monitoring: Data Structures and Constants
 **************************************************************/

typedef struct
{
    uint16_t                            optFlags;
    iowa_connectivity_monitoring_info_t info;
} connectivity_monitoring_object_t;

/**************************************************************
 * Connectivity stats: Data Structures and Constants
 **************************************************************/

typedef struct
{
    uint16_t optFlags;
    bool     running;
    int32_t  runningBeginTime;
    size_t   ipDataCount;
    size_t   smsTxCounter;
    size_t   smsRxCounter;
    size_t   txData;
    size_t   rxData;
    size_t   maxMessageSize;
    size_t   averageMessageSize;
    int      collectionPeriod;
} connectivity_stats_object_t;

/**************************************************************
 * Device: Data Structures and Constants
 **************************************************************/

#ifdef IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE
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
#ifdef IOWA_DEVICE_SUPPORT_RSC_MANUFACTURER
    const char                   *manufacturer;
#endif
    uint32_t                      errorCodesFlag;
#ifdef IOWA_DEVICE_SUPPORT_RSC_MODEL_NUMBER
    const char                   *modelNumber;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_SERIAL_NUMBER
    const char                   *serialNumber;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_FIRMWARE_VERSION
    const char                   *firmwareVersion;
#endif
    const char                   *binding;
#ifdef IOWA_DEVICE_SUPPORT_RSC_DEVICE_TYPE
    const char                   *deviceType;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_HARDWARE_VERSION
    const char                   *hardwareVersion;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_SOFTWARE_VERSION
    const char                   *softwareVersion;
#endif
    uint16_t                      optFlags;
#ifdef IOWA_DEVICE_SUPPORT_RSC_POWER_SOURCE
    device_power_source_t        *powerSourceListP;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_BATTERY
    uint8_t                       batteryLevel;
    iowa_device_battery_status_t  batteryStatus;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_CURRENT_TIME
    int32_t                       currentTime;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_UTC_OFFSET
    char                         *utcOffsetP;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_TIMEZONE
    char                         *timezoneP;
#endif
    uint32_t                      memoryTotal;
    uint32_t                      memoryFree;
    size_t                        extLinkCount;
    iowa_lwm2m_object_link_t     *extLinkArray;
#if defined(IOWA_DEVICE_SUPPORT_RSC_CURRENT_TIME) || defined(IOWA_DEVICE_SUPPORT_RSC_UTC_OFFSET) || defined(IOWA_DEVICE_SUPPORT_RSC_TIMEZONE)
    iowa_client_time_update_callback_t    dataTimeUpdateCallback;
#endif
#ifdef IOWA_DEVICE_SUPPORT_RSC_FACTORY_RESET
    iowa_client_factory_reset_callback_t  factoryResetCallback;
#endif
    void                                 *callbackUserDataP;
} device_object_t;

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

typedef struct _gps_instance_t
{
    struct _gps_instance_t *next;
    uint16_t  id;
    uint16_t  optFlags;
    char     *latitude;
    char     *longitude;
    char     *uncertainty;
    float     compassDirection;
    size_t    velocityLength;
    uint8_t  *velocity;
    int32_t   timestamp;
    char     *applicationType;
} gps_instance_t;

typedef struct
{
    gps_instance_t *instanceList;
} gps_object_t;

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
 * Light Control: Data Structures and Constants
 **************************************************************/

typedef struct _light_control_instance_t
{
    struct _light_control_instance_t *next;
    uint16_t  id;
    uint16_t  optFlags;
    bool      powerOn;
#ifdef IOWA_LIGHT_CONTROL_SUPPORT_RSC_DIMMER
    uint8_t       dimmer;
#endif
#ifdef IOWA_LIGHT_CONTROL_SUPPORT_RSC_ON_TIME
    uint32_t       timePowerOn;
    uint32_t       onTime;
#endif
#ifdef IOWA_LIGHT_CONTROL_SUPPORT_RSC_POWER_FACTOR
    float     powerFactor;
#endif
#ifdef IOWA_LIGHT_CONTROL_SUPPORT_RSC_CUMULATIVE_ACTIVE_POWER
    float     cumulativeActivePower;
#endif
#ifdef IOWA_LIGHT_CONTROL_SUPPORT_RSC_COLOUR
    char     *colour;
    char     *sensorUnits;
#endif
    void     *userDataCallback;
    iowa_light_control_update_state_callback_t updateStateCallback;
} light_control_instance_t;

typedef struct
{
    light_control_instance_t *instanceList;
} light_control_object_t;

/**************************************************************
 * Magnetometer : Data Structures and Constants
 **************************************************************/

typedef struct _magnetometer_instance_t
{
    struct _magnetometer_instance_t *next;
    uint16_t  id;
    int       optFlags;
    float     xValue;
    float     yValue;
    float     zValue;
    float     compassDirection;
    char     *sensorUnits;
} magnetometer_instance_t;

typedef struct
{
    magnetometer_instance_t *instanceList;
} magnetometer_object_t;

/**************************************************************
 * Digital output : Data Structures and Constants
 **************************************************************/

typedef struct _digital_output_instance_t
{
    struct _digital_output_instance_t *next;
    uint16_t  id;
    uint16_t  optFlags;
    bool      state;
    char     *applicationType;
    bool      polarity;
    void     *userDataCallback;
    iowa_digital_output_state_callback_t updateStateCallback;
} digital_output_instance_t;

typedef struct
{
    digital_output_instance_t *instanceList;
} digital_output_object_t;

/**************************************************************
 * Dimmer : Data Structures and Constants
 **************************************************************/

typedef struct _dimmer_instance_t
{
    struct _dimmer_instance_t *next;
    uint16_t  id;
    int       optFlags;
    float     level;
    int32_t   timePowerOn;
    int       onTime;
    int32_t   timePowerOff;
    int       offTime;
    char     *applicationType;
    void     *userDataCallback;
    iowa_dimmer_state_callback_t updateStateCallback;
} dimmer_instance_t;

typedef struct
{
    dimmer_instance_t *instanceList;
} dimmer_object_t;

/**************************************************************
 * Firmware Update : Data Structures and Constants
 **************************************************************/

#define LWM2M_FIRMWARE_PROTOCOL_SUPPORT_COUNT 6

typedef struct
{
    iowa_fw_state_t   state;
    iowa_fw_status_t  result;
    uint8_t           method;
    char             *uri;
    char             *name;
    char             *version;
    uint8_t           protocolSupportCount;
    uint8_t           protocolSupports[LWM2M_FIRMWARE_PROTOCOL_SUPPORT_COUNT];
    iowa_fw_download_callback_t  downloadCb;
    iowa_fw_write_callback_t     writeCb;
    iowa_fw_update_callback_t    updateCb;
    void                        *userData;
} firmware_object_t;

#define LWM2M_FIRMWARE_STATE_UNCHANGED   0xFF

#define LWM2M_FIRMWARE_METHOD_PULL 0
#define LWM2M_FIRMWARE_METHOD_PUSH 1
#define LWM2M_FIRMWARE_METHOD_BOTH 2

/**************************************************************
 * Location: Data Structures and Constants
 **************************************************************/

#define PRV_LOCATION_RSC_MASK 0x000F

typedef struct
{
    uint16_t  optFlags;
    float     latitude;
    float     longitude;
    float     altitude;
    float     radius;
    size_t    velocityLength;
    uint8_t  *velocity;
    int32_t   timestamp;
    float     speed;
} location_object_t;

/**************************************************************
 * MQTT Broker: Data Structures and Constants
 **************************************************************/

typedef struct _mqtt_broker_instance_t
{
    struct _mqtt_broker_instance_t *next;
    uint16_t                        id;
    uint16_t                        optFlags;
    iowa_mqtt_broker_t             *brokerP;
} mqtt_broker_instance_t;

typedef struct
{
    mqtt_broker_instance_t             *instanceList;
    iowa_mqtt_broker_update_callback_t  callback;
    void                               *userData;
} mqtt_broker_object_t;

/**************************************************************
 * MQTT Publication: Data Structures and Constants
 **************************************************************/

typedef struct _mqtt_publication_instance_t
{
    struct _mqtt_publication_instance_t        *next;
    uint16_t                                    id;
    uint16_t                                    optFlags;
    iowa_mqtt_publication_t                    *publicationP;
} mqtt_publication_instance_t;

typedef struct
{
    mqtt_publication_instance_t             *instanceList;
    iowa_mqtt_publication_update_callback_t  callback;
    void                                    *userData;
} mqtt_publication_object_t;

/**************************************************************
 * AT Command: Data Structures and Constants
 **************************************************************/

typedef struct _at_command_instance_t
{
    struct _at_command_instance_t  *next;
    uint16_t                        id;
    uint16_t                        optFlags;
    char                           *command;
    char                           *response;
    char                           *status;
    int32_t                        timeout;
    void                           *userDataCallback;
    iowa_at_command_run_t           run;
} at_command_instance_t;

typedef struct
{
    at_command_instance_t *instanceList;
} at_command_object_t;

/**************************************************************
 * Gyrometer: Data Structures and Constants
 **************************************************************/

typedef struct _gyrometer_instance_t
{
    struct _gyrometer_instance_t *next;
    uint16_t  id;
    uint16_t  optFlags;
    float     xValue;
    float     yValue;
    float     zValue;
    char     *sensorUnits;
    float     minXValue;
    float     maxXValue;
    float     minYValue;
    float     maxYValue;
    float     minZValue;
    float     maxZValue;
    float     minRangeValue;
    float     maxRangeValue;
    char     *applicationType;
} gyrometer_instance_t;

typedef struct
{
    gyrometer_instance_t *instanceList;
} gyrometer_object_t;

/**************************************************************
 * Software Component: Data Structures and Constants
 **************************************************************/

// Software component instance information
// Elements:
// - next: next instance.
// - id: instance id.
// - actionState: Use for callback called selection (PRV_ACTION_STATE_xx).
// - activationState: indicate if component is activated.
// - infoP: software component information.
typedef struct _sw_cmp_instance_t
{
    struct _sw_cmp_instance_t  *next;   // matches lwm2m_list_t::next
    uint16_t                    id;     // matches lwm2m_list_t::id
    // Use for callback called selection
    uint8_t                     actionState;
    // Optional Resources
    bool                        activationState;
    iowa_sw_cmp_info_t         *infoP;
} sw_cmp_instance_t;

// Software component object information
// Elements:
// - instanceListP: List of software component instances.
// - activationCb: The activate callback, called when the Server requests the device to activate or deactivate the software component.
// - userDataP: the data passed to callback.
typedef struct
{
    sw_cmp_instance_t                  *instanceListP;
    iowa_sw_cmp_update_callback_t       updateCb;
    iowa_sw_cmp_activation_callback_t   activationCb;
    void                               *userDataP;
} sw_cmp_object_t;

/**************************************************************
* Common Constants
**************************************************************/

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

// Add resource ID to ressources Array
// Parameters:
// OBJECT: resource Object Name
// ARRAY: ressources Array
// PT: array's index
// RES: ressource Name
#define ADD_RES_ID_TO_ARRAY(OBJECT, ARRAY, PT, RES) (PT)++; (ARRAY)[(PT)] = OBJECT##_ID_##RES

typedef struct
{
    list_16_bits_id_t *instanceList;
} object_data_t;

// Get data in a object.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
// - objectID: the id of an object.
void * objectGetData(iowa_context_t contextP,
                     uint16_t objectId);

// Get data in a instance.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
// - resourceID: the resource.
// - instanceID: the instance.
void * objectGetInstanceData(iowa_context_t contextP,
                             uint16_t objectId,
                             uint16_t instanceId);

// Set resources description.
// Returned value: none.
// Parameters:
// - rscDescP: an array of iowa_lwm2m_resource_desc_t composing the Object.
// - resId: the resourceID.
// - resType: the resource type.
// - resOp: the resource operation.
// - flags: the resource flags.
void objectSetRscDesc(iowa_lwm2m_resource_desc_t *rscDescP,
                      int *ptP,
                      uint16_t resId,
                      iowa_lwm2m_data_type_t resType,
                      uint8_t resOp,
                      uint8_t flags);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_PRV_OBJECT_INTERNALS_
