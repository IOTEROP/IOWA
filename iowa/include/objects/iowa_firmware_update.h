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
* Copyright (c) 2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

/*************************************************************************************
* Firmware Update
*
** Description
*** This LwM2M Object enables management of firmware which is to be updated.
*** This Object includes installing a firmware package, updating firmware, and performing actions after updating firmware.
*** The firmware update MAY require to reboot the device; it will depend on a number of factors, such as the operating system architecture and the extent of the updated software.
***
*** The envisioned functionality is to allow a LwM2M Client to connect to any LwM2M Server to obtain a firmware image using the object and resource structure defined in this section experiencing communication security protection using TLS/DTLS.
*** There are, however, other design decisions that need to be taken into account to allow a manufacturer of a device to securely install firmware on a device.
*** Examples for such design decisions are how to manage the firmware update repository at the server side (which may include user interface considerations),
*** the techniques to provide additional application layer security protection of the firmware image, how many versions of firmware images to store on the device,
*** and how to execute the firmware update process considering the hardware specific details of a given IoT hardware product.
*** These aspects are considered to be outside the scope of this version of the specification.
***
*** A LwM2M Server may also instruct a LwM2M Client to fetch a firmware image from a dedicated server (instead of pushing firmware images to the LwM2M Client).
*** The Package URI resource is contained in the Firmware object and can be used for this purpose.
***
*** A LwM2M Client MUST support block-wise transfer [CoAP_Blockwise] if it implements the Firmware Update object.
***
*** A LwM2M Server MUST support block-wise transfer. Other protocols, such as HTTP/HTTPs, MAY also be used for downloading firmware updates (via the Package URI resource).
*** For constrained devices it is, however, RECOMMENDED to use CoAP for firmware downloads to avoid the need for additional protocol implementations.
*
** Object Definition
*** Object Id: 5
*** Instances: Single
*** Optional
*************************************************************************************/

#ifndef _IOWA_FIRMWARE_UPDATE_INCLUDE_
#define _IOWA_FIRMWARE_UPDATE_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

typedef enum
{
    IOWA_FW_STATUS_SUCCESSFUL              = 1,
    IOWA_FW_STATUS_OUT_OF_STORAGE          = 2,
    IOWA_FW_STATUS_OUT_OF_MEMORY           = 3,
    IOWA_FW_STATUS_CONNECTION_LOST         = 4,
    IOWA_FW_STATUS_INTEGRITY_CHECK_FAILURE = 5,
    IOWA_FW_STATUS_UNSUPPORTED_TYPE        = 6,
    IOWA_FW_STATUS_INVALID_URI             = 7,
    IOWA_FW_STATUS_UPDATE_FAILED           = 8,
    IOWA_FW_STATUS_UNSUPPORTED_PROTOCOL    = 9
} iowa_fw_status_t;

#define IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAP      (1<<0)
#define IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAPS     (1<<1)
#define IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_HTTP      (1<<2)
#define IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_HTTPS     (1<<3)
#define IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAP_TCP  (1<<4)
#define IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_COAP_TLS  (1<<5)

// The download callback.
// Returned value: None.
// Parameters:
// - uri: URI to download the package from.
// - userData: the parameter to iowa_client_firmware_update_configure().
// - contextP: the IOWA context on which iowa_client_firmware_update_configure() was called.
typedef void(*iowa_fw_download_callback_t) (char * uri,
                                            void * userData,
                                            iowa_context_t contextP);

// The update callback.
// Returned value: None.
// Parameters:
// - userData: the parameter to iowa_client_firmware_update_configure().
// - contextP: the IOWA context on which iowa_client_firmware_update_configure() was called.
typedef void(*iowa_fw_update_callback_t) (void * userData,
                                          iowa_context_t contextP);

typedef enum
{
    IOWA_FW_PACKAGE_WRITE,
    IOWA_FW_PACKAGE_RESET
} iowa_fw_write_cmd_t;

// The write callback.
// Returned value: IOWA_FW_STATUS_SUCCESSFUL in case of success or an error status.
// Parameters:
// - cmd: IOWA_FW_PACKAGE_WRITE or IOWA_FW_PACKAGE_RESET
// - dataLength: length of data.
// - data: next chunk of the Firmware Package to write.
// - userData: the parameter to iowa_client_firmware_update_configure().
// - contextP: the IOWA context on which iowa_client_firmware_update_configure() was called.
typedef iowa_fw_status_t(*iowa_fw_write_callback_t) (iowa_fw_write_cmd_t cmd,
                                                     size_t dataLength,
                                                     uint8_t *data,
                                                     void *userData,
                                                     iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Configure the firmware update
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - packageName: name of the current firmware. This can be nil.
// - packageVersion: version of the current firmware. This can be nil.
// - downloadCb: callback to download a new firmware. This can be nil.
// - writeCb: callback to write chunks of the new firmware. This can be nil.
// - updateCb: callback called to update the device.
// - userData: past as argument to the callbacks.
iowa_status_t iowa_client_firmware_update_configure(iowa_context_t contextP,
                                                    const char *packageName,
                                                    const char *packageVersion,
                                                    iowa_fw_download_callback_t downloadCb,
                                                    iowa_fw_write_callback_t writeCb,
                                                    iowa_fw_update_callback_t updateCb,
                                                    void *userData);

// Configure the firmware update with the optional "protocol support" resource.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - packageName: name of the current firmware. This can be nil.
// - packageVersion: version of the current firmware. This can be nil.
// - protocolSupport: firmware update protocol support flags (IOWA_FIRMWARE_UPDATE_PROTOCOL_SUPPORT_xx). This can be 0.
// - downloadCb: callback to download a new firmware. This can be nil.
// - writeCb: callback to write chunks of the new firmware. This can be nil.
// - updateCb: callback called to update the device.
// - userData: past as argument to the callbacks.
iowa_status_t iowa_client_firmware_update_configure_full(iowa_context_t contextP,
                                                         const char *packageName,
                                                         const char *packageVersion,
                                                         uint8_t protocolSupport,
                                                         iowa_fw_download_callback_t downloadCb,
                                                         iowa_fw_write_callback_t writeCb,
                                                         iowa_fw_update_callback_t updateCb,
                                                         void *userData);

// Inform the IOWA stack of the result of the firmware update callbacks.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - status: the result of the callback.
iowa_status_t iowa_client_firmware_update_set_status(iowa_context_t contextP,
                                                     iowa_fw_status_t status);


#ifdef __cplusplus
}
#endif

#endif /*_IOWA_FIRMWARE_UPDATE_INCLUDE_*/
