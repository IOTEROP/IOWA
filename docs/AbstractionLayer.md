# System Abstraction Layer

The functions explained below are defined inside the file *include/iowa_platform.h*.

## Presentation

To port IOWA to your platform, you have to implement the following functions.

```c
void * iowa_system_malloc(size_t size);

void iowa_system_free(void * pointer);

int32_t iowa_system_gettime(void);

int iowa_system_connection_send(void * connP,
                                uint8_t * buffer,
                                size_t length,
                                void * userData);

int iowa_system_connection_recv(void * connP,
                                uint8_t * buffer,
                                size_t length,
                                void * userData);

int iowa_system_connection_select(void ** connArray,
                                  size_t connCount,
                                  int32_t timeout,
                                  void * userData);

void iowa_system_connection_close(void * connP,
                                  void * userData);

```

If you are implementing a LwM2M Client, you also have to implement these functions:

```c
void iowa_system_reboot(void *userData);

void * iowa_system_connection_open(iowa_connection_type_t type,
                                   char * hostname,
                                   char * port,
                                   void * userData);

```

These other functions are optional to implement. See Below.

```c
void iowa_system_trace(const char * format,
                       va_list varArgs);

void * iowa_system_queue_create(void * userData);

void iowa_system_queue_delete(void * queueP,
                              void * userData);

int iowa_system_queue_enqueue(void * queueP,
                              uint8_t * buffer,
                              size_t length,
                              void * userData);

size_t iowa_system_queue_dequeue(void * queueP,
                                 uint8_t * buffer,
                                 size_t length,
                                 void * userData);

size_t iowa_system_queue_peek(void * queueP,
                              uint8_t * buffer,
                              size_t length,
                              void * userData);

void iowa_system_queue_remove(void * queueP,
                              void * userData);

size_t iowa_system_queue_backup(void *queueP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData);

void *iowa_system_queue_restore(uint8_t *buffer,
                                size_t length,
                                void *userData);

size_t iowa_system_store_context(uint8_t *bufferP,
                                 size_t length,
                                 void *userData);

size_t iowa_system_retrieve_context(uint8_t **bufferP,
                                    void *userData);

void iowa_system_connection_interrupt_select(void * userData);

void iowa_system_mutex_lock(void * userData);

void iowa_system_mutex_unlock(void * userData);

size_t iowa_system_connection_get_peer_identifier(void * connP,
                                                  uint8_t *addrP,
                                                  size_t length,
                                                  void * userData);

int iowa_system_random_vector_generator(uint8_t *randomBuffer,
                                        size_t size,
                                        void *userData);

iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP);
```

\clearpage

## Data types

### iowa_connection_type_t

```c
typedef enum
{
    IOWA_CONN_UNDEFINED = 0,
    IOWA_CONN_DATAGRAM,
    IOWA_CONN_STREAM,
    IOWA_CONN_LORAWAN,
    IOWA_CONN_SMS
} iowa_connection_type_t;
```

This is an enumeration of the following values:

IOWA_CONN_UNDEFINED
: Connection type is unknown.

IOWA_CONN_DATAGRAM
: UDP connection.

IOWA_CONN_STREAM
: TCP connection.

IOWA_CONN_LORAWAN
: LoRaWAN transport.

IOWA_CONN_SMS
: SMS transport.

### iowa_security_operation_t

```c
typedef enum
{
    IOWA_SEC_READ,
    IOWA_SEC_FREE,
    IOWA_SEC_CREATE,
    IOWA_SEC_DELETE,
    IOWA_SEC_CHECK
} iowa_security_operation_t;
```

The `iowa_security_operation_t` enumeration is used to tell which security operation is requested when the function `iowa_system_security_data` is called.

IOWA_SEC_READ
: Read security keys from the Application.

IOWA_SEC_FREE
: Free the data allocated if any in the security structure. Always call after **IOWA_SEC_READ**.

IOWA_SEC_CREATE
: Add security keys to the Application.

IOWA_SEC_DELETE
: Remove security keys from the Application.

IOWA_SEC_CHECK
: Check if the security keys are present on the Application.

### iowa_psk_data_t

```c
typedef struct
{
    uint8_t *identity;
    size_t   identityLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} iowa_psk_data_t;
```

The `iowa_psk_data_t` structure is used to store the identity / private key pair information.

identity
: Identity associated with the private key.

identityLen
: Length of the identity.

privateKey
: Private key associated with the identity.

privateKeyLen
: Length of the private key.

### iowa_certificate_data_t

```c
typedef struct
{
    uint8_t *caCertificate;
    size_t   caCertificateLen;
    uint8_t *certificate;
    size_t   certificateLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} iowa_certificate_data_t;
```

The `iowa_certificate_data_t` structure is used to store information related to a certificate.

caCertificate
: Certificate authority used to generate the certificate. Must respect the DER (Distinguished Encoding Rules) format.

caCertificateLen
: Length of the certificate authority.

certificate
: Certificate. Must respect the DER (Distinguished Encoding Rules) format.

certificateLen
: Length of the certificate.

privateKey
: Private key used to generate the certificate. Must respect the DER (Distinguished Encoding Rules) format.

privateKeyLen
: Length of the private key.

### iowa_rpk_data_t

```c
typedef struct
{
    uint8_t *publicKeyX;
    size_t   publicKeyXLen;
    uint8_t *publicKeyY;
    size_t   publicKeyYLen;
    uint8_t *privateKey;
    size_t   privateKeyLen;
} iowa_rpk_data_t;
```

The `iowa_rpk_data_t` structure is used to store information related to raw public key information. Supported public keys must use the fixed curve *secp256r1*.

publicKeyX
: Public key X coordinate.

publicKeyXLen
: Length of the public key X coordinate.

publicKeyY
: Public key Y coordinate.

publicKeyYLen
: Length of the public key Y coordinate.

privateKey
: Private key used to generate the public key.

privateKeyLen
: Length of the private key.

### iowa_oscore_data_t

```c
typedef struct
{
    uint8_t *senderId;
    size_t   senderIdLen;
    uint8_t *recipientId;
    size_t   recipientIdLen;
    uint8_t *masterSecret;
    size_t   masterSecretLen;
} iowa_oscore_data_t;
```

The `iowa_oscore_data_t` structure is used to store information related to OSCORE key information.

senderId
: ID to use to protect sent CoAP messages.

senderIdLen
: Length of the Sender ID.

recipientId
: ID to use to verify received CoAP messages..

recipientIdLen
: Length of the Recipient ID.

masterSecret
: Private key associated to the Sender ID and Recipient ID.

masterSecretLen
: Length of the Master Secret.

### iowa_security_data_t

```c
typedef struct
{
    iowa_security_mode_t securityMode;
    union
    {
        iowa_psk_data_t         pskData;
        iowa_certificate_data_t certData;
        iowa_rpk_data_t         rpkData;
        iowa_oscore_data_t      oscoreData;
    } protocol;
} iowa_security_data_t;
```

The `iowa_security_data_t` structure is used to create, delete, read and free security data.

securityMode
: Security mode to determine the data in the union structure.

protocol.pskData
: Pre-shared key data.

protocol.certData
: Certificate data.

protocol.rpkData
: Raw public key data.

protocol.oscoreData
: Object Security for CORE key data.

\clearpage

## API

### iowa_system_malloc

** Prototype **

```c
void * iowa_system_malloc(size_t size);
```

** Description **

`iowa_system_malloc()` could map directly to C standard library `malloc()`. It allocates a memory block.

** Arguments **

*size*
: The size in bytes of the requested memory block.

** Return Value **

A pointer to the allocated memory or NULL in case of error.

** Header File **

iowa_platform.h

** Note **

This function is required by IOWA.

\clearpage

### iowa_system_free

** Prototype **

```c
void iowa_system_free(void * pointer);
```

** Description **

`iowa_system_free()` could map directly to C standard library `free()`. It releases a memory block previously allocated by [`iowa_system_malloc()`](AbstractionLayer.md#iowa_system_malloc).

** Arguments **

*pointer*
: A pointer to the memory block to release.

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required by IOWA.

From C standard, no action should occur if *pointer* argument is a null pointer. This is how the function `free()` from C standard library behaves. However some compilers or implementations do not respect this standard. So it can be necessary when implementing `iowa_system_free()` to check if the argument *pointer* is nil.

\clearpage

### iowa_system_gettime

** Prototype **

```c
int32_t iowa_system_gettime(void);
```

** Description **

`iowa_system_gettime()` is used by IOWA to determine the time elapsed.

** Return Value **

The number of seconds elapsed since a point of origin or a negative number in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required by IOWA.

If you are using the GPS or the Location object, this function will be used to timestamp the measure. In this case, the point of origin must be Epoch.

Else, the point of origin (Epoch, system boot, etc...) does not matter as this function is used only to determine the elapsed time between consecutive calls.

There is no safeguard if `iowa_system_gettime()` returns a value inferior to the one returned in a previous call.

\clearpage

### iowa_system_reboot

** Prototype **

```c
void iowa_system_reboot(void *userData);
```

** Description **

`iowa_system_reboot()` starts a system reboot.

** Arguments **

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required only for LwM2M Clients.

This feature is required by the Lightweight M2M protocol. However, a LwM2M device can be functional without it and this function can be a stub.

\clearpage

### iowa_system_trace

** Prototype **

```c
void iowa_system_trace(const char * format,
                       va_list varArgs);
```

** Description **

`iowa_system_trace()` outputs the logs when the stack is built with **IOWA_WITH_LOGS**.

It can be mapped directly to `vprintf()`.

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required by IOWA only when logs are activated. See [IOWA_LOG_LEVEL and IOWA_LOG_PART][IOWA_LOG_LEVEL and IOWA_LOG_PART].

IOWA takes care of freeing *varArgs* after this call returns.

To print a single line on the output, this function can be called multiple times. This has an impact when the flag **IOWA_THREAD_SUPPORT** is enabled. Several calls of [`iowa_system_trace()`](AbstractionLayer.md#iowa_system_trace) can occur from different threads at the same time. And thus, the output can be ruined. The implementation of this function must be thread safe.

\clearpage

### iowa_system_connection_open

** Prototype **

```c
void * iowa_system_connection_open(iowa_connection_type_t type,
                                   char * hostname,
                                   char * port,
                                   void * userData);
```

** Description **

`iowa_system_connection_open()` opens a connection to a host.

** Arguments **

*type*
: The type of connection to open.

*hostname*
: The hostname of the peer to connect to.

*port*
: The port to connect to. It may be NULL depending on the provided server URL.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

A pointer to an user-defined type or NULL in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only for LwM2M Clients.

When IOWA is used as a LwM2M Client, it calls this function to connect to the LwM2M Servers. See the [`iowa_client_add_server()`](ClientAPI.md#iowa_client_add_server) API.

\clearpage

### iowa_system_connection_send

** Prototype **

```c
int iowa_system_connection_send(void * connP,
                                uint8_t * buffer,
                                size_t length,
                                void * userData);
```

** Description **

`iowa_system_connection_send()` sends a buffer on a connection.

** Arguments **

*connP*
: The connection as returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*buffer*
: The data to send.

*length*
: The length of the data in bytes.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The number of bytes sent or a negative number in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required by IOWA.

On packet switched networks (eg. UDP), if *length* is bigger than the MTU, it is advised to not try to send the buffer and return the MTU.

\clearpage

### iowa_system_connection_get_peer_identifier

** Prototype **

```c
size_t iowa_system_connection_get_peer_identifier(void * connP,
                                                  uint8_t * addrP,
                                                  size_t length,
                                                  void * userData);
```

** Description **

`iowa_system_connection_get_peer_identifier()` returns an unique identifier for the peer of a connection (e.g. IP address, LoRaWAN DevEUI, SMS MSISDN).

** Arguments **

*connP*
: The connection as returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*addrP*
: A pre-allocated buffer to store the identifier.

*length*
: The length of *addrP* in bytes.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The number of bytes of the identifier or 0 in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with [IOWA_SECURITY_LAYER][IOWA_SECURITY_LAYER] different from **IOWA_SECURITY_LAYER_NONE**, or with **LWM2M_SERVER_MODE** or **LWM2M_BOOTSTRAP_SERVER_MODE** flags.

This is used when the endpoint name has not been found in the registration payload.

\clearpage

### iowa_system_connection_recv

** Prototype **

```c
int iowa_system_connection_recv(void * connP,
                                uint8_t * buffer,
                                size_t length,
                                void * userData);
```

** Description **

`iowa_system_connection_recv()` reads data from a connection in a non-blocking way.

** Arguments **

*connP*
 : The connection as returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*buffer*
: A buffer to store the received data.

*length*
: The length of the buffer in bytes.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The number of received bytes or a negative number in case of error.

** Header File **

iowa_platform.h

** Note **

This function is required by IOWA.

\clearpage

### iowa_system_connection_select

** Prototype **

```c
int iowa_system_connection_select(void ** connArray,
                                  size_t connCount,
                                  int32_t timeout,
                                  void * userData);
```

** Description **

`iowa_system_connection_select()` monitors a list of connections for incoming data during the specified time.

** Arguments **

*connArray*
: An array of connections as returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*connCount*
: The number of elements of connArray. This may be zero.

*timeout*
: The time to wait for data in seconds. This may be zero.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

: either:
: - a positive number if data are available.
: - zero if the time elapsed.
: - a negative number in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required by IOWA.

If data are available on one or more connections, `iowa_system_connection_select()` must modify the *connArray* elements :

- If data are available on a connection the matching element in *connArray* is left untouched.
- If no data are available, the matching element is set to NULL.
  If the timeout is reached (or in case of error), there is no need to modify the *connArray* elements.

When the application needs to be very responsive, this function is a good place to monitor application specific events without using a very short timeout in [`iowa_step()`](CommonAPI.md#iowa_step). For instance, the sample server waits for keyboard events here.

It is possible that IOWA calls `iowa_system_connection_select()` with a timeout of zero. In this case, `iowa_system_connection_select()` must not return an error, and should check if some data are already available on one of the connections.

It is also possible that IOWA calls `iowa_system_connection_select()` with no connections. In this case, `iowa_system_connection_select()` must not return an error, and should return after the timeout, which may also be zero, expires. This can occur when opening a connection to a LwM2M Server failed and IOWA is configured to wait a specific time before retrying to connect to the LwM2M Server.

\clearpage

### iowa_system_connection_interrupt_select

** Prototype **

```c
void iowa_system_connection_interrupt_select(void * userData);
```

** Description **

A call to `iowa_system_connection_interrupt_select()` makes [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select) return immediately if it is currently running.

** Arguments **

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **IOWA_MULTITHREAD_SUPPORT** flag.

The value returned by [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select) is zero. Calling `iowa_system_connection_interrupt_select()` is considered as a preemptive timeout for [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select).

\clearpage

### iowa_system_connection_close

** Prototype **

```c
void iowa_system_connection_close(void * connP,
                                  void * userData);
```

** Description **

`iowa_system_connection_close()` closes a connection.

** Arguments **

*connP*
: The connection as returned by [`iowa_system_connection_open()`](AbstractionLayer.md#iowa_system_connection_open).

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Note **

This function is required by IOWA.

\clearpage

### iowa_system_queue_create

** Prototype **

```c
void * iowa_system_queue_create(void * userData);
```

** Description **

`iowa_system_queue_create()` creates a storage queue to offload data from the memory.

** Arguments **

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

A pointer to an user-defined type or NULL in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **LWM2M_STORAGE_QUEUE_SUPPORT** or **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flags.

The storage queue must store data as separate entities and not as as a stream. Ideally, this is a FIFO.

\clearpage

### iowa_system_queue_delete

** Prototype **

```c
void iowa_system_queue_delete(void * queueP,
                             void * userData);
```

** Description **

`iowa_system_queue_delete()` closes a storage queue.

** Arguments **

*queueP*
: A storage queue as returned by [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create).

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **LWM2M_STORAGE_QUEUE_SUPPORT** or **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flags.

\clearpage

### iowa_system_queue_enqueue

** Prototype **

```c
int iowa_system_queue_enqueue(void * queueP,
                              uint8_t * buffer,
                              size_t length,
                              void * userData);
```

** Description **

`iowa_system_queue_enqueue()` stores data in a storage queue.

** Arguments **

*queueP*
: A storage queue as returned by [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create).

*buffer*
: The data to store.

*length*
: The length of the data in bytes.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The number of stored bytes or a negative number in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **LWM2M_STORAGE_QUEUE_SUPPORT** or **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flags.

The data must be stored as a single entity.

\clearpage

### iowa_system_queue_dequeue

** Prototype **

```c
size_t iowa_system_queue_dequeue(void * queueP,
                                 uint8_t * buffer,
                                 size_t length,
                                 void * userData);
```

** Description **

`iowa_system_queue_dequeue()` retrieves an entity from a storage queue or, the provided buffer is too small, it returns the size of the next entity to retrieve.

** Arguments **

*queueP*
: A storage queue as returned by [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create).

*buffer*
: A buffer to store the retrieved data. This can be nil.

*length*
: The length of the buffer in bytes. This can be zero.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The size in bytes of retrieved entity or zero if the queue is empty or in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **LWM2M_STORAGE_QUEUE_SUPPORT** flag.

If the provided buffer is nil or too small to contain the entity to retrieve, the entity is not removed from the queue.

\clearpage

### iowa_system_queue_peek

** Prototype **

```c
size_t iowa_system_queue_peek(void * queueP,
                              uint8_t * buffer,
                              size_t length,
                              void * userData);
```

** Description **

`iowa_system_queue_peek()` peeks a entity from a storage queue or, if the provided buffer is too small, it returns the size of the next entity to peek.

** Arguments **

*queueP*
: A storage queue as returned by [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create).

*buffer*
: A buffer to store the retrieved data. This can be nil.

*length*
: The length of the buffer in bytes. This can be zero.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The size in bytes of peeked entity or zero if the queue is empty or in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flag.

\clearpage

### iowa_system_queue_remove

** Prototype **

```c
void iowa_system_queue_remove(void * queueP,
                              void * userData);
```

** Description **

`iowa_system_queue_remove()` removes the first entity of a storage queue.

** Arguments **

*queueP*
: A storage queue as returned by [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create).

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flag.

\clearpage

### iowa_system_queue_backup

** Prototype **

```c
size_t iowa_system_queue_backup(void *queueP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData);
```

** Description **

`iowa_system_queue_backup()` returns a blob of data necessary to recreate a storage queue, or if the provided buffer is too small, the size of this blob of data.

** Arguments **

*queueP*
: A storage queue as returned by [`iowa_system_queue_create()`](AbstractionLayer.md#iowa_system_queue_create).

*buffer*
: A buffer to store the blob of data. This can be nil.

*length*
: The length of *buffer* in bytes. This can be zero.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The size in bytes of the blob of data to save or zero in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **IOWA_STORAGE_CONTEXT_SUPPORT** flag and with the **LWM2M_STORAGE_QUEUE_SUPPORT** or **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flags.

\clearpage

### iowa_system_queue_restore

** Prototype **

```c
void *iowa_system_queue_restore(uint8_t *buffer,
                                size_t length,
                                void *userData);
```

** Description **

`iowa_system_queue_restore()` recreates a storage queue from a blob of data.

** Arguments **

*buffer*
: A buffer containing the blob of data returned by [`iowa_system_queue_backup()`](AbstractionLayer.md#iowa_system_queue_backup).

*length*
: The length of *buffer* in bytes.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

A pointer to an user-defined type or NULL in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **IOWA_STORAGE_CONTEXT_SUPPORT** flag and with the **LWM2M_STORAGE_QUEUE_SUPPORT** or **LWM2M_STORAGE_QUEUE_PEEK_SUPPORT** flags.

\clearpage

### iowa_system_store_context

** Prototype **

```c
size_t iowa_system_store_context(uint8_t *bufferP,
                                 size_t length,
                                 void *userData);
```

** Description **

`iowa_system_store_context()` stores the IOWA context.

** Arguments **

*bufferP*
: The destination buffer.

*length*
: Length of the buffer.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The number of stored bytes or a zero in case of error.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **IOWA_STORAGE_CONTEXT_SUPPORT** flag.

\clearpage

### iowa_system_retrieve_context

** Prototype **

```c
size_t iowa_system_retrieve_context(uint8_t **bufferP,
                                    void *userData);
```

** Description **

`iowa_system_retrieve_context()` retrieves an IOWA context.

** Arguments **

*bufferP*
: The buffer containing the retrieved data.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

The size in bytes of retrieved data or zero if there is nothing or in case of error.

** Header File **

iowa_platform.h

** Notes **

- This function is required only if IOWA is built with the **IOWA_STORAGE_CONTEXT_SUPPORT** flag.
- *bufferP* must be allocated by the function. The buffer will next be freed by IOWA internally.

\clearpage

### iowa_system_mutex_lock

** Prototype **

```c
void iowa_system_mutex_lock(void * userData);
```

** Description **

`iowa_system_mutex_lock()` locks a mutex for the current thread.

** Arguments **

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **IOWA_MULTITHREAD_SUPPORT** flag.

IOWA uses only one mutex.

\clearpage

### iowa_system_mutex_unlock

** Prototype **

```c
void iowa_system_mutex_unlock(void * userData);
```

** Description **

`iowa_system_mutex_unlock()` releases a mutex.

** Arguments **

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

None.

** Header File **

iowa_platform.h

** Notes **

This function is required only if IOWA is built with the **IOWA_MULTITHREAD_SUPPORT** flag.

IOWA uses only one mutex.

\clearpage

### iowa_system_random_vector_generator

** Prototype **

```c
int iowa_system_random_vector_generator(uint8_t *randomBuffer,
                                        size_t size,
                                        void *userData);
```

** Description **

`iowa_system_random_vector_generator()` stores random values to a preallocated vector.

** Arguments **

*randomBuffer*
: The generated random vector.

*size*
: The size of the vector.

*userData*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

A code indicating if the vector has been generated:

* 0 if the vector has been generated successfully.
* Another value if an error occurred.

** Header File **

iowa_platform.h

** Notes **

This function is only required if IOWA is built with **IOWA_SECURITY_LAYER** different from **IOWA_SECURITY_LAYER_NONE**.

\clearpage

### iowa_system_security_data

** Prototype **

```c
iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP);
```

** Description **

`iowa_system_security_data()` is a function used by the security layer to CREATE, DELETE or READ the security data.

** Arguments **

*peerIdentity*
: The identity associating to the peer (can be an URI on client side, or the PSK identity on server side, etc).

*peerIdentitySize*
: Size of the identity.

*securityOp*
: The security operation.

*securityDataP*
: The security data.

*userDataP*
: The argument passed to [`iowa_init()`](CommonAPI.md#iowa_init).

** Return Value **

IOWA_COAP_NO_ERROR
: success.

IOWA_COAP_404_NOT_FOUND
: the security data based on *peerIdentity* has not been found (only when the security operation is not `IOWA_SEC_CREATE`).

IOWA_COAP_406_NOT_ACCEPTABLE
: the security data based on *peerIdentity* already exists (only when the security operation is `IOWA_SEC_CREATE`).

IOWA_COAP_500_INTERNAL_SERVER_ERROR
: a memory allocation failed.

** Header File **

iowa_platform.h

** Notes **

This function is only required if IOWA is built with **IOWA_SECURITY_LAYER** different from **IOWA_SECURITY_LAYER_NONE**.

The **IOWA_SEC_DELETE** operation is only used if a **IOWA_SEC_CREATE** operation happened. The **IOWA_SEC_CREATE** operation is only used on bootstrapping.

Regarding the memory allocation:

- On a **IOWA_SEC_CREATE** operation, the data is allocated by the stack. So the deallocation MUST not be done by the application.
- On a **IOWA_SEC_READ** operation, the data if needed is allocated by the application. The stack does not deallocate it. That's why after a **IOWA_SEC_READ** operation, the function `iowa_system_security_data` is called with **IOWA_SEC_FREE** to allow the application to deallocate the memory if needed.

*peerIdentity* is always a character string encoded in UTF-8 without the Null-terminated string:

- On a LwM2M Client, this is the LwM2M Server URI or the Bootstrap Server URI.
- On Server, this is Client Hint Identity.
