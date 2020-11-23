# Server Mode API Reference

The functions explained below are defined inside the file *include/iowa_server.h*.

## Server pseudo code

```c
#include "iowa_server.h"

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    int serverSocket;

    /******************
    * Initialization
    */

    serverSocket = open_udp_socket();

    iowaH = iowa_init(NULL);

    result = iowa_server_configure(iowaH, client_monitor, NULL, NULL);

    /******************
    * "Main loop"
    */

    while (result == IOWA_COAP_NO_ERROR)
    {
        result = iowa_step(iowaH, 10);

        if (isDataAvailable(serverSocket))
        {
            void * newConnection;

            newConnection = create_new_connection();
            result = iowa_server_new_incoming_connection(iowaH,
                                                         IOWA_CONN_DATAGRAM,
                                                         newConnection,
                                                         true);
        }
    }

    iowa_close(iowaH);
    close(serverSocket);

    return 0;
}
```

## Data types

### iowa_supported_format_t

```c
typedef uint8_t iowa_supported_format_t;

#define IOWA_SUPPORTED_FORMAT_UNKNOWN    0x00
#define IOWA_SUPPORTED_FORMAT_TLV        0x01
#define IOWA_SUPPORTED_FORMAT_JSON       0x02
#define IOWA_SUPPORTED_FORMAT_OLD_TLV    0x04
#define IOWA_SUPPORTED_FORMAT_OLD_JSON   0x08
#define IOWA_SUPPORTED_FORMAT_CBOR       0x10
#define IOWA_SUPPORTED_FORMAT_SENML_JSON 0x20
#define IOWA_SUPPORTED_FORMAT_SENML_CBOR 0x40
```

This contains the possible supported content format. It is an enumeration of the following values:

IOWA_SUPPORTED_FORMAT_UNKNOWN
: Unknown supported format.

IOWA_SUPPORTED_FORMAT_TLV
: TLV supported format.

IOWA_SUPPORTED_FORMAT_JSON
: JSON supported format.

IOWA_SUPPORTED_FORMAT_OLD_TLV
: Old TLV supported format.

IOWA_SUPPORTED_FORMAT_OLD_JSON
: Old JSON supported format.

IOWA_SUPPORTED_FORMAT_CBOR
: CBOR format.

IOWA_SUPPORTED_FORMAT_SENML_JSON
: SenML JSON supported format.

IOWA_SUPPORTED_FORMAT_SENML_CBOR
: SenML CBOR supported format.

### iowa_lwm2m_protocol_version_t

```c
typedef enum
{
    IOWA_LWM2M_VERSION_UNDEFINED = 0,
    IOWA_LWM2M_VERSION_1_0,
    IOWA_LWM2M_VERSION_1_1
} iowa_lwm2m_protocol_version_t;
```

This contains the possible LwM2M Enabler version. It is an enumeration of the following values:

IOWA_LWM2M_VERSION_UNDEFINED
: Unknown LwM2M enabler version.

IOWA_LWM2M_VERSION_1_0
: LwM2M Enabler version 1.0.

IOWA_LWM2M_VERSION_1_1
: LwM2M Enabler version 1.1.

### iowa_client_t

This structure describes a LwM2M Client known to the LwM2M Server.

```c
typedef struct {
    const char                    *name;
    uint16_t                       id;
    bool                           queueMode;
    iowa_supported_format_t        supportedFormats;
    const char                    *msisdn;
    size_t                         objectLinkCount;
    iowa_lwm2m_object_link_t      *objectLinkArray;
    uint32_t                       lifetime;
    iowa_connection_type_t         connectionType;
    bool                           secureConnection;
    iowa_lwm2m_protocol_version_t  lwm2mVersion;
} iowa_client_t;
```

*name*
: The unique name of the Client.

*id*
: The internal ID of the Client. To be used with `iowa_server_dm_...()` APIs.

*queueMode*
: Set to *true* if the LwM2M Client supports the Queue Mode.

*supportedFormats*
: The content formats supported by the Client.

*msisdn*
: The MSISDN to which the LwM2M Client is reachable for SMS trigger.

*objectLinkCount*
: The number of elements in the *objectLinkArray*.

*objectLinkArray*
: An array containing the Objects and Object Instances registered by the Client.
: If an Object has no Instances, `iowa_lwm2m_object_link_t::instanceID` is set to **IOWA_LWM2M_ID_ALL**.

*lifetime*
: The lifetime of the Client.

*connectionType*
: The type of the connection on which the Client reach the Server.

*secureConnection*
: Set to *true* if the connection is encrypted.

*lwm2mVersion*
: The LwM2M Enabler version used by the Client.

\clearpage

## Callbacks

### iowa_result_callback_t

The device management APIs ([`iowa_server_write()`](ServerAPI.md#iowa_server_write), [`iowa_server_dm_exec()`](ServerAPI.md#iowa_server_dm_exec), [`iowa_server_dm_create()`](ServerAPI.md#iowa_server_dm_create), [`iowa_server_dm_delete()`](ServerAPI.md#iowa_server_dm_delete), [`iowa_server_dm_discover()`](ServerAPI.md#iowa_server_dm_discover) and [`iowa_server_observe()`](ServerAPI.md#iowa_server_observe)) are using an `iowa_result_callback_t` to asynchronously return the result
of the operation.

```c
typedef void(*iowa_result_callback_t) (iowa_dm_operation_t operation,
                                       uint16_t clientId,
                                       uint16_t objectId,
                                       uint16_t instanceId,
                                       uint16_t resourceId,
                                       iowa_status_t status,
                                       size_t dataCount,
                                       iowa_lwm2m_data_t *dataArray,
                                       void *resultUserData,
                                       iowa_context_t contextP);
```

*operation*
: The type of command matching this result.

*clientId*
: The ID of the client targeted by the command.

*objectId*
: The ID of the Object targeted by the command.

*instanceId*
: The ID of the Instance targeted by the command. This may be **IOWA_LWM2M_ID_ALL**.

*resourceId*
: The ID of the Resource targeted by the command. This may be **IOWA_LWM2M_ID_ALL**.

*status*
: The status of the command or the notification counter if operation is **IOWA_DM_NOTIFY**.

*dataCount*
: The number of elements in the *dataArray*. This may be 0.

*dataArray*
: An array containing the Resource values returned by the Client. This may be nil.

*resultUserData*
: A pointer to application specific data. This is a parameter of the matching `iowa_server_dm_...()` API.

*contextP*
: The IOWA context on which the device management API ([`iowa_server_dm_exec()`](ServerAPI.md#iowa_server_dm_exec)) was called.

### iowa_monitor_callback_t

This is the client state monitoring callback, called when a LwM2M Client changes its registration to the Server or when a LwM2M Client connects to the Bootstrap Server.

```c
typedef void (*iowa_monitor_callback_t)(const iowa_client_t *clientP,
                                        iowa_state_t state,
                                        void *callbackUserData,
                                        iowa_context_t contextP);
```

*clientP*
: The information of the Client.

*state*
: The new state of the Client among:
: - **IOWA_STATE_REGISTERED**: when a new or returning client registers.
: - **IOWA_STATE_UPDATING**: when a client updates its registration.
: - **IOWA_STATE_UNREGISTERED**: when a client ends its registration or when the registration expires.
: - **IOWA_STATE_BOOTSTRAP_REQUIRED**: when a new or returning client connects to the bootstrap server.
: - **IOWA_STATE_BOOTSTRAPPING**: when a client is in bootstrapping state.
: - **IOWA_STATE_BOOTSTRAP_FAILED**: when the bootstrapping procedure of a client failed.
: - **IOWA_STATE_BOOTSTRAP_FINISHED**: when the bootstrapping procedure of a client succeeded.

*callbackUserData*
: A pointer to application specific data. This is a parameter of [`iowa_server_configure()](ServerAPI.md#iowa_server_configure).

*contextP*
: The IOWA context on which [`iowa_server_configure()`](ServerAPI.md#iowa_server_configure) was called.

When *state* is set to **IOWA_STATE_UNREGISTERED**, *clientP* contains only the ID of the former Client.

When *state* is set to **IOWA_STATE_BOOTSTRAP_REQUIRED**, **IOWA_STATE_BOOTSTRAPPING**, **IOWA_STATE_BOOTSTRAP_FAILED**, or **IOWA_STATE_BOOTSTRAP_FINISHED**, *clientP* contains only the ID, the name of the Client and the connection type information.

### iowa_resource_type_callback_t

This is the callback called to retrieve the data type of resources of non standard LwM2M Objects.

```c
typedef iowa_lwm2m_data_type_t(*iowa_resource_type_callback_t) (uint16_t objectID,
                                                                uint16_t resourceID,
                                                                void *callbackUserData);
```

** Arguments **

*objectID*
: The ID of the non standard LwM2M Objects.

*resourceID*
: The ID of the resource inside the non standard LwM2M Objects.

*callbackUserData*
: A pointer to application specific data. This is a parameter of [`iowa_server_configure()](ServerAPI.md#iowa_server_configure).

** Return Value **

The data type of the resource or **IOWA_LWM2M_TYPE_UNDEFINED**.

### iowa_verify_client_callback_t

This is the callback called when LwM2M Clients register to the Server. If the callback returns **IOWA_COAP_NO_ERROR**, the Client is accepted by the Server. Otherwise, to reject a Client the callback has to return **ONLY** the following values:

- **IOWA_COAP_400_BAD_REQUEST** if the Client is unknown or something does not match.
- **IOWA_COAP_409_CONFLICT** if on LoRaWAN transport, the Client didn't provide its objects list and this list is not present on Server side.

```c
typedef iowa_status_t (*iowa_verify_client_callback_t) (const iowa_client_t *clientP,
                                                        iowa_state_t state,
                                                        void *callbackUserData,
                                                        iowa_context_t contextP);
```

*clientP*
: The information of the Client.

*state*
: The new state of the Client among:
: - **IOWA_STATE_REGISTERING**: when a new or returning client registers.
: - **IOWA_STATE_UPDATING**: when a client updates its registration.
: - **IOWA_STATE_BOOTSTRAP_REQUIRED**: when a new or returning client connects to the bootstrap server.

*callbackUserData*
: A pointer to application specific data. This is a parameter of [`iowa_server_set_verify_client_callback()](ServerAPI.md#iowa_server_set_verify_client_callback).

*contextP*
: The IOWA context on which [`iowa_server_configure()`](ServerAPI.md#iowa_server_configure) was called.

When *state* is set to **IOWA_STATE_BOOTSTRAP_REQUIRED**, *clientP* contains only the ID, the name of the Client and the connection type information.

\clearpage

## API

### iowa_server_configure

** Prototype **

```c
iowa_status_t iowa_server_configure(iowa_context_t contextP,
                                    iowa_monitor_callback_t monitorCb,
                                    iowa_resource_type_callback_t resTypeCb,
                                    void * callbackUserData);
```

** Description **

`iowa_server_configure()` sets the monitoring callback called when LwM2M Clients register to the Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*monitorCb*
: The callback called when Clients update their status. This can be nil.

*resTypeCb*
: The callback called when parsing received data of non standard LwM2M Objects. This can be nil.

*callbackUserData*
: A pointer to application specific data. This is passed as argument to *monitorCb* and *resTypeCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

** Header File **

iowa_server.h

\clearpage

### iowa_server_set_verify_client_callback

** Prototype **

```c
void iowa_server_set_verify_client_callback(iowa_context_t contextP,
                                            iowa_verify_client_callback_t verifyClientCb,
                                            void *callbackUserData);
```

** Description **

`iowa_server_set_verify_client_callback()` sets the verify client callback called when LwM2M Clients register to the Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*verifyClientCb*
: The callback called when Clients register. This can be nil.

*callbackUserData*
: A pointer to application specific data. This is passed as argument to *verifyClientCb*. This can be nil.

** Return Value **

None.

** Header File **

iowa_server.h

** Notes **

If the verify client callback is not set, Clients will always be accepted.

\clearpage

### iowa_server_new_incoming_connection

** Prototype **

```c
iowa_status_t iowa_server_new_incoming_connection(iowa_context_t contextP,
                                                  iowa_connection_type_t type,
                                                  void * connP,
                                                  bool isSecure);
```

** Description **

`iowa_server_new_incoming_connection()` informs the stack of a new incoming connection.

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

IOWA_COAP_503_SERVICE_UNAVAILABLE
: either:
: - a memory allocation failed.
: - *type* is unsupported.

** Header File **

iowa_server.h

\clearpage

### iowa_server_configure_data_push

** Prototype **

```c
void iowa_server_configure_data_push(iowa_context_t contextP,
                                     iowa_response_callback_t responseCb,
                                     void *userDataP);
```

** Description **

`iowa_server_configure_data_push()` enables/disables the Data push operation for all clients.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*responseCb*
: The callback called when a client pushes data. If this callback is nil, data push possibility is disabled.

*userDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

** Return Value **

None.

** Header File **

iowa_server.h

** Notes **

The *responseCb* will be called with the operation set to **IOWA_DM_DATA_PUSH** and the status code set to **IOWA_COAP_205_CONTENT**.

To be able to use this function, [**LWM2M_DATA_PUSH_SUPPORT**][LWM2M_DATA_PUSH_SUPPORT] must be defined.

\clearpage

### iowa_server_read

** Prototype **

```c
iowa_status_t iowa_server_read(iowa_context_t contextP,
                               uint32_t clientId,
                               size_t uriCount,
                               iowa_lwm2m_uri_t *uriP,
                               iowa_response_callback_t responseCb,
                               void *userDataP)
```

** Description **

`iowa_server_read()` performs a Read operation on Client’s URIs.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*uriCount, uriP*
: An array of the URIs to read.

*responseCb*
: The callback called when the reply to this operation is known.

*userDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: Either:
: - *responseCb* is nil.
: - *uriCount* is zero or *uriP* is nil.
: - *uriP* targets Object ID **IOWA_LWM2M_ID_ALL** and [**LWM2M_READ_COMPOSITE_SUPPORT**][LWM2M_READ_COMPOSITE_SUPPORT] is not defined.
: - *uriP* targets Object ID **IOWA_LWM2M_ID_ALL**, [**LWM2M_READ_COMPOSITE_SUPPORT**][LWM2M_READ_COMPOSITE_SUPPORT] is defined, but *uriCount* is not equal to 1.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: A memory allocation failed.

IOWA_COAP_501_NOT_IMPLEMENTED
: *uriCount* is superior to 1 with [**LWM2M_READ_COMPOSITE_SUPPORT**][LWM2M_READ_COMPOSITE_SUPPORT] not defined.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Read was successful, the Client will return a **IOWA_COAP_205_CONTENT** status code.

The ability to read several URIs at once is only present in LwM2M version 1.1 or later, this means that to use it **LWM2M_VERSION_1_1_SUPPORT** must be defined. This feature is only operational on SenML JSON and SenML CBOR data encoding, so **LWM2M_SUPPORT_SENML_JSON** or **LWM2M_SUPPORT_SENML_CBOR** must be defined.

Some LwM2M Clients may not be able to read on several URIs in a single operation. In this case the *resultCb* will be called with an error status, typically **IOWA_COAP_405_METHOD_NOT_ALLOWED**.

Per LwM2M specification, when handling a read on several URIs in a single operation, the LwM2M Client treats the request as non-atomic and handles it as best effort. Hence the reply may not contain the values of all the requested URIs.

The *responseCb* will be called with the operation set to **IOWA_DM_READ**.

\clearpage

### iowa_server_observe

** Prototype **

```c
iowa_status_t iowa_server_observe(iowa_context_t contextP,
                                  uint32_t clientId,
                                  size_t uriCount,
                                  iowa_lwm2m_uri_t *uriP,
                                  iowa_response_callback_t responseCb,
                                  void *userDataP,
                                  uint16_t *observeIdP);
```

** Description **

`iowa_server_observe()` begins an observation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*uriCount, uriP*
: An array of the URIs to observe.

*responseCb*
: The callback called when the reply to this operation is known.

*userDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

*observeIdP*
: Used to store the ID of the observation.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: Either:
: - *responseCb* is nil.
: - *observeIdP* is nil.
: - *uriCount* is zero or *uriP* is nil.
: - *uriP* targets Object ID **IOWA_LWM2M_ID_ALL** and [**LWM2M_READ_COMPOSITE_SUPPORT**][LWM2M_READ_COMPOSITE_SUPPORT] is not defined.
: - *uriP* targets Object ID **IOWA_LWM2M_ID_ALL**, [**LWM2M_READ_COMPOSITE_SUPPORT**][LWM2M_READ_COMPOSITE_SUPPORT] is defined, but *uriCount* is not equal to 1.
: - At least one *uriP* is invalid: *instanceId* is equal to **IOWA_LWM2M_ID_ALL** but *resourceId* is not equal to **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_412_PRECONDITION_FAILED
: Observe was already launched. *observeIdP* is set to the value of the previous observation.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: A memory allocation failed.

IOWA_COAP_501_NOT_IMPLEMENTED
: *uriCount* is superior to 1 with [**LWM2M_OBSERVE_COMPOSITE_SUPPORT**][LWM2M_OBSERVE_COMPOSITE_SUPPORT] not defined.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Observe was successful, the Client will return a **IOWA_COAP_205_CONTENT** status code with the first notification.

The *responseCb* will be called with the operation set to **IOWA_DM_NOTIFY**:

- When the observation is internally deleted, *responseCb* will be called with *status* to **IOWA_COAP_202_DELETED**.
- When the client deregisters or when the connection with the client is lost, *responseCb* will be called with *status* to **IOWA_COAP_503_SERVICE_UNAVAILABLE**.
- Some LwM2M Clients may not be able to observe on several URIs in a single operation. In this case the *responseCb* will be called with an error status, typically **IOWA_COAP_405_METHOD_NOT_ALLOWED**.

When using an unreliable communication layer, notifications may be lost or arrive out of order.

\clearpage

### iowa_server_observe_cancel

** Prototype **

```c
iowa_status_t iowa_server_observe_cancel(iowa_context_t contextP,
                                         uint32_t clientId,
                                         uint16_t observeId);
```

** Description **

`iowa_server_observe_cancel()` cancels an observation on a Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*observeId*
: The ID of the observation as returned by [`iowa_server_observe`](ServerAPI.md#iowa_server_observe).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: either:
: - *clientId* does not match a known client.
: - *observeId* does not match a known observation.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: A memory allocation failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

When using an unreliable communication layer, the cancellation request from the LwM2M Server to the LwM2M Client may be lost. However the observation is always cancelled.

\clearpage

### iowa_server_write

** Prototype **

```c
iowa_status_t iowa_server_write(iowa_context_t contextP,
                                uint32_t clientId,
                                size_t dataCount,
                                iowa_lwm2m_data_t *dataArrayP,
                                iowa_response_callback_t responseCb,
                                void *userDataP)
```

** Description **

`iowa_server_write()` performs a Write operation on a Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*dataCount, dataArrayP*
: The data to write.

*responseCb*
: The callback called when the reply to this operation is known. This can be nil.

*userDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: Either:
: - *dataCount* is zero or *dataArrayP* is nil.
: - *dataArrayP[x].objectID* or *dataArrayP[x].instanceID* or *dataArrayP[x].resourceID* is **IOWA_LWM2M_ID_ALL**.
: - *dataArrayP* contains several data with incorrect type.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *dataArrayP* contains several data with different *objectID* or *instanceID* but **LWM2M_SUPPORT_SENML_JSON** or **LWM2M_SUPPORT_SENML_CBOR** are not defined.
: - *dataArrayP* contains several data with defined timestamp.
: - *dataArrayP* contains several data of unsigned integer type which are negative.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_501_NOT_IMPLEMENTED
: *dataArrayP* has different *objectID* or *instanceID* with [**LWM2M_WRITE_COMPOSITE_SUPPORT**][LWM2M_WRITE_COMPOSITE_SUPPORT] not defined.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Write was successful, the Client will return a **IOWA_COAP_204_CHANGED** status code.

To be able to write on different *dataArrayP[x].objectID* or *dataArrayP[x].instanceID* at once, **LWM2M_SUPPORT_SENML_JSON** or **LWM2M_SUPPORT_SENML_CBOR** must be defined.

Some LwM2M Clients may not be able to write on different *dataArrayP[x].objectID* or *dataArrayP[x].instanceID* in a single operation. In this case the *responseCb* will be called with an error status, typically **IOWA_COAP_405_METHOD_NOT_ALLOWED**.

The *responseCb* will be called with the operation set to **IOWA_DM_WRITE**.

\clearpage

### iowa_server_write_attributes_string

** Prototype **

```c
iowa_status_t iowa_server_write_attributes_string(iowa_context_t contextP,
                                                  uint32_t clientId,
                                                  iowa_lwm2m_uri_t *uriP,
                                                  const char *attributesStr,
                                                  iowa_response_callback_t responseCb,
                                                  void *userDataP);

```

** Description **

`iowa_server_write_attributes_string()` performs a Write-Attributes operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*uriP*
: The URI targeted by the operation.

*attributesStr*
: The attributes to write as a query string.

*responseCb*
: The callback called when the reply to this operation is known. This can be nil.

*userDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either
    : *uriP* is nil.
    : *uriP->objectId* is **IOWA_LWM2M_ID_ALL**.
    : *uriP->resInstanceId* is not **IOWA_LWM2M_ID_ALL** and **LWM2M_VERSION_1_1_SUPPORT** is not set.
    : *attributesStr* is nil or an empty string.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Write-Attributes was successful, the Client will return a **IOWA_COAP_204_CHANGED** status code.

The *responseCb* will be called with the operation set to **IOWA_DM_WRITE_ATTRIBUTES**.

LwM2M defines the following attributes:

| Name | Level | Description |
| ---- | --------------- | ------------------------------------------------ |
| pmin | Object, Object Instance, Resource | The minimum period in seconds to wait between notifications. |
| pmax | Object, Object Instance, Resource | The maximum period in seconds to wait between notifications. |
| gt | Numerical Resource | An upper threshold. A notification is sent when the resource value crosses this threshold. |
| lt | Numerical Resource | An lower threshold. A notification is sent when the resource value crosses this threshold. |
| st | Numerical Resource | A difference minimum in a resource value for a notification to be sent. |
| epmin | Object, Object Instance, Resource | The minimum sample time in seconds for the observed sensor in LwM2M 1.1 or later. |
| epmax | Object, Object Instance, Resource | The maximum sample time in seconds for the observed sensor in LwM2M 1.1 or later. |

Setting an attribute is in the form `Name "=" value` with some constraints:

* `lt` value < `gt` value
* `lt` value + 2 * `st` value < `gt` value
* If `pmax` < `pmin`, `pmax` is ignored
* `epmax` > `epmin`

Clearing an attribute is in the form `Name`.

** *attributesStr* Examples **

Receiving a notification every minute at most even if the observed URI did not change: `"pmax=60"`.

Receiving only one notification per hour even if the observed URI changed several times per minute: `"pmin=3600"`.

Receiving exactly one notification every sixty seconds: `"pmin=59&pmax=60"`.

Receiving a notification when the resource value exceeds 95 or falls below 10, and when the resource value returns below 95 or above 10: `"lt=10&gt=95"`.

Clearing the previously set minimum period and setting a maximum period of five minutes: `"pmin&pmax=300"`.

\clearpage

### iowa_server_dm_exec

** Prototype **

```c
iowa_status_t iowa_server_dm_exec(iowa_context_t contextP,
                                  uint32_t clientID,
                                  uint16_t objectID,
                                  uint16_t instanceID,
                                  uint16_t resourceID,
                                  iowa_result_callback_t resultCb,
                                  void * resultUserData);
```

** Description **

`iowa_server_dm_exec()` performs an Execute operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientID*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectID*
: The ID of the Object.

*instanceID*
: The ID of the instance.

*resourceID*
: The ID of the resource.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *objectID*, *instanceID* or *resourceID* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *clientID* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Execute was successful, the Client will return an **IOWA_COAP_204_CHANGED** status code.

Per LwM2M specification, a Server can do an Execute only on an URI in the form /*object*/*instance*/*resource*. Thus *instanceID*
and *resourceID* cannot be set **IOWA_LWM2M_ID_ALL**.

The *resultCb* will be called with the operation set to **IOWA_DM_EXECUTE**.

\clearpage

### iowa_server_dm_create

** Prototype **

```c
iowa_status_t iowa_server_dm_create(iowa_context_t contextP,
                                    uint32_t clientId,
                                    uint16_t objectId,
                                    uint16_t instanceId,
                                    size_t dataCount,
                                    iowa_lwm2m_data_t *dataArrayP,
                                    iowa_result_callback_t resultCb,
                                    void *resultUserData);
```

** Description **

`iowa_server_dm_create()` performs a Create operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectId*, *instanceId*
: The Object Instance targeted by the operation.

*dataCount*, *dataArrayP*
: The data to write.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *objectId* or *instanceId* is **IOWA_LWM2M_ID_ALL**.
: - *dataCount* is zero or *dataArrayP* is nil.
: - *dataArrayP* contains several data with unknown type.

IOWA_COAP_404_NOT_FOUND
: *clientID* does not match a known client.

IOWA_COAP_406_NOT_ACCEPTABLE
: Either:
: - *dataArrayP* contains several data with defined timestamp.
: - *dataArrayP* contains several data of unsigned integer type which are negative.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Create was successful, the Client will return a **IOWA_COAP_201_CREATED** status code.

The IDs contained in the data must match *objectId* and *instanceId*.

The *resultCb* will be called with the operation set to **IOWA_DM_CREATE**.

\clearpage

### iowa_server_dm_delete

** Prototype **

```c
iowa_status_t iowa_server_dm_delete(iowa_context_t contextP,
                                    uint32_t clientID,
                                    uint16_t objectID,
                                    uint16_t instanceID,
                                    iowa_result_callback_t resultCb,
                                    void * resultUserData);
```

** Description **

`iowa_server_dm_delete()` performs an Delete operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientID*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectID*
: The ID of the Object.

*instanceID*
: The ID of the instance to delete.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *objectID* or *instanceID* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *clientID* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Delete was successful, the Client will return an **IOWA_COAP_202_DELETED** status code.

Per LwM2M specification, a Server can do a Delete only on an URI in the form /*object*/*instance*. Thus *objectID* and *instanceID* cannot be set **IOWA_LWM2M_ID_ALL**.

The *resultCb* will be called with the operation set to **IOWA_DM_DELETE**.

\clearpage

### iowa_server_dm_discover

** Prototype **

```c
iowa_status_t iowa_server_dm_discover(iowa_context_t contextP,
                                      uint32_t clientID,
                                      uint16_t objectID,
                                      uint16_t instanceID,
                                      uint16_t resourceID,
                                      iowa_result_callback_t resultCb,
                                      void * resultUserData);
```

** Description **

`iowa_server_dm_discover()` performs a Discover operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientID*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectID*
: The ID of the Object.

*instanceID*
: The ID of the instance to delete. This can be **IOWA_LWM2M_ID_ALL**.

*resourceID*
: The ID of the resource to observe. This can be **IOWA_LWM2M_ID_ALL**.

*resultCb*
: The callback called when the reply to this operation is known.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *resultCb* is nil.
: - *objectID* is **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *clientID* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: either:
: - a memory allocation failed.
: - [`iowa_system_gettime()`](AbstractionLayer.md#iowa_system_gettime) returned an error.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: Communication with the LwM2M Client failed.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Discover was successful, the Client will return an **IOWA_COAP_205_CONTENT** status code.

Per LwM2M specification, a Server can do a Discover on an URI in the forms /*object*, /*object*/*instance* or /*object*/*instance*/*resource*.
Thus if *instanceID* is set to **IOWA_LWM2M_ID_ALL**, *resourceID* must be set to **IOWA_LWM2M_ID_ALL**.

The *resultCb* will be called with the operation set to **IOWA_DM_DISCOVER**.

\clearpage

### iowa_server_set_response_format

** Prototype **

```c
iowa_status_t iowa_server_set_response_format(iowa_context_t contextP,
                                              uint32_t clientID,
                                              iowa_content_format_t multiResourcesFormat,
                                              iowa_content_format_t singleResourceFormat);
```

** Description **

`iowa_server_set_response_format()` sets the content format to use when requesting data from a Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientID*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*multiResourcesFormat*
: format to use when requesting several resources.

*singleResourceFormat*
: format to use when requesting a single resource.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *multiResourcesFormat* is set to a content format which does not support multiple resources encoding.

IOWA_COAP_404_NOT_FOUND
: *clientID* does not match a known client.

** Header File **

iowa_server.h

** Notes **

By default, IOWA uses LwM2M TLV for all data encodings. If the flag `LWM2M_VERSION_1_0_REMOVE` is used, IOWA uses CBOR for single resource and SenML CBOR for multiples resources.

If the Client does not support the requested content format, it will switch to another one.

\clearpage

### iowa_server_set_payload_format

** Prototype **

```c
iowa_status_t iowa_server_set_payload_format(iowa_context_t contextP,
                                             uint32_t clientID,
                                             iowa_content_format_t multiResourcesFormat,
                                             iowa_content_format_t singleResourceFormat);
```

** Description **

`iowa_server_set_payload_format()` sets the content format to use when sending data to a Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientID*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*multiResourcesFormat*
: format to use when sending several resources.

*singleResourceFormat*
: format to use when sending a single resource.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *multiResourcesFormat* is set to a content format which does not support multiple resources encoding.

IOWA_COAP_404_NOT_FOUND
: *clientID* does not match a known client.

** Header File **

iowa_server.h

** Notes **

By default, IOWA uses LwM2M TLV for all data encodings. If the flag `LWM2M_VERSION_1_0_REMOVE` is used, IOWA uses CBOR for single resource and SenML CBOR for multiples resources.

If the Client does not support the encoding format of the data provided, it will return a **IOWA_COAP_415_UNSUPPORTED_CONTENT_FORMAT** error code in the callback of the [`iowa_server_write`](ServerAPI.md#iowa_server_write) call.

\clearpage

### iowa_server_create_registration_update_trigger_message

** Prototype **

```c
size_t iowa_server_create_registration_update_trigger_message(uint16_t serverInstanceId,
                                                              uint8_t **bufferP);
```

** Description **

`iowa_server_create_registration_update_trigger_message()` creates a Registration Update Trigger message.

When receiving a Registration Update Trigger message, a LwM2M Client updates its registration to the targeted LwM2M Server. This mechanism is useful to "wake" up a LwM2M Client which is not longer reachable on the current transport.

** Arguments **

*serverInstanceId*
: The Instance ID of the targeted LwM2M Server in the LwM2M Client's [`Server Object`][Server Object].

*bufferP*
: Used to store the Registration Update Trigger message.

** Return Value **

The length of the buffer in bytes, or 0 in case of an error.

** Header File **

iowa_server.h

** Notes **

*bufferP* will be allocated by the [`iowa_server_create_registration_update_trigger_message()`](ServerAPI.md#iowa_server_create_registration_update_trigger_message) function using [`iowa_system_malloc()`](AbstractionLayer.md#iowa_system_malloc). It is the caller responsibility to free the buffer.

It is the caller responsibility to send the Registration Update Trigger message to the LwM2M Client, typically using SMS.

\clearpage

### iowa_server_close_client_connection

** Prototype **

```c
iowa_status_t iowa_server_close_client_connection(iowa_context_t contextP,
                                                  uint32_t clientId);
```

** Description **

`iowa_server_close_client_connection()` closes the current connection with a Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

** Notes **

Depending of the transport used and the connection encryption, the Client will be informed or not of the closing connection.

Some examples:

- UDP / Not secure: Client is **not** informed
- UDP / Secure: Client is informed
- TCP / Not secure: Client is informed
- TCP / Secure: Client is informed
