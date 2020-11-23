# Common API Reference

The functions explained below are defined inside the file *include/iowa.h*.

## Presentation

Whatever the role of your application (Client or Server), it builds on the following skeleton:
```c
#include "iowa.h"

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;

    /******************
     * Initialization
     */

    iowaH = iowa_init(NULL);

    /******************
     * "Main loop"
     */

    do
    {
        result = iowa_step(iowaH, 5);
    } while (result == IOWA_COAP_NO_ERROR)

    iowa_close(iowaH);

    return 0;
}
```

## Data types

### iowa_status_t

This is the return type of most of the IOWA APIs. This is an enumeration matching the CoAP status codes, similar to the HTTP status codes.

```c
typedef uint8_t iowa_status_t;

#define IOWA_COAP_NO_ERROR                        0x00
#define IOWA_COAP_201_CREATED                     0x41
#define IOWA_COAP_202_DELETED                     0x42
#define IOWA_COAP_203_VALID                       0x43
#define IOWA_COAP_204_CHANGED                     0x44
#define IOWA_COAP_205_CONTENT                     0x45
#define IOWA_COAP_231_CONTINUE                    0x5F
#define IOWA_COAP_400_BAD_REQUEST                 0x80
#define IOWA_COAP_401_UNAUTHORIZED                0x81
#define IOWA_COAP_402_BAD_OPTION                  0x82
#define IOWA_COAP_403_FORBIDDEN                   0x83
#define IOWA_COAP_404_NOT_FOUND                   0x84
#define IOWA_COAP_405_METHOD_NOT_ALLOWED          0x85
#define IOWA_COAP_406_NOT_ACCEPTABLE              0x86
#define IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE   0x88
#define IOWA_COAP_409_CONFLICT                    0x89
#define IOWA_COAP_412_PRECONDITION_FAILED         0x8C
#define IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE    0x8D
#define IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT  0x8F
#define IOWA_COAP_422_UNPROCESSABLE_ENTITY        0x96
#define IOWA_COAP_429_TOO_MANY_REQUESTS           0x9D
#define IOWA_COAP_500_INTERNAL_SERVER_ERROR       0xA0
#define IOWA_COAP_501_NOT_IMPLEMENTED             0xA1
#define IOWA_COAP_502_BAD_GATEWAY                 0xA2
#define IOWA_COAP_503_SERVICE_UNAVAILABLE         0xA3
#define IOWA_COAP_504_GATEWAY_TIMEOUT             0xA4
#define IOWA_COAP_505_PROXYING_NOT_SUPPORTED      0xA5
```

### iowa_context_t

This opaque type is used to store the context of the IOWA stack engine. It is created by calling [`iowa_init()`](CommonAPI.md#iowa_init) and destroyed by calling [`iowa_close()`](CommonAPI.md#iowa_close).

Multiple `iowa_context_t` can be created within the same process.

### iowa_dm_operation_t

```c
typedef uint8_t iowa_dm_operation_t;

#define IOWA_DM_UNDEFINED        0
#define IOWA_DM_READ             1
#define IOWA_DM_FREE             2
#define IOWA_DM_WRITE            3
#define IOWA_DM_EXECUTE          4
#define IOWA_DM_CREATE           5
#define IOWA_DM_DELETE           6
#define IOWA_DM_DISCOVER         7
#define IOWA_DM_WRITE_ATTRIBUTES 8
#define IOWA_DM_NOTIFY           9
#define IOWA_DM_CANCEL           10
#define IOWA_DM_DATA_PUSH        11
#define IOWA_DM_READ_REQUEST     12
```

This is an enumeration of the following values:

IOWA_DM_UNDEFINED
: No specific LwM2M operation, this should never be used and only serves as a non default operation.

IOWA_DM_READ
: LwM2M Read operation is used to access the value of an Object, Object Instances, and Resources.

IOWA_DM_FREE
: Free operation is used to clean the allocated memory on *IOWA_DM_READ* operation.

IOWA_DM_WRITE
: LwM2M Write operation is used to change the value of an Object, Object Instances, and Resources.

IOWA_DM_EXECUTE
: LwM2M Execute operation is used to initiate some action, and can only be performed on individual Resources.

IOWA_DM_CREATE
: LwM2M Create operation is used to create Object Instance(s).

IOWA_DM_DELETE
: LwM2M Delete operation is used to delete an Object Instance.

IOWA_DM_DISCOVER
: LwM2M Discover operation is used to discover LwM2M Attributes attached to an Object, Object Instances, and Resources.

IOWA_DM_WRITE_ATTRIBUTES
: LwM2M Write-Attributes operation is used to change the LwM2M Attributes of an Object, Object Instances, and Resources.

IOWA_DM_NOTIFY
: LwM2M Notify operation is used to notify the change of a value during a valid Observation on an Object Instance or Resource.

IOWA_DM_CANCEL
: LwM2M Cancel operation is used to end an Observation.

IOWA_DM_DATA_PUSH
: LwM2M Data push operation is used when LwM2M Server receives the value of an Object, Object Instances, and Resources without requested it.

IOWA_DM_READ_REQUEST
: On the Client side, inform a custom Object that a LwM2M Server will perform a READ operation. See [iowa_lwm2m_resource_desc_t](ClientAPI.md#iowa_lwm2m_resource_desc_t).

#### Notes

This enumeration is used on both Server and Client side. But not all the values are used depending of the side.

Server callbacks can be called with:

- IOWA_DM_READ
- IOWA_DM_WRITE
- IOWA_DM_EXECUTE
- IOWA_DM_CREATE
- IOWA_DM_DELETE
- IOWA_DM_DISCOVER
- IOWA_DM_WRITE_ATTRIBUTES
- IOWA_DM_NOTIFY
- IOWA_DM_DATA_PUSH

Client callbacks can be called with:

- IOWA_DM_READ
- IOWA_DM_FREE
- IOWA_DM_WRITE
- IOWA_DM_EXECUTE
- IOWA_DM_CREATE
- IOWA_DM_DELETE
- IOWA_DM_DATA_PUSH

### iowa_bootstrap_operation_t

```c
typedef uint8_t iowa_bootstrap_operation_t;

#define IOWA_BOOTSTRAP_UNDEFINED               0
#define IOWA_BOOTSTRAP_READ                    101
#define IOWA_BOOTSTRAP_WRITE                   102
#define IOWA_BOOTSTRAP_DELETE                  103
#define IOWA_BOOTSTRAP_DISCOVER                104
#define IOWA_BOOTSTRAP_FINISH                  105
#define IOWA_BOOTSTRAP_ADD_SERVER              106
#define IOWA_BOOTSTRAP_REMOVE_SERVER           107
#define IOWA_BOOTSTRAP_ADD_BOOTSTRAP_SERVER    108
#define IOWA_BOOTSTRAP_REMOVE_BOOTSTRAP_SERVER 109
```

This is an enumeration of the following values:

IOWA_BOOTSTRAP_UNDEFINED
: No specific LwM2M operation, this should never be used and only serves as a non default operation.

IOWA_BOOTSTRAP_READ
: LwM2M Read operation is used to access the value of an Object, Object Instances, and Resources.

IOWA_BOOTSTRAP_WRITE
: LwM2M Write operation is used to change the value of an Object, and Object Instances regardless of an existence of the targeted Object Instance(s).

IOWA_BOOTSTRAP_DELETE
: LwM2M Delete operation is used to delete any Object Instance or all Instances of any Object including the Security Object (ID:0).

IOWA_BOOTSTRAP_DISCOVER
: LwM2M Bootstrap Discover operation is used to discover which LwM2M Objects and Object Instances are supported on a LwM2M Client. In particular, the list of Security Object Instances (ID:0) is reported.

IOWA_BOOTSTRAP_FINISH
: LwM2M Finish operation is used to terminate the Bootstrap Sequence.

IOWA_BOOTSTRAP_ADD_SERVER
: Custom LwM2M Bootstrap Add Server operation is used to write the proper Object Instances of Security Object (ID:0) and Server Object (ID:1) to add a LwM2M Server Account.

IOWA_BOOTSTRAP_REMOVE_SERVER
: Custom LwM2M Bootstrap Remove Server operation is used to delete the proper Object Instances of Security Object (ID:0) and Server Object (ID:1) associated to a LwM2M Server Account.

IOWA_BOOTSTRAP_ADD_BOOTSTRAP_SERVER
: Custom LwM2M Bootstrap Add Bootstrap Server operation is used to write the proper Object Instance of Security Object (ID:0) add a LwM2M Bootstrap Server Account.

IOWA_BOOTSTRAP_REMOVE_BOOTSTRAP_SERVER
: Custom LwM2M Bootstrap Remove Bootstrap Server operation is used to delete the proper Object Instance of Security Object (ID:0) associated to a LwM2M Bootstrap Server Account.

### iowa_lwm2m_data_type_t

```c
typedef uint8_t iowa_lwm2m_data_type_t;

#define IOWA_LWM2M_TYPE_UNDEFINED        0
#define IOWA_LWM2M_TYPE_STRING           1
#define IOWA_LWM2M_TYPE_OPAQUE           2
#define IOWA_LWM2M_TYPE_INTEGER          3
#define IOWA_LWM2M_TYPE_FLOAT            4
#define IOWA_LWM2M_TYPE_BOOLEAN          5
#define IOWA_LWM2M_TYPE_CORE_LINK        6
#define IOWA_LWM2M_TYPE_OBJECT_LINK      7
#define IOWA_LWM2M_TYPE_TIME             8
#define IOWA_LWM2M_TYPE_UNSIGNED_INTEGER 9
```

This is an enumeration of the following values:

IOWA_LWM2M_TYPE_UNDEFINED
: No specific data type: it is only used for Executable Resource.

IOWA_LWM2M_TYPE_STRING
: A UTF-8 string.

IOWA_LWM2M_TYPE_OPAQUE
: A sequence of binary octets.

IOWA_LWM2M_TYPE_INTEGER
: An 64-bit signed integer.

IOWA_LWM2M_TYPE_FLOAT
: A 32 or 64-bit floating point value.

IOWA_LWM2M_TYPE_BOOLEAN
: An unsigned integer with the value 0 for false and the value 1 for true.

IOWA_LWM2M_TYPE_CORE_LINK
: A UTF-8 string representing the relation between resources and links.

IOWA_LWM2M_TYPE_OBJECT_LINK
: Reference to an Instance of a given Object.

IOWA_LWM2M_TYPE_TIME
: A signed integer representing the number of seconds.

IOWA_LWM2M_TYPE_UNSIGNED_INTEGER
: An unsigned integer.

### iowa_lwm2m_data_t

When the LwM2M Server and the LwM2M Client exchange data, at the application level, they are presented in `iowa_lwm2m_data_t` structures.

```c
typedef struct
{
    uint16_t objectID;
    uint16_t instanceID;
    uint16_t resourceID;
    uint16_t resInstanceID;
    iowa_lwm2m_data_type_t type;
    union
    {
        bool    asBoolean;
        int64_t asInteger;
        double  asFloat;
        struct
        {
            size_t   length;
            uint8_t *buffer;
        } asBuffer;
        iowa_lwm2m_object_link_t asObjLink;
    } value;
    int32_t timestamp;
} iowa_lwm2m_data_t;
```

This structure contains the value of a LwM2M resource along its complete URI.

objectID
: ID of the Object containing the resource.

instanceID
: ID of the Object Instance containing the resource.

resourceID
: ID of the resource.

resInstanceID
: ID of the resource instance. For single instance resource, this is always **IOWA_LWM2M_ID_ALL**.

type
: The datatype of the resource.

value.asBoolean
: The value of the resource when type is **IOWA_LWM2M_TYPE_BOOLEAN**.

value.asInteger
: The value of the resource when type is **IOWA_LWM2M_TYPE_INTEGER**, **IOWA_LWM2M_TYPE_TIME** or **IOWA_LWM2M_TYPE_UNSIGNED_INTEGER**.

value.asFloat
: The value of the resource when type is **IOWA_LWM2M_TYPE_FLOAT**.

value.asBuffer
: The value of the resource when type is **IOWA_LWM2M_TYPE_CORE_LINK**, **IOWA_LWM2M_TYPE_STRING**, **IOWA_LWM2M_TYPE_OPAQUE** or **IOWA_LWM2M_TYPE_UNDEFINED**.

value.asObjLink
: The value of the resource when type is **IOWA_LWM2M_TYPE_OBJECT_LINK**.

timestamp
: The timestamp value in seconds. Time is always absolute, and timestamp is present when the value is greater than zero. This can not be negative.

### iowa_lwm2m_object_link_t

```c
typedef struct
{
    uint16_t objectId;
    uint16_t instanceId;
} iowa_lwm2m_object_link_t;
```

### iowa_content_format_t

```c
typedef uint16_t iowa_content_format_t;

#define IOWA_CONTENT_FORMAT_TEXT       0
#define IOWA_CONTENT_FORMAT_OPAQUE     42
#define IOWA_CONTENT_FORMAT_CBOR       60
#define IOWA_CONTENT_FORMAT_SENML_JSON 110
#define IOWA_CONTENT_FORMAT_SENML_CBOR 112
#define IOWA_CONTENT_FORMAT_TLV_OLD    1542
#define IOWA_CONTENT_FORMAT_JSON_OLD   1543
#define IOWA_CONTENT_FORMAT_TLV        11542
#define IOWA_CONTENT_FORMAT_JSON       11543
#define IOWA_CONTENT_FORMAT_UNSET      0xFFFF
```

This is an enumeration of the following values:

IOWA_CONTENT_FORMAT_TEXT
: Plain text encoding (e.g. "123", "-123.45"). Usable only for single resource encoding.

IOWA_CONTENT_FORMAT_OPAQUE
: A sequence of binary octets. Usable only for single resource encoding which data type is **Opaque**.

IOWA_CONTENT_FORMAT_CBOR
: CBOR encoding. Usable only for single resource encoding.

IOWA_CONTENT_FORMAT_SENML_JSON
: LwM2M specific SenML JSON encoding. This may not be supported by all Clients. See [`iowa_client_t`](ServerAPI.md#iowa_client_t).

IOWA_CONTENT_FORMAT_SENML_CBOR
: LwM2M specific SenML CBOR encoding. This may not be supported by all Clients. See [`iowa_client_t`](ServerAPI.md#iowa_client_t).

IOWA_CONTENT_FORMAT_TLV_OLD
: LwM2M specific binary Type-Length-Value format. Usually the most compact one. This one is not anymore used, it only serves as backward compatibility with old LwM2M stack implementation (previous 1.0).

IOWA_CONTENT_FORMAT_JSON_OLD
: LwM2M specific JSON encoding. This may not be supported by all Clients. See [`iowa_client_t`](ServerAPI.md#iowa_client_t). This one is not anymore used, it only serves as backward compatibility with old LwM2M stack implementation (previous 1.0).

IOWA_CONTENT_FORMAT_TLV
: LwM2M specific binary Type-Length-Value format. Usually the most compact one.

IOWA_CONTENT_FORMAT_JSON
: LwM2M specific JSON encoding. This may not be supported by all Clients. See [`iowa_client_t`](ServerAPI.md#iowa_client_t).

IOWA_CONTENT_FORMAT_UNSET
: Used to reset the encoding to the default one.

### iowa_lwm2m_uri_t

```c
typedef struct
{
    uint16_t objectId;
    uint16_t instanceId;
    uint16_t resourceId;
    uint16_t resInstanceId;
} iowa_lwm2m_uri_t;
```

This structure represents a LwM2M URI.

In the LwM2M resource model, resources are grouped into Objects. These Objects have instances. Hence the URI of a resource is in the form **`/{Object}/{Object Instance}/{Resource}`**. Moreover some resources, described are *multiple*, can have several instances, leading to URI in the form **`/{Object}/{Object Instance}/{Resource}/{Resource Instance}`**.

*objectId*
: ID of a LwM2M Object.

*instanceId*
: ID of the Object Instance.

*resourceId*
: ID of the resource.

*resInstanceId*
: ID of the resource instance.

When a segment of the URI is not set, the value of the corresponding field is set to **IOWA_LWM2M_ID_ALL**.

For instance, the URI `/3/0/9` is represented as:
```c
iowa_lwm2m_uri_t::objectId = 3
iowa_lwm2m_uri_t::instanceId = 0
iowa_lwm2m_uri_t::resourceId = 9
iowa_lwm2m_uri_t::resInstanceId = IOWA_LWM2M_ID_ALL
```

the URI `/5` is represented as:
```c
iowa_lwm2m_uri_t::objectId = 5
iowa_lwm2m_uri_t::instanceId = IOWA_LWM2M_ID_ALL
iowa_lwm2m_uri_t::resourceId = IOWA_LWM2M_ID_ALL
iowa_lwm2m_uri_t::resInstanceId = IOWA_LWM2M_ID_ALL
```
\clearpage

### iowa_response_content_t

This structure contains the response content from [`iowa_response_callback_t()`](CommonAPI.md#iowa_response_callback_t) according to its requested *operation*.
```c
typedef struct
{
    union
    {
        struct
        {
            size_t dataCount;
            iowa_lwm2m_data_t *dataP;
        } read;
        struct
        {
            uint32_t notificationNumber;
            size_t dataCount;
            iowa_lwm2m_data_t *dataP;
        } observe;
        struct
        {
            size_t dataCount;
            iowa_lwm2m_data_t *dataP;
        } dataPush;
    } details;
} iowa_response_content_t;
```

details.read
: The information related to **IOWA_DM_READ** operation.

details.read.dataCount
: The number of elements in the *details.read.dataP*. This may be 0.

details.read.dataP
: An array containing the Resource values returned by the Client. This may be nil.

details.observe
: The information related to **IOWA_DM_NOTIFY** operation.

details.observe.notificationNumber
: The notification counter.

details.observe.dataCount
: The number of elements in the *details.observe.dataP*. This may be 0.

details.observe.dataP
: An array containing the Resource values returned by the Client. This may be nil.

details.dataPush
: The information related to **IOWA_DM_DATA_PUSH** operation.

details.dataPush.dataCount
: The number of elements in the *details.dataPush.dataP*. This may be 0.

details.dataPush.dataP
: An array containing the Resource values send by the Client. This may be nil.

\clearpage

## Callbacks

### iowa_response_callback_t

The device management APIs ([`iowa_server_read()`](ServerAPI.md#iowa_server_read), [`iowa_server_observe()`](ServerAPI.md#iowa_server_observe), [`iowa_server_write()`](ServerAPI.md#iowa_server_write), [`iowa_server_write_attributes_string()`](ServerAPI.md#iowa_server_write_attributes_string), [`iowa_server_configure_data_push()`](ServerAPI.md#iowa_server_configure_data_push), [`iowa_bootstrap_server_read()`](BootstrapServerAPI.md#iowa_bootstrap_server_read), [`iowa_client_send_sensor_data()`](ClientAPI.md#iowa_client_send_sensor_data), [`iowa_client_send_data()`](ClientAPI.md#iowa_client_send_data)) are using an `iowa_response_callback_t` to asynchronously return the result of the operation.

```c
typedef void(*iowa_response_callback_t) (uint32_t sourceId,
                                         uint8_t operation,
                                         iowa_status_t status,
                                         iowa_response_content_t *contentP,
                                         void *userDataP,
                                         iowa_context_t contextP);
```

*sourceId*
: The ID of the client targeted by the command for the server APIs ([`iowa_server_read()`](ServerAPI.md#iowa_server_read), [`iowa_server_observe()`](ServerAPI.md#iowa_server_observe), [`iowa_server_write()`](ServerAPI.md#iowa_server_write), [`iowa_server_write_attributes_string()`](ServerAPI.md#iowa_server_write_attributes_string), [`iowa_server_configure_data_push()`](ServerAPI.md#iowa_server_configure_data_push)).
: The ID of the server targeted by the command for the client APIs ([`iowa_client_send_sensor_data()`](ClientAPI.md#iowa_client_send_sensor_data), [`iowa_client_send_data()`](ClientAPI.md#iowa_client_send_data)).

*operation*
: The type of command matching this result.

*status*
: The status of the command.

*contentP*
: The content of the operation. It is nil for **IOWA_DM_WRITE**, **IOWA_DM_EXECUTE**, **IOWA_DM_CREATE**, **IOWA_DM_DELETE** and **IOWA_DM_WRITE_ATTRIBUTES** operations.
: It is also nil for **IOWA_DM_DATA_PUSH** if operation comes from the client APIs.
: It is also nil for **IOWA_DM_READ**, **IOWA_DM_NOTIFY**, **IOWA_DM_DISCOVER** and **IOWA_DM_DATA_PUSH** if the the *status* is different from **IOWA_205_COAP_CONTENT**

*userDataP*
: A pointer to application specific data. This is a parameter of the matching device management API.

*contextP*
: The IOWA context on which the device management API was called.

** Notes **

The status can have the following values:

| status                                   | Description                                                                                                           |
| ---------------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| IOWA_COAP_201_CREATED                    | **IOWA_DM_CREATE** operation completed successfully                                                                   |
| IOWA_COAP_202_DELETED                    | **IOWA_DM_DELETE** operation completed successfully                                                                   |
| IOWA_COAP_204_CHANGED                    | **IOWA_DM_WRITE**, **IOWA_DM_EXECUTE**, or **IOWA_DM_WRITE_ATTRIBUTES** operation completed successfully              |
| IOWA_COAP_205_CONTENT                    | **IOWA_DM_READ**, **IOWA_DM_NOTIFY**, **IOWA_DM_DISCOVER**, or **IOWA_DM_DATA_PUSH** operation completed successfully |
| IOWA_COAP_400_BAD_REQUEST                | Undetermined error occurred, when the request is ill-formed                                                           |
| IOWA_COAP_401_UNAUTHORIZED               | Access Right Permission Denied                                                                                        |
| IOWA_COAP_404_NOT_FOUND                  | URI of the operation is not found                                                                                     |
| IOWA_COAP_405_METHOD_NOT_ALLOWED         | Target is not allowed for the operation                                                                               |
| IOWA_COAP_406_NOT_ACCEPTABLE             | None of the preferred Content-Formats can be returned                                                                 |
| IOWA_COAP_408_REQUEST_ENTITY_INCOMPLETE  | The request or answer is incomplete                                                                                   |
| IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE   | The request or answer is too large for the system                                                                     |
| IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT | The specified format is not supported                                                                                 |
| IOWA_COAP_500_INTERNAL_SERVER_ERROR      | An internal server error occurred                                                                                     |

\clearpage

### iowa_load_callback_t

This callback is called during a context load with the data stored inside the context backup.

```c
typedef void(*iowa_load_callback_t) (uint16_t callbackId,
                                     uint8_t * buffer,
                                     size_t bufferLength,
                                     void *userDataP);
```

*callbackId*
: The identifier of the callback as passed to [`iowa_backup_register_callback()`](CommonAPI.md#iowa_backup_register_callback).

*buffer*
: The data loaded from the backup. This can be nil.

*bufferLength*
: The length of *buffer* in bytes.

*userDataP*
: A pointer to application specific data as passed to [`iowa_backup_register_callback()`](CommonAPI.md#iowa_backup_register_callback).

\clearpage

### iowa_save_callback_t

This callback is called during a context save. It is called first to retrieve the length of the data to save, then a second time with an allocated buffer to fill with the data to save.

```c
typedef size_t(*iowa_save_callback_t) (uint16_t callbackId,
                                       uint8_t * buffer,
                                       size_t bufferLength,
                                       void *userDataP);
```

*callbackId*
: The identifier of the callback as passed to [`iowa_backup_register_callback()`](CommonAPI.md#iowa_backup_register_callback).

*buffer*
: A buffer to store the data. This can be nil.

*bufferLength*
: The length of *buffer* in bytes.

*userDataP*
: A pointer to application specific data as passed to [`iowa_backup_register_callback()`](CommonAPI.md#iowa_backup_register_callback).

\clearpage

## API

### iowa_init

** Prototype **

```c
iowa_context_t iowa_init(void * userData);
```

** Description **

`iowa_init()` initializes an IOWA context.

** Arguments **

*userData*
: Pointer to application-specific data. This is passed as argument to the Communication Abstraction Interface functions. This can be nil.

** Return Value **

An [`iowa_context_t`](CommonAPI.md#iowa_context_t) in case of success or NULL in case of memory allocation error.

** Header File **

iowa.h

\clearpage

### iowa_step

** Prototype **

```c
iowa_status_t iowa_step(iowa_context_t contextP,
                        int32_t timeout);
```

** Description **

`iowa_step()` runs the stack engine during the specified time.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*timeout*
: The allowed time to run in seconds.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: one of the [`iowa_system_connection...()`](AbstractionLayer.md#iowa_system_connection_open) functions returned an error.
: LwM2M Client only: when the Client failed to connect to any Server. ([iowa_event_callback_t](ClientAPI.md#iowa_event_callback_t) is called with a **IOWA_EVENT_REG_FAILED** event.)

** Header File **

iowa.h

** Notes **

If *timeout* is a negative value:

- `iowa_step()` will return only in case of error.
- [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select) will be called with **INT32_MAX**.

For LwM2M Clients: if `iowa_step()` returns an **IOWA_COAP_503_SERVICE_UNAVAILABLE** error because it is not registered to any LwM2M Server, subsequent call to `iowa_step()`, will retry to register to the known LwM2M Servers.

[`iowa_stop()`](CommonAPI.md#iowa_stop) can be used to make `iowa_step()`return immediately.

\clearpage

### iowa_flush_before_pause

** Prototype **

```c
iowa_status_t iowa_flush_before_pause(iowa_context_t contextP,
                                      int32_t duration,
                                      uint32_t *delayP);
```

** Description **

`iowa_flush_before_pause()` is used to inform the stack that the device will pause. `iowa_flush_before_pause()` performs all the pending and required operations of the stack engine before returning.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*duration*
: The duration of the planned pause in seconds.

*delayP*
: The delay before the next IOWA scheduled operation.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *duration* is negative.
: - for a LwM2M Client: *duration* is longer than one of the LwM2M Server registration lifetime.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: one of the [`iowa_system_connection...()`](AbstractionLayer.md#iowa_system_connection_open) functions returned an error.

** Header File **

iowa.h

** Notes **

In LwM2M Client mode: if no server are configured, the function returns immediately with no error.

A LwM2M Server should never stop. For a LwM2M Server, `iowa_flush_before_pause()` will just wait for all pending CoAP transactions to finish.

\clearpage

### iowa_stop

** Prototype **

```c
void iowa_stop(iowa_context_t contextP);
```

** Description **

`iowa_stop()` stops the stack engine and make [`iowa_step()`](CommonAPI.md#iowa_step) return immediately.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

None.

** Header File **

iowa.h

\clearpage

### iowa_close

** Prototype **

```c
void iowa_close(iowa_context_t contextP);
```

** Description **

`iowa_close()` closes an IOWA context.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

None.

** Header File **

iowa.h

\clearpage

### iowa_clock_reset

** Prototype **

```c
iowa_status_t iowa_clock_reset(iowa_context_t contextP);
```

** Description **

`iowa_clock_reset()` informs IOWA that [`iowa_system_gettime()`](iowa_system_gettime) has lost track of time and that IOWA must re-synchronize its internal timers.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: - the [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) function returned an error.

** Header File **

iowa.h

\clearpage

### iowa_save_context

** Prototype **

```c
iowa_status_t iowa_save_context(iowa_context_t contextP);
```

** Description **

`iowa_save_context()` saves the current IOWA context.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - the [`iowa_system_store_context()`](AbstractionLayer.md#iowa_system_store_context) function returned an error.

** Header File **

iowa.h

** Notes **

Currently, this API is only for a LwM2M Client.

If IOWA is built with the **IOWA_STORAGE_CONTEXT_AUTOMATIC_BACKUP** flag, the context will be automatically saved:

* After a LwM2M Bootstrap Server or a LwM2M Server were added.
* After a successful Bootstrap procedure.
* After a LwM2M Server operation on resources related to Server Accounts.

\clearpage

### iowa_save_context_snapshot

** Prototype **

```c
iowa_status_t iowa_save_context_snapshot(iowa_context_t contextP);
```

** Description **

`iowa_save_context_snapshot()` saves the current IOWA context with runtime information, observations and attributes.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - the [`iowa_system_store_context()`](AbstractionLayer.md#iowa_system_store_context) function returned an error.

** Header File **

iowa.h

** Notes **

Currently, this API is only for a LwM2M Client.

\clearpage

### iowa_load_context

** Prototype **

```c
iowa_status_t iowa_load_context(iowa_context_t contextP);
```

** Description **

`iowa_load_context()` loads a saved IOWA context.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: either:
: - no previous saved IOWA context found.
: - In LwM2M client mode: no Security or Server objects found, the function [`iowa_client_configure`](ClientAPI.md#iowa_client_configure) must be called first.

IOWA_COAP_409_CONFLICT
: *contextP* isn't in init state.

IOWA_COAP_422_UNPROCESSABLE_ENTITY
: either:
: - failed to decode the buffer retrieved from [`iowa_system_retrieve_context()`](AbstractionLayer.md#iowa_system_retrieve_context) function.
: - context version isn't present or is incorrect.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - the [`iowa_system_retrieve_context()`](AbstractionLayer.md#iowa_system_retrieve_context) function returned an error.

** Header File **

iowa.h

** Notes **

Currently, this API is only for a LwM2M Client.

\clearpage

### iowa_backup_register_callback

** Prototype **

```c
iowa_status_t iowa_backup_register_callback(iowa_context_t contextP,
                                            uint16_t id,
                                            iowa_save_callback_t saveCallback,
                                            iowa_load_callback_t loadCallback,
                                            void *userDataP);
```

** Description **

`iowa_backup_register_callback()` registers context callbacks from IOWA.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: The id of the callback. Must be greater than 0xF000.

*saveCallback*
: The function called during context saving.

*loadCallback*
: The function called during context loading.

*userDataP*
: Pointer to application-specific data. This is passed as argument to the callback. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_403_FORBIDDEN
: either:
: - id is incorrect.
: - one of the callback is NULL.

IOWA_COAP_409_CONFLICT
: id is already used.

IOWA_COAP_422_UNPROCESSABLE_ENTITY
: failed to decode the buffer retrieved from [`iowa_system_retrieve_context()`](AbstractionLayer.md#iowa_system_retrieve_context) function.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa.h

** Notes **

Currently, this API is only for a LwM2M Client.

\clearpage

### iowa_backup_deregister_callback

** Prototype **

```c
void iowa_backup_deregister_callback(iowa_context_t contextP,
                                     uint16_t id);
```

** Description **

`iowa_backup_deregister_callback()` deregisters context callbacks from IOWA.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*id*
: The id of the callback. Must be greater than 0xF000.

** Return Value **

None.

** Header File **

iowa.h

** Notes **

Currently, this API is only for a LwM2M Client.

\clearpage

### iowa_connection_closed

** Prototype **

```c
void iowa_connection_closed(iowa_context_t contextP,
                            void *connP);
```

** Description **

`iowa_connection_closed()` informs IOWA that a connection was closed by an external event (e.g. peer disconnection).

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*connP*
: The closed connection of the same user-defined type as the one returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

** Return Value **

None.

** Header File **

iowa.h

\clearpage

### iowa_data_get_block_info

** Prototype **

```c
iowa_status_t iowa_data_get_block_info(iowa_lwm2m_data_t *dataP,
                                       uint32_t *numberP,
                                       bool *moreP,
                                       uint16_t *sizeP);
```

** Description **

`iowa_data_get_block_info()` retrieves the block information from an [iowa_lwm2m_data_t](CommonAPI.md#iowa_lwm2m_data_t).

** Arguments **

*dataP*
: the [iowa_lwm2m_data_t](CommonAPI.md#iowa_lwm2m_data_t) to retrieve the block information from.

*numberP*
: OUT. The block number.

*moreP*
: OUT. *true* if there are more blocks coming, *false* otherwise.

*sizeP*
: OUT. The size of the block in bytes.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *dataP* does not contain any block information.

IOWA_COAP_400_BAD_REQUEST
: either:
: - one of the parameters is nil.
: - the block information in *dataP* is invalid.

** Header File **

iowa.h

\clearpage

### iowa_data_set_block_info

** Prototype **

```c
iowa_status_t iowa_data_set_block_info(iowa_lwm2m_data_t *dataP,
                                       uint32_t number,
                                       bool more,
                                       uint16_t size);
```

** Description **

`iowa_data_set_block_info()` sets the block information of an [iowa_lwm2m_data_t](CommonAPI.md#iowa_lwm2m_data_t).

** Arguments **

*dataP*
: the [iowa_lwm2m_data_t](CommonAPI.md#iowa_lwm2m_data_t) to set the block information to.

*number*
: The block number.

*more*
: *true* if there are more blocks coming, *false* otherwise.

*size*
: The size of the block in bytes.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *dataP* is nil.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *number* is greater than 262 143.
: - *more* is set to *true* and *size* is different than 16, 32, 64, 128, 256, 512, or 1024.
: - *more* is set to *false* and *size* is either zero or greater than 1024.
: - *dataP* data type is neither **IOWA_LWM2M_TYPE_STRING**, **IOWA_LWM2M_TYPE_OPAQUE**, nor **IOWA_LWM2M_TYPE_CORE_LINK**.

** Header File **

iowa.h