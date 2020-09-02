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
* Bearer selection
*
** Description
*** This object specifies resources to enable a device to choose a PLMN/network on which to attach/register and what type of bearer to then connect.
*** This object allows via remote bearer and network configuration to overwrite automatic network and bearer selection e.g. as supported by the UICC.
*** An equivalent example for overwriting automatic selection is a user doing manual network and bearer selection on a smart phone.
*
** Object Definition
*** Object Id: 13
*** Instances: Single
*** Optional
*************************************************************************************/

#ifndef _IOWA_BEARER_SELECTION_INCLUDE_
#define _IOWA_BEARER_SELECTION_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Defines used when declaring a new accelerometer object.
#define IOWA_BEARER_SELECTION_RSC_PREFERRED_COMM_BEARER         (1<<0)
#define IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSSI_GSM           (1<<1)
#define IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSCP_UMTS          (1<<2)
#define IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_LTE           (1<<3)
#define IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSSI_EV_DO         (1<<4)
#define IOWA_BEARER_SELECTION_RSC_CELL_LOCK_LIST                (1<<5)
#define IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST                 (1<<6)
#define IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST_MODE            (1<<7)
#define IOWA_BEARER_SELECTION_RSC_AVAILABLE_PLMNS               (1<<8)
#define IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_NB_IOT        (1<<9)
#define IOWA_BEARER_SELECTION_RSC_PLMN_SEARCH_TIMER             (1<<10)
#define IOWA_BEARER_SELECTION_RSC_ATTACH_WO_PDN_CONNECTION      (1<<11)

// Bearer selection information
// Elements:
// - preferredCommBearer: preferred communications bearer.
// - acceptableGsm: provides guide to the application when performing manual network selection.
// - acceptableUmts: provides guide to the application when performing manual network selection.
// - acceptableLte: provides guide to the application when performing manual network selection.
// - acceptableEvDo: provides guide to the application when performing manual network selection.
// - cellLockList: list of allowed Global Cell Identities.
// - operatorList: list of MCC+MNC of operators, in priority order.
// - operatorListMode: indicates whether resource operator list represents the allowed operator list (white list), or, the preferred operator list.
// - availablePlmns: allows server to see results of network scan.
// - acceptableRsrpNbIot: provides guide to the application when performing manual network selection.
// - plmnSearchTimer: interval between periodic searches for higher priority PLMNs.
// - attachWoPdnConnection: 0=attach with PDN connection, 1=attach without PDN connection
typedef struct
{
    int        *preferredCommBearerList;
    uint16_t    preferredCommBearerNumber;
    int         acceptableGsm;
    int         acceptableUmts;
    int         acceptableLte;
    int         acceptableEvDo;
    char       *cellLockList;
    char       *operatorList;
    bool        operatorListMode;
    char       *availablePlmns;
    int         acceptableRsrpNbIot;
    int         plmnSearchTimer;
    bool        attachWoPdnConnection;
} iowa_bearer_selection_info_t;

// The callback called to change the state of the Bearer selection.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - id: instance of the Bearer selection.
// - infoP: bearer selection info
// - userDataCallback: user data callback.
// - contextP: the IOWA context.
typedef iowa_status_t (*iowa_bearer_selection_update_state_callback_t)(iowa_sensor_t id,
                                                                       iowa_bearer_selection_info_t *infoP,
                                                                       void *userDataCallback,
                                                                       iowa_context_t contextP);


/**************************************************************
 * API
 **************************************************************/

// Add a Bearer selection object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional resources.
// - updateStateCallback: callback used when the state changed.
// - userDataCallback: user data callback.
// - idP: OUT. ID of the created instance of the bearer selection object.
iowa_status_t iowa_client_add_bearer_selection_object(iowa_context_t contextP,
                                                      uint16_t optFlags,
                                                      iowa_bearer_selection_update_state_callback_t updateStateCallback,
                                                      void *userDataCallback,
                                                      iowa_sensor_t *idP);

// Remove a Bearer selection object created with iowa_client_add_bearer_selection_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the Bearer selection object.
iowa_status_t iowa_client_remove_bearer_selection_object(iowa_context_t contextP,
                                                         iowa_sensor_t id);

// Update the Bearer selection.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the Bearer selection object.
// - flags: specify resources to update.
// - infoP: bearer selection info.
iowa_status_t iowa_client_bearer_selection_update(iowa_context_t contextP,
                                                  iowa_sensor_t id,
                                                  uint16_t flags,
                                                  iowa_bearer_selection_info_t *infoP);


#ifdef __cplusplus
}
#endif

#endif /*_IOWA_BEARER_SELECTION_INCLUDE_*/
