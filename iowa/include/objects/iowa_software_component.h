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
* LWM2M Software Component
*
** Description
*** This LwM2M objects provides the resources needed to activate/deactivate software components on the device.
*** If some Objects are not supported after software update, the LwM2M Client MUST delete all the Object Instances of the Objects that are not supported.
*
** Object Definition
*** Object Id: 14
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_SOFTWARE_COMPONENT_INCLUDE_
#define _IOWA_SOFTWARE_COMPONENT_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Repertory software component information that users can set
// Elements:
// - identityP: Name or identifier of the software component, with size < 255. This can be nil.
// - packP, packLength: Link to opaque data describing the software component. This can be nil.
// - versionP: Version of the software component, with size < 255. This can be nil.
// Note: This structure will at least provide an identity (*identityP*) or a pack (*packP*) to identify the component.
typedef struct
{
    const char     *identityP;
    const uint8_t  *packP;
    size_t          packLength;
    const char     *versionP;
} iowa_sw_cmp_info_t;

// The update callback called when the Server adds or removes the software components.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: ID of the corresponding software component.
// - operation: the operation performed by the Server on this software component (either IOWA_DM_CREATE or IOWA_DM_DELETE).
// - infoP: software component information.
// - activationState: initial activated state. Should be ignored if activationCb is nil.
// - userDataP: user data callback.
// - contextP: the IOWA context.
typedef iowa_status_t(*iowa_sw_cmp_update_callback_t)(iowa_sensor_t id,
                                                      iowa_dm_operation_t operation,
                                                      iowa_sw_cmp_info_t *infoP,
                                                      bool activationState,
                                                      void *userDataP,
                                                      iowa_context_t contextP);

// The activation callback, called when the Server requests the device to activate or deactivate the software component.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: ID of the corresponding software component.
// - activationState: activation state asked.
// - userDataP: the data passed to iowa_client_enable_software_component().
// - contextP: the IOWA context on which iowa_client_enable_software_component() was called.
typedef iowa_status_t(*iowa_sw_cmp_activation_callback_t) (iowa_sensor_t id,
                                                           bool activationState,
                                                           void *userDataP,
                                                           iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Enable the software component feature.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - updateCb: The update callback called when the Server adds or removes the software components. This can be nil.
// - activationCb: The activate callback, called when the Server requests the device to activate or deactivate the software Package. This can be nil.
// - userDataP: Past as argument to the callback. This can be nil.
iowa_status_t iowa_client_enable_software_component(iowa_context_t contextP,
                                                    iowa_sw_cmp_update_callback_t updateCb,
                                                    iowa_sw_cmp_activation_callback_t activationCb,
                                                    void *userDataP);

// Disable the software component feature enabled with iowa_client_enable_software_component().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_client_disable_software_component(iowa_context_t contextP);

// Add a software component.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - infoP: software component information.
// - activationState: current activation state of the software component. Ignored if activationCb is nil.
// - idP: OUT. Used to store the ID of the software component.
// Notes: - infoP must provide at least an identity or a pack to identify component.
//        - The "const" elements pointed by the fields of *infoP* are not duplicated nor freed by IOWA.
iowa_status_t iowa_client_add_software_component(iowa_context_t contextP,
                                                 iowa_sw_cmp_info_t *infoP,
                                                 bool activationState,
                                                 iowa_sensor_t *idP);

// Remove a software component added with iowa_client_add_software_component().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the corresponding software component.
iowa_status_t iowa_client_remove_software_component(iowa_context_t contextP,
                                                    iowa_sensor_t id);

// Update a software component's state.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the corresponding software component.
// - activationState: New activation state of the software component. Ignored if activationCb is nil.
iowa_status_t iowa_client_software_component_update_state(iowa_context_t contextP,
                                                          iowa_sensor_t id,
                                                          bool activationState);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_SOFTWARE_COMPONENT_INCLUDE_*/
