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
* Cellular connectivity
*
** Description
*** This object specifies resources to enable a device to connect to a 3GPP or 3GPP2 bearer, including GPRS/EDGE, UMTS, LTE, NB-IoT, SMS.
*** For cellular connectivity, this object focuses on Packet Switched (PS) connectivity and doesn't aim to provide comprehensive Circuit Switched (CS) connectivity management.
*
** Object Definition
*** Object Id: 10
*** Instances: Single
*** Optional
*************************************************************************************/

#ifndef _IOWA_CELLULAR_CONNECTIVITY_INCLUDE_
#define _IOWA_CELLULAR_CONNECTIVITY_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

#define IOWA_CELLULAR_CONNECTIVITY_RSC_SMSC_ADDRESS             (1<<0)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_DISABLE_RADIO_PERIOD     (1<<1)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_MODULE_ACTIVATION_CODE   (1<<2)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_PSM_TIMER                (1<<3)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_ACTIVE_TIMER             (1<<4)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_PLMN_RATE_CONTROL        (1<<5)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_IU_MODE       (1<<6)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_WB_S1_MODE    (1<<7)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_NB_S1_MODE    (1<<8)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_EDRX_PARAM_A_GB_MODE     (1<<9)
#define IOWA_CELLULAR_CONNECTIVITY_RSC_ACTIVATED_PROFILE_NAMES  (1<<10)

// Cellular connectivity information
// Elements:
// - activatedProfileNamesList: list of links to instances of the APN connection profile object representing every APN connection profile that has an activated connection to a PDN.
// - activatedProfileNamesNumber: number of links to instances of the APN connection profile object representing every APN connection profile that has an activated connection to a PDN.
// - smsc: address of the sms center.
// - disableRadioPeriod: time period for which the device shall disconnect from cellular radio.
// - moduleActivationCode: configurable in case the application needs to issue a code.
// - psmTimer: Power Saving Mode timer.
// - activeTimer: active timer.
// - servingPlmnRateControl: maximum number of allowed uplink PDU transmissions.
// - edrxParamIuMode: Extended DRX parameters for lu mode.
// - edrxParamWbS1Mode: Extended DRX parameters for WB-S1 mode.
// - edrxParamNbS1Mode: Extended DRX parameters for NB-S1 mode.
// - edrxParamAGbmMode: Extended DRX parameters for A/Gb mode.
typedef struct
{
    iowa_lwm2m_object_link_t *activatedProfileNamesList;
    uint16_t                  activatedProfileNamesNumber;
    char                     *smsc;
    int32_t                   disableRadioPeriod;
    char                     *moduleActivationCode;
    int32_t                   psmTimer;
    int32_t                   activeTimer;
    uint32_t                  servingPlmnRateControl;
    uint8_t                  *edrxParamIuMode;
    uint8_t                  *edrxParamWbS1Mode;
    uint8_t                  *edrxParamNbS1Mode;
    uint8_t                  *edrxParamAGbmMode;
} iowa_cellular_connectivity_info_t;

// The callback called to change the state of the cellular connectivity.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: instance of the cellular connectivity.
// - infoP: cellular connectivity information.
// - userDataCallback: user data callback.
// - contextP: the IOWA context.
typedef iowa_status_t (*iowa_cellular_connectivity_update_state_callback_t)(iowa_sensor_t id,
                                                                            iowa_cellular_connectivity_info_t *infoP,
                                                                            void *userDataCallback,
                                                                            iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Add a cellular connectivity object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional resources.
// - updateStateCallback: callback used when the state changed.
// - userDataCallback: user data callback.
// - idP: OUT. ID of the created instance of the accelerometer object.
iowa_status_t iowa_client_add_cellular_connectivity_object(iowa_context_t contextP,
                                                           uint16_t optFlags,
                                                           iowa_cellular_connectivity_update_state_callback_t updateStateCallback,
                                                           void *userDataCallback,
                                                           iowa_sensor_t *idP);

// Remove a cellular connectivity object created with iowa_client_add_cellular_connectivity_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the cellular connectivity object.
iowa_status_t iowa_client_remove_cellular_connectivity_object(iowa_context_t contextP,
                                                              iowa_sensor_t id);

// Update the Cellular connectivity.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the cellular connectivity object.
// - flags: specify resources to update.
// - infoP: cellular connectivity information.
iowa_status_t iowa_client_cellular_connectivity_update(iowa_context_t contextP,
                                                       iowa_sensor_t id,
                                                       uint16_t flags,
                                                       iowa_cellular_connectivity_info_t *infoP);


#ifdef __cplusplus
}
#endif

#endif /*_IOWA_CELLULAR_CONNECTIVITY_INCLUDE_*/
