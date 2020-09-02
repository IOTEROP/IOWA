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

#ifndef _IOWA_INCLUDE_
#define _IOWA_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define IOWA_VERSION "2020-03"

/**************************************************************
* Types
**************************************************************/

typedef struct _iowa_context_t * iowa_context_t;

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

#define IOWA_LWM2M_ID_ALL 0xFFFF

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

typedef uint8_t iowa_lwm2m_data_type_t;

#define IOWA_LWM2M_TYPE_UNDEFINED         0
#define IOWA_LWM2M_TYPE_STRING            1
#define IOWA_LWM2M_TYPE_OPAQUE            2
#define IOWA_LWM2M_TYPE_INTEGER           3
#define IOWA_LWM2M_TYPE_FLOAT             4
#define IOWA_LWM2M_TYPE_BOOLEAN           5
#define IOWA_LWM2M_TYPE_CORE_LINK         6
#define IOWA_LWM2M_TYPE_OBJECT_LINK       7
#define IOWA_LWM2M_TYPE_TIME              8
#define IOWA_LWM2M_TYPE_UNSIGNED_INTEGER  9
#define IOWA_LWM2M_TYPE_STRING_BLOCK      101
#define IOWA_LWM2M_TYPE_OPAQUE_BLOCK      102
#define IOWA_LWM2M_TYPE_CORE_LINK_BLOCK   106

typedef struct
{
    uint16_t objectId;
    uint16_t instanceId;
} iowa_lwm2m_object_link_t;

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
        struct
        {
            uint32_t details;
            uint8_t *buffer;
        } asBlock;
        iowa_lwm2m_object_link_t asObjLink;
    } value;
    int32_t timestamp;
} iowa_lwm2m_data_t;

typedef struct
{
    uint16_t objectId;
    uint16_t instanceId;
    uint16_t resourceId;
    uint16_t resInstanceId;
} iowa_lwm2m_uri_t;

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

typedef enum
{
    IOWA_CONN_UNDEFINED = 0,
    IOWA_CONN_DATAGRAM,
    IOWA_CONN_STREAM,
    IOWA_CONN_LORAWAN,
    IOWA_CONN_SMS
} iowa_connection_type_t;

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

/**************************************************************
 * Common Callbacks
 **************************************************************/

/****************************
 * Callback use to get response to handle operation
 */

// The operation response callback.
// Returned value: None.
// Parameters:
// - sourceId: the ID of the client or the server targeted by the operation.
// - operation: the type of the operation.
// - status: the status of the operation.
// - contentP: the content of the operation. It can be nil.
// - userDataP: a pointer to application specific data. This is a parameter of the matching device management API.
// - contextP: the IOWA context on which the device management API was called.
typedef void(*iowa_response_callback_t) (uint32_t sourceId,
                                         uint8_t operation,
                                         iowa_status_t status,
                                         iowa_response_content_t *contentP,
                                         void *userDataP,
                                         iowa_context_t contextP);

/****************************
 * Callback use to save external data with the IOWA context
 */

// This callback is called during a context load with the data stored inside the context backup.
// Returned value: None.
// Parameters:
// - callbackId: the identifier of the callback.
// - buffer: the data loaded from the backup. This can be nil.
// - bufferLength: the length of the buffer in bytes.
// - userDataP: Pointer to application-specific data.
typedef void(*iowa_load_callback_t) (uint16_t callbackId,
                                     uint8_t *buffer,
                                     size_t bufferLength,
                                     void *userDataP);

// This callback is called during a context save. It is called first to retrieve the length of the
// data to save, then a second time with an allocated buffer to fill with the data to save.
// Returned value: The size of the buffer.
// Parameters:
// - callbackId: the identifier of the callback.
// - buffer: a buffer to store the data. This can be nil.
// - bufferLength: the length of the buffer in bytes.
// - userDataP: Pointer to application-specific data.
typedef size_t(*iowa_save_callback_t) (uint16_t callbackId,
                                       uint8_t *buffer,
                                       size_t bufferLength,
                                       void *userDataP);

/**************************************************************
 * Common APIs
 **************************************************************/

// Initialize an IOWA context.
// Returned value: an iowa_context_t in case of success or NULL in case of error.
// Parameters:
// - userData: Pointer to application-specific data.
iowa_context_t iowa_init(void * userData);

// Close an IOWA context.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
void iowa_close(iowa_context_t contextP);

// Run the stack engine during the specified time.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - timeout: allowed time to run. 0 for for "immediate" return, negative value for "infinite".
iowa_status_t iowa_step(iowa_context_t contextP,
                        int32_t timeout);

// Stop the stack engine and make iowa_step() return immediately.
// Returned value: none.
// Parameters:
// - contextP: returned by iowa_init().
void iowa_stop(iowa_context_t contextP);

// Perform all stack pending operations before the device pause for some time.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - duration: the duration of the pause in seconds.
// - delayP: OUT. The delay before the next iowa scheduled operation.
iowa_status_t iowa_flush_before_pause(iowa_context_t contextP,
                                      int32_t duration,
                                      uint32_t *delayP);

// Save the current IOWA context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_save_context(iowa_context_t contextP);

// Save the current IOWA context with runtime informations, observations and attributes.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_save_context_snapshot(iowa_context_t contextP);

// Load a saved IOWA context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_load_context(iowa_context_t contextP);

// Register backup callbacks that will be called when loading or saving the context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - callbackId: the identifier of the callback. Must be equal to or greater than 0xF000.
// - saveCallback: callback called when saving the context.
// - loadCallback: callback called when loading the context.
// - userDataP: a pointer to application specific data.
iowa_status_t iowa_backup_register_callback(iowa_context_t contextP,
                                            uint16_t callbackId,
                                            iowa_save_callback_t saveCallback,
                                            iowa_load_callback_t loadCallback,
                                            void *userDataP);

// Deregister backup callbacks.
// Returned value: None.
// Parameters:
// - contextP: returned by iowa_init().
// - callbackId: the identifier of the callback.
void iowa_backup_deregister_callback(iowa_context_t contextP,
                                     uint16_t callbackId);

// Inform the stack a connection was closed.
// Parameters:
// - contextP: returned by iowa_init().
// - connP: the connection of the same opaque type as returned by iowa_system_connection_open().
void iowa_connection_closed(iowa_context_t contextP,
                            void *connP);

// The possible size of the data block when "more" is true.
#define IOWA_DATA_BLOCK_SIZE_16    16
#define IOWA_DATA_BLOCK_SIZE_32    32
#define IOWA_DATA_BLOCK_SIZE_64    64
#define IOWA_DATA_BLOCK_SIZE_128   128
#define IOWA_DATA_BLOCK_SIZE_256   256
#define IOWA_DATA_BLOCK_SIZE_512   512
#define IOWA_DATA_BLOCK_SIZE_1024  1024

// Get block information from an iowa_lwm2m_data_t.
// Returned value: IOWA_COAP_NO_ERROR in case of success or IOWA_COAP_404_NOT_FOUND if there are no block information to retrieve.
// Parameters:
// - dataP: the iowa_lwm2m_data_t to retrieve the block info from.
// - numberP: OUT. the block number.
// - moreP: OUT. true if there are more blocks coming.
// - sizeP: OUT. the size of the block.
iowa_status_t iowa_data_get_block_info(iowa_lwm2m_data_t *dataP,
                                       uint32_t *numberP,
                                       bool *moreP,
                                       uint16_t *sizeP);

// Set block information of an iowa_lwm2m_data_t.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP: the iowa_lwm2m_data_t to set the info to.
// - number: the block number.
// - more: true if there are more blocks coming.
// - size: the size of the block. It must be inferior to 1024.
iowa_status_t iowa_data_set_block_info(iowa_lwm2m_data_t *dataP,
                                       uint32_t number,
                                       bool more,
                                       uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
