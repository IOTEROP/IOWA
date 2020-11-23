# Deprecated API Reference

Deprecated APIs are no longer supported which means that regression bugs will not be fixed. These APIs should not be used anymore. You are strongly advised to use their replacement.

Note that deprecated API can be removed in a future IOWA release.

## Deprecated Compilation Flags

### IOWA_SINGLE_CONNECTION_MODE

Support only one connection at a time. Useful for constrained devices in a single LwM2M Server environment.

#### Notes

This define has no more effect when set.

### LWM2M_SINGLE_SERVER_MODE

This is only relevant when IOWA is in Client mode. When set, the Client supports only one Server configuration at a time. Useful for constrained devices in a single LwM2M Server environment.

This define cannot be set when [`LWM2M_BOOTSTRAP`][LWM2M_BOOTSTRAP] is already set.

#### Notes

This define has no more effect when set.

### LWM2M_OLD_CONTENT_FORMAT_SUPPORT

During the development of the Lightweight M2M protocol, some LwM2M products were released. These products were using temporary numbers for the content format of the LwM2M payload. Setting this flag allows IOWA to interact with these old implementations. It should be seldom required.

#### Notes

This define has no more effect when set. By default, the old content formats code are always supported.

### IOWA_LORAWAN_MINIMAL_SUPPORT

Minimal support for LoRaWAN transport. URI scheme is in the form "lorawan://". The Endpoint will not be able to send its Objects list in the Registration message and Registration Update message if needs. This define should only be considered if the code size of IOWA is a concern.

#### Notes

This define has no more effect when set.

### LWM2M_NOTIFICATION_QUEUE_SIZE

This is only relevant when IOWA is in Client mode. When set, this define sets the maximum stored notification values when the notification are not able to reach the Server. The values are saved in RAM by IOWA internally inside a First In First Out Queue. The maximum values stored is per observation.

By default, when the define is not set, the value is 4.

#### Notes

This define has no more effect when set.

### LWM2M_STORAGE_QUEUE_PEEK_SUPPORT

When a LwM2M Server observing some resources is not reachable, the LwM2M Client stores the notifications until the connectivity is restored. By default, IOWA stores the last notifications in memory. When this flag is set, IOWA discharges the storage of these notifications to the platform. New version using a peek/remove mechanism instead of a dequeue mechanism.

This feature requires the system abstraction functions [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create), [`iowa_system_queue_enqueue()`](AbstractionLayer.md#iowa_system_queue_enqueue), [`iowa_system_queue_peek()`](AbstractionLayer.md#iowa_system_queue_peek), [`iowa_system_queue_remove()`](AbstractionLayer.md#iowa_system_queue_remove), and [`iowa_system_queue_delete()`](AbstractionLayer.md#iowa_system_queue_delete) to be implemented.

#### Notes

This define will be later replaced by **LWM2M_STORAGE_QUEUE_SUPPORT**. It means the Storage Queue with Peek behavior will be the default.

### LwM2M features removal

LwM2M mandatory features can be removed depending of the use case. Removing a feature should only be done to reduce the code size of IOWA on constrained devices, and should not be considered for other case:

**LWM2M_READ_OPERATION_REMOVE**
: Remove the ability to handle a Read Operation. Only relevant for LwM2M Client mode. When this flag is set, **LWM2M_OBSERVE_OPERATION_REMOVE** and **LWM2M_WRITE_ATTRIBUTES_OPERATION_REMOVE** are also set.

**LWM2M_DISCOVER_OPERATION_REMOVE**
: Remove the ability to handle a Discover Operation. Only relevant for LwM2M Client mode.

**LWM2M_WRITE_OPERATION_REMOVE**
: Remove the ability to handle a Write Operation. Only relevant for LwM2M Client mode.

**LWM2M_WRITE_ATTRIBUTES_OPERATION_REMOVE**
: Remove the ability to handle a Write-Attributes Operation. Only relevant for LwM2M Client mode.

**LWM2M_EXECUTE_OPERATION_REMOVE**
: Remove the ability to handle a Execute Operation. Only relevant for LwM2M Client mode.

**LWM2M_CREATE_OPERATION_REMOVE**
: Remove the ability to handle a Create Operation. Only relevant for LwM2M Client mode.

**LWM2M_DELETE_OPERATION_REMOVE**
: Remove the ability to handle a Delete Operation. Only relevant for LwM2M Client mode.

**LWM2M_OBSERVE_OPERATION_REMOVE**
: Remove the ability to handle a Observe Operation. Only relevant for LwM2M Client mode. When this flag is set, **LWM2M_WRITE_ATTRIBUTES_OPERATION_REMOVE** is also set.

**LWM2M_OPAQUE_CONTENT_FORMAT_REMOVE**
: Remove the ability to decode/encode the Opaque Content Format.

**LWM2M_TEXT_CONTENT_FORMAT_REMOVE**
: Remove the ability to decode/encode the Plain Text Content Format.

#### Notes

These defines have no more effect when set.

## Deprecated Enumeration Values

Deprecated enumeration values are no longer supported which means that regression bugs will not be fixed. These enumeration values should not be used anymore. You are strongly advised to use their replacement.

Note that deprecated Enumeration can be removed in a future IOWA release.

### IOWA_IPSO_LEVEL_CONTROL

An [`iowa_IPSO_ID_t`][iowa_IPSO_ID_t] enumeration value, used to define percentage sensors.

#### Notes

The Dimmer API is now responsible of adding, removing and updating percentage sensors, by using the following functions:

[`iowa_client_add_dimmer_object()`](ClientAPI.md#iowa_client_add_dimmer_object)
: to add a new dimmer sensor.

[`iowa_client_remove_dimmer_object()`](ClientAPI.md#iowa_client_remove_dimmer_object)
: to remove a dimmer sensor.

[`iowa_client_dimmer_update_value()`](ClientAPI.md#iowa_client_dimmer_update_value)
: to update the dimmer's level value.

[`iowa_client_dimmer_update_values()`](ClientAPI.md#iowa_client_dimmer_update_value)
: to update the dimmer's multiple level values, .

## Deprecated API

### iowa_server_configuration_set

This API has been replaced by [`iowa_client_set_server_configuration()`](ClientAPI.md#iowa_client_set_server_configuration).

** Prototype **

```c
iowa_status_t iowa_server_configuration_set(iowa_context_t contextP,
                                            uint16_t shortId,
                                            iowa_server_setting_id_t settingId,
                                            void *argP);
```

** Description **

`iowa_server_configuration_set()` configures the settings of a LwM2M Server.

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