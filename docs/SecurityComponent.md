## Security Component

The functions explained below are defined inside the file *src/security/iowa_prv_security.h*.

```c
iowa_status_t securityInit(iowa_context_t contextP);

void securityClose(iowa_context_t contextP);

iowa_status_t securityStep(iowa_context_t contextP);

iowa_security_session_t securityClientNewSession(iowa_context_t contextP,
                                                 const char *uri,
                                                 iowa_security_mode_t securityMode);

iowa_security_session_t securityServerNewSession(iowa_context_t contextP,
                                                 iowa_connection_type_t type,
                                                 void *connP,
                                                 bool isSecure);

void securityDeleteSession(iowa_context_t contextP,
                           iowa_security_session_t securityS);

void securitySetEventCallback(iowa_context_t contextP,
                              iowa_security_session_t securityS,
                              security_event_callback_t eventCb,
                              void *userDataCb);

iowa_status_t securityConnect(iowa_context_t contextP,
                              iowa_security_session_t securityS);

void securityDisconnect(iowa_context_t contextP,
                        iowa_security_session_t securityS);

int securitySend(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);

int securityRecv(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);

bool securityGetIsSecure(iowa_context_t contextP,
                         iowa_security_session_t securityS);

size_t securityGetIdentity(iowa_context_t contextP,
                           iowa_security_session_t securityS,
                           uint8_t *buffer,
                           size_t length);

iowa_status_t securityAddKey(iowa_context_t contextP,
                             const char *uri,
                             iowa_security_data_t *securityDataP);

iowa_status_t securityRemoveKey(iowa_context_t contextP,
                                const char *uri);
```

\clearpage

### Data types

#### iowa_security_context_t

This type is used to store the context of the IOWA security stack engine. It is created by calling [`securityInit()`][securityInit] and destroyed by calling [`securityClose()`][securityClose].

#### iowa_security_session_t

This type is used to store the session of the IOWA security stack engine. Multiple sessions can exist at the same time. It is created by calling [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession] and destroyed by calling [`securityDeleteSession()`][securityDeleteSession].

\clearpage

### Functions

#### securityInit

#** Prototype **

```c
iowa_status_t securityInit(iowa_context_t contextP);
```

#** Description **

`securityInit()` initializes a security context.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityClose

#** Prototype **

```c
void securityClose(iowa_context_t contextP);
```

#** Description **

`securityClose()` closes a security context.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

None.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityStep

#** Prototype **

```c
iowa_status_t securityStep(iowa_context_t contextP);
```

#** Description **

`securityStep()` does a security step to handle the handshaking, the timeout, ... for all the security sessions.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_412_INTERNAL_SERVER_ERROR
: attempt to do handshake on a security session which doesn't need it.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: error during the handshake of a security session.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityClientNewSession

#** Prototype **

```c
iowa_security_session_t securityClientNewSession(iowa_context_t contextP,
                                                 const char *uri,
                                                 iowa_security_mode_t securityMode);
```

#** Description **

`securityClientNewSession()` initializes a security session for a client.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*uri*
: URI of the server, used as identity.

*securityMode*
: The security mode to use when connecting to this LwM2M Server.

#** Return Value **

An [`iowa_security_session_t`](Security.md#iowa_security_session_t) in case of success or NULL in case of error.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityServerNewSession

#** Prototype **

```c
iowa_security_session_t securityServerNewSession(iowa_context_t contextP,
                                                 iowa_connection_type_t type,
                                                 void *connP,
                                                 bool isSecure);
```

#** Description **

`securityServerNewSession()` initializes a security session for a server.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*type*
: The connection type of the incoming connection.

*connP*
: The connection as returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*isSecure*
: Inform if the incoming connection is secure.

** Return Value **

An [`iowa_security_session_t`](Security.md#iowa_security_session_t) in case of success or NULL in case of error.

** Header File **

iowa_prv_security.h

\clearpage

#### securityDeleteSession

#** Prototype **

```c
void securityDeleteSession(iowa_context_t contextP,
                           iowa_security_session_t securityS);
```

#** Description **

`securityDeleteSession()` closes a security session.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

#** Return Value **

None.

#** Header File **

iowa_prv_security.h

\clearpage

#### securitySetEventCallback

#** Prototype **

```c
void securitySetEventCallback(iowa_context_t contextP,
                              iowa_security_session_t securityS,
                              security_event_callback_t eventCb,
                              void *userDataCb);
```

#** Description **

`securitySetEventCallback()` sets the event callback for a security session.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

*eventCb*
: The event callback called when a new event occurs.

*userDataCb*
: The user data pass to the event callback when called.

#** Return Value **

None.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityConnect

#** Prototype **

```c
iowa_status_t securityConnect(iowa_context_t contextP,
                              iowa_security_session_t securityS);
```

#** Description **

`securityConnect()` connects to a new connection.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityDisconnect

#** Prototype **

```c
void securityDisconnect(iowa_context_t contextP,
                        iowa_security_session_t securityS);
```

#** Description **

`securityDisconnect()` disconnects from a connection.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

#** Return Value **

None.

#** Header File **

iowa_prv_security.h

\clearpage

#### securitySend

#** Prototype **

```c
int securitySend(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);
```

#** Description **

`securitySend()` sends a buffer on a connection. Must call the platform function [`iowa_system_connection_send`](AbstractionLayer.md#iowa_system_connection_send).

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

*buffer*
: The data to send.

*length*
: The length of the data in bytes.

#** Return Value **

The number of bytes sent or a negative number in case of error.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityRecv

#** Prototype **

```c
int securityRecv(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);
```

#** Description **

`securityRecv()` reads data from a connection. Must call the platform function [`iowa_system_connection_recv`](AbstractionLayer.md#iowa_system_connection_recv).

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

*buffer*
: A buffer to store the received data.

*length*
: The length of the buffer in bytes.

#** Return Value **

The number of received bytes or a negative number in case of error.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityGetIsSecure

#** Prototype **

```c
bool securityGetIsSecure(iowa_context_t contextP,
                         iowa_security_session_t securityS);
```

#** Description **

`securityGetIsSecure()` returns if a security session is encrypted.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

#** Return Value **

'true' if encrypted, 'false' otherwise.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityGetIdentity

#** Prototype **

```c
size_t securityGetIdentity(iowa_context_t contextP,
                           iowa_security_session_t securityS,
                           uint8_t *buffer,
                           size_t length);
```

#** Description **

`securityGetIdentity()` retrieves the identity of a secured peer. Must call the platform function [`iowa_system_connection_get_peer_identifier`](AbstractionLayer.md#iowa_system_connection_get_peer_identifier).

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*securityS*
: An [`iowa_security_session_t`](Security.md#iowa_security_session_t) as returned by [`securityClientNewSession()`][securityClientNewSession] or [`securityServerNewSession()`][securityServerNewSession]. Should not be checked at runtime.

*buffer*
: A buffer to store the identity.

*length*
: The length of the buffer in bytes.

#** Return Value **

The length in bytes of the identity.

#** Header File **

iowa_prv_security.h

\clearpage

#### securityAddKey

#** Prototype **

```c
iowa_status_t securityAddKey(iowa_context_t contextP,
                             const char *uri,
                             iowa_security_data_t *securityDataP);
```

#** Description **

`securityAddKey()` stores a key.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*uri*
: The URI of the peer the key is associated to.

*securityDataP*
: Data containing the key.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

#** Header File **

iowa_prv_security.h

#** Notes **

1. If IOWA is built with **IOWA_SECURITY_LAYER** different from **IOWA_SECURITY_LAYER_NONE**, [`securityAddKey`][securityAddKey] will call the platform function [`iowa_system_security_data`](AbstractionLayer.md#iowa_system_security_data).
But there is no prerequisite to call this platform function if IOWA is built with **IOWA_SECURITY_LAYER** equals to **IOWA_SECURITY_LAYER_USER**.

2. If [`iowa_system_security_data`](AbstractionLayer.md#iowa_system_security_data) has to be called, the argument *securityOp* must be set to IOWA_SEC_CREATE.

\clearpage

#### securityRemoveKey

#** Prototype **

```c
iowa_status_t securityRemoveKey(iowa_context_t contextP,
                                const char *uri);
```

#** Description **

`securityRemoveKey()` removes the keys.

#** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*uri*
: The URI of the peer the keys are associated to.

#** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: the associated keys have not been found.

#** Header File **

iowa_prv_security.h

#** Notes **

1. If IOWA is built with **IOWA_SECURITY_LAYER** different from **IOWA_SECURITY_LAYER_NONE**, [`securityRemoveKey`][securityRemoveKey] will call the platform function [`iowa_system_security_data`](AbstractionLayer.md#iowa_system_security_data).
But there is no prerequisite to call this platform function if IOWA is built with **IOWA_SECURITY_LAYER** equals to **IOWA_SECURITY_LAYER_USER**.

2. If [`iowa_system_security_data`](AbstractionLayer.md#iowa_system_security_data) has to be called, the argument *securityOp* must be set to IOWA_SEC_DELETE.