# Bootstrap Server Mode API Reference

The functions explained below are defined inside the file *include/iowa_server.h*.

## Bootstrap Server pseudo code

```c
#include "iowa_server.h"

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    int bootstrapServerSocket;

    /******************
    * Initialization
    */

    bootstrapServerSocket = open_udp_socket();

    iowaH = iowa_init(NULL);

    result = iowa_bootstrap_server_configure(iowaH, client_monitor, NULL);

    /******************
    * "Main loop"
    */

    while (result == IOWA_COAP_NO_ERROR)
    {
        result = iowa_step(iowaH, 10);

        if (isDataAvailable(bootstrapServerSocket))
        {
            void * newConnection;

            newConnection = create_new_connection();
            result = iowa_server_new_incoming_connection(iowaH,
                                                         IOWA_CONN_DATAGRAM,
                                                         newConnection);
        }
    }

    iowa_close(iowaH);
    close(bootstrapServerSocket);

    return 0;
}
```

## Callbacks

### iowa_bootstrap_result_callback_t

The bootstrap APIs ([`iowa_bootstrap_server_write()`](BootstrapServerAPI.md#iowa_bootstrap_server_write), [`iowa_bootstrap_server_delete()`](BootstrapServerAPI.md#iowa_bootstrap_server_delete), [`iowa_bootstrap_server_finish()`](BootstrapServerAPI.md#iowa_bootstrap_server_finish)) and [`iowa_bootstrap_server_add_server()`](BootstrapServerAPI.md#iowa_bootstrap_server_add_server)) are using an `iowa_bootstrap_result_callback_t` to asynchronously return the result
of the operation.

```c
typedef void(*iowa_bootstrap_result_callback_t) (iowa_bootstrap_operation_t operation,
                                                 uint16_t clientId,
                                                 uint16_t objectId, uint16_t instanceId, uint16_t resourceId,
                                                 iowa_status_t status,
                                                 size_t length, uint8_t *buffer,
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
: The status of the command.

*length*
: The length of the payload when the *operation* is **IOWA_BOOTSTRAP_DISCOVER**.

*buffer*
: The payload containing the CoRE Link information when the *operation* is **IOWA_BOOTSTRAP_DISCOVER**.

*resultUserData*
: A pointer to application specific data. This is a parameter of the matching `iowa_bootstrap_server...()` API.

*contextP*
: The IOWA context on which the bootstrap API was called.

\clearpage

## API

### iowa_bootstrap_server_configure

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_configure(iowa_context_t contextP,
                                              iowa_monitor_callback_t monitorCb,
                                              void *callbackUserData);
```

** Description **

`iowa_bootstrap_server_configure()` sets the monitoring callback called when LwM2M Clients connect to the LwM2M Bootstrap Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*monitorCb*
: The callback called when Clients update their status. This can be nil.

*callbackUserData*
: A pointer to application specific data. This is passed as argument to *monitorCb* and *resTypeCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

** Header File **

iowa_server.h

\clearpage

### iowa_bootstrap_server_new_incoming_connection

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_new_incoming_connection(iowa_context_t contextP,
                                                            iowa_connection_type_t type,
                                                            void * connP,
                                                            bool isSecure);
```

** Description **

`iowa_bootstrap_server_new_incoming_connection()` informs the stack of a new incoming connection.

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
: - *type* is not supported.

** Header File **

iowa_server.h

\clearpage

### iowa_bootstrap_server_read

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_read(iowa_context_t contextP,
                                         uint32_t clientId,
                                         iowa_lwm2m_uri_t *uriP,
                                         iowa_response_callback_t responseCb,
                                         void *userDataP);
```

** Description **

`iowa_bootstrap_server_read()` performs a Bootstrap Read operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*uriP*
: The URI targeted by the operation.

*responseCb*
: The callback called when the reply to this operation is known.

*userDataP*
: A pointer to application specific data. This is passed as argument to *responseCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *uriP* is nil.
: - *responseCb* is nil.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_406_NOT_ACCEPTABLE
: either:
: - *uriP* cannot target Root, Resource or Resource Instance level
: - *uriP->objectId* must target the [Server Object][Server Object] or the [Access Control List Object][Access Control List Object].

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

This API requires the compilation flag [**LWM2M_VERSION_1_1_SUPPORT**][LWM2M_VERSION_1_1_SUPPORT].

Per LwM2M specification, if the Bootstrap Read was successful, the Client will return a **IOWA_COAP_205_CONTENT** status code.

The *responseCb* will be called with the operation set to **IOWA_BOOTSTRAP_READ**.

\clearpage

### iowa_bootstrap_server_write

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_write(iowa_context_t contextP,
                                          uint32_t clientId,
                                          uint16_t objectId,
                                          uint16_t instanceId,
                                          size_t dataCount,
                                          iowa_lwm2m_data_t *dataArray,
                                          iowa_bootstrap_result_callback_t resultCb,
                                          void *resultUserData);
```

** Description **

`iowa_bootstrap_server_write()` performs a Bootstrap Write operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectId*
: The Object targeted by the operation.

*instanceId*
: The Instance object targeted by the operation.

*dataCount*
: The number of data to write.

*dataArray*
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
: - *dataCount* is zero or *dataArray* is nil.

IOWA_COAP_404_NOT_FOUND
: either:
: - *clientId* does not match a known client.
: - *clientId* is **IOWA_LWM2M_ID_ALL**

IOWA_COAP_406_NOT_ACCEPTABLE
: Either:
: - *dataArrayP* contains several data with defined timestamp.
: - *dataArrayP* contains several data with unsigned integer which are negative.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Bootstrap Write was successful, the Client will return a **IOWA_COAP_204_CHANGED** status code.

The IDs contained in the data must match *objectId* and *instanceId*.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_WRITE**.

\clearpage

### iowa_bootstrap_server_delete

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_delete(iowa_context_t contextP,
                                           uint32_t clientId,
                                           uint16_t objectId,
                                           uint16_t instanceId,
                                           iowa_bootstrap_result_callback_t resultCb,
                                           void *resultUserData);
```

** Description **

`iowa_bootstrap_server_delete()` performs a Bootstrap Delete operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectId*
: The ID of the Object to delete. This can be **IOWA_LWM2M_ID_ALL**.

*instanceId*
: The ID of the instance to delete. This can be **IOWA_LWM2M_ID_ALL**.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: either:
: - *clientId* does not match a known client.
: - *clientId* is **IOWA_LWM2M_ID_ALL**

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Bootstrap Delete was successful, the Client will return an **IOWA_COAP_204_CHANGED** status code.

Per LwM2M specification, the Bootstrap Server can request the Client to delete all Objects and Object Instances (except for the Bootstrap Server account) in a single operation by setting *objectId* and *instanceId* to **IOWA_LWM2M_ID_ALL**.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_DELETE**.

\clearpage

### iowa_bootstrap_server_discover

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_discover(iowa_context_t contextP,
                                             uint32_t clientId,
                                             uint16_t objectId,
                                             iowa_bootstrap_result_callback_t resultCb,
                                             void * resultUserData);
```

** Description **

`iowa_bootstrap_server_discover()` performs a Bootstrap Discover operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*objectId*
: The ID of the Object targeted by the operation. This can be **IOWA_LWM2M_ID_ALL**.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't received all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, the Bootstrap Discover operation only returns the list of Objects and Object Instances with some attributes: LwM2M Enabler version ("lwm2m="), Short Server ID ("ssid="), and LwM2M Server URI ("uri=").

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_DISCOVER**.

\clearpage

### iowa_bootstrap_server_finish

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_finish(iowa_context_t contextP,
                                           uint32_t clientId,
                                           iowa_bootstrap_result_callback_t resultCb,
                                           void *resultUserData);
```

** Description **

`iowa_bootstrap_server_finish()` performs a Bootstrap Server operation on a Client’s URI.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: either:
: - *clientId* does not match a known client.
: - *clientId* is **IOWA_LWM2M_ID_ALL**

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

Per LwM2M specification, if the Bootstrap Finish was successful, the Client will return an **IOWA_COAP_204_CHANGED** status code. Otherwise the Client will return an **IOWA_COAP_406_NOT_ACCEPTABLE** status code.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_FINISH**.

\clearpage

### iowa_bootstrap_server_add_server

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_add_server(iowa_context_t contextP,
                                               uint32_t clientId,
                                               uint16_t shortServerId,
                                               const char *uri,
                                               uint32_t lifetime,
                                               iowa_security_data_t *securityDataP,
                                               iowa_bootstrap_result_callback_t resultCb,
                                               void *resultUserData);
```

** Description **

`iowa_bootstrap_server_add_server()` adds the proper Security and Server object to the client to configure a Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*shortServerId*
: The short ID of the Server.

*uri*
: The URI of the Server.

*lifetime*
: The lifetime in seconds of the registration.

*securityDataP*
: The security data to use to connect properly to the Server. This can be nil.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: either:
: - *uri* is nil.
: - *shortServerId* is 0 or **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

`iowa_bootstrap_server_add_server()` calls internally [`iowa_bootstrap_server_discover`](BootstrapServerAPI.md#iowa_bootstrap_server_discover) to retrieve the [`Security Object`][Security Object] Instances and [`Server Object`][Server Object] Instances, then calls[`iowa_bootstrap_server_write`](BootstrapServerAPI.md#iowa_bootstrap_server_write) two times to write a new [`Security Object`][Security Object] Instance and a new [`Server Object`][Server Object] Instance.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_ADD_SERVER**.

Per LwM2M specification, if adding the server was successful, the Client will return an **IOWA_COAP_204_CHANGED** status code.

Currently, it is not possible to configure both OSCORE and another transport security through this API. If needed, use [`iowa_bootstrap_server_write()`](BootstrapServerAPI.md#iowa_bootstrap_server_write).

\clearpage

### iowa_bootstrap_server_remove_server

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_remove_server(iowa_context_t contextP,
                                                  uint32_t clientId,
                                                  uint16_t shortServerId,
                                                  iowa_bootstrap_result_callback_t resultCb,
                                                  void *resultUserData);
```

** Description **

`iowa_bootstrap_server_remove_server()` removes the proper Security and [`Server Object`][Server Object] Instance associated to the Short Server ID from a client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*shortServerId*
: The short ID of the Server.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_400_BAD_REQUEST
: *shortServerId* is 0 or **IOWA_LWM2M_ID_ALL**.

IOWA_COAP_404_NOT_FOUND
: either:
: - *clientId* does not match a known client.
: - *clientId* is **IOWA_LWM2M_ID_ALL**

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

`iowa_bootstrap_server_remove_server()` calls internally [`iowa_bootstrap_server_discover`](BootstrapServerAPI.md#iowa_bootstrap_server_discover) to retrieve the [`Security Object`][Security Object] Instances and [`Server Object`][Server Object] Instances, then calls [`iowa_bootstrap_server_delete`](BootstrapServerAPI.md#iowa_bootstrap_server_delete) two times to delete a [`Security Object`][Security Object] Instance and a [`Server Object`][Server Object] Instance.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_REMOVE_SERVER**.

Per LwM2M specification:

- if removing the server was successful, the Client will return an **IOWA_COAP_202_DELETED** status code.

If the [`Security Object`][Security Object] Instance and/or the [`Server Object`][Server Object] Instance associated to the LwM2M Server have not been found after the Discover operation, the result callback will be called with the following parameters:

- objectId: **IOWA_LWM2M_SECURITY_OBJECT_ID** or **IOWA_LWM2M_SERVER_OBJECT_ID**
- instanceId: **IOWA_LWM2M_ID_ALL**
- resourceId: **IOWA_LWM2M_ID_ALL**
- status: **IOWA_COAP_404_NOT_FOUND**

\clearpage

### iowa_bootstrap_server_add_bootstrap_server

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_add_bootstrap_server(
    iowa_context_t contextP,
    uint32_t clientId,
    const char *uri,
    int32_t clientHoldOff,
    uint32_t bootstrapAccountTimeout,
    iowa_security_data_t *securityDataP,
    iowa_bootstrap_result_callback_t resultCb,
    void *resultUserData
);
```

** Description **

`iowa_bootstrap_server_add_bootstrap_server()` adds the proper Security object to the client to configure a Bootstrap Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*uri*
: The URI of the Server.

*clientHoldOff*
: The number of seconds to wait before initiating a Client Initiated Bootstrap.

*bootstrapAccountTimeout*
: Time to wait by the client before to purge the LwM2M Bootstrap-Server Account.

*securityDataP*
: The security data to use to connect properly to the Server.

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

IOWA_COAP_400_BAD_REQUEST
: *uri* is nil.

IOWA_COAP_404_NOT_FOUND
: *clientId* does not match a known client.

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [iowa_system_connection_send()](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

`iowa_bootstrap_server_add_bootstrap_server()` calls internally [`iowa_bootstrap_server_discover`](BootstrapServerAPI.md#iowa_bootstrap_server_discover) to retrieve the [`Security Object`][Security Object] Instances, then [`iowa_bootstrap_server_write`](BootstrapServerAPI.md#iowa_bootstrap_server_write) to write a [`Security Object`][Security Object] Instance.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_ADD_BOOTSTRAP_SERVER**.

Per LwM2M specification, if adding the bootstrap server was successful, the Client will return an **IOWA_COAP_204_CHANGED** status code.

Currently, it is not possible to configure both OSCORE and another transport security through this API. If needed, use [`iowa_bootstrap_server_write()`](BootstrapServerAPI.md#iowa_bootstrap_server_write).

\clearpage

### iowa_bootstrap_server_remove_bootstrap_server

** Prototype **

```c
iowa_status_t iowa_bootstrap_server_remove_bootstrap_server(
    iowa_context_t contextP,
    uint32_t clientId,
    iowa_bootstrap_result_callback_t resultCb,
    void *resultUserData
);
```

** Description **

`iowa_bootstrap_server_remove_bootstrap_server()` removes the proper [`Security Object`][Security Object] Instance associated to the Bootstrap Server from a client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID of the client as reported in the [`iowa_monitor_callback_t`](ServerAPI.md#iowa_monitor_callback_t).

*resultCb*
: The callback called when the reply to this operation is known. This can be nil.

*resultUserData*
: A pointer to application specific data. This is passed as argument to *resultCb*. This can be nil.

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: either:
: - *clientId* does not match a known client.
: - *clientId* is **IOWA_LWM2M_ID_ALL**

IOWA_COAP_413_REQUEST_ENTITY_TOO_LARGE
: The Platform abstraction didn't send all the data. One possible assumption is the packet was too large for the transport.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation or a call to [iowa_system_gettime()](AbstractionLayer.md#iowa_system_gettime) failed.

IOWA_COAP_503_SERVICE_UNAVAILABLE
: [`iowa_system_connection_send()`](AbstractionLayer.md#iowa_system_connection_send) returned an error.

** Header File **

iowa_server.h

** Notes **

`iowa_bootstrap_server_remove_bootstrap_server()` calls internally [`iowa_bootstrap_server_discover`](BootstrapServerAPI.md#iowa_bootstrap_server_discover) to retrieve the [`Security Object`][Security Object] Instances, then calls [`iowa_bootstrap_server_delete`](BootstrapServerAPI.md#iowa_bootstrap_server_delete) to delete a [`Security Object`][Security Object] Instance.

The *resultCb* will be called with the operation set to **IOWA_BOOTSTRAP_REMOVE_BOOTSTRAP_SERVER**.

Per LwM2M specification:

- if the removing the bootstrap server was successful, the Client will return an **IOWA_COAP_202_DELETED** status code.

If the [`Security Object`][Security Object] Instance associated to the LwM2M Bootstrap Server have not been found after the Discover operation, the result callback will be called with the following parameters:

- objectId: **IOWA_LWM2M_SECURITY_OBJECT_ID**
- instanceId: **IOWA_LWM2M_ID_ALL**
- resourceId: **IOWA_LWM2M_ID_ALL**
- status: **IOWA_COAP_404_NOT_FOUND**