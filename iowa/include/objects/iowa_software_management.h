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
* LWM2M Software Management
*
** Description
*** This LwM2M objects provides the resources needed to perform software management on the device.
*** Each software component is managed via a dedicated Software Management Object instance.
*
** Object Definition
*** Object Id: 9
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_SOFTWARE_MANAGEMENT_INCLUDE_
#define _IOWA_SOFTWARE_MANAGEMENT_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
typedef enum
{
    IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL                  = 0,    // success of any operation made on the software package (verification, installation, uninstallation, activation, deactivation)
    IOWA_SW_PKG_UPDATE_RESULT_DOWNLOADING_SUCCESSFUL      = 3,    // success of the new software package download. (*downloadCb* or *writeCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_OUT_OF_STORAGE              = 50,   // not enough storage for the new software package. (*downloadCb* or *writeCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_OUT_OF_MEMORY               = 51,   // out of memory error during the download of the new software package. (*downloadCb* or *writeCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_CONNECTION_LOST             = 52,   // connection lost during the download of the new software package. (*downloadCb* or *writeCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_INTEGRITY_CHECK_FAILURE     = 53,   // integrity check failure of the new software package. (*downloadCb* or *writeCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_UNSUPPORTED_TYPE            = 54,   // unsupported new software package type.
    IOWA_SW_PKG_UPDATE_RESULT_INVALID_URI                 = 56,   // invalid URI to download the new software package. (*downloadCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_UPDATE_FAILED               = 57,   // device defined update error.
    IOWA_SW_PKG_UPDATE_RESULT_INSTALLED_FAILURE           = 58,   // new software installation failure. (*installCb* only)
    IOWA_SW_PKG_UPDATE_RESULT_UNINSTALLED_FAILURE         = 59    // software uninstallation failure. (*installCb* only)
} iowa_sw_pkg_result_t;

typedef enum
{
    IOWA_SW_PKG_STATE_UNINSTALLED,     // software is uninstalled (default value)
    IOWA_SW_PKG_STATE_INSTALLED,       // software is installed
    IOWA_SW_PKG_STATE_ACTIVATED        // software is activate (useful only if Software components are linked, otherwise same behavior than installed)
}iowa_sw_pkg_state_t;

// Repertory software package optional information that users can set
// Elements:
// - swComponentLinkP, swComponentLinkCount: Software Components downloaded and installed in scope of the present SW Update Package. This can be nil.
typedef struct
{
    iowa_sensor_t  *swComponentLinkP;
    uint16_t        swComponentLinkCount;
} iowa_sw_pkg_optional_info_t;

// The update callback called when the Server adds or removes the software packages.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: ID of the corresponding software package.
// - operation: the operation performed by the Server on this software package (either IOWA_DM_CREATE or IOWA_DM_DELETE).
// - pkgNameP: Name of the software package.
// - pkgVersionP: Version of the software package.
// - optP: optional information. This can be nil.
// - userDataP: user data callback.
// - contextP: the IOWA context.
typedef iowa_status_t(*iowa_sw_pkg_update_callback_t) (iowa_sensor_t id,
                                                       iowa_dm_operation_t operation,
                                                       const char *pkgNameP,
                                                       const char *pkgVersionP,
                                                       iowa_sw_pkg_optional_info_t *optP,
                                                       void *userDataP,
                                                       iowa_context_t contextP);

// The download callback, called when the Server requests the device to download a new software Package (new value in "Package URI").
// Returned value: None.
// Parameters:
// - id: ID of the corresponding software package.
// - uriP: URI to download the package from.
// - userNameP: User Name for access to SW Update Package in pull mode, with size < 255. Key based mechanism can alternatively use for talking to the component server instead of user name and password combination. This can be nil.
// - passwordP: Password for access to SW Update Package in pull mode, with size < 255. This can be nil.
// - userDataP: the data passed to iowa_client_enable_software_package_management().
// - contextP: the IOWA context on which iowa_client_enable_software_package_management() was called.
// Note: When packet is downloaded, call iowa_client_set_software_package_command_result() with IOWA_SW_PKG_UPDATE_RESULT_DOWNLOADING_SUCCESSFUL result if success or error result.
//       When packet is checked, call iowa_client_set_software_package_command_result() with IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL result if success or error result.
typedef void(*iowa_sw_pkg_download_callback_t) (iowa_sensor_t id,
                                                const char *uriP,
                                                const char *userNameP,
                                                const char *passwordP,
                                                void *userDataP,
                                                iowa_context_t contextP);

typedef enum
{
    IOWA_SW_PKG_COMMAND_RESET, // To start software package packet writing.
    IOWA_SW_PKG_COMMAND_WRITE  // To indicate other software package piece of the complete packet.
} iowa_sw_pkg_write_cmd_t;

// The write callback, called several times when the Server pushes the new Firmware Package to the device (new value in "Package").
// The expected behavior is the same as writing to a file stream i.e. unless it is reset, written data are appended to the previous ones.
// Returned value: IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL in case of success or an error status.
// Parameters:
// - id: ID of the corresponding software package.
// - cmd: IOWA_SW_PKG_COMMAND_RESET (To start software package writing) or IOWA_SW_PKG_COMMAND_WRITE
// - dataLength: length of data.
// - dataP: next chunk of the Firmware Package to write.
// - userDataP: the data passed to iowa_client_enable_software_package_management().
// - contextP: the IOWA context on which iowa_client_enable_software_package_management() was called.
typedef iowa_sw_pkg_result_t(*iowa_sw_pkg_write_callback_t) (iowa_sensor_t id,
                                                             iowa_sw_pkg_write_cmd_t cmd,
                                                             size_t dataLength,
                                                             uint8_t *dataP,
                                                             void *userDataP,
                                                             iowa_context_t contextP);

typedef enum
{
    IOWA_SW_PKG_COMMAND_INSTALL,            // software installation is requested.
    IOWA_SW_PKG_COMMAND_UNINSTALL,          // software uninstallation is requested.
    IOWA_SW_PKG_COMMAND_PREPARE_FOR_UPDATE  // software uninstallation is requested to prepare an update.
} iowa_sw_pkg_install_cmd_t;

// The install callback, called when the Server requests the device to install or uninstall the software Package.
// Returned value: None.
// Parameters:
// - id: ID of the corresponding software package.
// - cmd: installed state asked. Default value is uninstall (IOWA_SW_PKG_COMMAND_UNINSTALL).
// - userDataP: the data passed to iowa_client_enable_software_package_management().
// - contextP: the IOWA context on which iowa_client_enable_software_package_management() was called.
// Note: call iowa_client_set_software_package_command_result() with IOWA_SW_PKG_UPDATE_RESULT_SUCCESSFUL result if success or error result.
typedef void(*iowa_sw_pkg_install_callback_t) (iowa_sensor_t id,
                                               iowa_sw_pkg_install_cmd_t cmd,
                                               void *userDataP,
                                               iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Enable the software package feature.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - updateCb: The update callback called when the Server adds or removes the software packages.. This can be nil.
// - downloadCb: the download callback, called when the Server requests the device to download a new software Package (new value in "Package URI"). This can be nil.
// - writeCb: the write callback, called several times when the Server pushes the new Firmware Package to the device (new value in "Package"). This can be nil.
// - installCb: the install callback, called when the Server requests the device to install or uninstall the software Package.
// - userDataP: Past as argument to the callbacks. This can be nil.
// Note : downloadCb and writeCb are optional callbacks, but users must at least provide one of them.
iowa_status_t iowa_client_enable_software_package_management(iowa_context_t contextP,
                                                             iowa_sw_pkg_update_callback_t updateCb,
                                                             iowa_sw_pkg_download_callback_t downloadCb,
                                                             iowa_sw_pkg_write_callback_t writeCb,
                                                             iowa_sw_pkg_install_callback_t installCb,
                                                             void *userDataP);

// Disable the software package feature enabled with iowa_client_enable_software_package_management().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_client_disable_software_package_management(iowa_context_t contextP);

// Add a software package instance.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - pkgNameP: Name of the software package.
// - pkgVersionP: Version of the software package.
// - state: state of the software package. (default: IOWA_SW_PKG_STATE_UNINSTALLED)
// - optP: optional information. This can be nil.
// - idP: OUT. Used to store the ID of the created software package instance. Not checked at runtime.
iowa_status_t iowa_client_add_software_package(iowa_context_t contextP,
                                               const char *pkgNameP,
                                               const char *pkgVersionP,
                                               iowa_sw_pkg_state_t state,
                                               iowa_sw_pkg_optional_info_t *optP,
                                               iowa_sensor_t *idP);

// Remove a software package instance added with iowa_client_add_software_package().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the corresponding software package.
iowa_status_t iowa_client_remove_software_package(iowa_context_t contextP,
                                                  iowa_sensor_t id);

// Update a software package instance's information.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the corresponding software package.
// - state: state of the software package.
iowa_status_t iowa_client_software_package_update_state(iowa_context_t contextP,
                                                        iowa_sensor_t id,
                                                        iowa_sw_pkg_state_t state);

// Inform the IOWA stack of the result of the callbacks downloadCb and installCb of iowa_client_enable_software_package_management().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the corresponding software package.
// - result: the result of the callbacks.
iowa_status_t iowa_client_set_software_package_command_result(iowa_context_t contextP,
                                                              iowa_sensor_t id,
                                                              iowa_sw_pkg_result_t result);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_SOFTWARE_MANAGEMENT_INCLUDE_*/
