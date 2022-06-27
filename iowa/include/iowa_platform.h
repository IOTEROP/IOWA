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
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PLATFORM_INCLUDE_
#define _IOWA_PLATFORM_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "iowa.h"
#include "iowa_security.h"

/**************************************************************
* Abstraction functions to be implemented by the user.
**************************************************************/

/*******************************
* System Abstraction Interface
*
* To be implemented by the user.
*/

// Same function as C standard library malloc().
// It returns the address of a memory block of size bytes or NULL in case of error.
void * iowa_system_malloc(size_t size);

// Same function as C standard library free().
// It releases the memory block at address p.
void iowa_system_free(void * pointer);

// This function returns the number of seconds elapsed since origin or a negative value in case of error.
// If you are using the GPS or the Location object, this function will be used to timestamp the measure. In this case, the point of origin must be Epoch.
// Else, the origin(Epoch, system boot, etc...) does not matter as this function is used only to determine the elapsed time since the last call to it.
int32_t iowa_system_gettime(void);

// This function starts a reboot of the system.
void iowa_system_reboot(void *userData);


/*******************************
* Logging Abstraction Interface
*
* To be implemented by the user.
*/

// This function is called to output the logs when the stack is built with IOWA_WITH_LOGS.
// It can be mapped directly to vprintf().
// Parameters:
// - format: same kind of nil-terminated format string as printf() functions.
// - varArgs: the variable arguments.
void iowa_system_trace(const char * format,
                       va_list varArgs);


/*************************************
* Communication Abstraction Interface
*
* To be implemented by the user.
*/

// This function opens a connection to a host.
// Returned value: a pointer to an user-defined type or NULL in case of error.
// Parameters:
// - type: the type of connection to open.
// - hostname: the hostname to connect to as a nil-terminated string.
// - port: the port to connect to as a nil-terminated string or NULL if irrelevant.
// - userData: the iowa_init() parameter.
void * iowa_system_connection_open(iowa_connection_type_t type,
                                   char * hostname,
                                   char * port,
                                   void * userData);

// This function opens a connection to a LwM2M Server.
// Used when IOWA_ABSTRACTION_EXTENSION is defined.
// Returned value: a pointer to an user-defined type or NULL in case of error.
// Parameters:
// - type: the type of connection to open.
// - hostname: the hostname to connect to as a nil-terminated string.
// - port: the port to connect to as a nil-terminated string or NULL if irrelevant.
// - ssid: the Short Server ID associated to the LwM2M Server.
// - userData: the iowa_init() parameter.
void * iowa_system_connection_open_server(iowa_connection_type_t type,
                                          char *hostname,
                                          char *port,
                                          uint16_t ssid,
                                          void *userData);

// This function sends a buffer on a connection.
// Returned value: the number of bytes sent or a negative number in case of error.
// Parameters:
// - connP: the connection as returned by iowa_system_connection_open().
// - buffer, length: data to send.
// - userData: the iowa_init() parameter.
int iowa_system_connection_send(void * connP,
                                uint8_t * buffer,
                                size_t length,
                                void * userData);

// This function reads data from a connection in a non-blocking way.
// Returned value: the number of bytes read or a negative number in case of error.
// Parameters:
// - connP: the connection as returned by iowa_system_connection_open().
// - buffer: to store the read data.
// - length: the number of bytes to read.
// - userData: the iowa_init() parameter.
int iowa_system_connection_recv(void * connP,
                                uint8_t * buffer,
                                size_t length,
                                void * userData);

// This function returns an unique identifier for the peer of a connection (e.g. IP address, LoRaWAN DevEUI, SMS MSISDN).
// Returned value: the length of the identifier or 0 in case of error.
// Parameters:
// - connP: the connection as returned by iowa_system_connection_open().
// - buffer: to store the identifier data.
// - length: the length of buffer.
// - userData: the iowa_init() parameter.
size_t iowa_system_connection_get_peer_identifier(void * connP,
                                                  uint8_t *addrP,
                                                  size_t length,
                                                  void * userData);

// This functions monitors a list of connections for incoming data during the specified time.
// Returned value: a positive number if data are available, 0 if the time elapsed or a negative number in case of error.
// Parameters:
// - connArray: an array of connections as returned by iowa_system_connection_open().
// - connCount: The size of the array
// - timeout: the time to wait for data in seconds.
// - userData: the iowa_init() parameter.
int iowa_system_connection_select(void ** connArray,
                                  size_t connCount,
                                  int32_t timeout,
                                  void * userData);

// This functions closes a connection.
// Returned value: none.
// Parameters:
// - connP: the connection as returned by iowa_system_connection_open().
// - userData: the iowa_init() parameter.
void iowa_system_connection_close(void * connP,
                                  void * userData);


/*******************************
* Mutex Interface
*
* To be implemented by the user if the define IOWA_THREAD_SUPPORT is used.
*/

// This functions interrupts any on-going iowa_system_connection_select().
// Returned value: none.
// Parameters:
// - userData: the iowa_init() parameter.
void iowa_system_connection_interrupt_select(void * userData);

// This function locks a mutex.
// Returned value: none.
// Parameters:
// - userData: the iowa_init() parameter.
void iowa_system_mutex_lock(void * userData);

// This function releases a mutex.
// Returned value: none.
// Parameters:
// - userData: the iowa_init() parameter.
void iowa_system_mutex_unlock(void * userData);

/*************************************
* Storage Queue Abstraction Interface
*
* To be implemented by the user if the define LWM2M_STORAGE_QUEUE_SUPPORT or LWM2M_STORAGE_QUEUE_PEEK_SUPPORT is used.
*/

// This function creates a storage queue to offload data from the memory.
// Returned value: a pointer to an user-defined type or NULL in case of error.
// Parameters:
// - userData: the iowa_init() parameter.
void * iowa_system_queue_create(void * userData);

// This function deletes a storage queue.
// Returned value: none.
// Parameters:
// - queueP: a storage queue as returned by iowa_system_queue_create().
// - userData: the iowa_init() parameter.
void iowa_system_queue_delete(void * queueP,
                              void * userData);

// This function stores data in a storage queue.
// Returned value: the number of stored bytes or a negative number in case of error.
// Parameters:
// - queueP: a storage queue as returned by iowa_system_queue_create().
// - buffer: the data to store.
// - length: the length of the data in bytes.
// - userData: the iowa_init() parameter.
int iowa_system_queue_enqueue(void * queueP,
                              uint8_t * buffer,
                              size_t length,
                              void * userData);

// This function retrieves an entity from a storage queue or, if the provided buffer
// is too small, it returns the size of the next entity to retrieve.
// Only if the define LWM2M_STORAGE_QUEUE_SUPPORT is used.
// Returned value: the size in bytes of retrieved entity or zero if the queue is empty
//                 or in case of error.
// Parameters:
// - queueP: a storage queue as returned by iowa_system_queue_create().
// - buffer: a buffer a store the retrieved data. This can be nil.
// - length: the length of the buffer in bytes. This can be zero.
// - userData: the iowa_init() parameter.
size_t iowa_system_queue_dequeue(void * queueP,
                                 uint8_t * buffer,
                                 size_t length,
                                 void * userData);

// This function peeks an entity from a storage queue or, if the provided buffer
// is too small, it returns the size of the next entity to pick.
// Only if the define LWM2M_STORAGE_QUEUE_PEEK_SUPPORT is used.
// Returned value: the size in bytes of picked entity or zero if the queue is empty
//                 or in case of error.
// Parameters:
// - queueP: a storage queue as returned by iowa_system_queue_create().
// - buffer: a buffer a store the picked data. This can be nil.
// - length: the length of the buffer in bytes. This can be zero.
// - userData: the iowa_init() parameter.
size_t iowa_system_queue_peek(void * queueP,
                              uint8_t * buffer,
                              size_t length,
                              void * userData);

// This function removes the first entity of a storage queue.
// Only if the define LWM2M_STORAGE_QUEUE_PEEK_SUPPORT is used.
// Returned value: none.
// Parameters:
// - queueP: a storage queue as returned by iowa_system_queue_create().
// - userData: the iowa_init() parameter.
void iowa_system_queue_remove(void * queueP,
                              void * userData);

// This function returns a blob of data necessary to recreate a storage queue, or if the provided buffer is too small, the size of this blob of data.
// Returned value: The size in bytes of the blob of data to save or zero in case of error.
// Parameters:
// - queueP: a storage queue as returned by iowa_system_queue_create().
// - buffer: A buffer to store the blob of data. This can be nil.
// - length: The length of buffer in bytes. This can be zero.
// - userData: the iowa_init() parameter.
size_t iowa_system_queue_backup(void *queueP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData);

// This function recreates a storage queue from a blob of data.
// Returned value: A pointer to an user-defined type or NULL in case of error.
// Parameters:
// - buffer: A buffer containing the blob of data.
// - length: The length of buffer in bytes.
// - userData: the iowa_init() parameter.
void *iowa_system_queue_restore(uint8_t *buffer,
                                size_t length,
                                void *userData);

/*************************************
* Storage Context Abstraction Interface
*
* To be implemented by the user if the define IOWA_STORAGE_CONTEXT_SUPPORT is used.
*/

// This function stores the IOWA context.
// Returned value: the number of stored bytes or a zero in case of error.
// Parameters:
// - bufferP: the data to store.
// - length: the length of the data in bytes.
// - userData: the iowa_init() parameter.
size_t iowa_system_store_context(uint8_t *bufferP,
                                 size_t length,
                                 void *userData);

// This function retrieves an IOWA context.
// Returned value: the size in bytes of retrieved data or zero if there is nothing or in case of error.
// - bufferP: buffer containing the retrieved data.
// - userData: the iowa_init() parameter.
size_t iowa_system_retrieve_context(uint8_t **bufferP,
                                    void *userData);

/*************************************
* Security Abstraction Interface
*
* To be implemented by the user if the define IOWA_SECURITY_LAYER is not IOWA_SECURITY_LAYER_NONE.
*/

// This function returns a random vector of the specified size.
// Returned value: 0 if the vector has been generated, else if an error occurred.
// Parameters:
// - randomBuffer: the generated buffer.
// - size: the size of the buffer.
// - userData: the iowa_init() parameter.
int iowa_system_random_vector_generator(uint8_t *randomBuffer,
                                        size_t size,
                                        void *userData);

// The security data function.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - peerIdentity: the identity associating to the peer (can be an URI on client side, or the PSK identity on server side, etc).
// - peerIdentitySize: size of the identity.
// - securityOp: the security operation.
// - securityDataP: the security data.
// - userDataP: the iowa_init() parameter.
iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP);

#ifdef __cplusplus
}
#endif

#endif
