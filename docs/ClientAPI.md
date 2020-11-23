# Client Mode API Reference

The functions explained below are defined inside the file *include/iowa_client.h* and the Objects folder *include/objects*.

## Client pseudo code

```c
#include "iowa_client.h"
#include "iowa_ipso.h"

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    iowa_device_info_t devInfo;
    iowa_sensor_t sensorId;

    /******************
    * Initialization
    */

    iowaH = iowa_init(NULL);

    devInfo.manufacturer = "IOTEROP";
    devInfo.deviceType = "Example device";
    devInfo.modelNumber = "1";
    devInfo.serialNumber = NULL;
    devInfo.hardwareVersion = NULL;
    devInfo.softwareVersion = NULL;
    devInfo.optFlags = 0;
    result = iowa_client_configure(iowaH, "IOWA_Sample_Client", devInfo, NULL);

    result = iowa_client_IPSO_add_sensor(iowaH,
                                         IOWA_IPSO_VOLTAGE, 12.0,
                                         "V", "Test DC", 0.0, 0.0,
                                         &sensorId);

    result = iowa_client_add_server(iowaH, 1234, "coap://localhost:5683", 0, 0, IOWA_SEC_NONE);

    /******************
    * "Main loop"
    */

    while (result == IOWA_COAP_NO_ERROR)
    {
        float sensorValue;

        result = iowa_step(iowaH, 5);

        sensorValue = read_battery_voltage();
        result = iowa_client_IPSO_update_value(iowaH,
                                               sensorId,
                                               sensorValue);
    }

    iowa_client_IPSO_remove_sensor(iowaH, sensorId);
    iowa_close(iowaH);

    return 0;
}
```

\clearpage

## Data types

### iowa_server_setting_id_t

```c
typedef uint8_t iowa_server_setting_id_t;
```

#### Possible Values

**IOWA_SERVER_SETTING_QUEUE_MODE**
: A flag to set the queue mode of a server.

**IOWA_SERVER_SETTING_LIFETIME**
: A flag to set the lifetime of a server.

### iowa_lwm2m_binding_t

```c
typedef uint8_t iowa_lwm2m_binding_t;
```

**IOWA_LWM2M_BINDING_UNKNOWN**
: The flag to define Unknown binding.

**IOWA_LWM2M_BINDING_UDP**
: The flag to define UDP binding.

**IOWA_LWM2M_BINDING_TCP**
: The flag to define TCP binding.

**IOWA_LWM2M_BINDING_SMS**
: The flag to define SMS binding.

**IOWA_LWM2M_BINDING_NON_IP**
: The flag to define Non-IP binding.

### iowa_server_info_t

```c
typedef struct
{
    uint16_t                      shortId;
    const char                   *uriP;
    int32_t                       lifetime;
    iowa_lwm2m_binding_t          binding;
    bool                          queueMode;
} iowa_server_info_t;
```

The `iowa_server_info_t` structure is used to get the shortID and the parameters of the configured servers.

shortId
: the Short ID assigned to the server.

uriP
: The uri to connect with the server.

lifetime
: The lifetime in seconds of the registration to the server.

binding
: The binding used to connect to the server.

queueMode
: The state of the Queue mode of the server.

### iowa_device_info_t

```c
typedef struct
{
    const char *manufacturer;
    const char *deviceType;
    const char *modelNumber;
    const char *serialNumber;
    const char *hardwareVersion;
    const char *firmwareVersion;
    const char *softwareVersion;
    const char *msisdn;
    uint16_t    optFlags;
    const char *utcOffsetP;
    const char *timezoneP;
    iowa_client_time_update_callback_t    dataTimeUpdateCallback;
    iowa_client_factory_reset_callback_t  factoryResetCallback;
    void                                 *callbackUserDataP;
} iowa_device_info_t;
```

The `iowa_device_info_t` structure exists only for the sake of the readability of `iowa_client_configure()`. It contains pointers to nil-terminated strings described below. As all these information are optional in a LwM2M Client, these pointers can be nil. The LwM2M standard does not mandate any format for these strings. They are manufacturer specific.

manufacturer
: A human readable manufacturer name.

deviceType
: The type of the device.

modelNumber
: The number of the model.

serialNumber
: The serial number of the device.

hardwareVersion
: The current version of the device hardware.

firmwareVersion
: The current version of the device firmware.

softwareVersion
: The current version of the device software.

msisdn
: The phone number of the device.

optFlags
: Flags used to enable optional features. This value is a combination of:

* **IOWA_DEVICE_RSC_BATTERY**: enables the battery level and status exposed in the [`Device Object`][Device Object]. To update battery level you need to call [`iowa_client_device_update_battery()`](ClientAPI.md#iowa_client_device_update_battery).
* **IOWA_DEVICE_RSC_POWER_SOURCE**: enables the power sources information in the [`Device Object`][Device Object]. To add any new power source you need to call [`iowa_client_add_device_power_source()`](ClientAPI.md#iowa_client_add_device_power_source).
* **IOWA_DEVICE_RSC_CURRENT_TIME**: enables the use of current time in the [`Device Object`][Device Object] (default value: 0).
* **IOWA_DEVICE_RSC_UTC_OFFSET**: enables the use of UTC offset in the [`Device Object`][Device Object] (default value: utcOffsetP).
* **IOWA_DEVICE_RSC_TIMEZONE**: enables the use of timezone in the [`Device Object`][Device Object] (default value: 0).

utcOffsetP
: Indicates the UTC offset currently in effect for this LwM2M Device. It should be in the ISO 8601 format (UTC+X).

timezoneP
: Indicates in which time zone the LwM2M Device is located, in IANA Timezone (TZ) database format.

dataTimeUpdateCallback
: The callback called when the time information is updated by the LwM2M Server.

factoryResetCallback
: The callback called on a Factory Reset.

callbackUserDataP
: Passed as argument to the callbacks `dataTimeUpdateCallback` and `factoryResetCallback`.

** Notes **

To update time information (current time, UTC offset, timezone) you need to call [`iowa_client_update_device_time_information()`](ClientAPI.md#iowa_client_update_device_time_information)

To update device related information (manufacturer, model number, serial number, firmware version, device type, hardware version, software version.) you need to call [`iowa_client_update_device_information()`](ClientAPI.md#iowa_client_update_device_information).

### iowa_event_type_t

```c
typedef enum
{
    IOWA_EVENT_UNDEFINED = 0,
    IOWA_EVENT_REG_UNREGISTERED,
    IOWA_EVENT_REG_REGISTERING,
    IOWA_EVENT_REG_REGISTERED,
    IOWA_EVENT_REG_UPDATING,
    IOWA_EVENT_REG_FAILED,
    IOWA_EVENT_REG_UPDATE_FAILED,
    IOWA_EVENT_BS_PENDING,
    IOWA_EVENT_BS_FINISHED,
    IOWA_EVENT_BS_FAILED,
    IOWA_EVENT_OBSERVATION_STARTED,
    IOWA_EVENT_OBSERVATION_NOTIFICATION,
    IOWA_EVENT_OBSERVATION_CANCELED,
    IOWA_EVENT_OBJECT_INSTANCE_CREATED,
    IOWA_EVENT_OBJECT_INSTANCE_DELETED,
    IOWA_EVENT_EVALUATION_PERIOD,
    IOWA_EVENT_READ
} iowa_event_type_t;
```

The `iowa_event_type_t` contains the possible events that can be reported by the IOWA stack.

### iowa_event_t

```c
typedef struct
{
    iowa_event_type_t eventType;
    uint16_t          serverShortId;
    union
    {
        struct
        {
            uint32_t lifetime;
        } registration;
        struct
        {
            iowa_sensor_t sensorId;
            uint16_t resourceId;
            uint32_t minPeriod;
            uint32_t maxPeriod;
            uint32_t minEvalPeriod;
            uint32_t maxEvalPeriod;
        } observation;
        struct
        {
            iowa_lwm2m_uri_t * uriP;
        } objectInstance;
        struct
        {
            iowa_lwm2m_uri_t * uriP;
            uint32_t minEvalPeriod;
            uint32_t maxEvalPeriod;
        } evalPeriod;
        struct
        {
            iowa_sensor_t sensorId;
        } sensor;
    } details;
} iowa_event_t;
```

eventType
: the event type.

serverShortId
: the short server ID of the LwM2M Server generating this event.

details
: the details of the event.

details::registration
: filled when the event is of type **IOWA_EVENT_REG_UNREGISTERED**, **IOWA_EVENT_REG_REGISTERING**, **IOWA_EVENT_REG_REGISTERED**, **IOWA_EVENT_REG_UPDATING**, **IOWA_EVENT_REG_FAILED** or **IOWA_EVENT_REG_UPDATE_FAILED**.

details::registration::lifetime
: the lifetime of the registration to the LwM2M Server generating this event.

details::observation
: filled when the event is of type **IOWA_EVENT_OBSERVATION_STARTED**,  **IOWA_EVENT_OBSERVATION_NOTIFICATION**, or **IOWA_EVENT_OBSERVATION_CANCELED**

details::observation::sensorId
: the ID of the sensor under observation.

details::observation::resourceId
: the ID of the specific resource under observation of the sensor. This may be **IOWA_LWM2M_ID_ALL**.

details::observation::minPeriod
: the minimum time in seconds to wait between notifications for the observation. If not set the minPeriod is to 0.

details::observation::maxPeriod
: the maximum time in seconds to wait between notifications for the observation. If not set the maxPeriod is to UINT32_MAX.

details::observation::minEvalPeriod
: the minimum sample time in seconds for the observed sensor in LwM2M 1.1 or later. If not set the minEvalPeriod is to 0.

details::observation::maxEvalPeriod
: the maximum sample time in seconds for the observed sensor in LwM2M 1.1 or later. If not set the maxEvalPeriod is to UINT32_MAX.

details::instance
: filled when the event is of type **IOWA_EVENT_OBJECT_INSTANCE_CREATED** or **IOWA_EVENT_OBJECT_INSTANCE_DELETED**.

details::instance::uri
: a pointer to the `iowa_lwm2m_uri_t` of the instance that has been created or deleted.

details::evalPeriod
: filled when the event is of type **IOWA_EVENT_EVALUATION_PERIOD**. Available when the flag IOWA_LWM2M_VERSION_1_1 is set.

details::evalPeriod::uriP
: a pointer to the `iowa_lwm2m_uri_t` of the uri where evaluation period has been set.

details::evalPeriod::minEvalPeriod
: the minimum sample time in seconds for the concerned uri. If the LwM2M Server unsets it or does not set it, the value is 0.

details::evalPeriod::maxEvalPeriod
: the maximum sample time in seconds for the concerned uri. If the LwM2M Server unsets it or does not set it, the value is UINT32_MAX.

details::sensor
: filled when the event is of type **IOWA_EVENT_READ**.

details::sensor::sensorId
: the ID of the sensor targeted by a Read operation from the LwM2M Server.

The `iowa_event_t` is used by `iowa_event_callback_t` when an event occurred on a client. These events are described by `iowa_event_type_t`.

> The IOWA stack handles the minimum and maximum observation periods. They are provided in `iowa_event_t` as an information for the application. Embedded devices may use this information to tune their measurement or sleeping schedule.

> The IOWA stack does not handle the minimum and maximum evaluation observation periods. They are provided in `iowa_event_t` as sample times for the application. Embedded devices may use those sample times to tune their measurement or sleeping schedule.

### iowa_device_time_info_t

```c
typedef struct
{
    uint16_t    flags;
    int32_t     currentTime;
    const char *utcOffsetP;
    const char *timezoneP;
} iowa_device_time_info_t;
```

flags
: Flags used to enable optional time information. This value is a combination of:

* **IOWA_DEVICE_RSC_CURRENT_TIME** : current time has a new current time value from server
* **IOWA_DEVICE_RSC_UTC_OFFSET** : current time has a new UTC offset value from server
* **IOWA_DEVICE_RSC_TIMEZONE** : current time has a new timezone value from server

currentTime
: Current UNIX time of the LwM2M Client in seconds.

utcOffsetP
: Indicates the UTC offset currently in effect for this LwM2M Device. It should be in the ISO 8601 format (UTC+X). Could be nil, if not UTC offset is enable.

timezoneP
: Indicates in which time zone the LwM2M Device is located, in IANA Timezone (TZ) database format. Could be nil, if not Timezone is enable.

### iowa_ipso_timed_value_t

```c
typedef struct
{
    float value;
    int32_t timestamp;
} iowa_ipso_timed_value_t;
```

value
: The timestamped value.

timestamp
: The timestamp associated to the value in seconds. This can not be negative.

### iowa_sensor_t

This must be treated as an opaque type. It is internally mapped to a 32-bit unsigned integer.

#### Special Values

**IOWA_INVALID_SENSOR_ID**
: Used to indicate an error by APIs returning an `iowa_sensor_t`.

**IOWA_DEVICE_TIME_SENSOR_ID**
: The sensor ID of the Current Time of the device. This is internally mapped to the resource 13 in the [Device Object][Device Object].

### iowa_lwm2m_resource_desc_t

This structure contains the description of a LwM2M resource.
```c
typedef struct {
    uint16_t id;
    iowa_lwm2m_data_type_t type;
    uint8_t operations;
    uint8_t flags;
} iowa_lwm2m_resource_desc_t;
```

*id*
: ID of the resource.

*type*
: The datatype of the resource.

*operations*
: The operations allowed on the resource.
: This is a mask of values **IOWA_OPERATION_READ**, **IOWA_OPERATION_WRITE** and **IOWA_OPERATION_EXECUTE**.

*flags*
: The flags of the resource.
: This is a mask of values **IOWA_RESOURCE_FLAG_NONE**, **IOWA_RESOURCE_FLAG_OPTIONAL**, **IOWA_RESOURCE_FLAG_MANDATORY**, **IOWA_RESOURCE_FLAG_MULTIPLE**, and **IOWA_RESOURCE_FLAG_ASYNCHRONOUS**.

### iowa_sensor_uri_t

This structure describes a sensor URI.

```c
typedef struct
{
    iowa_sensor_t id;
    uint16_t resourceId;
} iowa_sensor_uri_t;
```

*id*
: ID of the object.

*resourceId*
: The ID of the resource. This can be **IOWA_LWM2M_ID_ALL**.

\clearpage

## Callbacks

### iowa_event_callback_t

This is the event callback, called when an event such as registration update or unregister occurred.

```c
typedef void(*iowa_event_callback_t) (iowa_event_t* eventP,
                                      void * userData,
                                      iowa_context_t contextP);
```

*eventP*
: The event stored in a structure.

*userData*
: A pointer to application specific data. This is a parameter of [`iowa_init()`](CommonAPI.md#iowa_init).

*contextP*
: The IOWA context on which [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) was called.

### iowa_client_time_update_callback_t

This callback is called when time information are updated by server.

```c
typedef void(*iowa_client_time_update_callback_t) (iowa_device_time_info_t *timeInfoP,
                                                   void  *userDataP,
                                                   iowa_context_t contextP);
```

*timeInfoP*
: Current device time information.

*userDataP*
: A pointer to application specific data. This is the parameter of [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

*contextP*
: The IOWA context on which [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) was called.

### iowa_client_factory_reset_callback_t

This callback is called when a factory reset is requested.

```c
typedef void(*iowa_client_factory_reset_callback_t) (void *userDataP,
                                                     iowa_context_t contextP);
```

*userDataP*
: A pointer to application specific data. This is the parameter of [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

*contextP*
: The IOWA context on which [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) was called.

### iowa_RWE_callback_t

This callback is called when a Read, Write or Execute operation is performed on a resource of a custom LwM2M Object.
```c
typedef iowa_status_t(*iowa_RWE_callback_t) (iowa_dm_operation_t operation,
                                             iowa_lwm2m_data_t * dataP,
                                             size_t numData,
                                             void * userData,
                                             iowa_context_t contextP);
```

*operation*
: The operation to perform on the resource among **IOWA_DM_READ**, **IOWA_DM_WRITE** and **IOWA_DM_EXECUTE**.

*dataP*
: An array of the URIs of the targeted resources.
: For a Write operation, it also contains the value to write.
: For a Read operation, the result is to be stored in this.

*numData*
: Number of resources in dataP.

*userData*
: A pointer to application specific data. This is the parameter of [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object).

*contextP*
: The IOWA context on which [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object) was called.

#### Notes

- Before calling this callback, the IOWA stack performs checks on the resource existence and its allowed operations. In case of a Write operation, the data type is also checked for conformance.
- The LwM2M Execute operation may have parameters. If so, they are provided as a string in *dataP*.
- After [**IOWA_DM_READ**](CommonAPI.md#iowa_dm_operation_t) operation, the callback is called with [**IOWA_DM_FREE**](CommonAPI.md#iowa_dm_operation_t) operation to permit the deallocation of memory that may have been allocated by the callback previously.

### iowa_CD_callback_t

This callback is called when a Create or Delete operation is performed on an instance of a custom LwM2M Object.
```c
typedef iowa_status_t(*iowa_CD_callback_t) (iowa_dm_operation_t operation,
                                            uint16_t objectID,
                                            uint16_t instanceID,
                                            void * userData,
                                            iowa_context_t contextP);
```

*operation*
: The operation to perform on the resource among **IOWA_DM_CREATE** and **IOWA_DM_DELETE**.

*objectID*
: The ID of the targeted Object.

*instanceID*
: The ID of the targeted instance.

*userData*
: A pointer to application specific data. This is the parameter of [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object).

*contextP*
: The IOWA context on which [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object) was called.

### iowa_RI_callback_t

This callback is called to retrieve the list of current resource instance IDs for a multiple resource.
```c
typedef iowa_status_t(*iowa_RI_callback_t) (uint16_t objectID,
                                            uint16_t instanceID,
                                            uint16_t resourceID,
                                            uint16_t * nbResInstanceP,
                                            uint16_t ** resInstanceArrayP,
                                            void * userData,
                                            iowa_context_t contextP);
```

*objectID*
: The ID of the Object the resource belongs to.

*instanceID*
: The ID of the Object Instance the resource belongs to.

*resourceID*
: The ID of the targeted resource.

*nbResInstanceP*
: Used to store the number of elements in resInstanceArrayP.

*resInstanceArrayP*
: Used to store an array containing the resource instances IDs. This array will be freed by the caller by calling [`iowa_system_free()`](AbstractionLayer.md#iowa_system_free).

*userData*
: A pointer to application specific data. This is the parameter of [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object).

*contextP*
: The IOWA context on which [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object) was called.

\clearpage

## API

### iowa_client_configure

** Prototype **

```c
iowa_status_t iowa_client_configure(iowa_context_t contextP,
                                    const char * identity,
                                    iowa_device_info_t * infoP,
                                    iowa_event_callback_t eventCb);
```
** Description **

`iowa_client_configure()` sets the information of the LwM2M Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*identity*
: The unique identity of the LwM2M Client as a nil-terminated string.

*infoP*
: The optional information of the LwM2M Client. This can be nil.

*eventCb*
: The callback called when an event occurred. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

COAP_400_BAD_REQUEST
: either:
: - *identity* is nil or empty and **LWM2M_VERSION_1_1_SUPPORT** is not set.
: - the maximum length of *infoP->msisdn* is 15 digits.
: - *infoP->msisdn* is not nil, but **IOWA_SMS_SUPPORT** is not defined.

IOWA_COAP_412_PRECONDITION_FAILED
: the client was already configured in this context. To reconfigure the client, close than reopen a fresh IOWA context with [`iowa_close()`](CommonAPI.md#iowa_close) and [`iowa_init()`](CommonAPI.md#iowa_init).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_client.h

** Notes **

The nil-terminated strings pointed by the fields of *infoP* are not duplicated nor freed by IOWA. Make sure they are available until [`iowa_close()`](CommonAPI.md#iowa_close) is called. It is advised to use static strings.

The LwM2M Client information are exposed to the LwM2M Server through the Resources of the [`Device Object`][Device Object] (ID: 3). The following table explained the mapping:

| Resource ID | Resource Name | API                                           |
| ----------- | ------------- | --------------------------------------------- |
| 0 | Manufacturer | *manufacturer* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 1 | Model Number | *modelNumber* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 2 | Serial Number | *serialNumber* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 3 | Firmware Version | *firmwareVersion* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 4 | Reboot | |
| 5 | Factory Reset | |
| 6 | Available Power Sources | Set the flag **IOWA_DEVICE_RSC_POWER_SOURCE** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_..._device_power_source`](ClientAPI.md#iowa_client_add_device_power_source) to control it. |
| 7 | Power Source Voltage | Set the flag **IOWA_DEVICE_RSC_POWER_SOURCE** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_..._device_power_source`](ClientAPI.md#iowa_client_add_device_power_source) to control it. |
| 8 | Power Source Current | Set the flag **IOWA_DEVICE_RSC_POWER_SOURCE** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_..._device_power_source`](ClientAPI.md#iowa_client_add_device_power_source) to control it. |
| 9 | Battery Level | Set the flag **IOWA_DEVICE_RSC_BATTERY** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_device_update_battery`](ClientAPI.md#iowa_client_device_update_battery) to control it. |
| 10 | Memory Free | Not exposed by IOWA. |
| 11 | Error Code | Use [`iowa_client_..._device_error_code`](ClientAPI.md#iowa_client_set_device_error_code) to control it. |
| 12 | Reset Error Code | Set the flag **IOWA_DEVICE_RSC_RESET_ERROR** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). |
| 13 | Current Time | Set the flag **IOWA_DEVICE_RSC_CURRENT_TIME** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_update_device_time_information`](ClientAPI.md#iowa_client_update_device_time_information) to control it. |
| 14 | UTC Offset | *utcOffsetP* field and set the flag **IOWA_DEVICE_RSC_UTC_OFFSET** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_update_device_time_information`](ClientAPI.md#iowa_client_update_device_time_information) to control it. |
| 15 | Timezone | *timezoneP* field and set the flag **IOWA_DEVICE_RSC_TIMEZONE** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_update_device_time_information`](ClientAPI.md#iowa_client_update_device_time_information) to control it. |
| 16 | Supported Binding and Modes | Cannot be updated directly but depends on the Server URI schema. |
| 17 | Device Type | *deviceType* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 18 | Hardware Version | *hardwareVersion* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 19 | Software Version | *softwareVersion* field of the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. |
| 20 | Battery Status | Set the flag **IOWA_DEVICE_RSC_BATTERY** in *optFlags* field of the structure [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t). Then use [`iowa_client_device_update_battery`](ClientAPI.md#iowa_client_device_update_battery) to control it. |
| 21 | Memory Total | Not exposed by IOWA. |
| 22 | ExtDevInfo | Not exposed by IOWA. |

\clearpage

### iowa_client_new_incoming_connection

** Prototype **

```c
iowa_status_t iowa_client_new_incoming_connection(iowa_context_t contextP,
                                                  iowa_connection_type_t type,
                                                  void *connP,
                                                  bool isSecure);
```

** Description **

`iowa_client_new_incoming_connection()` informs the stack of a new incoming connection.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*type*
: The type of the new connection. See [`iowa_connection_type_t`](AbstractionLayer.md#iowa_connection_type_t).

*connP*
: The new connection of the same user-defined type as the one returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*isSecure*
: Set to *true* if the security must be enabled on this connection.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - the connection type is not supported. Make sure to verify the corresponding `IOWA_..._SUPPORT` flag has been enabled during IOWA build.
: - *isSecure* is true, but no security layer has been built. Make sure to verify the corresponding `IOWA_SECURITY_LAYER_...` flag has been enabled during IOWA build.

** Header File **

iowa_client.h

** Notes **

`iowa_client_new_incoming_connection()` can only be called when IOWA is built with the flag **LWM2M_CLIENT_INCOMING_CONNECTION_SUPPORT**.

\clearpage

### iowa_client_add_bootstrap_server

** Prototype **

```c
iowa_status_t iowa_client_add_bootstrap_server(iowa_context_t contextP,
                                               const char *uri,
                                               iowa_security_mode_t securityMode);
```

** Description **

`iowa_client_add_bootstrap_server()` declares a new LwM2M Bootstrap Server for the LwM2M Client to connect to.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime. The context MUST be configured with [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) to add a bootstrap server.

*uri*
: The URI to reach this bootstrap server as a nil-terminated string e.g. "coaps://[::1]:5684", "coap://lwm2m.example.org:5683" or "sms://+331020304050".

*securityMode*
: The security mode to use when connecting to this LwM2M Bootstrap Server. See [iowa_security_mode_t](Security.md#iowa_security_mode_t).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *uri* is nil.

IOWA_COAP_403_FORBIDDEN
: a bootstrap server is already configured. To reconfigure the LwM2M Bootstrap Server, call first [`iowa_client_remove_bootstrap_server`](ClientAPI.md#iowa_client_add_bootstrap_server).

IOWA_COAP_404_NOT_FOUND
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_406_NOT_ACCEPTABLE
: *uri* is invalid. For example, if the transport is not supported or if *uri* does not match *securityMode*.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_client.h

** Notes **

`iowa_client_add_bootstrap_server()` can only be called when IOWA is built with the flag **LWM2M_BOOTSTRAP**.

*uri* is duplicated internally by IOWA and can be freed by the caller.

Only one bootstrap server can be configured.

\clearpage

### iowa_client_remove_bootstrap_server

** Prototype **

```c
iowa_status_t iowa_client_remove_bootstrap_server(iowa_context_t contextP);
```

** Description **

`iowa_client_remove_bootstrap_server()` removes a LwM2M Bootstrap Server added by [`iowa_client_add_bootstrap_server()`](ClientAPI.md#iowa_client_add_bootstrap_server) from the LwM2M Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: no bootstrap server is configured. [`iowa_client_add_bootstrap_server()`](ClientAPI.md#iowa_client_add_bootstrap_server) was not called before, or failed.

** Header File **

iowa_client.h

** Notes **

`iowa_client_remove_bootstrap_server()` can only be called when IOWA is built with the flag **LWM2M_BOOTSTRAP**.

\clearpage

### iowa_client_set_bootstrap_server_hold_off

** Prototype **

```c
iowa_status_t iowa_client_set_bootstrap_server_hold_off(iowa_context_t contextP,
                                                        int32_t holdOff);
```

** Description **

`iowa_client_set_bootstrap_server_hold_off()` sets the Bootstrap Hold Off time.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*holdOff*
: The Hold Off time.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *holdOff* is negative.

IOWA_COAP_404_NOT_FOUND
: no bootstrap server is configured. [`iowa_client_add_bootstrap_server()`](ClientAPI.md#iowa_client_add_bootstrap_server) was not called before, or failed.

** Header File **

iowa_client.h

** Notes **

`iowa_client_set_bootstrap_server_hold_off()` can only be called when IOWA is built with the flag **LWM2M_BOOTSTRAP**.

\clearpage

### iowa_client_get_bootstrap_server_coap_peer

** Prototype **

```c
iowa_coap_peer_t * iowa_client_get_bootstrap_server_coap_peer(iowa_context_t contextP);
```

** Description **

`iowa_client_get_bootstrap_server_coap_peer()` returns the CoAP peer associated to a LwM2M Bootstrap Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

A pointer to the [iowa_coap_peer_t](CoapAPI.md#iowa_coap_peer_t) associated to the LwM2M Bootstrap Server.

This pointer may be nil if IOWA did not yet initiate, or has finished, the Bootstrap process.

** Header File **

iowa_client.h

** Notes **

`iowa_client_get_bootstrap_server_coap_peer()` can only be called when IOWA is built with the flag **LWM2M_BOOTSTRAP**.

\clearpage

### iowa_client_add_server

** Prototype **

```c
iowa_status_t iowa_client_add_server(iowa_context_t contextP,
                                     uint16_t shortID,
                                     const char *uri,
                                     uint32_t lifetime,
                                     uint16_t configFlags,
                                     iowa_security_mode_t securityMode);
```

** Description **

`iowa_client_add_server()` declares a new LwM2M Server for the LwM2M Client to connect to.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime. The context MUST be configured with [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) to add a server.

*shortID*
: The ID assigned to this server. This cannot be zero nor **IOWA_LWM2M_ID_ALL** nor an existing one.

*uri*
: The URI to reach this server as a nil-terminated string e.g. "coaps://[::1]:5684", "coap://lwm2m.example.org:5683" or "sms://+331020304050".

*lifetime*
: The lifetime in seconds of the registration to this server.

*configFlags*
: A bit-mask of configuration flags for this LwM2M Server.

*securityMode*
: The security mode to use when connecting to this LwM2M Server. See [iowa_security_mode_t](Security.md#iowa_security_mode_t).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *uri* is nil.

IOWA_COAP_403_FORBIDDEN
: *shortID* is either zero, **IOWA_LWM2M_ID_ALL** or already in use.

IOWA_COAP_404_NOT_FOUND
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_406_NOT_ACCEPTABLE
: *uri* is invalid. For example, if the transport is not supported or if *uri* does not match *securityMode*.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_client.h

** Notes **

If *lifetime* is set to zero, the registration lifetime is set to a default value of:

- 30 days (2,592,000 seconds) for LoRaWAN transport
- 24 hours (86,400 seconds) for other transports (UDP, TCP, SMS ...)

*uri* is duplicated internally by IOWA and can be freed by the caller.

*configFlags* is a combination of the following:

- **IOWA_LWM2M_QUEUE_MODE**: Enable LwM2M Queue Mode for this LwM2M Server.

\clearpage

### iowa_client_remove_server

** Prototype **

```c
iowa_status_t iowa_client_remove_server(iowa_context_t contextP,
                                        uint16_t shortID);
```

** Description **

`iowa_client_remove_server()` removes a LwM2M Server added by [`iowa_client_add_server()`](ClientAPI.md#iowa_client_add_server) from the LwM2M Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

shortID
: The ID assigned to this server or *IOWA_LWM2M_ID_ALL*.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *shortID* is zero.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

** Header File **

iowa_client.h

\clearpage

### iowa_client_set_server_configuration

** Prototype **

```c
iowa_status_t iowa_client_set_server_configuration(iowa_context_t contextP,
                                                   uint16_t shortId,
                                                   iowa_server_setting_id_t settingId,
                                                   void *argP);
```

** Description **

`iowa_client_set_server_configuration()` configures the settings of a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortId*
: The short ID of a LwM2M Server.

*settingId*
: The setting to set. See [`iowa_server_setting_id_t`](ClientAPI.md#iowa_server_setting_id_t).

*argP*
: A pointer to the setting value. Dependent on *settingId*.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *shortId* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *shortId* does not match any known server.

IOWA_COAP_405_METHOD_NOT_ALLOWED
: *settingId* is nil.

IOWA_COAP_501_NOT_IMPLEMENTED
: invalid *settingId* value.

** Header File **

iowa_client.h

\clearpage

### iowa_client_set_server_msisdn

** Prototype **

```c
iowa_status_t iowa_client_set_server_msisdn(iowa_context_t contextP,
                                            uint16_t shortID,
                                            const char * msisdn);
```

** Description **

`iowa_client_set_server_msisdn()` sets the MSISDN of a previously added LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortID*
: the Short ID assigned to a LwM2M Server.

*msisdn*
: the MSISDN to reach this Server e.g. "0102030405" or "+33102030405". This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: the maximum length of *msisdn* is 15 digits.

IOWA_COAP_403_FORBIDDEN
: *shortID* is either zero or **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_client.h

** Notes **

`iowa_client_set_server_msisdn()` can only be called when IOWA is built with the flag **IOWA_SMS_SUPPORT**.

To unset the MSISDN, the parameter *msisdn* can take the value NULL.

An MSISDN can not be set for the Bootstrap Server.

\clearpage

### iowa_client_set_server_registration_behaviour

** Prototype **

```c
iowa_status_t iowa_client_set_server_registration_behaviour(iowa_context_t contextP,
                                                            uint16_t shortId,
                                                            uint16_t priorityOrder,
                                                            int32_t initialDelayTimer,
                                                            bool blockOnFailure,
                                                            bool bootstrapOnFailure);
```

** Description **

`iowa_client_set_server_registration_behaviour()` set the registration behaviour of a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortID*
: The ID assigned to the server.

*priorityOrder*
: The Server priority order for the registration sequence.

*initialDelayTimer*
: The initial delay to wait before to send the registration.

*blockOnFailure*
: If registration fails and true is set, the registration sequence is interrupted.

*bootstrapOnFailure*
: If registration fails and true is set, a bootstrap sequence is initiated.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *initialDelayTimer* is negative.
: - *bootstrapOnFailure* is equals to true but [`LWM2M_BOOTSTRAP`][LWM2M_BOOTSTRAP] is not set.

IOWA_COAP_403_FORBIDDEN
: *shortID* is either zero or **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

** Header File **

iowa_client.h

** Notes **

This API requires **LWM2M_VERSION_1_1_SUPPORT** to be set.

If **IOWA_SERVER_RSC_REGISTRATION_BEHAVIOUR_REMOVE** is set, this API cannot be called.

\clearpage

### iowa_client_set_server_communication_attempts

** Prototype **

```c
iowa_status_t iowa_client_set_server_communication_attempts(iowa_context_t contextP,
                                                            uint16_t shortId,
                                                            uint8_t retryCount,
                                                            int32_t retryDelayTimer,
                                                            uint8_t sequenceRetryCount,
                                                            int32_t sequenceDelayTimer);
```

** Description **

`iowa_client_set_server_communication_attempts()` set the communication attempts of a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortID*
: The ID assigned to the server.

*retryCount*
: The number of successive registration attempts before which a registration sequence is considered as failed.

*retryDelayTimer*
: The number to wait between each registration sequence. The value is multiplied by two to the power of the registration retry attempt minus one (2**(retry attempt-1)) to create an exponential back-off.

*sequenceRetryCount*
: The number of successive registration sequences before which a registration attempt is considered as failed.

*sequenceDelayTimer*
: The number to wait between each successive registration sequences. The value is multiplied by two to the power of the registration retry attempt minus one (2**(retry attempt-1)) to create an exponential back-off.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *retryCount* is superior to 32.
: - *retryDelayTimer* is negative.
: - *sequenceRetryCount* is superior to 32.
: - *sequenceDelayTimer* is negative.

IOWA_COAP_403_FORBIDDEN
: *shortID* is either zero or **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

** Header File **

iowa_client.h

** Notes **

This API requires **LWM2M_VERSION_1_1_SUPPORT** to be set.

If **IOWA_SERVER_RSC_COMMUNICATION_ATTEMPTS_REMOVE** is set, this API cannot be called.

\clearpage

### iowa_client_get_server_count

** Prototype **

```c
iowa_status_t iowa_client_get_server_count(iowa_context_t contextP,
                                           size_t *serversCountP);
```
** Description **

`iowa_client_get_server_count()` get the count of all configured LwM2M Servers.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*serversCountP*
: A pointer to the count of all configured LwM2M Servers.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

** Header File **

iowa_client.h

\clearpage

### iowa_client_get_server_array

** Prototype **

```c
iowa_status_t iowa_client_get_server_array(iowa_context_t contextP,
                                           size_t serversCount,
                                           iowa_server_info_t *serverArrayP);
```
** Description **

`iowa_client_get_server_array()` retrieves the configured LwM2M Servers with theirs associated information.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*serversCount*
: A pointer to the count of LwM2M Servers.

*serverArrayP*
: An Array of `iowa_server_info_t` with a size equal to the count of the current configured LwM2M Servers.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: *serverListP* is nil.

IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE
: The count of configured LwM2M servers has been changed since the call of `iowa_client_get_server_count()`.

** Header File **

iowa_client.h

** Notes **

To get the count of the current configured LwM2M Servers you need to call `iowa_client_get_server_count()`.

\clearpage

### iowa_client_get_server_coap_peer

** Prototype **

```c
iowa_coap_peer_t * iowa_client_get_server_coap_peer(iowa_context_t contextP,
                                                    uint16_t shortId);
```

** Description **

`iowa_client_get_server_coap_peer()` returns the CoAP peer associated to a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortID*
: The ID assigned to the server.

** Return Value **

A pointer to the [iowa_coap_peer_t](CoapAPI.md#iowa_coap_peer_t) associated to the LwM2M Server.

This pointer may be nil if `shortId` is invalid or if IOWA did not yet initiate the registration to the LwM2M Server.

** Header File **

iowa_client.h

\clearpage

### iowa_client_set_notification_default_periods

** Prototype **

```c
iowa_status_t iowa_client_set_notification_default_periods(iowa_context_t contextP,
                                                           uint16_t shortID,
                                                           uint32_t minPeriod,
                                                           uint32_t maxPeriod);
```

** Description **

`iowa_client_set_notification_default_periods()` configures the default periods for notifications sent to a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortID*
: The ID assigned to the server or *IOWA_LWM2M_ID_ALL*.

*minPeriod*
: The default minimum time in seconds between two notifications sent to the LwM2M Server for the same observation.

*maxPeriod*
: The default maximum time in seconds between two notifications sent to the LwM2M Server for the same observation.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *shortID* is zero.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

** Header File **

iowa_client.h

** Notes **

When *IOWA_LWM2M_ID_ALL* is used as *shortID*, only already known LwM2M Servers will have the default periods configured. If a LwM2M Server is added after the call to this API, by default it will not have default periods.

Setting the default periods does not affect already running observations.

A minimum period set to zero is equivalent to having no minimum period defined. Same for maximum period.

If *maxPeriod* is inferior to *minPeriod*, it is cleared (i.e. set to zero).

\clearpage

### iowa_client_use_reliable_notifications

** Prototype **

```c
iowa_status_t iowa_client_use_reliable_notifications(iowa_context_t contextP,
                                                     uint16_t shortId,
                                                     bool enable);
```

** Description **

`iowa_client_use_reliable_notifications()` configures the LwM2M Client to ensure that notifications are received by the LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortId*
: The ID assigned to the server or *IOWA_LWM2M_ID_ALL*.

*enable*
: If true, notifications will be reliable.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *shortID* is zero.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

** Header File **

iowa_client.h

** Notes **

When *IOWA_LWM2M_ID_ALL* is used as *shortID*, only already known LwM2M Servers will have reliable notifications. If a LwM2M Server is added after the call to this API, by default it will not use reliable notifications.

If *enable* is true:

* on unreliable transports like UDP, the notifications are sent as Confirmable messages.
* if a notification does not reach the LwM2M Server, IOWA stores it until the LwM2M Server is reachable again. See [**LWM2M_STORAGE_QUEUE_SUPPORT**][LWM2M_STORAGE_QUEUE_SUPPORT] and [**LWM2M_STORAGE_QUEUE_PEEK_SUPPORT**][LWM2M_STORAGE_QUEUE_PEEK_SUPPORT].

\clearpage

### iowa_client_object_set_mode

** Prototype **

```c
iowa_status_t iowa_client_object_set_mode(iowa_context_t contextP,
                                          iowa_sensor_t id,
                                          uint8_t mode);
```

** Description **

`iowa_client_object_set_mode()` sets the sensor mode.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: The ID of the sensor.

*mode*
: Flags used to enable modes.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *id* does not match any known sensor.

** Header File **

iowa_client.h

** Notes **

To use this API, the compilation flag [**LWM2M_CLIENT_ASYNCHRONOUS_OPERATION_SUPPORT**][LWM2M_CLIENT_ASYNCHRONOUS_OPERATION_SUPPORT] must be set.

To set the sensor mode, you can use the following flag:

- IOWA_OBJECT_MODE_DEFAULT
- IOWA_OBJECT_MODE_ASYNCHRONOUS

By default, sensors are synchronous.

A call to `iowa_client_object_set_mode()` affects all the sensors of the same type.

\clearpage

### iowa_client_device_update_battery

** Prototype **

```c
iowa_status_t iowa_client_device_update_battery(iowa_context_t contextP,
                                                uint8_t batteryLevel,
                                                iowa_device_battery_status_t batteryStatus);
```

** Description **

`iowa_client_device_update_battery()` updates the battery level and status exposed in the [`Device Object`][Device Object].

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*batteryLevel*
: The battery level in percent.

*batteryStatus*
: The battery status.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_405_METHOD_NOT_ALLOWED
: client has been configured without the flag **IOWA_DEVICE_RSC_BATTERY** in the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. To reconfigure the client, close than reopen a fresh IOWA Client context with [`iowa_close()`](CommonAPI.md#iowa_close), [`iowa_init()`](CommonAPI.md#iowa_init) and [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_406_NOT_ACCEPTABLE
: *batteryLevel* is outside the range [0; 100].

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

** Header File **

iowa_client.h

** Notes **

For the device to expose its battery level and status, [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) must have been to called with the **IOWA_DEVICE_RSC_BATTERY** flag.

Before the first call to `iowa_client_device_update_battery()`, default value of batteryStatus is **IOWA_DEVICE_BATTERY_STATUS_UNKNOWN**.

`iowa_client_device_update_battery()` can only be called when IOWA is built WITHOUT the flag **IOWA_DEVICE_RSC_BATTERY_REMOVE**.

#### iowa_device_battery_status_t

This is an enumeration of the following values:

IOWA_DEVICE_BATTERY_STATUS_NORMAL
: The battery is operating normally and not on power.

IOWA_DEVICE_BATTERY_STATUS_CHARGING
: The battery is currently charging.

IOWA_DEVICE_BATTERY_STATUS_CHARGE_COMPLETE
: The battery is fully charged and still on power.

IOWA_DEVICE_BATTERY_STATUS_DAMAGED
: The battery has some problem.

IOWA_DEVICE_BATTERY_STATUS_LOW_BATTERY
: The battery is low on charge.

IOWA_DEVICE_BATTERY_STATUS_NOT_INSTALLED
: The battery is not installed.

IOWA_DEVICE_BATTERY_STATUS_UNKNOWN
: The battery information is not available.

\clearpage

### iowa_client_add_device_power_source

** Prototype **

```c
iowa_status_t iowa_client_add_device_power_source(iowa_context_t context,
                                                  iowa_power_source_type_t type,
                                                  int voltageValue,
                                                  int currentValue,
                                                  iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_device_power_source()` adds a power source to Device object with initial value of voltage and current.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*type*
: power source type.

*voltageValue*
: initial voltage value (mV).

*currentValue*
: initial current value (mA).

*idP*
: Used to store the ID of the created power source. Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_405_METHOD_NOT_ALLOWED
: client has been configured without the flag **IOWA_DEVICE_RSC_POWER_SOURCE** in the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. To reconfigure the client, close than reopen a fresh IOWA Client context with [`iowa_close()`](CommonAPI.md#iowa_close), [`iowa_init()`](CommonAPI.md#iowa_init) and [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_client.h

** Notes **

For the device to expose its power source information, [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) must have been to called with the `IOWA_DEVICE_RSC_POWER_SOURCE` flag.

To update a power source values, you need to call [`iowa_client_update_device_power_source()`](ClientAPI.md#iowa_client_update_device_power_source).

To remove a power source, you need to call [`iowa_client_remove_device_power_source()`](ClientAPI.md#iowa_client_remove_device_power_source).

`iowa_client_add_device_power_source()` can only be called when IOWA is built WITHOUT the flag **IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE**.

#### iowa_power_source_type_t

This is an enumeration of the following values:

IOWA_POWER_SOURCE_DC_POWER
: DC power supply.

IOWA_POWER_SOURCE_INTERNAL_BATTERY
: Internal battery.

IOWA_POWER_SOURCE_EXTERNAL_BATTERY
: External battery.

IOWA_POWER_SOURCE_FUEL_CELL
: Fuel Cell

IOWA_POWER_SOURCE_POWER_OVER_ETHERNET
: Power Over Ethernet.

IOWA_POWER_SOURCE_USB
: USB.

IOWA_POWER_SOURCE_AC_MAIN_POWER
: AC power supply.

IOWA_POWER_SOURCE_SOLAR
: Solar energy.

\clearpage

### iowa_client_remove_device_power_source

** Prototype **

```c
iowa_status_t iowa_client_remove_device_power_source(iowa_context_t context,
                                                     iowa_sensor_t id);
```

** Description **

`iowa_client_remove_device_power_source()` removes a power source from the Device object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the power source to remove.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_404_NOT_FOUND
: *id* is not a device's power source. Valid *id* are only returned by [`iowa_client_add_device_power_source()`](ClientAPI.md#iowa_client_add_device_power_source).

IOWA_COAP_405_METHOD_NOT_ALLOWED
: client has been configured without the flag **IOWA_DEVICE_RSC_POWER_SOURCE** in the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. To reconfigure the client, close than reopen a fresh IOWA Client context with [`iowa_close()`](CommonAPI.md#iowa_close), [`iowa_init()`](CommonAPI.md#iowa_init) and [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_client.h

** Notes **

For the device to expose its power source information, [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) must have been to called with the `IOWA_DEVICE_RSC_POWER_SOURCE` flag.

`iowa_client_remove_device_power_source()` can only be called when IOWA is built WITHOUT the flag **IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE**.

\clearpage

### iowa_client_update_device_power_source

** Prototype **

```c
iowa_status_t iowa_client_update_device_power_source(iowa_context_t context,
                                                     iowa_sensor_t id,
                                                     int voltageValue,
                                                     int currentValue);
```

** Description **

`iowa_client_update_device_power_source()` updates a power source values to Device object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the power source.

*voltageValue*
: new voltage value (mV).

*currentValue*
: new current value (mA).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_404_NOT_FOUND
: id is not a device's power source. Valid *id* are only returned by [`iowa_client_add_device_power_source()`](ClientAPI.md#iowa_client_add_device_power_source).

IOWA_COAP_405_METHOD_NOT_ALLOWED
: client has been configured without the flag **IOWA_DEVICE_RSC_POWER_SOURCE** in the [`iowa_device_info_t`](ClientAPI.md#iowa_device_info_t) structure. To reconfigure the client, close than reopen a fresh IOWA Client context with [`iowa_close()`](CommonAPI.md#iowa_close), [`iowa_init()`](CommonAPI.md#iowa_init) and [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

** Header File **

iowa_client.h

** Notes **

For the device to expose its power source information, [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) must have been to called with the `IOWA_DEVICE_RSC_POWER_SOURCE` flag.

`iowa_client_update_device_power_source()` can only be called when IOWA is built WITHOUT the flag **IOWA_DEVICE_RSC_POWER_SOURCE_REMOVE**.

\clearpage

### iowa_client_update_device_information

** Prototype **

```c
iowa_status_t iowa_client_update_device_information(iowa_context_t contextP,
                                                    iowa_device_info_t *deviceInfoP);
```

** Description **

`iowa_client_update_device_information()` updates the device information.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*infoP*
: pointer to the new device's information.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *deviceInfoP* is nil.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

** Header File **

iowa_client.h

** Notes **

This function update only the supported device resources that has been set previously in [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

The parameters that can be updated are :

- Manufacturer.
- Model number.
- Serial number.
- Firmware version.
- Device type.
- Hardware version.
- Software version.

Nil values in *deviceInfoP* will be ignored and won't make any changes.

\clearpage

### iowa_client_set_device_error_code

** Prototype **

```c
iowa_status_t iowa_client_set_device_error_code(iowa_context_t context,
                                                uint8_t errorCode);
```

** Description **

`iowa_client_set_device_error_code()` sets an error code on Device object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*errorCode*
: The error code value to set between 1 and 32.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_402_BAD_OPTION
: *errorCode* is not a valid parameter.

IOWA_COAP_404_NOT_FOUND
: *errorCode* is IOWA_ERROR_CODE_NO_ERROR but there is no error to clear.

IOWA_COAP_409_CONFLICT
: *errorCode* has already been set.

** Header File **

iowa_client.h

** Notes **

To clear one error code, you need to call [`iowa_client_clear_device_error_code()`](ClientAPI.md#iowa_client_clear_device_error_code).

To clear all error codes, you can call [`iowa_client_set_device_error_code()`](ClientAPI.md#iowa_client_set_device_error_code) with *errorCode* argument set to IOWA_ERROR_CODE_NO_ERROR.

The error code is an integer between 1 and 32. The LwM2M protocol defines eight values between 1 and 8 detailed below. Values between 9 and 15 are reserved for future use. The device maker or the application can use the values between 16 and 32 as proprietary error codes.

LwM2M defined error code values are:

- IOWA_ERROR_CODE_NO_ERROR
  : No error.

- IOWA_ERROR_CODE_LOW_BATTERY_POWER (1)
  : Low battery power.

- IOWA_ERROR_CODE_EXTERNAL_POWER_SUPPLY_OFF (2)
  : External power supply off.

- IOWA_ERROR_CODE_GPS_MODULE_FAILURE (3)
  : GPS module failure.

- IOWA_ERROR_CODE_LOW_RECEIVED_SIGNAL_STRENGTH (4)
: Low received signal strength.

- IOWA_ERROR_CODE_OUT_OF_MEMORY (5)
: Out of memory.

- IOWA_ERROR_CODE_SMS_FAILURE (6)
: SMS failure.

- IOWA_ERROR_CODE_IP_CONNECTIVITY_FAILURE (7)
: IP connectivity failure.

- IOWA_ERROR_CODE_PERIPHERAL_MALFUNCTION (8)
  : Peripheral malfunction.

\clearpage

### iowa_client_clear_device_error_code

** Prototype **

```c
iowa_status_t iowa_client_clear_device_error_code(iowa_context_t context,
                                                  uint8_t errorCode);
```

** Description **

`iowa_client_clear_device_error_code()` clears an error code from the Device object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*errorCode*
: The error code to clear between 1 and 32. It can't be *IOWA_ERROR_CODE_NO_ERROR*.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

IOWA_COAP_402_BAD_OPTION
: The error code *IOWA_ERROR_CODE_NO_ERROR* can't be cleared.

IOWA_COAP_404_NOT_FOUND
: The error code is not set.

** Header File **

iowa_client.h

** Notes **

LwM2M defined error code values are enumerated in [`iowa_client_set_device_error_code()`](ClientAPI.md#iowa_client_set_device_error_code).

\clearpage

### iowa_client_update_device_time_information

** Prototype **

```c
iowa_status_t iowa_client_update_device_time_information(iowa_context_t contextP,
                                                         iowa_device_time_info_t *timeInfoP);
```

** Description **

`iowa_client_update_device_time_information()` updates time information to Device object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*timeInfoP*
: Current device time information: [`iowa_device_time_info_t`](ClientAPI.md#iowa_device_time_info_t).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: *timeInfoP* is nil.
: currentTime in *timeInfoP* is set with a negative value.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not configured. Call first [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure).

** Header File **

iowa_client.h

** Notes **

For the device to expose its time information, [`iowa_client_configure()`](ClientAPI.md#iowa_client_configure) must have been to called with time information used.

\clearpage

### iowa_client_add_custom_object

** Prototype **

```c
iowa_status_t iowa_client_add_custom_object(iowa_context_t contextP,
                                            uint16_t objectID,
                                            size_t instanceCount,
                                            uint16_t * instanceIDs,
                                            size_t resourceCount,
                                            iowa_lwm2m_resource_desc_t * resourceArray,
                                            iowa_RWE_callback_t dataCallback,
                                            iowa_CD_callback_t instanceCallback,
                                            iowa_RI_callback_t resInstanceCallback,
                                            void * userData);
```

** Description **

`iowa_client_add_custom_object()` adds a new custom Object for the LwM2M Client to handle. The object is defined by its ID and a the list of the resources it contains.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectID*
: The ID of the Object.

*instanceCount*
: The number of elements in *instanceIDs*. This can be 0.

*instanceIDs*
: The IDs of the instances of the Object. This can be nil.

*resourceCount*
: The number of elements in *resourceArray*.

*resourceArray*
: An array of [`iowa_lwm2m_resource_desc_t`](ClientAPI.md#iowa_lwm2m_resource_desc_t) composing the Object.

*dataCallback*
: The callback to perform Read, Write and Execute operations on the resources.

*instanceCallback*
: The callback to perform Create and Delete operations on Object instances. This can be nil.

*resInstanceCallback*
: The callback to retrieve the list of instances of resources declared as multiple. This can be nil.

*userData*
: Passed as argument to the callbacks.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_403_FORBIDDEN
: objectID is 0, 1 or 3 which are reserved Object IDs.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *objectID* is *IOWA_LWM2M_ID_ALL* (65535).
: - *resourceCount* is zero.
: - *resourceArray* is nil.
: - *dataCallback* is nil.
: - *instanceIDs* is nil and *instanceCount* is not zero.
: - *resInstanceCallback* is nil and one of the resources in *resourceArray* has the **IOWA_RESOURCE_FLAG_MULTIPLE** flag set.

IOWA_COAP_409_CONFLICT
: this object already exists. Call first [`iowa_client_remove_custom_object()`](ClientAPI.md#iowa_client_remove_custom_object).

** Header File **

iowa_client.h

** Notes **

Object IDs 0, 1 and 3 are reserved and cannot be used.

Per Lightweight M2M specification, the ID of the instance of a single-instance Object is 0. When creating a single-instance Object, you can set *instanceCount* to zero and *instanceCallback* to nil. IOWA will automatically create an instance with ID 0.

When the LwM2M Server creates a new instance of the custom object, *instanceCallback* is first called with the new instance ID then *dataCallback* is called with *operation* set to **IOWA_DM_WRITE** to initialize the instance. Thus if *instanceCallback* is defined, *dataCallback* must handle the Write operation even on resources declared as read-only.

\clearpage

### iowa_client_remove_custom_object

** Prototype **

```c
iowa_status_t iowa_client_remove_custom_object(iowa_context_t contextP,
                                               uint16_t objectID);
```

** Description **

`iowa_client_remove_custom_object()` removes a custom Object created with [`iowa_client_add_custom_object()`](ClientAPI.md#iowa_client_add_custom_object).

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectID*
: The ID of the Object.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *objectID* is 0, 1 or 3 which are reserved Object IDs.

IOWA_COAP_404_NOT_FOUND
: *objectID* does not match any known object.

IOWA_COAP_406_NOT_ACCEPTABLE
: *objectID* is **IOWA_LWM2M_ID_ALL** (65535).

** Header File **

iowa_client.h

\clearpage

### iowa_client_object_resource_changed

** Prototype **

```c
iowa_status_t iowa_client_object_resource_changed(iowa_context_t contextP,
                                                  uint16_t objectID,
                                                  uint16_t instanceID,
                                                  uint16_t resourceID);
```

** Description **

`iowa_client_object_resource_changed()` informs the IOWA stack that the value of a LwM2M Object resource changed.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectID*
: The ID of the Object containing the resource.

*instanceID*
: The ID of the Instance containing the resource. This can be **IOWA_LWM2M_ID_ALL**.

*resourceID*
: The ID of the resource. This can be **IOWA_LWM2M_ID_ALL**.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *objectID* is 0, 1 or 3 which are reserved Object IDs.

IOWA_COAP_406_NOT_ACCEPTABLE
: *objectID* is **IOWA_LWM2M_ID_ALL** (65535).

** Header File **

iowa_client.h

** Notes **

This API does not check if the LwM2M Object resource exists. That's why this API does not return IOWA_COAP_404_NOT_FOUND.
Actually, `iowa_client_object_resource_changed()` is only searching a match between the running observation and the URI provided. If a match is found a notification is sent, else nothing happens.

\clearpage

### iowa_client_object_instance_changed

** Prototype **

```c
iowa_status_t iowa_client_object_instance_changed(iowa_context_t contextP,
                                                  uint16_t objectID,
                                                  uint16_t instanceID,
                                                  iowa_dm_operation_t operation);
```

** Description **

`iowa_client_object_instance_changed()` informs the IOWA stack that an instance of a LwM2M Object was created or deleted.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectID*
: The ID of the Object containing the instance.

*instanceID*
: The ID of the created or deleted Instance.

*operation*
: **IOWA_DM_CREATE** if it is a new instance. **IOWA_DM_DELETE** if the instance was removed.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_403_FORBIDDEN
: *objectID* is 0, 1 or 3 which are reserved Object IDs.

IOWA_COAP_404_NOT_FOUND
: either:
: - *objectID* does not match any known object.
: - *operation* is **IOWA_DM_DELETE** and *instanceID* does not match any known instance.

IOWA_COAP_405_METHOD_NOT_ALLOWED
: *operation* is neither **IOWA_DM_CREATE** nor **IOWA_DM_DELETE**.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *objectID* is **IOWA_LWM2M_ID_ALL** (65535).
: - *operation* is **IOWA_DM_CREATE** and **instanceID** was already present.

** Header File **

iowa_client.h

\clearpage

### iowa_client_notification_lock

** Prototype **

```c
void iowa_client_notification_lock(iowa_context_t contextP,
                                   bool enter);
```

** Description **

`iowa_client_notification_lock()` prevents or allows the IOWA stack to send notifications and registration updates.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*enter*
: **true** to stop the notification, **false** to resume the notification.

** Return Value **

None.

** Header File **

iowa_client.h

** Notes **

The main use is to perform several calls to [`iowa_client_object_resource_changed()`](ClientAPI.md#iowa_client_object_resource_changed) on an Object without generating a notification each time if the Object is under observation.

Registration updates are also blocked, allowing to add or remove several Objects or Object Instances.

This function is useful only if IOWA is built with the **IOWA_MULTITHREAD_SUPPORT** flag.
Inside a custom object callback, notifications are already disabled.

\clearpage

### iowa_client_send_heartbeat

** Prototype **

```c
iowa_status_t iowa_client_send_heartbeat(iowa_context_t contextP,
                                         uint16_t shortID);
```

** Description **

`iowa_client_send_heartbeat()` sends an heartbeat message to a server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortID*
: The Short ID assigned to this Server. Can be equal to **IOWA_LWM2M_ID_ALL**.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *shortID* is zero.

IOWA_COAP_404_NOT_FOUND
: *shortID* does not match any known server.

IOWA_COAP_412_PRECONDITION_FAILED
: client is not connected to the server with *shortID*. This can happen when:
: - The Server is a Bootstrap Server and the Client is already connect to a Server.
: - The Client is configured with more than one Server and has established the connection with only one.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: heartbeat message has not been sent by the platform.

** Header File **

iowa_client.h

** Notes **

If *shortID* is equal to **IOWA_LWM2M_ID_ALL**, the heartbeat message will be sent to all servers.

For non LoRaWAN Servers, a registration update message is sent to the Server. The [iowa_event_callback_t](ClientAPI.md#iowa_event_callback_t) will be called with a **IOWA_EVENT_REG_UPDATING** event. Then, if a reply is received from the Server, the [iowa_event_callback_t](ClientAPI.md#iowa_event_callback_t) will be called with either a **IOWA_EVENT_REG_REGISTERED** or **IOWA_EVENT_REG_FAILED** event. Nothing is done when no reply is received from the Server.

In the **IOWA_EVENT_REG_REGISTERED** case, the registration lifetime timer for the LwM2M Server is resetted.

\clearpage

### iowa_client_send_sensor_data

** Prototype **

```c
iowa_status_t iowa_client_send_sensor_data(iowa_context_t contextP,
                                           uint16_t shortId,
                                           iowa_sensor_uri_t *sensorUriP,
                                           size_t sensorUriCount,
                                           iowa_response_callback_t responseCb,
                                           void *userDataP);
```

** Description **

`iowa_client_send_sensor_data()` sends data from iowa_sensor_t to server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortId*
: The ID of the server. It can be **IOWA_LWM2M_ID_ALL** to send to all registered servers.

*sensorUriP*, *sensorUriCount*
: The sensor uri to send.

*responseCb*
: The callback called when the reply to this operation is known. This can be nil.

*userDataP*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *sensorUriCount* is zero or *sensorUriP* is nil.

IOWA_COAP_401_UNAUTHORIZED
: The destination LwM2M Server does not have the Read Access Right to the sent data. Refer to the [Access Control List Object][Access Control List Object] for details.

IOWA_COAP_403_FORBIDDEN
: *shortId* is not an acceptable value.

IOWA_COAP_404_NOT_FOUND
: either:
: - *shortId* does not match a known server.
: - at least one *sensorUriP[x]* does not match a known resource.

IOWA_COAP_405_METHOD_NOT_ALLOWED
: at least one *sensorUriP[x]*'s resource is not readable.

IOWA_COAP_412_PRECONDITION_FAILED
: the receiving LwM2M Server has muted the Send feature. See the Mute Send resource of the [Server Object][Server Object].

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: the client is not registered to the requested LwM2M Server or the communication with the requested LwM2M Server failed.

** Header File **

iowa_client.h

** Notes **

This API requires the compilation flag [**LWM2M_DATA_PUSH_SUPPORT**][LWM2M_DATA_PUSH_SUPPORT].

The *responseCb* will be called with the operation set to **IOWA_DM_DATA_PUSH**.

If *shortId* is **IOWA_LWM2M_ID_ALL**, the *responseCb* will be called for each registered LwM2M Server which has not muted the Client.

\clearpage

### iowa_client_send_data

** Prototype **

```c
iowa_status_t iowa_client_send_data(iowa_context_t contextP,
                                    uint16_t shortId,
                                    iowa_lwm2m_data_t *dataArrayP,
                                    size_t dataCount,
                                    iowa_response_callback_t responseCb,
                                    void *userDataP);
```

** Description **

`iowa_client_send_data()` sends data to server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortId*
: The ID of the server. It can be **IOWA_LWM2M_ID_ALL** to send to all registered servers.

*dataArrayP*, *dataCount*
: The data to send.

*responseCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - at least one *dataArrayP[x].resourceID* is **IOWA_LWM2M_ID_ALL**.
: - *dataCount* is zero or *dataArrayP* is nil.

IOWA_COAP_401_UNAUTHORIZED
: The destination LwM2M Server does not have the Read Access Right to the sent data. Refer to the [Access Control List Object][Access Control List Object] for details.

IOWA_COAP_403_FORBIDDEN
: *shortId* is not an acceptable value.

IOWA_COAP_404_NOT_FOUND
: either:
: - *shortId* does not match a known server.
: - at least one *dataArrayP[x]* does not match a known resource.

IOWA_COAP_405_METHOD_NOT_ALLOWED
: at least one *dataArrayP[x]*'s resource is not readable.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - one of the timestamped value has an negative timestamp.
: - at least one *dataArrayP[x]* has negative value with unsigned integer type

IOWA_COAP_412_PRECONDITION_FAILED
: the receiving LwM2M Server has muted the Send feature. See the Mute Send resource of the [Server Object][Server Object].

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: the client is not registered to the requested LwM2M Server or the communication with the requested LwM2M Server failed.

** Header File **

iowa_client.h

** Notes **

This API requires the compilation flag [**LWM2M_DATA_PUSH_SUPPORT**][LWM2M_DATA_PUSH_SUPPORT].

The *responseCb* will be called with the operation set to **IOWA_DM_DATA_PUSH**.

If *shortId* is **IOWA_LWM2M_ID_ALL**, the *responseCb* will be called for each registered LwM2M Server which has not muted the Client.

\clearpage

## Accelerometer Object API

This IPSO object can be used to represent a 1-3 axis accelerometer.

To be able to use this object, `iowa_accelerometer.h` must be included.

### iowa_client_add_accelerometer_object

** Prototype **

```c
iowa_status_t iowa_client_add_accelerometer_object(iowa_context_t context,
                                                   uint16_t optFlags,
                                                   float minRangeValue,
                                                   float maxRangeValue,
                                                   const char *sensorUnits,
                                                   iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_accelerometer_object()` creates an accelerometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*minRangeValue*
: Minimal range value for the accelerometer.

*maxRangeValue*
: Maximal range value for the accelerometer.

*sensorUnits*
: Measurement units definition

*idP*
: Used to store the ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *minRangeValue* argument is superior to *maxRangeValue* argument.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_accelerometer.h

** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_ACCELEROMETER_RSC_Y_VALUE
- IOWA_ACCELEROMETER_RSC_Z_VALUE
- IOWA_ACCELEROMETER_RSC_MIN_RANGE_VALUE
- IOWA_ACCELEROMETER_RSC_MAX_RANGE_VALUE

Moreover, you can add several optional resources at one time by using the following flags:

- IOWA_ACCELEROMETER_3_AXIS
- IOWA_ACCELEROMETER_RANGE_VALUE

\clearpage

### iowa_client_remove_accelerometer_object

** Prototype **

```c
iowa_status_t iowa_client_remove_accelerometer_object(iowa_context_t context,
                                                      iowa_sensor_t id);
```

** Description **

`iowa_client_remove_accelerometer_object()` removes an accelerometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not an accelerometer object. Valid *id* are only returned by [`iowa_client_add_accelerometer_object()`](ClientAPI.md#iowa_client_add_accelerometer_object).

IOWA_COAP_404_NOT_FOUND
: accelerometer referred by **id** does not exist.

** Header File **

objects/iowa_accelerometer.h

\clearpage

### iowa_client_accelerometer_update_axis

** Prototype **

```c
iowa_status_t iowa_client_accelerometer_update_axis(iowa_context_t context,
                                                    iowa_sensor_t id,
                                                    float xValue,
                                                    float yValue,
                                                    float zValue);
```

** Description **

`iowa_client_accelerometer_update_axis()` updates values of an accelerometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*xValue*
: X value axis

*yValue*
: Y value axis

*zValue*
: Z value axis

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not an accelerometer object. Valid *id* are only returned by [`iowa_client_add_accelerometer_object()`](ClientAPI.md#iowa_client_add_accelerometer_object).

IOWA_COAP_404_NOT_FOUND
: accelerometer referred by **id** does not exist.

** Header File **

objects/iowa_accelerometer.h

\clearpage

## Access Control List Object API

This LwM2M Object is used to check whether the LwM2M Server has access right for performing an operation.

To be able to use this object, `iowa_access_control_list.h` must be included and the define [**IOWA_SUPPORT_ACCESS_CONTROL_LIST_OBJECT**][IOWA_SUPPORT_ACCESS_CONTROL_LIST_OBJECT] must bet set.

### iowa_client_acl_rights_server_set

** Prototype **

```c
iowa_status_t iowa_client_acl_rights_server_set(iowa_context_t contextP,
                                                uint16_t objectId,
                                                uint16_t instanceId,
                                                uint16_t serverId,
                                                uint8_t accessRights);
```

** Description **

`iowa_client_acl_rights_server_set()` set the access rights for a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectId*
: ID of the Object.

*instanceId*
: ID of the Object Instance.

*serverId*
: Short Server ID of a LwM2M Server or **IOWA_ACL_DEFAULT_ID**.

*accessRights*
: new access rights to set.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: either:
: if *instanceId* is **IOWA_LWM2M_ID_ALL**, **accessRights** must be *IOWA_ACL_CREATE_RIGHT*.
: if *instanceId* is not **IOWA_LWM2M_ID_ALL**, **accessRights** cannot included *IOWA_ACL_CREATE_RIGHT*.

IOWA_COAP_403_FORBIDDEN
: either:
: *objectId* is **IOWA_LWM2M_ID_ALL**.
: *serverId* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: either:
: *objectId* does not refer to a supported Object.
: if *instanceId* is not **IOWA_LWM2M_ID_ALL**, *instanceId* does not refer to an instanciated Object Instance.
: *serverId* does not refer to a known Server Short ID nor **IOWA_ACL_DEFAULT_ID**.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_access_control_list.h

** Notes **

*accessRights* is a bit field which can contain the following values:

- *IOWA_ACL_NONE_RIGHT*: No access
- *IOWA_ACL_READ_RIGHT*: Read access
- *IOWA_ACL_WRITE_RIGHT*: Write access
- *IOWA_ACL_EXECUTE_RIGHT*: Execute access
- *IOWA_ACL_DELETE_RIGHT*: Delete access
- *IOWA_ACL_CREATE_RIGHT*: Create access

If access rights are already set for the targeted *objectId*, *instanceId* and *serverId*, they will be overwritten.

Access rights set through `iowa_client_acl_rights_server_set()` cannot be modified by any Server, since the Server Owner ID will be **IOWA_LWM2M_ID_ALL** (means Bootstrap Server).

To set the default access rights, *serverId* can be **IOWA_ACL_DEFAULT_ID**.

\clearpage

### iowa_client_acl_rights_server_clear

** Prototype **

```c
iowa_status_t iowa_client_acl_rights_server_clear(iowa_context_t contextP,
                                                  uint16_t objectId,
                                                  uint16_t instanceId,
                                                  uint16_t serverId);
```

** Description **

`iowa_client_acl_rights_server_clear()` unset the access rights for a LwM2M Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectId*
: ID of the Object.

*instanceId*
: ID of the Object Instance.

*serverId*
: Short Server ID of a LwM2M Server or **IOWA_ACL_DEFAULT_ID**.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *objectId* or *serverId* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *objectId*, *instanceId* or *serverId* do not have access rights set.

** Header File **

objects/iowa_access_control_list.h

\clearpage

### iowa_client_acl_rights_object_clear

** Prototype **

```c
iowa_status_t iowa_client_acl_rights_object_clear(iowa_context_t contextP,
                                                  uint16_t objectId,
                                                  uint16_t instanceId);
```

** Description **

`iowa_client_acl_rights_object_clear()` clears the access rights for an Object/Object Instance.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*objectId*
: ID of the Object.

*instanceId*
: ID of the Object Instance.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: *objectId* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *objectId* or *instanceId* do not have access rights set.

** Header File **

objects/iowa_access_control_list.h

\clearpage

## APN Connection Profile Object API

This LwM2M object specifies resources to enable a device to connect to an APN.

To be able to use this object, `iowa_apn_connection_profile.h` must be included.

### Data Structures and Constants

#### iowa_apn_connection_profile_details_t

```c
typedef struct
{
    char       *apn;
    bool        autoSelect;
    bool        enableStatus;
    uint8_t     authenticationType;
    char       *userName;
    char       *secret;
    char       *reconnectSchedule;
    char      **validityList;
    uint16_t    validityNumber;
    uint32_t   *connectionEstablishmentTimeList;
    uint16_t    connectionEstablishmentTimeNumber;
    uint8_t    *connectionEstablishmentResultList;
    uint16_t    connectionEstablishmentResultNumber;
    uint8_t    *connectionEstablishmentRejectCauseList;
    uint16_t    connectionEstablishmentRejectCauseNumber;
    uint32_t   *connectionEndTimeList;
    uint16_t    connectionEndTimeNumber;
    uint32_t    totalBytesSent;
    uint32_t    totalBytesReceived;
    char      **ipAddressList;
    uint16_t    ipAddressNumber;
    char      **prefixLengthList;
    uint16_t    prefixLengthNumber;
    char      **subnetMaskList;
    uint16_t    subnetMaskNumber;
    char      **gatewayList;
    uint16_t    gatewayNumber;
    char      **primaryDnsAddressList;
    uint16_t    primaryDnsAddressNumber;
    char      **secondaryDnsAddressList;
    uint16_t    secondaryDnsAddressNumber;
    uint8_t     qci;
    uint32_t    totalPacketsSent;
    uint8_t     pdnType;
    uint32_t    apnRateControl;
} iowa_apn_connection_profile_details_t;
```

*apn*
: APN of the APN connection profile.

*autoSelect*
: It enables the device to choose an APN according to a device specific algorithm.

*enableStatus*
: Allows the profile to be remotely activated or deactivated.

*authenticationType*
: 0: PAP, 1: CHAP, 2: PAP or CHAP, 3: None.

*userName*
: Used with e.g. PAP.

*secret*
: Used with e.g. PAP.

*reconnectSchedule*
: List of retry delay values in seconds to be used in case of unsuccessful connection establishment attempts.

*validity*
: Coma separated mobile country code, then mobile network code.

*connectionEstablishmentTime*
: UTC time of connection request.

*connectionEstablishmentResult*
: 0 = accepted, 1 = rejected.

*connectionEstablishmentRejectCause*
: Reject cause.

*connectionEndTime*
: UTC time of connection end.

*totalBytesSent*
: Rolling counter for total number of bytes sent via this interface since last device reset.

*totalBytesReceived*
: Rolling counter for total number of bytes sent via this interface since last device reset.

*ipAddress*
: May be IPv4 or IPv6 address.

*prefixLength*
: Associated with IPv6 address.

*subnetMask*
: Subnet mask.

*gateway*
: Gateway.

*primaryDnsAddress*
: Primary DNS address.

*secondaryDnsAddress*
: Secondary DNS address.

*qci*
: Quality of service Class Identifier. For LTE and NB-IoT only.

*totalPacketsSent*
: Rolling counter for total number of packets sent via this interface since last device reset.

*pdnType*
: 0=Non-IP, 1=IPv4, 2=IPv6, 3=IPv4v6.

*apnRateControl*
: Number of allowed uplink PDU transmissions per time interval per APN.

\clearpage

### Callbacks

#### iowa_apn_connection_profile_update_callback_t

This callback is called when the Server writes new information on the APN connection profile object.

```c
typedef iowa_status_t(*iowa_apn_connection_profile_update_callback_t)(
    char *profileName,
    iowa_dm_operation_t operation,
    uint32_t flags,
    iowa_apn_connection_profile_details_t *detailsP,
    void *userDataCallback,
    iowa_context_t contextP
);
```

*profileName*
: Unique name of the APN connection profile. This may be new.

*operation*
: The operation performed by the Server on this APN connection profile (creation, deletion, or write).

*flags*
: Specify values set in detailsP.

*detailsP*
: APN connection profile details. This may be nil.

*userDataCallback*
: User data callback.

*contextP*
: The IOWA context.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_enable_apn_connection_profile_object

#** Prototype **

```c
iowa_status_t iowa_client_enable_apn_connection_profile_object(
    iowa_context_t contextP,
    iowa_apn_connection_profile_update_callback_t updateCallback,
    void *userDataCallback
);
```

#** Description **

`iowa_client_enable_apn_connection_profile_object()` enables APN connection profiles management.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*updateCallback*
: Called to update state of the APN connection profile. This is called when the server request a new state.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: no update state callback provided means *updateCallback* is nil.

IOWA_COAP_409_CONFLICT
: APN connection profiles management was already enabled. Call first [`iowa_client_disable_apn_connection_profile_object()`](ClientAPI.md#iowa_client_disable_apn_connection_profile_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_apn_connection_profile.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

\clearpage

#### iowa_client_disable_apn_connection_profile_object

#** Prototype **

```c
iowa_status_t iowa_client_disable_apn_connection_profile_object(iowa_context_t contextP);
```

#** Description **

`iowa_client_disable_apn_connection_profile_object()` disables APN connection profiles management.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: APN connection profiles management was not enabled. [`iowa_client_enable_apn_connection_profile_object()`](ClientAPI.md#iowa_client_enable_apn_connection_profile_object) was not called before, or failed.

#** Header File **

objects/iowa_apn_connection_profile.h

\clearpage

#### iowa_client_add_apn_connection_profile

#** Prototype **

```c
iowa_status_t iowa_client_add_apn_connection_profile(iowa_context_t contextP,
                                                     const char *profileName,
                                                     uint32_t optFlags,
                                                     iowa_apn_connection_profile_details_t *detailsP);
```

#** Description **

`iowa_client_add_apn_connection_profile()` add an APN connection profile.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*profileName*
: Unique name of the APN connection profile.

*optFlags*
: Optional flags to add optional resources.

*detailsP*
: Apn connection profile details.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: inconsistent data inside *detailsP*.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - no profile name provided means *profileName* is nil.
: - no details provided means *detailsP* is nil.
: - *authenticationType*'s value is outside the [0, 3] range.
: - *connectionEstablishmentResult*'s value is different of 0 or 1.
: - *connectionEstablishmentRejectCause*'s value is outside the [0, 111] range.
: - *qci*'s value is outside the [0, 9] range.
: - *pdnType*'s value is outside the [0, 3] range.

IOWA_COAP_409_CONFLICT
: APN connection profile with *profileName* already exists.

IOWA_COAP_412_PRECONDITION_FAILED
: APN connection profile management was not enabled. Call first [`iowa_client_enable_apn_connection_profile_object()`](ClientAPI.md#iowa_client_enable_apn_connection_profile_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_apn_connection_profile.h

#** Notes **

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_APN_CONNECTION_PROFILE_RSC_APN
- IOWA_APN_CONNECTION_PROFILE_RSC_AUTO_SELECT_APN_DEVICE
- IOWA_APN_CONNECTION_PROFILE_RSC_ENABLE_STATUS
- IOWA_APN_CONNECTION_PROFILE_RSC_USER_NAME
- IOWA_APN_CONNECTION_PROFILE_RSC_SECRET
- IOWA_APN_CONNECTION_PROFILE_RSC_RECONNECT_SCHEDULE
- IOWA_APN_CONNECTION_PROFILE_RSC_VALIDITY
- IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_TIME
- IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_RESULT
- IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_REJECT_CAUSE
- IOWA_APN_CONNECTION_PROFILE_RSC_CONNECTION_END_TIME
- IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_BYTES_SENT
- IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_BYTES_RECEIVED
- IOWA_APN_CONNECTION_PROFILE_RSC_IP_ADDRESS
- IOWA_APN_CONNECTION_PROFILE_RSC_PREFIX_LENGTH
- IOWA_APN_CONNECTION_PROFILE_RSC_SUBNET_MASK
- IOWA_APN_CONNECTION_PROFILE_RSC_GATEWAY
- IOWA_APN_CONNECTION_PROFILE_RSC_PRIMARY_DNS_ADDRESS
- IOWA_APN_CONNECTION_PROFILE_RSC_SECONDARY_DNS_ADDRESS
- IOWA_APN_CONNECTION_PROFILE_RSC_QCI
- IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_PACKETS_SENT
- IOWA_APN_CONNECTION_PROFILE_RSC_PDN_TYPE
- IOWA_APN_CONNECTION_PROFILE_RSC_APN_RATE_CONTROL

\clearpage

#### iowa_client_remove_apn_connection_profile

#** Prototype **

```c
iowa_status_t iowa_client_remove_apn_connection_profile(iowa_context_t contextP,
                                                        const char *profileName);
```

#** Description **

`iowa_client_remove_apn_connection_profile()` removes an APN connection profile created with [`iowa_client_add_apn_connection_profile()`](ClientAPI.md#iowa_client_add_apn_connection_profile).

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*profileName*
: Unique name of the APN connection profile.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *profileName* does not match any known APN connection profile.

IOWA_COAP_406_NOT_ACCEPTABLE
: no profile name provided meaning *profileName* is nil.

IOWA_COAP_412_PRECONDITION_FAILED
: APN connection profile management was not enabled. Call first [`iowa_client_enable_apn_connection_profile_object()`](ClientAPI.md#iowa_client_enable_apn_connection_profile_object).

#** Header File **

objects/iowa_apn_connection_profile.h

\clearpage

#### iowa_client_update_apn_connection_profile

#** Prototype **

```c
iowa_status_t iowa_client_update_apn_connection_profile(
    iowa_context_t contextP,
    const char *profileName,
    uint32_t flags,
    iowa_apn_connection_profile_details_t *detailsP
);
```

#** Description **

`iowa_client_update_apn_connection_profile()` updates an APN connection profile.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*profileName*
: Unique name of the APN connection profile.

*flags*
: Specify resources to update.

*detailsP*
: The APN connection profile details.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: inconsistent data inside *detailsP*.

IOWA_COAP_404_NOT_FOUND
: APN connection profile does not exist. Add first the profile with [`iowa_client_add_apn_connection_profile()`](ClientAPI.md#iowa_client_add_apn_connection_profile).

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - no profile name provided means *profileName* is nil.
: - no details provided means *detailsP* is nil.
: - *authenticationType*'s value is outside the [0, 3] range.
: - *connectionEstablishmentResult*'s value is different of 0 or 1.
: - *connectionEstablishmentRejectCause*'s value is outside the [0, 111] range.
: - *qci*'s value is outside the [0, 9] range.
: - *pdnType*'s value is outside the [0, 3] range.

IOWA_COAP_412_PRECONDITION_FAILED
: APN connection profile management was not enabled. Call first [`iowa_client_enable_apn_connection_profile_object()`](ClientAPI.md#iowa_client_enable_apn_connection_profile_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_apn_connection_profile.h

#** Notes **

To specify resources to update, you can use the following flags:

- IOWA_APN_CONNECTION_PROFILE_RSC_APN
- IOWA_APN_CONNECTION_PROFILE_RSC_AUTO_SELECT_APN_DEVICE
- IOWA_APN_CONNECTION_PROFILE_RSC_ENABLE_STATUS
- IOWA_APN_CONNECTION_PROFILE_RSC_AUTHENTICATION_TYPE
- IOWA_APN_CONNECTION_PROFILE_RSC_USER_NAME
- IOWA_APN_CONNECTION_PROFILE_RSC_SECRET
- IOWA_APN_CONNECTION_PROFILE_RSC_RECONNECT_SCHEDULE
- IOWA_APN_CONNECTION_PROFILE_RSC_VALIDITY
- IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_TIME
- IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_RESULT
- IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_REJECT_CAUSE
- IOWA_APN_CONNECTION_PROFILE_RSC_CONNECTION_END_TIME
- IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_BYTES_SENT
- IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_BYTES_RECEIVED
- IOWA_APN_CONNECTION_PROFILE_RSC_IP_ADDRESS
- IOWA_APN_CONNECTION_PROFILE_RSC_PREFIX_LENGTH
- IOWA_APN_CONNECTION_PROFILE_RSC_SUBNET_MASK
- IOWA_APN_CONNECTION_PROFILE_RSC_GATEWAY
- IOWA_APN_CONNECTION_PROFILE_RSC_PRIMARY_DNS_ADDRESS
- IOWA_APN_CONNECTION_PROFILE_RSC_SECONDARY_DNS_ADDRESS
- IOWA_APN_CONNECTION_PROFILE_RSC_QCI
- IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_PACKETS_SENT
- IOWA_APN_CONNECTION_PROFILE_RSC_PDN_TYPE
- IOWA_APN_CONNECTION_PROFILE_RSC_APN_RATE_CONTROL

\clearpage

#### iowa_client_get_apn_connection_profile_object_link

#** Prototype **

```c
iowa_status_t iowa_client_get_apn_connection_profile_object_link(
    iowa_context_t contextP,
    const char *profileName,
    iowa_lwm2m_object_link_t *objectLinkP
);
```

#** Description **

`iowa_client_get_apn_connection_profile_object_link()` retrieves the LwM2M Object Link to an APN connection profile.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*profileName*
: Unique name of the APN connection profile.

*objectLinkP*
: Pointer to an [iowa_lwm2m_object_link_t](CommonAPI.md#iowa_lwm2m_object_link_t) where to store the LwM2M Object Link to the APN connection profile.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: APN connection profile does not exist. Add first the profile with [`iowa_client_add_apn_connection_profile()`](ClientAPI.md#iowa_client_add_apn_connection_profile).

IOWA_COAP_406_NOT_ACCEPTABLE
: *profileName* or *objectLinkP* is nil.

IOWA_COAP_412_PRECONDITION_FAILED
: APN connection profile management was not enabled. Call first [`iowa_client_enable_apn_connection_profile_object()`](ClientAPI.md#iowa_client_enable_apn_connection_profile_object).

#** Header File **

objects/iowa_apn_connection_profile.h

#** Notes **

This function is useful to fill the *activatedProfileNamesList* field of the [iowa_cellular_connectivity_info_t](ClientAPI.md#iowa_cellular_connectivity_info_t) structure.

\clearpage

## AT Command Object API

This LwM2M object can be used to execute an AT command on a cellular modem.

To be able to use this object, `iowa_at_command.h` must be included.

### Callbacks

#### iowa_at_command_run_t

This callback is used to execute an AT command.

```c
typedef iowa_status_t (*iowa_at_command_run_t)(iowa_sensor_t id,
                                               char *command,
                                               int timeout,
                                               void *userDataCallback,
                                               iowa_context_t contextP);
```

*id*
: ID of the object.

*command*
: The AT command to run.

*timeout*
: Amount of time in seconds allowed for the modem to respond to the command.

*userDataCallback*
: Application specific data from [`iowa_client_add_at_command_object`](ClientAPI.md#iowa_client_add_digital_output_object). Can be nil.

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

#** Header File **

objects/iowa_at_command.h

\clearpage

### API

#### iowa_client_add_at_command_object

#** Prototype **

```c
iowa_status_t iowa_client_add_at_command_object(iowa_context_t contextP,
                                                uint16_t optFlags,
                                                iowa_at_command_run_t run,
                                                void *userDataCallback,
                                                iowa_sensor_t *idP);
```

#** Description **

`iowa_client_add_at_command_object()` creates an AT Command object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*run*
: Called to send an AT command to the modem.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

*idP*
: Used to store the ID of the object.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: no run callback provided means *run* is nil.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_at_command.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_AT_COMMAND_RSC_TIMEOUT

\clearpage

#### iowa_client_remove_at_command_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_at_command_object(iowa_context_t contextP,
                                                   iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_at_command_object()` removes an AT Command object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not an AT Command object. Valid *id* are only returned by [`iowa_client_add_at_command_object()`](ClientAPI.md#iowa_client_add_at_command_object).

IOWA_COAP_404_NOT_FOUND
: AT Command referred by **id** does not exist.

#** Header File **

objects/iowa_at_command.h

\clearpage

#### iowa_client_at_command_set_response

#** Prototype **

```c
iowa_status_t iowa_client_at_command_set_response(iowa_context_t contextP,
                                                  iowa_sensor_t id,
                                                  const char *command,
                                                  const char *response,
                                                  const char *status);
```

#** Description **

`iowa_client_at_command_set_response()` updates result values after having executed an AT command.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object.

*command*
: The executed AT command.

*response*
: Response to the command.

*status*
: Status of the command execution as returned by the modem.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not an AT Command object. Valid *id* are only returned by [`iowa_client_add_at_command_object()`](ClientAPI.md#iowa_client_add_at_command_object).

IOWA_COAP_404_NOT_FOUND
: AT Command referred by **id** does not exist.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_at_command.h

#** Notes **

*response* and *status* can spread on multiple lines.

To reset *response* and *status* update their values with empty string :
- `iowa_client_at_command_set_response(contextP, Id, NULL, NULL, "");` to reset the AT command's status.
- `iowa_client_at_command_set_response(contextP, Id, NULL, "", NULL);` to reset the AT command's response.

\clearpage

## Bearer Selection Object API

This LwM2M object allows via remote bearer and network configuration to overwrite automatic network and bearer selection e.g. as supported by the UICC.

To be able to use this object, `iowa_bearer_selection.h` must be included.

### Data Structures and Constants

#### iowa_bearer_selection_info_t

```c
typedef struct
{
    uint8_t    *preferredCommBearerList;
    uint16_t    preferredCommBearerNumber;
    int8_t      acceptableGsm;
    int8_t      acceptableUmts;
    int16_t     acceptableLte;
    int16_t     acceptableEvDo;
    char       *cellLockList;
    char       *operatorList;
    bool        operatorListMode;
    char       *availablePlmns;
    int16_t     acceptableRsrpNbIot;
    int32_t     plmnSearchTimer;
    bool        attachWoPdnConnection;
} iowa_bearer_selection_info_t;
```

*preferredCommBearer*
: Preferred communications bearer.

*acceptableGsm*
: Provides guide to the application when performing manual network selection.

*acceptableUmts*
: Provides guide to the application when performing manual network selection.

*acceptableLte*
: Provides guide to the application when performing manual network selection.

*acceptableEvDo*
: Provides guide to the application when performing manual network selection.

*cellLockList*
: List of allowed Global Cell Identities.

*operatorList*
: List of MCC+MNC of operators, in priority order.

*operatorListMode*
: Indicates whether resource operator list represents the allowed operator list (white list), or, the preferred operator list.

*availablePlmns*
: Allows server to see results of network scan.

*acceptableRsrpNbIot*
: Provides guide to the application when performing manual network selection.

*plmnSearchTimer*
: Interval between periodic searches for higher priority PLMNs.

*attachWoPdnConnection*
: 0=attach with PDN connection, 1=attach without PDN connection

\clearpage

### Callbacks

#### iowa_bearer_selection_update_state_callback_t

This callback is called when the Server writes new information on the Bearer selection object.

```c
typedef iowa_status_t (*iowa_bearer_selection_update_state_callback_t) (
    iowa_sensor_t id,
    iowa_bearer_selection_info_t *infoP,
    void *userDataCallback,
    iowa_context_t contextP
);
```

*id*
: The instance of the Bearer selection.

*infoP*
: The bearer selection info.

*userDataCallback*
: The user data callback.

*contextP*
: The IOWA context.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_add_bearer_selection_object

#** Prototype **

```c
iowa_status_t iowa_client_add_bearer_selection_object(
    iowa_context_t contextP,
    uint16_t optFlags,
    iowa_bearer_selection_update_state_callback_t updateStateCallback,
    void *userDataCallback,
    iowa_sensor_t *idP
);
```

#** Description **

`iowa_client_add_bearer_selection_object()` creates a Bearer selection object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*updateStateCallback*
: Called to update state of the bearer selection. This is called when the server request a new state.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

*idP*
: Used to store the ID of the object.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *optFlags* is equals to zero.
: - no update state callback provided means *updateStateCallback* is nil.

IOWA_COAP_409_CONFLICT
: a bearer selection object already exists. To reconfigure the bearer selection object, call first [`iowa_client_remove_bearer_selection_object()`](ClientAPI.md#iowa_client_remove_bearer_selection_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_bearer_selection.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

Since this object has no mandatory resource, at least one optional resource must be used. To add optional resources, you can use the following flags:

- IOWA_BEARER_SELECTION_RSC_PREFERRED_COMM_BEARER
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSSI_GSM
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSCP_UMTS
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_LTE
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSSI_EV_DO
- IOWA_BEARER_SELECTION_RSC_CELL_LOCK_LIST
- IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST
- IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST_MODE
- IOWA_BEARER_SELECTION_RSC_AVAILABLE_PLMNS
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_NB_IOT
- IOWA_BEARER_SELECTION_RSC_PLMN_SEARCH_TIMER
- IOWA_BEARER_SELECTION_RSC_ATTACH_WO_PDN_CONNECTION

\clearpage

#### iowa_client_remove_bearer_selection_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_bearer_selection_object(iowa_context_t contextP,
                                                         iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_bearer_selection_object()` removes a Bearer selection object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a bearer selection object. Valid *id* are only returned by [`iowa_client_add_bearer_selection_object()`](ClientAPI.md#iowa_client_add_bearer_selection_object).

IOWA_COAP_404_NOT_FOUND
: bearer selection referred by **id** does not exist.

#** Header File **

objects/iowa_bearer_selection.h

\clearpage

#### iowa_client_bearer_selection_update

#** Prototype **

```c
iowa_status_t iowa_client_bearer_selection_update(iowa_context_t contextP,
                                                  iowa_sensor_t id,
                                                  uint16_t flags,
                                                  iowa_bearer_selection_info_t *infoP);
```

#** Description **

`iowa_client_bearer_selection_update()` updates the Bearer selection information.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object.

*flags*
: Optional flags to update resources.

*info*
: The Bearer selection information to update.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a bearer selection object. Valid *id* are only returned by [`iowa_client_add_bearer_selection_object()`](ClientAPI.md#iowa_client_add_bearer_selection_object).

IOWA_COAP_404_NOT_FOUND
: bearer selection referred by **id** does not exist.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *acceptableGsm*'s value is outside the [-48, -110] range.
: - *acceptableUmts*'s value is outside the [-25, -120] range.
: - *acceptableLte*'s value is outside the [-44, -140] range.
: - *acceptableRsrpNbIot*'s value is outside the [-44, -158] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_bearer_selection.h

#** Notes **

To specify resources to update, you can use the following flags:

- IOWA_BEARER_SELECTION_RSC_PREFERRED_COMM_BEARER
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSSI_GSM
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSCP_UMTS
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_LTE
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSSI_EV_DO
- IOWA_BEARER_SELECTION_RSC_CELL_LOCK_LIST
- IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST
- IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST_MODE
- IOWA_BEARER_SELECTION_RSC_AVAILABLE_PLMNS
- IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_NB_IOT
- IOWA_BEARER_SELECTION_RSC_PLMN_SEARCH_TIMER
- IOWA_BEARER_SELECTION_RSC_ATTACH_WO_PDN_CONNECTION

\clearpage

## Cellular Connectivity Object API

This LwM2M object specifies resources to enable a device to connect to a 3GPP or 3GPP2 bearer, including GPRS/EDGE, UMTS, LTE, NB-IoT, SMS.

To be able to use this object, `iowa_cellular_connectivity.h` must be included.

### Data Structures and Constants

#### iowa_cellular_connectivity_info_t

```c
typedef struct
{
    iowa_lwm2m_object_link_t *activatedProfileNamesList;
    uint16_t                  activatedProfileNamesNumber;
    char                     *smsc;
    int32_t                   disableRadioPeriod;
    char                     *moduleActivationCode;
    int32_t                   psmTimer;
    int32_t                   activeTimer;
    uint32_t                  servingPlmnRateControl;
    char                     *edrxParamIuMode;
    char                     *edrxParamWbS1Mode;
    char                     *edrxParamNbS1Mode;
    char                     *edrxParamAGbmMode;
} iowa_cellular_connectivity_info_t;
```

*activatedProfileNamesList*
: list of links to instances of the APN connection profile object representing every APN connection profile that has an activated connection to a PDN.

*activatedProfileNamesNumber*
: number of links to instances of the APN connection profile object representing every APN connection profile that has an activated connection to a PDN.

*smsc*
: address of the sms center.

*disableRadioPeriod*
: time period for which the device shall disconnect from cellular radio.

*moduleActivationCode*
: configurable in case the application needs to issue a code.

*psmTimer*
: Power Saving Mode timer.

*activeTimer*
: active timer.

*servingPlmnRateControl*
: maximum number of allowed uplink PDU transmissions.

*edrxParamIuMode*
: Extended DRX parameters for lu mode.

*edrxParamWbS1Mode*
: Extended DRX parameters for WB-S1 mode.

*edrxParamNbS1Mode*
: Extended DRX parameters for NB-S1 mode.

*edrxParamAGbmMode*
: Extended DRX parameters for A/Gb mode.

\clearpage

### Callbacks

#### iowa_cellular_connectivity_update_state_callback_t

This callback is called when the Server writes new information on the Cellular connectivity object.

```c
typedef iowa_status_t (*iowa_cellular_connectivity_update_state_callback_t)(
    iowa_sensor_t id,
    iowa_cellular_connectivity_info_t *infoP,
    void *userDataCallback,
    iowa_context_t contextP
);
```

*id*
: The instance of the Cellular connectivity.

*infoP*
: The Cellular connectivity info.

*userDataCallback*
: The user data callback.

*contextP*
: The IOWA context.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_add_cellular_connectivity_object

#** Prototype **

```c
iowa_status_t iowa_client_add_cellular_connectivity_object(
    iowa_context_t contextP,
    uint16_t optFlags,
    iowa_cellular_connectivity_update_state_callback_t updateStateCallback,
    void *userDataCallback,
    iowa_sensor_t *idP
);
```

#** Description **

`iowa_client_add_cellular_connectivity_object()` creates a Cellular connectivity object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*updateStateCallback*
: Called to update state of the cellular connectivity. This is called when the server request a new state.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

*idP*
: Used to store the ID of the object.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: no update state callback provided means *updateStateCallback* is nil.

IOWA_COAP_409_CONFLICT
: a cellular connectivity object already exists. Call first [`iowa_client_remove_cellular_connectivity_object()`](ClientAPI.md#iowa_client_remove_cellular_connectivity_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_cellular_connectivity.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_CELLULAR_CONNECTIVITY_RSC_SMSC_ADDRESS
- IOWA_CELLULAR_CONNECTIVITY_RSC_DISABLE_RADIO_PERIOD
- IOWA_CELLULAR_CONNECTIVITY_RSC_MODULE_ACTIVATION_CODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_PSM_TIMER
- IOWA_CELLULAR_CONNECTIVITY_RSC_ACTIVE_TIMER
- IOWA_CELLULAR_CONNECTIVITY_RSC_PLMN_RATE_CONTROL
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_IU_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_WB_S1_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_NB_S1_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_A_GB_MODE

\clearpage

#### iowa_client_remove_cellular_connectivity_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_cellular_connectivity_object(iowa_context_t contextP,
                                                              iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_cellular_connectivity_object()` removes a Cellular connectivity object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a cellular connectivity object. Valid *id* are only returned by [`iowa_client_add_cellular_connectivity_object()`](ClientAPI.md#iowa_client_add_cellular_connectivity_object).

IOWA_COAP_404_NOT_FOUND
: cellular connectivity referred by **id** does not exist.

#** Header File **

objects/iowa_cellular_connectivity.h

\clearpage

#### iowa_client_cellular_connectivity_update

#** Prototype **

```c
iowa_status_t iowa_client_cellular_connectivity_update(
    iowa_context_t contextP,
    iowa_sensor_t id,
    uint16_t flags,
    iowa_cellular_connectivity_info_t *infoP
);
```

#** Description **

`iowa_client_cellular_connectivity_update()` updates the Cellular connectivity information.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object.

*flags*
: Optional flags to update resources.

*infoP*
: The Cellular connectivity information to update.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a cellular connectivity object. Valid *id* are only returned by [`iowa_client_add_cellular_connectivity_object()`](ClientAPI.md#iowa_client_add_cellular_connectivity_object).

IOWA_COAP_404_NOT_FOUND
: cellular connectivity referred by **id** does not exist.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_cellular_connectivity.h

#** Notes **

To specify resources to update, you can use the following flags:

- IOWA_CELLULAR_CONNECTIVITY_RSC_SMSC_ADDRESS
- IOWA_CELLULAR_CONNECTIVITY_RSC_DISABLE_RADIO_PERIOD
- IOWA_CELLULAR_CONNECTIVITY_RSC_MODULE_ACTIVATION_CODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_PSM_TIMER
- IOWA_CELLULAR_CONNECTIVITY_RSC_ACTIVE_TIMER
- IOWA_CELLULAR_CONNECTIVITY_RSC_PLMN_RATE_CONTROL
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_IU_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_WB_S1_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_NB_S1_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_A_GB_MODE
- IOWA_CELLULAR_CONNECTIVITY_RSC_ACTIVATED_PROFILE_NAMES

\clearpage

## Connectivity Monitoring Object API

This LwM2M Object enables monitoring of parameters related to network connectivity.

To be able to use this object, `iowa_connectivity_monitoring.h` must be included.

### Data Structures and Constants

#### iowa_connectivity_monitoring_info_t

```c
typedef struct
{
    uint8_t     networkBearer;
    uint8_t    *availableNetworkBearerList;
    uint8_t     availableNetworkBearerNumber;
    int16_t     radioSignalStrength;
    int16_t     linkQuality;
    char      **ipAddressList;
    uint16_t    ipAddressNumber;
    char      **routerIpAddressesList;
    uint16_t    routerIpAddressesNumber;
    uint8_t     linkUtilization;
    char      **apnList;
    uint16_t    apnNumber;
    uint64_t    cellId;
    uint16_t    smnc;
    uint16_t    smcc;
} iowa_connectivity_monitoring_info_t;
```

*networkBearer*
: Network bearer used for the current session.

*availableNetworkBearerList*
: List of current available network bearers.

*availableNetworkBearerNumber*
: Number of current available network bearers.

*radioSignalStrength*
: Average value of the received signal strength indication.

*linkQuality*
: Received link quality.

*ipAddressList*
: List of IP addresses assigned to the connectivity interface.

*ipAddressNumber*
: Number of IP addresses assigned to the connectivity interface.

*routerIpAddressesList*
: List of IP addresses of the next-hop IP router.

*routerIpAddressesNumber*
: Number of IP addresses of the next-hop IP router.

*linkUtilization*
: The percentage indicating the average utilization of the link to the next-hop IP router.

*apnList*
: List of Access Point Names.

*apnNumber*
: Number of Access Point Names.

*cellId*
: Serving Cell ID.

*smnc*
: Serving Mobile Network Code.

*smcc*
: Serving Mobile Country Code.

\clearpage

### API

#### iowa_client_add_connectivity_monitoring_object

#** Prototype **

```c
iowa_status_t iowa_client_add_connectivity_monitoring_object(iowa_context_t contextP,
                                                             uint16_t optFlags,
                                                             iowa_sensor_t *idP);
```

#** Description **

`iowa_client_add_connectivity_monitoring_object()` creates a Connectivity monitoring object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*idP*
: Used to store the ID of the object.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_409_CONFLICT
: a connectivity monitoring object already exists. Call first [`iowa_client_remove_connectivity_monitoring_object()`](ClientAPI.md#iowa_client_remove_connectivity_monitoring_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_connectivity_monitoring.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_CONNECTIVITY_MONITORING_RSC_LINK_QUALITY
- IOWA_CONNECTIVITY_MONITORING_RSC_ROUTER_IP_ADDR
- IOWA_CONNECTIVITY_MONITORING_RSC_LINK_USAGE
- IOWA_CONNECTIVITY_MONITORING_RSC_APN
- IOWA_CONNECTIVITY_MONITORING_RSC_CELL_ID
- IOWA_CONNECTIVITY_MONITORING_RSC_SMNC
- IOWA_CONNECTIVITY_MONITORING_RSC_SMCC

\clearpage

#### iowa_client_remove_connectivity_monitoring_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_connectivity_monitoring_object(iowa_context_t contextP,
                                                                iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_connectivity_monitoring_object()` removes a Connectivity monitoring object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a connectivity monitoring object. Valid *id* are only returned by [`iowa_client_add_connectivity_monitoring_object()`](ClientAPI.md#iowa_client_add_connectivity_monitoring_object).

IOWA_COAP_404_NOT_FOUND
: connectivity monitoring referred by **id** does not exist.

#** Header File **

objects/iowa_connectivity_monitoring.h

\clearpage

#### iowa_client_connectivity_monitoring_update

#** Prototype **

```c
iowa_status_t iowa_client_connectivity_monitoring_update(
    iowa_context_t contextP,
    iowa_sensor_t id,
    uint16_t flags,
    iowa_connectivity_monitoring_info_t *infoP
);
```

#** Description **

`iowa_client_connectivity_monitoring_update()` updates the Connectivity monitoring information.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object.

*flags*
: Optional flags to update resources.

*infoP*
: The Connectivity monitoring information to update.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a connectivity monitoring object. Valid *id* are only returned by [`iowa_client_add_connectivity_monitoring_object()`](ClientAPI.md#iowa_client_add_connectivity_monitoring_object).

IOWA_COAP_404_NOT_FOUND
: connectivity monitoring referred by **id** does not exist.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *networkBearer*'s value is outside the [0, 50] range.
: - *availableNetworkBearerList*'s value is outside the [0, 50] range.
: - *linkUtilization*'s value is outside the [0, 100] range.
: - *smnc*'s value is outside the [0, 999] range.
: - *smcc*'s value is outside the [0, 999] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_connectivity_monitoring.h

#** Notes **

To specify resources to update, you can use the following flags:

- IOWA_CONNECTIVITY_MONITORING_RSC_BEARER
- IOWA_CONNECTIVITY_MONITORING_RSC_AVAILABLE_BEARER
- IOWA_CONNECTIVITY_MONITORING_RSC_SIGNAL_STRENGTH
- IOWA_CONNECTIVITY_MONITORING_RSC_LINK_QUALITY
- IOWA_CONNECTIVITY_MONITORING_RSC_IP_ADDR
- IOWA_CONNECTIVITY_MONITORING_RSC_ROUTER_IP_ADDR
- IOWA_CONNECTIVITY_MONITORING_RSC_LINK_USAGE
- IOWA_CONNECTIVITY_MONITORING_RSC_APN
- IOWA_CONNECTIVITY_MONITORING_RSC_CELL_ID
- IOWA_CONNECTIVITY_MONITORING_RSC_SMNC
- IOWA_CONNECTIVITY_MONITORING_RSC_SMCC

\clearpage

## Connectivity Statistics Object API

This LwM2M Object enables client to collect statistical information and enables the LwM2M Server to retrieve these information, set the collection duration and reset the statistical parameters.

To be able to use this object, `iowa_connectivity_stats.h` must be included.

### iowa_client_add_connectivity_stats_object

** Prototype **

```c
iowa_status_t iowa_client_add_connectivity_stats_object(iowa_context_t context,
                                                        uint16_t optFlags,
                                                        iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_connectivity_stats_object()` creates a connectivity statistics object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*idP*
: Used to store the ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_409_CONFLICT
: a connectivity statistics object already exists. To reconfigure the connectivity statistics object, call first [`iowa_client_remove_connectivity_stats_object()`](ClientAPI.md#iowa_client_remove_connectivity_stats_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_connectivity_stats.h

** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_CONNECTIVITY_STATS_RSC_SMS_TX_COUNTER
- IOWA_CONNECTIVITY_STATS_RSC_SMS_RX_COUNTER
- IOWA_CONNECTIVITY_STATS_RSC_TX_DATA
- IOWA_CONNECTIVITY_STATS_RSC_RX_DATA
- IOWA_CONNECTIVITY_STATS_RSC_MAX_MESSAGE_SIZE
- IOWA_CONNECTIVITY_STATS_RSC_AVERAGE_MESSAGE_SIZE
- IOWA_CONNECTIVITY_STATS_RSC_COLLECTION_PERIOD

Moreover, you can add several optional resources at one time by using the following flags:

- IOWA_CONNECTIVITY_STATS_SMS
- IOWA_CONNECTIVITY_STATS_IP_DATA

\clearpage

### iowa_client_remove_connectivity_stats_object

** Prototype **

```c
iowa_status_t iowa_client_remove_connectivity_stats_object(iowa_context_t contextP,
                                                           iowa_sensor_t id);
```

** Description **

`iowa_client_remove_connectivity_stats_object()` removes a connectivity statistics object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a connectivity statistics object. Valid *id* are only returned by [`iowa_client_add_connectivity_stats_object()`](ClientAPI.md#iowa_client_add_connectivity_stats_object).

IOWA_COAP_404_NOT_FOUND
: no connectivity statistics object to remove. [`iowa_client_add_connectivity_stats_object()`](ClientAPI.md#iowa_client_add_connectivity_stats_object) was not called before, or failed.

** Header File **

objects/iowa_connectivity_stats.h

\clearpage

### iowa_client_connectivity_stats_update_sms

** Prototype **

```c
iowa_status_t iowa_client_connectivity_stats_update_sms(iowa_context_t context,
                                                        iowa_sensor_t id,
                                                        uint8_t direction);
```

** Description **

`iowa_client_connectivity_stats_update_sms()` updates the SMS TX or RX statistics.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*direction*
: Specify if this is a reception or a transmission trigger.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: Bad value for argument *direction* or *id* is not a connectivity statistics object. Valid *id* are only returned by [`iowa_client_add_connectivity_stats_object()`](ClientAPI.md#iowa_client_add_connectivity_stats_object).

IOWA_COAP_404_NOT_FOUND
: no connectivity statistics object added.

** Header File **

objects/iowa_connectivity_stats.h

** Notes **

Argument *direction* of `iowa_client_connectivity_stats_update_sms()` can be one of the following values:

- IOWA_CONNECTIVITY_STATS_TX (0)
- IOWA_CONNECTIVITY_STATS_RX (1)

\clearpage

### iowa_client_connectivity_stats_update_ip_data

** Prototype **

```c
iowa_status_t iowa_client_connectivity_stats_update_ip_data(iowa_context_t context,
                                                            iowa_sensor_t id,
                                                            uint8_t direction,
                                                            size_t length);
```

** Description **

`iowa_client_connectivity_stats_update_ip_data()` updates the IP data statistics.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object.

*direction*
: Specify if this is a reception or a transmission trigger.

*length*
: Length in bytes of the transmitted or received data.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: Bad value for argument *direction* or *id* is not a connectivity statistics object. Valid *id* are only returned by [`iowa_client_add_connectivity_stats_object()`](ClientAPI.md#iowa_client_add_connectivity_stats_object).

IOWA_COAP_404_NOT_FOUND
: no connectivity statistics object added.

** Header File **

objects/iowa_connectivity_stats.h

** Notes **

Argument *direction* of `iowa_client_connectivity_stats_update_ip_data()` can be one of the following values:

- IOWA_CONNECTIVITY_STATS_TX (0)
- IOWA_CONNECTIVITY_STATS_RX (1)

\clearpage

## Digital Output Object API

This IPSO object represents generic digital output for non-specific actuators.

To be able to use this object, `iowa_digital_output.h` must be included.

### Callbacks

#### iowa_digital_output_state_callback_t

This callback is used to update the state of the digital output. Request from a server to a client.

```c
typedef iowa_status_t (*iowa_digital_output_state_callback_t)(iowa_sensor_t id,
                                                              bool state,
                                                              bool polarity,
                                                              void *userDataCallback,
                                                              iowa_context_t contextP);
```

*id*
: ID of the object

*state*
: New state

*polarity*
: New polarity

*userDataCallback*
: Application specific data from [`iowa_client_add_digital_output_object`](ClientAPI.md#iowa_client_add_digital_output_object). Can be nil.

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_add_digital_output_object

#** Prototype **

```c
iowa_status_t iowa_client_add_digital_output_object(
    iowa_context_t context,
    uint16_t optFlags,
    iowa_digital_output_state_callback_t updateStateCallback,
    void *userDataCallback,
    const char *applicationType,
    iowa_sensor_t *idP
);
```

#** Description **

`iowa_client_add_digital_output_object()` creates a digital output object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*updateStateCallback*
: Called to update state of the digital output. This is called when the server request a new state.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

*applicationType*
: The application type

*idP*
: Used to store the ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: no update state callback provided means *updateStateCallback* is nil.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_digital_output.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_DIGITAL_OUTPUT_STATS_RSC_POLARITY

\clearpage

#### iowa_client_remove_digital_output_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_digital_output_object(iowa_context_t context,
                                                       iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_digital_output_object()` removes a digital output object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a digital output object. Valid *id* are only returned by [`iowa_client_add_digital_output_object()`](ClientAPI.md#iowa_client_add_digital_output_object).

IOWA_COAP_404_NOT_FOUND
: digital output referred by **id** does not exist.

#** Header File **

objects/iowa_digital_output.h

\clearpage

#### iowa_client_digital_output_update_state

#** Prototype **

```c
iowa_status_t iowa_client_digital_output_update_state(iowa_context_t context,
                                                      iowa_sensor_t id,
                                                      bool state,
                                                      bool polarity);
```

#** Description **

`iowa_client_digital_output_update_state()` updates values of a digital output object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*state*
: New state

*polarity*
: New polarity

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a digital output object. Valid *id* are only returned by [`iowa_client_add_digital_output_object()`](ClientAPI.md#iowa_client_add_digital_output_object).

IOWA_COAP_404_NOT_FOUND
: digital output referred by **id** does not exist.

#** Header File **

objects/iowa_digital_output.h

\clearpage

## Dimmer Object API

This IPSO object represents a dimmer.

To be able to use this object, `iowa_dimmer.h` must be included.

### Callbacks

#### iowa_dimmer_state_callback_t

This callback is used to update the state of the dimmer. Request from a server to a client.

```c
typedef iowa_status_t (*iowa_dimmer_state_callback_t)(iowa_sensor_t id,
                                                      float level,
                                                      void *userDataCallback,
                                                      iowa_context_t contextP);
```

*id*
: ID of the object

*level*
: New level.

*userDataCallback*
: Application specific data from [`iowa_client_add_dimmer_object`](ClientAPI.md#iowa_client_add_dimmer_object). Can be nil.

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_add_dimmer_object

#** Prototype **

```c
iowa_status_t iowa_client_add_dimmer_object(iowa_context_t context,
                                            uint16_t optFlags,
                                            float level,
                                            iowa_dimmer_state_callback_t updateStateCallback,
                                            void *userDataCallback,
                                            const char *applicationType,
                                            iowa_sensor_t *idP);
```

#** Description **

`iowa_client_add_dimmer_object()` creates a dimmer object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*level*
: The initial value measured by the dimmer, must be inside the range [0.0; 100.0].

*updateStateCallback*
: Called to update state of the dimmer. This is called when the server request a new state.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

*applicationType*
: The application type

*idP*
: Used to store the ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: *level* is outside the range [0.0; 100.0].

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.

#** Header File **

objects/iowa_dimmer.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_DIMMER_RSC_ON_TIME
- IOWA_DIMMER_RSC_OFF_TIME

\clearpage

#### iowa_client_remove_dimmer_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_dimmer_object(iowa_context_t context,
                                               iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_dimmer_object()` removes a dimmer object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a dimmer object. Valid *id* are only returned by [`iowa_client_add_dimmer_object()`](ClientAPI.md#iowa_client_add_dimmer_object).

IOWA_COAP_404_NOT_FOUND
: dimmer referred by *id* does not exist.

#** Header File **

objects/iowa_dimmer.h

\clearpage

#### iowa_client_dimmer_update_value

#** Prototype **

```c
iowa_status_t iowa_client_dimmer_update_value(iowa_context_t contextP,
                                              iowa_sensor_t id,
                                              float level);
```

#** Description **

`iowa_client_dimmer_update_value()` updates dimmer's level value.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*level*
: New level value, must be inside the range [0.0; 100.0].

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a dimmer object. Valid *id* are only returned by [`iowa_client_add_dimmer_object()`](ClientAPI.md#iowa_client_add_dimmer_object).

IOWA_COAP_404_NOT_FOUND
: dimmer referred by *id* does not exist.

IOWA_COAP_406_NOT_ACCEPTABLE
: *level* is outside the range [0.0; 100.0].

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.

#** Header File **

objects/iowa_dimmer.h

\clearpage

#### iowa_client_dimmer_update_values

#** Prototype **

```c
iowa_status_t iowa_client_dimmer_update_values(iowa_context_t contextP,
                                               iowa_sensor_t id,
                                               size_t valueCount,
                                               iowa_ipso_timed_value_t *valueArray);
```

#** Description **

`iowa_client_dimmer_update_values()` updates dimmer's level value.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*valueCount*
: The number of values in *valueArray*.

*valueArray*
: The [`iowa_ipso_timed_value_t`](ClientAPI.md#iowa_ipso_timed_value_t) list of new level values.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a dimmer object. Valid *id* are only returned by [`iowa_client_add_dimmer_object()`](ClientAPI.md#iowa_client_add_dimmer_object).

IOWA_COAP_404_NOT_FOUND
: dimmer referred by *id* does not exist.

IOWA_COAP_406_NOT_ACCEPTABLE
: at least one *valueArray* value is outside the range [0.0; 100.0].

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed when either:
: - the new level array's last value is greater than 0 and the previous one is 0.
: - the new level array's last value is 0 and the previous one is greater than 0.

#** Header File **

objects/iowa_dimmer.h

\clearpage

## Firmware Update Object API

This LwM2M Object enables management of firmware which is to be updated.

To be able to use this object, `iowa_firmware_update.h` must be included and **IOWA_SUPPORT_FIRMWARE_UPDATE_OBJECT** must be defined before building the library.

The [`Device Update`][Device Update]/[`Firmware Update`][Firmware Update] part of this specification adds more explanation about its mechanism and how to use it with IOWA.

### Data Structures and Constants

#### iowa_fw_status_t

This is an enumeration of the following values:

IOWA_FW_STATUS_SUCCESSFUL
: success of the new firmware package download or of the firmware update.

IOWA_FW_STATUS_OUT_OF_STORAGE
: not enough storage for the new firmware package. (*downloadCb* only)

IOWA_FW_STATUS_OUT_OF_MEMORY
: out of memory error during the download of the new firmware package. (*downloadCb* only)

IOWA_FW_STATUS_CONNECTION_LOST
: connection lost during the download of the new firmware package. (*downloadCb* only)

IOWA_FW_STATUS_INTEGRITY_CHECK_FAILURE
: integrity check failure of the new firmware package.

IOWA_FW_STATUS_UNSUPPORTED_TYPE
: unsupported new firmware package type.

IOWA_FW_STATUS_INVALID_URI
: invalid URI to download the new firmware package. (*downloadCb* only)

IOWA_FW_STATUS_UPDATE_FAILED
: firmware update failed. (*updateCb* only)

IOWA_FW_STATUS_UNSUPPORTED_PROTOCOL
: unsupported protocol in URI to download the new firmware package. (*downloadCb* only)

\clearpage

### Callbacks

#### iowa_fw_download_callback_t

This callback is called when the Server requests the device to download a new Firmware Package.

```c
typedef void(*iowa_fw_download_callback_t) (char * uri,
                                            void * userData,
                                            iowa_context_t contextP);
```

*uri*
: The URI to download the package from.

*userData*
: The parameter to [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

*contextP*
: The IOWA context on which [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure) was called.

*uri* can be nil. In this case, a current download must be aborted.

\clearpage

#### iowa_fw_write_callback_t

This callback is called several times when the Server pushes the new Firmware Package to the device. The expected behavior is the same as writing to a file stream i.e. unless it is reset, written data are appended to the previous ones.

```c
typedef iowa_fw_status_t(*iowa_fw_write_callback_t) (iowa_fw_write_cmd_t cmd,
                                                     size_t dataLength,
                                                     uint8_t *data,
                                                     void *userData,
                                                     iowa_context_t contextP);
```

At the start of the push of the Firmware Package or if the LwM2M Server cancels it, this callback is called with the following parameters:

*cmd*
: **IOWA_FW_PACKAGE_RESET**

*dataLength*
: 0

*data*
: NULL

*userData*
: The parameter to [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

*contextP*
: The IOWA context on which [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure) was called.

When the Firmware Package is received, this callback is called several times with the following parameters:

*cmd*
: **IOWA_FW_PACKAGE_WRITE**

*dataLength*
: The length of the buffer pointed by *data*.

*data*
: The next chunk of the Firmware Package to write.

*userData*
: The parameter to [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

*contextP*
: The IOWA context on which [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure) was called.

At the end of the push of the Firmware package, this callback is called with the following parameters:

*cmd*
: **IOWA_FW_PACKAGE_WRITE**

*dataLength*
: 0

*data*
: NULL

*userDataP*
: The data passed to [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

*contextP*
: The IOWA context on which [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure) was called.

#** Return Value **

IOWA_FW_STATUS_SUCCESSFUL
: success.

IOWA_FW_STATUS_OUT_OF_STORAGE
: not enough storage for the new Firmware Package.

IOWA_FW_STATUS_OUT_OF_MEMORY
: out of memory error.

IOWA_FW_STATUS_INTEGRITY_CHECK_FAILURE
: integrity check failure of the new Firmware Package.

IOWA_FW_STATUS_UNSUPPORTED_TYPE
: unsupported new Firmware Package type.

\clearpage

#### iowa_fw_update_callback_t

This callback is called when the Server requests the device to update itself with the new Firmware Package.

```c
typedef void(*iowa_fw_update_callback_t) (void * userData,
                                          iowa_context_t contextP);
```

*userData*
: The parameter to [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

*contextP*
: The IOWA context on which [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure) was called.

\clearpage

### API

#### iowa_client_firmware_update_configure

#** Prototype **

```c
iowa_status_t iowa_client_firmware_update_configure(iowa_context_t contextP,
                                                    const char *packageName,
                                                    const char *packageVersion,
                                                    iowa_fw_download_callback_t downloadCb,
                                                    iowa_fw_write_callback_t writeCb,
                                                    iowa_fw_update_callback_t updateCb,
                                                    void *userData);
```

#** Description **

`iowa_client_firmware_update_configure()` configures the firmware update feature of the IOWA stack.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

 *packageName*
 : The user-defined name of the current firmware. This can be nil.

 *packageVersion*
 : The user-defined version of the current firmware. This can be nil.

 *downloadCb*
 : The callback called to download a new firmware. This can be nil.

 *writeCb*
 : The callback called to write chunks of the new firmware to the device storage. This can be nil.

 *updateCb*
 : The callback called to update the device with the new firmware.

 *userData*
 : Passed as argument to the callbacks.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *updateCb* is nil.
: - both *downloadCb* and *writeCb* are nil. At least one must be defined.

IOWA_COAP_409_CONFLICT
: the firmware update feature is already configured.

#** Header File **

objects/iowa_firmware_update.h

#** Notes **

The LwM2M Server has two methods to provide the Firmware Package:

- the pull method: the LwM2M Server provides the URI of the Firmware Package and the LwM2M Client downloads it directly. To use this method, *downloadCb* must be set.

- the push method: the LwM2M Server writes the Firmware Package in a LwM2M Resource exposed by the Client. To use this method, *writeCb* must be set.

The Client can support both methods at the same time.

*downloadCb* and *updateCb* do not return any value. The progress and result of their operation are indicated asynchronously by calling [`iowa_client_firmware_update_set_status()`](ClientAPI.md#iowa_client_firmware_update_set_status).

\clearpage

#### iowa_client_firmware_update_configure_full

#** Prototype **

```c
iowa_status_t iowa_client_firmware_update_configure_full(
    iowa_context_t contextP,
    const char *packageName,
    const char *packageVersion,
    uint8_t protocolSupport,
    iowa_fw_download_callback_t downloadCb,
    iowa_fw_write_callback_t writeCb,
    iowa_fw_update_callback_t updateCb,
    void *userData
);
```

#** Description **

`iowa_client_firmware_update_configure_full()` configures the firmware update feature of the IOWA stack with the "protocol support" resource.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

 *packageName*
 : The user-defined name of the current firmware. This can be nil.

 *packageVersion*
 : The user-defined version of the current firmware. This can be nil.

 *protocolSupport*
 : A bit-mask indicating supported protocols in *downloadCb*. This can be 0.

 *downloadCb*
 : The callback called to download a new firmware. This can be nil.

 *writeCb*
 : The callback called to write chunks of the new firmware to the device storage. This can be nil.

 *updateCb*
 : The callback called to update the device with the new firmware.

 *userData*
 : Passed as argument to the callbacks.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *updateCb* is nil.
: - both *downloadCb* and *writeCb* are nil.
: - *downloadCb* is nil and *protocolSupport* is not 0.

IOWA_COAP_409_CONFLICT
: the firmware update feature is already configured.

#** Header File **

objects/iowa_firmware_update.h

#** Notes **

***protocolSupport*** is a combination of the following:

- **IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAP**: Constrained Application Protocol (CoAP)
- **IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAPS**: DTLS-Secured CoAP
- **IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_HTTP**: HTTP 1.1
- **IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_HTTPS**: TLS-Secured HTTP 1.1
- **IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAP_TCP**: CoAP over TCP
- **IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAP_TLS**: CoAP over TLS

\clearpage

#### iowa_client_firmware_update_set_status

#** Prototype **

```c
iowa_status_t iowa_client_firmware_update_set_status(iowa_context_t contextP,
                                                     iowa_fw_status_t status);
```

#** Description **

`iowa_client_firmware_update_set_status()` informs the IOWA stack of the result of the callbacks *downloadCb* and *updateCb* of [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

 *status*
 : The result of the current firmware update operation.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: the firmware update feature is not configured. Call first [`iowa_client_firmware_update_configure()`](ClientAPI.md#iowa_client_firmware_update_configure).

IOWA_COAP_412_PRECONDITION_FAILED
: the value of *status* is unexpected. See [`iowa_fw_status_t`](ClientAPI.md#iowa_fw_status_t) for the possible value depending of the context.

#** Header File **

objects/iowa_firmware_update.h

\clearpage

## GPS Object API

This IPSO object represents GPS coordinates.

To be able to use this object, `iowa_gps.h` must be included.

### iowa_client_add_gps_object

** Prototype **

```c
iowa_status_t iowa_client_add_gps_object(iowa_context_t context,
                                         uint16_t optFlags,
                                         const char *applicationType,
                                         iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_gps_object()` creates a GPS object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*applicationType*
: The application type

*idP*
: Used to store the ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_gps.h

** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_GPS_RSC_UNCERTAINTY
- IOWA_GPS_RSC_COMPASS_DIRECTION
- IOWA_GPS_RSC_VELOCITY
- IOWA_GPS_RSC_TIMESTAMP

\clearpage

### iowa_client_remove_gps_object

** Prototype **

```c
iowa_status_t iowa_client_remove_gps_object(iowa_context_t context,
                                            iowa_sensor_t id);
```

** Description **

`iowa_client_remove_gps_object()` removes a GPS object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a GPS object. Valid *id* are only returned by [`iowa_client_add_gps_object()`](ClientAPI.md#iowa_client_add_gps_object).

IOWA_COAP_404_NOT_FOUND
: GPS referred by **id** does not exist.

** Header File **

objects/iowa_gps.h

\clearpage

### iowa_client_gps_update_location

** Prototype **

```c
iowa_status_t iowa_client_gps_update_location(iowa_context_t context,
                                              iowa_sensor_t id,
                                              const char *latitude,
                                              const char *longitude);
```

** Description **

`iowa_client_gps_update_location()` updates values of a GPS object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*latitude*
: New latitude

*longitude*
: New longitude

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a GPS object. Valid *id* are only returned by [`iowa_client_add_gps_object()`](ClientAPI.md#iowa_client_add_gps_object).

IOWA_COAP_404_NOT_FOUND
: GPS referred by **id** does not exist.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

** Header File **

objects/iowa_gps.h

\clearpage

### iowa_client_gps_update_location_full

** Prototype **

```c
iowa_status_t iowa_client_gps_update_location_full(iowa_context_t context,
                                                   iowa_sensor_t id,
                                                   const char *latitude,
                                                   const char *longitude,
                                                   const char *uncertainty,
                                                   float compassDirection,
                                                   size_t velocityLength,
                                                   uint8_t *velocity);
```

** Description **

`iowa_client_gps_update_location_full()` updates values of a GPS object. Optionals resources are included.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*latitude*
: New latitude

*longitude*
: New longitude

*uncertainty*
: The accuracy of the position in meters.

*compassDirection*
: Measured Direction between 0 and 360 deg.

*velocityLength*
: Length of the velocity array

*velocity*
: The velocity of the device as defined in 3GPP 23.032 GAD specification.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a GPS object. Valid *id* are only returned by [`iowa_client_add_gps_object()`](ClientAPI.md#iowa_client_add_gps_object).

IOWA_COAP_404_NOT_FOUND
: GPS referred by **id** does not exist.

IOWA_COAP_406_NOT_ACCEPTABLE
: *compassDirection*'s value is outside the [0.0, 360.0] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

** Header File **

objects/iowa_gps.h

\clearpage

## Gyrometer Object API

This IPSO Object is used to report the current reading of a gyrometer sensor in 3 axes.

To be able to use this object, `iowa_gyrometer.h` must be included.

### iowa_client_add_gyrometer_object

** Prototype **

```c
iowa_status_t iowa_client_add_gyrometer_object(iowa_context_t context,
                                               uint16_t optFlags,
                                               float minRangeValue,
                                               float maxRangeValue,
                                               const char *sensorUnits,
                                               const char *applicationType,
                                               iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_gyrometer_object()` creates a gyrometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*minRangeValue*
: Minimal range value for the gyrometer.

*maxRangeValue*
: Maximal range value for the gyrometer.

*sensorUnits*
: Measurement units definition

*applicationType*
: The application type

*idP*
: Used to store the ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *minRangeValue* argument is superior to *maxRangeValue* argument.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_gyrometer.h

** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_GYROMETER_RSC_Y_VALUE
- IOWA_GYROMETER_RSC_Z_VALUE
- IOWA_GYROMETER_RSC_MIN_X_VALUE
- IOWA_GYROMETER_RSC_MAX_X_VALUE
- IOWA_GYROMETER_RSC_MIN_Y_VALUE
- IOWA_GYROMETER_RSC_MAX_Y_VALUE
- IOWA_GYROMETER_RSC_MIN_Z_VALUE
- IOWA_GYROMETER_RSC_MAX_Z_VALUE
- IOWA_GYROMETER_RSC_RESET_MIN_MAX_VALUES
- IOWA_GYROMETER_RSC_MIN_RANGE_VALUE
- IOWA_GYROMETER_RSC_MAX_RANGE_VALUE

Moreover, you can add several optional resources at one time by using the following flags:

- IOWA_GYROMETER_3_AXIS
- IOWA_GYROMETER_MIN_MAX_VALUES
- IOWA_GYROMETER_RANGE_VALUE

\clearpage

### iowa_client_remove_gyrometer_object

** Prototype **

```c
iowa_status_t iowa_client_remove_gyrometer_object(iowa_context_t context,
                                                  iowa_sensor_t id);
```

** Description **

`iowa_client_remove_gyrometer_object()` removes a gyrometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a gyrometer object. Valid *id* are only returned by [`iowa_client_add_gyrometer_object()`](ClientAPI.md#iowa_client_add_gyrometer_object).

IOWA_COAP_404_NOT_FOUND
: gyrometer referred by **id** does not exist.

** Header File **

objects/iowa_gyrometer.h

\clearpage

### iowa_client_gyrometer_update_axis

** Prototype **

```c
iowa_status_t iowa_client_gyrometer_update_axis(iowa_context_t context,
                                                iowa_sensor_t id,
                                                float xValue,
                                                float yValue,
                                                float zValue);
```

** Description **

`iowa_client_gyrometer_update_axis()` updates values of a gyrometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*xValue*
: X value axis

*yValue*
: Y value axis

*zValue*
: Z value axis

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a gyrometer object. Valid *id* are only returned by [`iowa_client_add_gyrometer_object()`](ClientAPI.md#iowa_client_add_gyrometer_object).

IOWA_COAP_404_NOT_FOUND
: gyrometer referred by **id** does not exist.

** Header File **

objects/iowa_gyrometer.h

\clearpage

## IPSO Objects

This part allows the possibility to manipulate several IPSO Objects.

To be able to use these objects, `iowa_ipso.h` must be included.

### iowa_client_IPSO_add_sensor

** Prototype **

```c
iowa_status_t iowa_client_IPSO_add_sensor(iowa_context_t contextP,
                                          iowa_IPSO_ID_t type,
                                          float value,
                                          const char * unit,
                                          const char * appType,
                                          float rangeMin, float rangeMax,
                                          iowa_sensor_t * idP);
```

** Description **

`iowa_client_IPSO_add_sensor()` adds a new IPSO sensor for the LwM2M Client to handle. The sensor is defined by its type.

The unit, the application type and the range are only informative and reported as-is to the LwM2M Server. Note that the LwM2M Server can modify the application type.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*type*
: The type of sensor. See below.

*value*
: The initial value measured by the sensor.

*unit*
: The unit of the measured value as a nil-terminated string. This can be nil.

*appType*
: The application type of the sensor as a free-form nil-terminated string. This can be nil.

*rangeMin*
: The minimum value that can be measured by the sensor.

*rangeMax*
: The maximum value that can be measured by the sensor.

*idP*
: Used to store the ID of the created sensor. Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *type* is unknown.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - the sensor uses a Boolean value and *value* is neither 0.0 nor 1.0.
: - the sensor uses a percentage value and *value* is outside the [0.0, 100.0] range.
: - the sensor uses a compass direction value and *value* is outside the [0.0, 360.0] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.

** Header File **

objects/iowa_ipso.h

** Notes **

If both *rangeMin* and *rangeMax* are set to zero, the matching resources are ignored in the LwM2M Object.

*unit* is not duplicated nor freed by IOWA. Make sure it is available until [`iowa_close()`](CommonAPI.md#iowa_close) or [`iowa_client_IPSO_remove_sensor()`](ClientAPI.md#iowa_client_IPSO_remove_sensor) is called. It is advised to use static strings.

*appType* is duplicated internally by IOWA and can be reused or freed by the caller.

Only a call to [`iowa_client_IPSO_remove_sensor()`](ClientAPI.md#iowa_client_IPSO_remove_sensor) can free the memory allocated by `iowa_client_IPSO_add_sensor()`.

If *type* equal to IOWA_IPSO_LEVEL_CONTROL this function will call [`iowa_client_add_dimmer_object()`](ClientAPI.md#iowa_client_add_dimmer_object).

#### iowa_IPSO_ID_t

This is an enumeration of the LwM2M IDs of the supported sensor types. See below.

##### Float value sensors

- IOWA_IPSO_ANALOG_INPUT (3202)
- IOWA_IPSO_GENERIC (3300)
- IOWA_IPSO_ILLUMINANCE (3301)
- IOWA_IPSO_TEMPERATURE (3303)
- IOWA_IPSO_HUMIDITY (3304)
- IOWA_IPSO_BAROMETER (3315)
- IOWA_IPSO_VOLTAGE (3316)
- IOWA_IPSO_CURRENT (3317)
- IOWA_IPSO_FREQUENCY (3318)
- IOWA_IPSO_DEPTH (3319)
- IOWA_IPSO_PERCENTAGE (3320)
- IOWA_IPSO_ALTITUDE (3321)
- IOWA_IPSO_LOAD (3322)
- IOWA_IPSO_PRESSURE (3323)
- IOWA_IPSO_LOUDNESS (3324)
- IOWA_IPSO_CONCENTRATION (3325)
- IOWA_IPSO_ACIDITY (3326)
- IOWA_IPSO_CONDUCTIVITY (3327)
- IOWA_IPSO_POWER (3328)
- IOWA_IPSO_POWER_FACTOR (3329)
- IOWA_IPSO_RATE (3346)
- IOWA_IPSO_DISTANCE (3330)
- IOWA_IPSO_ENERGY (3331)

##### Boolean value sensors

For these sensors, the value must be either 0.0 or 1.0:

- IOWA_IPSO_DIGITAL_INPUT (3200)
- IOWA_IPSO_PRESENCE (3302)
- IOWA_IPSO_ON_OFF_SWITCH (3342)
- IOWA_IPSO_PUSH_BUTTON (3347)

##### Percentage value sensors

For these sensors, the value must be between 0.0 and 100.0:

- IOWA_IPSO_LEVEL_CONTROL (3343)

##### Compass direction value sensors

For these sensors, the value must be between 0.0 and 360.0:

- IOWA_IPSO_DIRECTION (3332)

\clearpage

### iowa_client_IPSO_update_value

** Prototype **

```c
iowa_status_t iowa_client_IPSO_update_value(iowa_context_t contextP,
                                            iowa_sensor_t id,
                                            float value);
```

** Description **

`iowa_client_IPSO_update_value()` updates the value of an IPSO sensor.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
 : The ID of the sensor as returned by [`iowa_client_IPSO_add_sensor()`](ClientAPI.md#iowa_client_IPSO_add_sensor).

*value*
 : The new value measured by the sensor.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *id* does not match any known sensor.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - the sensor uses a Boolean value and *value* is neither 0.0 nor 1.0.
: - the sensor uses a percentage value and *value* is outside the [0.0, 100.0] range.
: - the sensor uses a compass direction value and *value* is outside the [0.0, 360.0] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.

** Header File **

objects/iowa_ipso.h

** Notes **

If *type* equal to IOWA_IPSO_LEVEL_CONTROL this function will call [`iowa_client_dimmer_update_value()`](ClientAPI.md#iowa_client_dimmer_update_value).

\clearpage

### iowa_client_IPSO_update_values

** Prototype **

```c
iowa_status_t iowa_client_IPSO_update_values(iowa_context_t contextP,
                                             iowa_sensor_t id,
                                             size_t valueCount,
                                             iowa_ipso_timed_value_t *valueArray);
```

** Description **

`iowa_client_IPSO_update_values()` updates multiple times the value of an IPSO Object sensor.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: The ID of the sensor as returned by [`iowa_client_IPSO_add_sensor()`](ClientAPI.md#iowa_client_IPSO_add_sensor).

*valueCount*
: The number of values in *valueArray*.

*valueArray*
: The [`iowa_ipso_timed_value_t`](ClientAPI.md#iowa_ipso_timed_value_t) list of new values.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *id* does not match any known sensor.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - one of the timestamped value has an negative value.
: - the sensor uses a Boolean value and *value* is neither 0.0 nor 1.0.
: - the sensor uses a percentage value and *value* is outside the [0.0, 100.0] range.
: - the sensor uses a compass direction value and *value* is outside the [0.0, 360.0] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.

** Header File **

objects/iowa_ipso.h

** Notes **

`iowa_client_IPSO_update_values()` can only be used when the define [**LWM2M_SUPPORT_TIMESTAMP**][LWM2M_SUPPORT_TIMESTAMP] is set.

The timestamp must be absolute and not relative to the current time, meaning negative values are not accepted. If the timestamp is equal to zero, it is ignored.

Calling `iowa_client_IPSO_update_values()` will overwrite the previous values list. This has multiple consequences:

- If the values have not been sent to the Server, the previous values are lost. Values are only sent if the Server do a Read operation or if the Server has set an Observation.
- If the values are in the way to be sent to the Server and `iowa_client_IPSO_update_values()` is called during the process, some old values will be lost. This API is trying to send the values in best effort. Recent timestamped values are processed in priority before the oldest ones.

Timestamp information is only present if the used Content Format is:

- JSON: [`LWM2M_SUPPORT_JSON`][LWM2M_SUPPORT_JSON]
- SenML JSON: [`LWM2M_SUPPORT_SENML_JSON`][LWM2M_SUPPORT_SENML_JSON]
- SenML CBOR: [`LWM2M_SUPPORT_SENML_CBOR`][LWM2M_SUPPORT_SENML_CBOR]

If *type* equal to IOWA_IPSO_LEVEL_CONTROL this function will call [`iowa_client_dimmer_update_values()`](ClientAPI.md#iowa_client_dimmer_update_values).

\clearpage

### iowa_client_IPSO_remove_sensor

** Prototype **

```c
iowa_status_t iowa_client_IPSO_remove_sensor(iowa_context_t contextP,
                                             iowa_sensor_t id);
```

** Description **

`iowa_client_IPSO_remove_sensor()` removes from the LwM2M Client an IPSO sensor created with [`iowa_client_IPSO_add_sensor()`](ClientAPI.md#iowa_client_IPSO_add_sensor).

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: The ID of the sensor as returned by [`iowa_client_IPSO_add_sensor()`](ClientAPI.md#iowa_client_IPSO_add_sensor).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not an IPSO sensor. Valid *id* are only returned by [`iowa_client_IPSO_add_sensor()`](ClientAPI.md#iowa_client_IPSO_add_sensor).

IOWA_COAP_404_NOT_FOUND
: IPSO referred by **id** does not exist.

** Header File **

objects/iowa_ipso.h

** Notes **

If *type* equal to IOWA_IPSO_LEVEL_CONTROL this function will call [`iowa_client_remove_dimmer_object()`](ClientAPI.md#iowa_client_remove_dimmer_object).

\clearpage

## Ligth Control Object API

This IPSO object is used to control a light source, such as a LED or other light.

To be able to use this object, `iowa_light_control.h` must be included.

### Callbacks

#### iowa_light_control_update_state_callback_t

This callback is called when the LwM2M Server request the Client to update the state of the light.

```c
typedef iowa_status_t (*iowa_light_control_update_state_callback_t)(iowa_sensor_t id,
                                                                    bool powerOn,
                                                                    int dimmer,
                                                                    char *colour,
                                                                    void *userDataCallback,
                                                                    iowa_context_t contextP);
```

*id*
: ID of the object

*powerOn*
: Light power

*dimmer*
: Dimmer settings

*colour*
: A string representing a value in some color space

*userDataCallback*
: Application specific data from [`iowa_client_add_light_control_object`](ClientAPI.md#iowa_client_add_light_control_object). Can be nil.

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_add_light_control_object

#** Prototype **

```c
iowa_status_t iowa_client_add_light_control_object(
    iowa_context_t context,
    uint16_t optFlags,
    const float powerFactor,
    const char *colorSpace,
    iowa_light_control_update_state_callback_t updateStateCallback,
    void *userDataCallback,
    iowa_sensor_t *idP
);
```

#** Description **

`iowa_client_add_light_control_object()` creates a light control object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*powerFactor*
: The power factor of the light.

*colorSpace*
: Color space of the light.

*updateStateCallback*
: Called to update state of the light. This is called when the server request a new state.

*userDataCallback*
: Application specific data pass to the callback. Can be nil.

*applicationType*
: The application type

*idP*
: Used to store the ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: no update state callback provided means *updateStateCallback* is nil.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_light_control.h

#** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_LIGHT_CONTROL_RSC_DIMMER
- IOWA_LIGHT_CONTROL_RSC_ON_TIME
- IOWA_LIGHT_CONTROL_RSC_CUMULATIVE_ACTIVE_POWER
- IOWA_LIGHT_CONTROL_RSC_POWER_FACTOR

Moreover, you can add several optional resources at one time by using the following flag:

- IOWA_LIGHT_CONTROL_POWER

The argument *colorSpace* must reflect the color representation of the light. Find below a non-exhaustive list of color spaces:

- RGB
- sRGB
- CMYK
- ...

\clearpage

#### iowa_client_remove_light_control_object

#** Prototype **

```c
iowa_status_t iowa_client_remove_light_control_object(iowa_context_t context,
                                                      iowa_sensor_t id);
```

#** Description **

`iowa_client_remove_light_control_object()` removes a light control object.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a light control object. Valid *id* are only returned by [`iowa_client_add_light_control_object()`](ClientAPI.md#iowa_client_add_light_control_object).

IOWA_COAP_404_NOT_FOUND
: light control referred by **id** does not exist.

#** Header File **

objects/iowa_light_control.h

\clearpage

#### iowa_client_light_control_set_state

#** Prototype **

```c
iowa_status_t iowa_client_light_control_set_state(iowa_context_t context,
                                                  iowa_sensor_t id,
                                                  bool powerOn,
                                                  int dimmer,
                                                  const char *colour);
```

#** Description **

`iowa_client_light_control_set_state()` updates values of a light control object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*powerOn*
: Light power

*dimmer*
: Dimmer settings between 0 and 100 %

*colour*
: A string representing a value in some color space

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a light control object. Valid *id* are only returned by [`iowa_client_add_light_control_object()`](ClientAPI.md#iowa_client_add_light_control_object).

IOWA_COAP_404_NOT_FOUND
: light control referred by **id** does not exist.

IOWA_COAP_412_PRECONDITION_FAILED
: cannot affect the color. Color space has not been provided. To reconfigure the light control object, delete then readd the object or just add a new one with [`iowa_client_add_light_control_object()`](ClientAPI.md#iowa_client_add_light_control_object) and [`iowa_client_remove_light_control_object()`](ClientAPI.md#iowa_client_remove_light_control_object).

IOWA_COAP_406_NOT_ACCEPTABLE
: *dimmer*'s value is outside the [0, 100] range.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error when powerOn value changes and IOWA_LIGHT_CONTROL_RSC_ON_TIME resource is present.

#** Header File **

objects/iowa_light_control.h

#** Notes **

The Light Control Object may contain an "On Time" resource and/or a "Cumulative Active Power" resource. Calling this API updates their respective values.

If *colour* is nil, the value of the resource Colour is not updated.

\clearpage

## Location Object API

This LwM2M Object contains information on the device position and speed.

To be able to use this object, `iowa_location.h` must be included.

### iowa_client_add_location_object

** Prototype **

```c
iowa_status_t iowa_client_add_location_object(iowa_context_t context,
                                              uint16_t optFlags,
                                              iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_location_object()` creates a location object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*idP*
: Used to store the ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_409_CONFLICT
: a location object already exists. To reconfigure the location object, first delete the object with [`iowa_client_remove_location_object()`](ClientAPI.md#iowa_client_remove_location_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_location.h

** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_LOCATION_RSC_ALTITUDE
- IOWA_LOCATION_RSC_RADIUS
- IOWA_LOCATION_RSC_VELOCITY
- IOWA_LOCATION_RSC_SPEED

\clearpage

### iowa_client_remove_location_object

** Prototype **

```c
iowa_status_t iowa_client_remove_location_object(iowa_context_t context,
                                                 iowa_sensor_t id);
```

** Description **

`iowa_client_remove_location_object()` removes a location object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a location object. Valid *id* are only returned by [`iowa_client_add_location_object()`](ClientAPI.md#iowa_client_add_location_object).

IOWA_COAP_404_NOT_FOUND
: no location object to remove. [`iowa_client_add_location_object()`](ClientAPI.md#iowa_client_add_location_object) was not called before, or failed.

** Header File **

objects/iowa_location.h

\clearpage

### iowa_client_location_update

** Prototype **

```c
iowa_status_t iowa_client_location_update(iowa_context_t context,
                                          iowa_sensor_t id,
                                          float latitude,
                                          float longitude);
```

** Description **

`iowa_client_location_update()` updates values of a location object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*latitude*
: New latitude

*longitude*
: New longitude

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a location object. Valid *id* are only returned by [`iowa_client_add_location_object()`](ClientAPI.md#iowa_client_add_location_object).

IOWA_COAP_404_NOT_FOUND
: object has not been created. First call [`iowa_client_add_location_object()`](ClientAPI.md#iowa_client_add_location_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

** Header File **

objects/iowa_location.h

\clearpage

### iowa_client_location_update_full

** Prototype **

```c
iowa_status_t iowa_client_location_update_full(iowa_context_t context,
                                               iowa_sensor_t id,
                                               float latitude,
                                               float longitude,
                                               float altitude,
                                               float radius,
                                               size_t velocityLength,
                                               uint8_t* velocity,
                                               float speed);
```

** Description **

`iowa_client_location_update_full()` updates values of a location object. Optionals resources are included.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*latitude*
: New latitude

*longitude*
: New longitude

*altitude*
: New altitude

*radius*
: Indicates the size in meters of a circular area around a point of geometry.

*velocityLength*
: Length of the velocity array

*velocity*
: The velocity of the device as defined in 3GPP 23.032 GAD specification.

*speed*
: Speed is the time rate of change in position of a LwM2M Client without regard for direction: the scalar component of velocity.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a location object. Valid *id* are only returned by [`iowa_client_add_location_object()`](ClientAPI.md#iowa_client_add_location_object).

IOWA_COAP_404_NOT_FOUND
: object has not been created. First call [`iowa_client_add_location_object()`](ClientAPI.md#iowa_client_add_location_object).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

** Header File **

objects/iowa_location.h

\clearpage

## Magnetometer Object API

This IPSO object can be used to represent a 1-3 axis magnetometer with optional compass direction.

To be able to use this object, `iowa_magnetometer.h` must be included.

### iowa_client_add_magnetometer_object

** Prototype **

```c
iowa_status_t iowa_client_add_magnetometer_object(iowa_context_t context,
                                                  uint16_t optFlags,
                                                  const char *sensorUnits,
                                                  iowa_sensor_t *idP);
```

** Description **

`iowa_client_add_magnetometer_object()` creates a magnetometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*sensorUnits*
: Measurement units definition.

*idP*
: Used to store the ID of the object.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

objects/iowa_magnetometer.h

** Notes **

Please refer to the [OMA LightweightM2M (LwM2M) Object and Resource Registry](http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html) to see how the object is defined: resources, resources type, ...

When no optional flags are provided only mandatory resources of the object are implemented.

To add optional resources, you can use the following flags:

- IOWA_MAGNETOMETER_RSC_Y_VALUE
- IOWA_MAGNETOMETER_RSC_Z_VALUE
- IOWA_MAGNETOMETER_RSC_COMPASS_DIRECTION

Moreover, you can add several optional resources at one time by using the following flag:

- IOWA_MAGNETOMETER_3_AXIS

\clearpage

### iowa_client_remove_magnetometer_object

** Prototype **

```c
iowa_status_t iowa_client_remove_magnetometer_object(iowa_context_t context,
                                                     iowa_sensor_t id);
```

** Description **

`iowa_client_remove_magnetometer_object()` removes a magnetometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a magnetometer object. Valid *id* are only returned by [`iowa_client_add_magnetometer_object()`](ClientAPI.md#iowa_client_add_magnetometer_object).

IOWA_COAP_404_NOT_FOUND
: magnetometer referred by **id** does not exist.

** Header File **

objects/iowa_magnetometer.h

\clearpage

### iowa_client_magnetometer_update_values

** Prototype **

```c
iowa_status_t iowa_client_magnetometer_update_values(iowa_context_t context,
                                                     iowa_sensor_t id,
                                                     float xValue,
                                                     float yValue,
                                                     float zValue,
                                                     float compassDirection);
```

** Description **

`iowa_client_magnetometer_update_values()` updates values of a magnetometer object.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the object

*xValue*
: X value axis

*yValue*
: Y value axis

*zValue*
: Z value axis

*compassDirection*
: Measured Direction between 0 and 360 deg.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a magnetometer object. Valid *id* are only returned by [`iowa_client_add_magnetometer_object()`](ClientAPI.md#iowa_client_add_magnetometer_object).

IOWA_COAP_404_NOT_FOUND
: magnetometer referred by **id** does not exist.

IOWA_COAP_406_NOT_ACCEPTABLE
: *compassDirection*'s value is outside the [0.0, 360.0] range.

** Header File **

objects/iowa_magnetometer.h

\clearpage

## Software Component Object API

This LwM2M object provides the resources needed to activate/deactivate software components on the device.

To be able to use this object, `iowa_software_component.h` must be included and **IOWA_SUPPORT_SOFTWARE_COMPONENT_OBJECT** must be defined before building IOWA.

### Data Structures and Constants

#### iowa_sw_cmp_info_t

This structure contains the description of a software component's information which could be set by users.

```c
typedef struct
{
    const char     *identityP;
    const uint8_t  *packP;
    size_t          packLength;
    const char     *versionP;
} iowa_sw_cmp_info_t;
```

*identityP*
: Name or identifier of the software component, with size < 255. This can be nil.

*packP*
: Link to opaque data describing the software component. This can be nil.

*packLength*
: Length in bytes of the opaque data pointed by *packP*.

*versionP*
: Version of the software component, with size < 255. This can be nil.

** Note **

This structure will at least provide an identity (*identityP*) or a pack (*packP*) to identify the component.

\clearpage

### Callbacks

#### iowa_sw_cmp_update_callback_t

This is the update callback, called when the Server adds or removes the software components.

```c
typedef iowa_status_t(*iowa_sw_cmp_update_callback_t)(iowa_sensor_t id,
                                                      iowa_dm_operation_t operation,
                                                      iowa_sw_cmp_info_t *infoP,
                                                      bool activationState,
                                                      void *userDataP,
                                                      iowa_context_t contextP);
```

*id*
: ID of the corresponding software component.

*operation*
: the operation performed by the Server on this software component (either **IOWA_DM_CREATE** or **IOWA_DM_DELETE**).

*infoP*
: software component information.

*activationState*
: initial activation state. Should be ignored if no [`iowa_sw_cmp_activation_callback_t()`](ClientAPI.md#iowa_sw_cmp_activation_callback_t) was passed to [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component).

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

#### iowa_sw_cmp_activation_callback_t

This is the activation callback, called when the Server requests the device to activate or deactivate a software component.

```c
typedef iowa_status_t(*iowa_sw_cmp_activation_callback_t) (iowa_sensor_t id,
                                                           bool activationState,
                                                           void *userDataP,
                                                           iowa_context_t contextP);
```

*id*
: ID of the corresponding software component.

*activationState*
: activation state requested.

*userDataP*
: The data passed to [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component) was called.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

\clearpage

### API

#### iowa_client_enable_software_component

#** Prototype **

```c
iowa_status_t iowa_client_enable_software_component(
    iowa_context_t contextP,
    iowa_sw_cmp_update_callback_t updateCb,
    iowa_sw_cmp_activation_callback_t activationCb,
    void *userDataP
);
```
#** Description **

`iowa_client_enable_software_component()` enables the software component feature.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*updateCb*
: The update callback called when the Server adds or removes a software component. This can be nil.

*activateCb*
: The activate callback, called when the Server requests the device to activate or deactivate a software component. This can be nil.

*userDataP*
: Passed as argument to the callback. This can be nil.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_409_CONFLICT
: Software component feature is already configured. To reconfigure the software component, disable it before with [`iowa_client_disable_software_component()`](ClientAPI.md#iowa_client_disable_software_component).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_software_component.h

\clearpage

#### iowa_client_disable_software_component

#** Prototype **

```c
iowa_status_t iowa_client_disable_software_component(iowa_context_t contextP);
```
#** Description **

`iowa_client_disable_software_component()` disables the software component feature.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: Software component feature was not enabled. [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component) was not called before, or failed.

#** Header File **

objects/iowa_software_component.h

\clearpage

#### iowa_client_add_software_component

#** Prototype **

```c
iowa_status_t iowa_client_add_software_component(iowa_context_t contextP,
                                                 iowa_sw_cmp_info_t *infoP,
                                                 bool activationState,
                                                 iowa_sensor_t *idP);
```
#** Description **

`iowa_client_add_software_component()` adds a software component.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*infoP*
: software component information.

*activationState*
: current activation state of the software component. Ignored if no [`iowa_sw_cmp_activation_callback_t()`](ClientAPI.md#iowa_sw_cmp_activation_callback_t) was passed to [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component).

*idP*
: Used to store the ID of the created software component. Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *infoP* is nil.
: - both *infoP::identityP* and *infoP::packP* are nil.
: - a string in *infoP* is longer than 255 characters.

IOWA_COAP_406_NOT_ACCEPTABLE
: Software component feature was not enabled. Call first [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_software_component.h

#** Notes **

*infoP* must provide at least an identity (*infoP::identityP*) or a pack (*infoP::packP*) to identify the component.

The "const" elements pointed by the fields of *infoP* are not duplicated nor freed by IOWA. Make sure they are available until corresponding [`iowa_client_remove_software_component()`](ClientAPI.md#iowa_client_remove_software_component), [`iowa_client_disable_software_component()`](ClientAPI.md#iowa_client_disable_software_component), or [`iowa_close()`](CommonAPI.md#iowa_close) is called.

\clearpage

#### iowa_client_remove_software_component

#** Prototype **

```c
iowa_status_t iowa_client_remove_software_component(iowa_context_t contextP,
                                                    iowa_sensor_t id);
```
#** Description **

`iowa_client_remove_software_component()` removes a software component.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the corresponding software component.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a software component. Valid *id* are only returned by [`iowa_client_add_software_component()`](ClientAPI.md#iowa_client_add_software_component).

IOWA_COAP_404_NOT_FOUND
: software component referred by **id** does not exist.

IOWA_COAP_412_PRECONDITION_FAILED
: Software component feature was not enabled. Call first [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component).

#** Header File **

objects/iowa_software_component.h

\clearpage

#### iowa_client_software_component_update_state

#** Prototype **

```c
iowa_status_t iowa_client_software_component_update_state(iowa_context_t contextP,
                                                          iowa_sensor_t id,
                                                          bool activationState);
```
#** Description **

`iowa_client_software_component_update_state()` updates a software component's activation state.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the corresponding software component.

*activationState*
: New activation state of the software component.

** Note **

This API has no effect if no [`iowa_sw_cmp_activation_callback_t()`](ClientAPI.md#iowa_sw_cmp_activation_callback_t) was passed to [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component) since the Activation State resource is not presented to the LwM2M Server.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *id* does not match any known software component.

IOWA_COAP_406_NOT_ACCEPTABLE
: Software component feature was not enabled. Call first [`iowa_client_enable_software_component()`](ClientAPI.md#iowa_client_enable_software_component).

#** Header File **

objects/iowa_software_component.h

\clearpage

## Software Management Object API

In LightweightM2M, the Software Management mechanism is used to install and activate software components to a LwM2M Client.

To be able to use this object, `iowa_software_management.h` must be included and **IOWA_SUPPORT_SOFTWARE_MANAGEMENT_OBJECT** must be defined before building the library.

The [Device Update][Device Update]/[Software Management][Software Management] part of this specification adds more explanation about its mechanism and how to use it with IOWA.

**Note:** As there is currently some confusion on the layout of of the Software Management Object, IOWA uses the definition provided in [LwM2M Overview][LwM2M Overview]/[Software Management Object][Software Management Object].

### Data Structures and Constants

#### iowa_sw_pkg_result_t

This enumeration is used to update the operation result from [`iowa_sw_pkg_download_callback_t()`](ClientAPI.md#iowa_sw_pkg_download_callback_t), [`iowa_sw_pkg_write_callback_t()`](ClientAPI.md#iowa_sw_pkg_write_callback_t) and [`iowa_sw_pkg_install_callback_t()`](ClientAPI.md#iowa_sw_pkg_install_callback_t) callbacks. It has the following values:

IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL
: success of any operation made on the software package (verification, installation, uninstallation, activation, deactivation)

IOWA_SW_PKG_UPDATE_RESULT_DOWNLOADING_SUCCESSFUL
: success of the new software package download. (*downloadCb* only)

IOWA_SW_PKG_UPDATE_RESULT_OUT_OF_STORAGE
: not enough storage for the new software package. (*downloadCb* or *writeCb* only)

IOWA_SW_PKG_UPDATE_RESULT_OUT_OF_MEMORY
: out of memory error during the download of the new software package. (*downloadCb* or *writeCb* only)

IOWA_SW_PKG_UPDATE_RESULT_CONNECTION_LOST
: connection lost during the download of the new software package. (*downloadCb* or *writeCb* only)

IOWA_SW_PKG_UPDATE_RESULT_INTEGRITY_CHECK_FAILURE
: integrity check failure of the new software package. (*downloadCb* or *writeCb* only)

IOWA_SW_PKG_UPDATE_RESULT_UNSUPPORTED_TYPE
: unsupported new software package type.

IOWA_SW_PKG_UPDATE_RESULT_INVALID_URI
: invalid URI to download the new software package. (*downloadCb* only)

IOWA_SW_PKG_UPDATE_RESULT_UPDATE_FAILED
: device defined update error.

IOWA_SW_PKG_UPDATE_RESULT_INSTALLED_FAILURE
: new software installation failure. (*installCb* only)

IOWA_SW_PKG_UPDATE_RESULT_UNINSTALLED_FAILURE
: software uninstallation failure. (*installCb* only)

#### iowa_sw_pkg_state_t

This enumeration is used to control current state in [`iowa_client_add_software_package()`](ClientAPI.md#iowa_client_add_software_package) and [`iowa_client_software_package_update_state()`](ClientAPI.md#iowa_client_software_package_update_state). It has the following values:

IOWA_SW_PKG_STATE_UNINSTALLED
: software is uninstalled. (default value)

IOWA_SW_PKG_STATE_INSTALLED
: software is installed.

IOWA_SW_PKG_STATE_ACTIVATED
: software is activate. Useful only if Software components are linked, otherwise same behavior than installed.

#### iowa_sw_pkg_write_cmd_t

This enumeration is used in [`iowa_sw_pkg_write_callback_t()`](ClientAPI.md#iowa_sw_pkg_write_callback_t) callback. It has the following values:

IOWA_SW_PKG_COMMAND_RESET
: To start software package packet writing.

IOWA_SW_PKG_COMMAND_WRITE
: To indicate other software package piece of the complete packet.

#### iowa_sw_pkg_install_cmd_t

This enumeration is used in [`iowa_sw_pkg_install_callback_t()`](ClientAPI.md#iowa_sw_pkg_install_callback_t) callback. It has the following values:

IOWA_SW_PKG_COMMAND_INSTALL
: software installation is requested.

IOWA_SW_PKG_COMMAND_UNINSTALL
: software uninstallation is requested.

IOWA_SW_PKG_COMMAND_PREPARE_FOR_UPDATE
: software uninstallation is requested to prepare an update.

#### iowa_sw_pkg_optional_info_t

This structure contains the description of a optional software package's information which could be set by users.

```c
typedef struct
{
    iowa_sensor_t  *swComponentLinkP;
    uint16_t        swComponentLinkCount;
} iowa_sw_pkg_optional_info_t;
```

*swComponentLinkP*
: Software Components downloaded and installed in scope of the present SW Update Package. This can be nil.
: Each *swComponentLinkP* sensor id must have been provided by [`Software Component Object APIs`][Software Component Object API].

*swComponentLinkCount*
: Software Components Link count.

\clearpage

### Callbacks

#### iowa_sw_pkg_update_callback_t

This is the update callback, called when the Server adds or removes the software packages.

```c
typedef iowa_status_t(*iowa_sw_pkg_update_callback_t) (iowa_sensor_t id,
                                                       iowa_dm_operation_t operation,
                                                       const char *pkgNameP,
                                                       const char *pkgVersionP,
                                                       iowa_sw_pkg_optional_info_t *optP,
                                                       void *userDataP,
                                                       iowa_context_t contextP);
```

*id*
: ID of the corresponding software package.

*operation*
: the operation performed by the Server on this software package (either IOWA_DM_CREATE or IOWA_DM_DELETE).

*pkgNameP*
: Name of the software package.

*pkgVersionP*
: Version of the software package.

*optP*
: Optional information. This can be nil.

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

#** Return Value **

IOWA_COAP_NO_ERROR in case of success or an error status.

#### iowa_sw_pkg_download_callback_t

This is the download callback, called when the Server requests the device to download a new software Package (new value in "Package URI").

When the packet is downloaded, users should call [`iowa_client_set_software_package_command_result()`](ClientAPI.md#iowa_client_set_software_package_command_result) with IOWA_SW_PKG_UPDATE_RESULT_DOWNLOADING_SUCCESSFUL result if successful or an error result otherwise.

When the packet is verified, users should call [`iowa_client_set_software_package_command_result()`](ClientAPI.md#iowa_client_set_software_package_command_result) with IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL result if successful or an error result otherwise.

```c
typedef void(*iowa_sw_pkg_download_callback_t) (iowa_sensor_t id,
                                                const char *uriP,
                                                const char *userNameP,
                                                const char *passwordP,
                                                void *userDataP,
                                                iowa_context_t contextP);
```

*id*
: ID of the corresponding software package instance.

*uriP*
: URI to download the package from.

*userNameP*
: User Name for access to SW Update Package in pull mode, with size < 255. Key based mechanism can alternatively use for talking to the component server instead of user name and password combination. This can be nil.

*passwordP*
: Password for access to SW Update Package in pull mode, with size < 255. This can be nil.

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

#### iowa_sw_pkg_write_callback_t

This is the write callback, called several times when the Server pushes the new software package to the device (new value in "Package"). The expected behavior is the same as writing to a file stream i.e. unless it is reset, written data are appended to the previous ones.

```c
typedef iowa_sw_pkg_result_t(*iowa_sw_pkg_write_callback_t) (iowa_sensor_t id,
                                                             iowa_sw_pkg_write_cmd_t cmd,
                                                             size_t dataLength,
                                                             uint8_t *dataP,
                                                             void *userDataP,
                                                             iowa_context_t contextP);
```

At the start of the push of the software package or if the LwM2M Server cancels it, this callback is called with the following parameters:

*id*
: ID of the corresponding software package instance.

*cmd*
: **IOWA_SW_PKG_COMMAND_RESET**

*dataLength*
: 0

*dataP*
: NULL

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

When the software package is received, this callback is called several times with the following parameters:

*id*
: ID of the corresponding software package instance.

*cmd*
: **IOWA_SW_PKG_COMMAND_WRITE**

*dataLength*
: The length of the buffer pointed by *dataP*.

*data*
: The next chunk of the software package to write.

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

At the end of the push of the software package, this callback is called with the following parameters:

*id*
: ID of the corresponding software package instance.

*cmd*
: **IOWA_SW_MGMT_PACKAGE_WRITE**

*dataLength*
: 0

*data*
: NULL

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

#** Return Value **

IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL in case of success or an error status.

#### iowa_sw_pkg_install_callback_t

This is the install callback, called when the Server requests the device to install or uninstall the software Package.

When the installation finishes, users should call [`iowa_client_set_software_package_command_result()`](ClientAPI.md#iowa_client_set_software_package_command_result) with IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL result if successful or an error result otherwise.

```c
typedef void(*iowa_sw_pkg_install_callback_t) (iowa_sensor_t id,
                                               iowa_sw_pkg_install_cmd_t cmd,
                                               void *userDataP,
                                               iowa_context_t contextP);
```

*id*
: ID of the corresponding software package instance.

*cmd*
: installed state requested. See [`iowa_sw_pkg_install_cmd_t`](ClientAPI.md#iowa_sw_pkg_install_cmd_t).

*userDataP*
: The data passed to [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

*contextP*
: The IOWA context on which [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was called.

\clearpage

### API

#### iowa_client_enable_software_package_management

#** Prototype **

```c
iowa_status_t iowa_client_enable_software_package_management(
    iowa_context_t contextP,
    iowa_sw_pkg_update_callback_t updateCb,
    iowa_sw_pkg_download_callback_t downloadCb,
    iowa_sw_pkg_write_callback_t writeCb,
    iowa_sw_pkg_install_callback_t installCb,
    void *userDataP
);
```
#** Description **

`iowa_client_enable_software_package_management()` enables the software package management feature.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*updateCb*
: The update callback called when the Server adds or removes the software packages. This can be nil.

*downloadCb*
: The download callback, called when the Server requests the device to download a new software Package (new value in "Package URI"). This can be nil.

*writeCb*
: The write callback, called several times when the Server pushes the new software Package to the device (new value in "Package"). This can be nil.

*installCb*
: The install callback, called when the Server requests the device to install or uninstall the software Package.

*userDataP*
: Passed as argument to the callbacks. This can be nil.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

COAP_400_BAD_REQUEST
: either:
: - *installCb* is nil.
: - both *downloadCb* and *writeCb* are nil. At least one must be defined.

IOWA_COAP_409_CONFLICT
: Software package feature is already configured. To reconfigure the software package, disable it before with [`iowa_client_disable_software_package_management()`](ClientAPI.md#iowa_client_disable_software_package_management).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_software_management.h

#** Notes **

The LwM2M Server has two methods to provide the software package:

- the "pull" method: the LwM2M Server provides the URI of the software package and the LwM2M Client downloads it directly. To use this method, *downloadCb* must be set.

- the "push" method: the LwM2M Server writes the software package in a LwM2M Resource exposed by the Client. To use this method, *writeCb* must be set.

The Client can support both methods at the same time and must at least provide one of them.

*downloadCb* and *updateCb* do not return any value. The progress and result of their operation are indicated asynchronously by calling [`iowa_client_set_software_package_command_result()`](ClientAPI.md#iowa_client_set_software_package_command_result).

\clearpage

#### iowa_client_disable_software_package_management

#** Prototype **

```c
iowa_status_t iowa_client_disable_software_package_management(iowa_context_t contextP);
```
#** Description **

`iowa_client_disable_software_package_management()` disables the software package management feature.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: Software package feature was not enabled. [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management) was not called before, or failed.

#** Header File **

objects/iowa_software_management.h

\clearpage

#### iowa_client_add_software_package

#** Prototype **

```c
iowa_status_t iowa_client_add_software_package(iowa_context_t contextP,
                                               const char *pkgNameP,
                                               const char *pkgVersionP,
                                               iowa_sw_pkg_state_t state,
                                               iowa_sw_pkg_optional_info_t *optP,
                                               iowa_sensor_t *idP);
```
#** Description **

`iowa_client_add_software_package()` adds a software package instance.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*pkgNameP*
: Name of the software package.

*pkgVersionP*
: Version of the software package.

*state*
: State of the software package. (default value: IOWA_SW_PKG_STATE_UNINSTALLED)

*optP*
: Optional information. This can be nil.

*idP*
: Used to store the ID of the created software package instance. Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *pkgNameP* is nil.
: - *pkgVersionP* is nil.
: - *optP* has invalid format.
: - Any string is larger than 255 characters.

IOWA_COAP_406_NOT_ACCEPTABLE
: Software package feature was not enabled. Call first [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_software_management.h

\clearpage

#### iowa_client_remove_software_package

#** Prototype **

```c
iowa_status_t iowa_client_remove_software_package(iowa_context_t contextP,
                                                  iowa_sensor_t id);
```
#** Description **

`iowa_client_remove_software_package()` removes a software package instance.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the corresponding software package instance.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *id* is not a software package. Valid *id* are only returned by [`iowa_client_add_software_package()`](ClientAPI.md#iowa_client_add_software_package).

IOWA_COAP_404_NOT_FOUND
: software package referred by **id** does not exist.

IOWA_COAP_412_PRECONDITION_FAILED
: Software package feature was not enabled. Call first [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

#** Header File **

objects/iowa_software_management.h

\clearpage

#### iowa_client_software_package_update_state

#** Prototype **

```c
iowa_status_t iowa_client_software_package_update_state(iowa_context_t contextP,
                                                        iowa_sensor_t id,
                                                        iowa_sw_pkg_state_t state);
```
#** Description **

`iowa_client_software_package_update_state()` updates a software package instance's state.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the corresponding software package instance.

*state*
: state of the software package.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *id* does not match any known software package.

IOWA_COAP_406_NOT_ACCEPTABLE
: Software package feature was not enabled. Call first [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

#** Header File **

objects/iowa_software_management.h

\clearpage

#### iowa_client_set_software_package_command_result

#** Prototype **

```c
iowa_status_t iowa_client_set_software_package_command_result(iowa_context_t contextP,
                                                              iowa_sensor_t id,
                                                              iowa_sw_pkg_result_t result);
```
#** Description **

`iowa_client_set_software_package_command_result()` informs the IOWA stack of the result of the callbacks *downloadCb* and *installCb* of [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: ID of the corresponding software package instance.

*result*
: The result of the software package callbacks.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *id* does not match any known software package.

IOWA_COAP_406_NOT_ACCEPTABLE
: Software package feature was not enabled. Call first [`iowa_client_enable_software_package_management()`](ClientAPI.md#iowa_client_enable_software_package_management).

#** Header File **

objects/iowa_software_management.h

\clearpage

## MQTT Object API

This part allows the possibility to connect with an MQTT Broker.

To be able to use these objects, `iowa_mqtt_objects.h` must be included.

### Data Structures and Constants

#### iowa_mqtt_broker_t

```c
typedef struct
{
    char     *uri;
    char     *clientId;
    bool      cleanSession;
    uint16_t  keepAlive;
    char     *userName;
    uint8_t  *password;
    size_t    passwordLength;

    iowa_security_mode_t    securityMode;
    iowa_cert_usage_mode_t  certificateUsage;
    uint8_t                *identity;
    size_t                  identityLength;
    uint8_t                *brokerIdentity;
    size_t                  brokerIdentityLength;
    uint8_t                *privateKey;
    size_t                  privateKeyLength;
} iowa_mqtt_broker_t;
```

*uri*
: The URI to reach the MQTT Broker as a nil-terminated string e.g. "tcp://[::1]:1883".

*clientId*
: MQTT Client Identifier to use when connecting to this MQTT broker.

*cleanSession*
: A boolean that's indicate to the MQTT broker to create a persistent session.

*keepAlive*
: The maximum time in seconds that's the client take to send or receive a message.

*userName*
: The User Name to declare in the MQTT CONNECT message.

*password*
: The Password value to declare in the MQTT CONNECT message.

*passwordLength*
: The length of the broker's password.

*securityMode*
: The security mode to use when connecting to this LwM2M Server. See [iowa_security_mode_t](Security.md#iowa_security_mode_t).

*certificateUsage*
: The Certificate Usage Resource specifies the semantic of the certificate or raw public key stored in the "MQTT Broker Public Key" Resource, which is used to match the certificate presented in the TLS/DTLS handshake. See [iowa_cert_usage_mode_t](Security.md#iowa_cert_usage_mode_t).
: When this Resource is absent, value **IOWA_CERTIFICATE_USAGE_DOMAIN_ISSUED_CERTIFICATE** for domain issued certificate mode is assumed.

*identity*
: Stores the Device's certificate, public key (RPK mode) or PSK Identity (PSK mode).

*identityLength*
: The identity length.

*brokerIdentity*
: Stores the MQTT Broker's certificate, public key (RPK mode) or trust anchor. The Certificate Usage Resource determines the content of this resource.

*brokerIdentityLength*
: The length of the broker's Identity.

*privateKey*
: Stores the secret key (PSK mode) or private key (RPK or certificate mode).

*privateKeyLength*
: The private key's length

#### iowa_mqtt_publication_t

```c
typedef struct
{
    iowa_sensor_t           brokerId;
    char                   *source;
    char                   *topic;
    uint8_t                 qos;
    bool                    retain;
    bool                    active;
    iowa_content_format_t   encoding;
} iowa_mqtt_publication_t;
```

*brokerId*
: The ID of the broker to be used.

*source*
: The source of the data to publish (e.g. "", or ";"). If this Resource is empty, the published data are implementation dependent.

*topic*
: The MQTT topic to publish to.

*qos*
: The Quality of Service value to use when publishing.

*retain*
: The RETAIN flag value to use when publishing.

*active*
: A boolean to indicate if the Resource is not present, the Device publishes the data pointed by the Source Resource to the MQTT Broker pointed by the Broker Resource using the MQTT topic indicated in the Topic Resource. If false, the Device does nothing.

*encoding*
: A CoAP Content-Format value used to encode the data in the MQTT Publish message. If this Resource is not present or equal to 65535, the encoding of the data is implementation dependent.

\clearpage

### Callbacks

#### iowa_mqtt_broker_update_callback_t

The callback called when a LwM2M Server modifies the MQTT Broker Object.

```c
typedef void (*iowa_mqtt_broker_update_callback_t) (iowa_dm_operation_t operation,
                                                    iowa_sensor_t brokerId,
                                                    iowa_mqtt_broker_t *brokerDetailsP,
                                                    void *userData,
                                                    iowa_context_t contextP);
```

*operation*
: the type of the operation among IOWA_DM_READ, IOWA_DM_WRITE and IOWA_DM_DELETE.

*brokerId*
: the ID of the modified MQTT broker.

*brokerDetailsP*
: the details of the modified MQTT broker.

*userData*
: Passed as argument to the callbacks. This can be nil.

*contextP*
: the IOWA context on which iowa_client_enable_mqtt_broker() was called.

#### iowa_mqtt_publication_update_callback_t

The callback called when a LwM2M Server modifies the MQTT Publication Object.

```c
typedef void (*iowa_mqtt_publication_update_callback_t) (iowa_dm_operation_t operation,
                                                         iowa_sensor_t publicationId,
                                                         iowa_mqtt_publication_t *publicationDetailsP,
                                                         void *userData,
                                                         iowa_context_t contextP);
```

*operation*
: the type of the operation among IOWA_DM_READ and IOWA_DM_WRITE.

*publicationId*
: the ID of the modified MQTT Publication.

*publicationDetailsP*
: the details of the modified MQTT Publication.

*userData*
: Passed as argument to the callbacks. This can be nil.

*contextP*
: the IOWA context on which iowa_client_enable_mqtt_publication() was called.

\clearpage

### API

#### iowa_client_enable_mqtt_broker

#** Prototype **

```c
iowa_status_t iowa_client_enable_mqtt_broker(iowa_context_t contextP,
                                             iowa_mqtt_broker_update_callback_t brokerCb,
                                             void * userData);
```

#** Description **

`iowa_client_enable_mqtt_broker()` enables the MQTT brokers management.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*brokerCb*
: The broker callback called when a LwM2M Server modify the MQTT brokers.

*userDataP*
: Passed as argument to the callbacks. This can be nil.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: *brokerCb* is nil.

IOWA_COAP_409_CONFLICT
: MQTT brokers management is already enabled.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_mqtt_objects.h

\clearpage

#### iowa_client_disable_mqtt_broker

#** Prototype **

```c
iowa_status_t iowa_client_disable_mqtt_broker(iowa_context_t contextP);

```
#** Description **

`iowa_client_disable_mqtt_broker()` disables the MQTT brokers management.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: MQTT brokers management is not enabled.

#** Header File **

objects/iowa_mqtt_objects.h

\clearpage

#### iowa_client_add_mqtt_broker

#** Prototype **

```c
iowa_status_t iowa_client_add_mqtt_broker(iowa_context_t contextP,
                                          uint16_t optFlags,
                                          const iowa_mqtt_broker_t * brokerDetailsP,
                                          iowa_sensor_t * brokerIdP);
```

#** Description **

`iowa_client_add_mqtt_broker()` adds an MQTT broker instance.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to add optional resources.

*brokerDetailsP*
: The details of the MQTT Broker. Copied internally by IOWA.

*brokerIdP*
: Used to store the ID of the created MQTT Broker instance.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - Invalid *brokerDetailsP*.
: - *brokerIdP* is nil.

IOWA_COAP_412_PRECONDITION_FAILED
MQTT brokers management is not enabled. Call [`iowa_client_enable_mqtt_broker()`](ClientAPI.md#iowa_client_enable_mqtt_broker) first.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.

#** Header File **

objects/iowa_mqtt_objects.h

#** Notes **

The invalid broker's details are
: either:
: - The client identity is nil.
: - The password length is 0 and the password is not nil.
: - The identity length is 0 and the identity is not nil.
: - The private key length is 0 and the private key is not nil.
: - The broker's identity length is 0 and the broker's identity is not nil.
: - The security mode is IOWA_SEC_NONE and the identity length and/or private key length and/or broker's identity length are greater than 0.
: - The security mode is different than IOWA_SEC_NONE and the identity length and/or private key length equal to 0.
: - The security mode is IOWA_SEC_RAW_PUBLIC_KEY or IOWA_SEC_CERTIFICATE and the broker identity length equal to 0.
: - The security mode value is unknown.
: - The certificate usage value is unknown.

To add optional resource, you can use the following flag:

- IOWA_MQTT_BROKER_CERTIFICATE_USAGE : a flag to set the broker certificate usage resource, if this resource is not set by the user, domain issued certificate mode is assumed.

\clearpage

#### iowa_client_remove_mqtt_broker

#** Prototype **

```c
iowa_status_t iowa_client_remove_mqtt_broker(iowa_context_t contextP,
                                             iowa_sensor_t brokerId);
```
#** Description **

`iowa_client_remove_mqtt_broker()` removes an MQTT broker instance.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*brokerId*
: The ID assigned to the MQTT Broker by IOWA.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *brokerId* is not valid. Valid *brokerId* are only returned by [`iowa_client_add_mqtt_broker()`](ClientAPI.md#iowa_client_add_mqtt_broker).

IOWA_COAP_404_NOT_FOUND
: *brokerId* does not match a known MQTT broker.

IOWA_COAP_412_PRECONDITION_FAILED
MQTT brokers management is not enabled. Call [`iowa_client_enable_mqtt_broker()`](ClientAPI.md#iowa_client_enable_mqtt_broker) first.

#** Header File **

objects/iowa_mqtt_objects.h

\clearpage

#### iowa_client_get_mqtt_broker

#** Prototype **

```c
iowa_mqtt_broker_t * iowa_client_get_mqtt_broker(iowa_context_t contextP,
                                                 iowa_sensor_t brokerId);
```
#** Description **

`iowa_client_get_mqtt_broker()` retrieves the details of an MQTT broker.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*brokerId*
: The ID assigned to the MQTT Broker by IOWA.

#** Return Value **

The MQTT broker's details, or NULL if the MQTT brokers management is not enabledor if *brokerId* does not match a known MQTT broker.

#** Notes **

The returned pointer points to the internal data of IOWA and not to duplicated information. It is advised to not modify it.

#** Header File **

objects/iowa_mqtt_objects.h

\clearpage

#### iowa_client_enable_mqtt_publication

#** Prototype **

```c
iowa_status_t iowa_client_enable_mqtt_publication(iowa_context_t contextP,
                                                  iowa_mqtt_publication_update_callback_t publicationCB,
                                                  void *userData);
```
#** Description **

`iowa_client_enable_mqtt_publication()` enables the MQTT Publication management.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*publicationCB*
: The broker callback called when a LwM2M Server modify the MQTT brokers.

*userDataP*
: Passed as argument to the callbacks. This can be nil.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: *publicationCB* is nil.

IOWA_COAP_409_CONFLICT
: MQTT Publication Object already exists.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

objects/iowa_mqtt_objects.h

\clearpage

#### iowa_client_disable_mqtt_publication

#** Prototype **

```c
iowa_status_t iowa_client_disable_mqtt_publication(iowa_context_t contextP);
```
#** Description **

`iowa_client_disable_mqtt_publication()` disables the MQTT Publication management.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: MQTT publication Object not found.

#** Header File **

objects/iowa_mqtt_objects.h

\clearpage

#### iowa_client_add_mqtt_publication

#** Prototype **

```c
iowa_status_t iowa_client_add_mqtt_publication(iowa_context_t contextP,
                                               uint16_t optFlags,
                                               const iowa_mqtt_publication_t *publicationDetailsP,
                                               iowa_sensor_t *publicationIdP);
```
#** Description **

`iowa_client_add_mqtt_publication()` adds an MQTT publication instance.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*optFlags*
: Optional flags to indicate optional resources.

*publicationDetailsP*
: publicationDetailsP: details of the MQTT Publication. Copied internally by IOWA.

*publicationIdP*
: Used to store the ID of the created MQTT Publication instance.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - Invalid *publicationDetailsP*.
: - *publicationIdP* is nil.

IOWA_COAP_412_PRECONDITION_FAILED
: MQTT publication management was not enabled. Call [`iowa_client_enable_mqtt_publication()`](ClientAPI.md#iowa_client_enable_mqtt_publication) first.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) failed.
: Invalid publication source format.

#** Header File **

objects/iowa_mqtt_objects.h

#** Notes **

The invalid publication's details are either:
: - The publication's topic is nil.
: - The publication's source is nil.
: - The publication's QOS is greater than 2.

To add optional resources, you can use the following flag:

- IOWA_MQTT_PUBLICATION_RSC_ENCODING : a flag to set the publication encoding resource, if this resource is not set by the user, the encoding of the data is implementation dependent.
- IOWA_MQTT_PUBLICATION_RSC_ACTIVE : a flag to set the publication active resource. if this resource is not set by the user, it will be assumed to be true.

\clearpage

#### iowa_client_remove_mqtt_publication

#** Prototype **

```c
iowa_status_t iowa_client_remove_mqtt_publication(iowa_context_t contextP,
                                                  iowa_sensor_t publicationId);
```
#** Description **

`iowa_client_remove_mqtt_publication()` removes an MQTT Publication instance.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*publicationId*
: The ID assigned to the MQTT Publication by IOWA.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_402_BAD_OPTION
: *publicationId* is not an MQTT object. Valid *publicationId* are only returned by [`iowa_client_add_mqtt_publication()`](ClientAPI.md#iowa_client_add_mqtt_publication).

IOWA_COAP_404_NOT_FOUND
: MQTT object referred by **publicationId** does not exist.

IOWA_COAP_412_PRECONDITION_FAILED
: MQTT publication management was not enabled. Call [`iowa_client_enable_mqtt_publication()`](ClientAPI.md#iowa_client_enable_mqtt_publication) first.

#** Header File **

objects/iowa_mqtt_objects.h