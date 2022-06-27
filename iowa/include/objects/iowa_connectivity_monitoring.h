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
* Connectivity monitoring
*
** Description
*** This LwM2M Object enables monitoring of parameters related to network connectivity.
*** In this general connectivity Object, the Resources are limited to the most general cases common to most network bearers.
*** It is recommended to read the description, which refers to relevant standard development organizations (e.g. 3GPP, IEEE).
*** The goal of the Connectivity Monitoring Object is to carry information reflecting the more up to date values of the current connection for monitoring purposes.
*** Resources such as Link Quality, Radio Signal Strength, Cell ID are retrieved during connected mode at least for cellular networks.
*
** Object Definition
*** Object Id: 4
*** Instances: Single
*** Optional
*************************************************************************************/

#ifndef _IOWA_CONNECTIVITY_MONITORING_INCLUDE_
#define _IOWA_CONNECTIVITY_MONITORING_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

// Defines used when declaring a new connectivity monitoring object or when updating.
#define IOWA_CONNECTIVITY_MONITORING_RSC_BEARER             (1<<0)
#define IOWA_CONNECTIVITY_MONITORING_RSC_AVAILABLE_BEARER   (1<<1)
#define IOWA_CONNECTIVITY_MONITORING_RSC_SIGNAL_STRENGTH    (1<<2)
#define IOWA_CONNECTIVITY_MONITORING_RSC_LINK_QUALITY       (1<<3)
#define IOWA_CONNECTIVITY_MONITORING_RSC_IP_ADDR            (1<<4)
#define IOWA_CONNECTIVITY_MONITORING_RSC_ROUTER_IP_ADDR     (1<<5)
#define IOWA_CONNECTIVITY_MONITORING_RSC_LINK_USAGE         (1<<6)
#define IOWA_CONNECTIVITY_MONITORING_RSC_APN                (1<<7)
#define IOWA_CONNECTIVITY_MONITORING_RSC_CELL_ID            (1<<8)
#define IOWA_CONNECTIVITY_MONITORING_RSC_SMNC               (1<<9)
#define IOWA_CONNECTIVITY_MONITORING_RSC_SMCC               (1<<10)

// Connectivity monitoring information
// Elements:
// - networkBearer: network bearer used for the current session.
// - availableNetworkBearerList: list of current available network bearers. (can be nil)
// - availableNetworkBearerNumber: number of current available network bearers.
// - radioSignalStrength: average value of the received signal strength indication.
// - linkQuality: received link quality.
// - ipAddressList: list of IP addresses assigned to the connectivity interface. (can be nil)
// - ipAddressNumber: number of IP addresses assigned to the connectivity interface.
// - routerIpAddressesList: list of IP addresses of the next-hop IP router. (can be nil)
// - routerIpAddressesNumber: number of IP addresses of the next-hop IP router.
// - linkUtilization: The percentage indicating the average utilization of the link to the next-hop IP router.
// - apnList: list of Access Point Names. (can be nil)
// - apnNumber: number of Access Point Names.
// - cellId: Serving Cell ID.
// - smnc: Serving Mobile Network Code.
// - smcc: Serving Mobile Country Code.
typedef struct
{
    uint8_t     networkBearer;
    uint8_t    *availableNetworkBearerList;
    uint8_t     availableNetworkBearerNumber;
    int16_t     radioSignalStrength;
    int16_t     linkQuality;
    char      **ipAddressList;
    uint16_t    ipAddressNumber;
    char      **routerIpAddressesList;
    uint16_t    routerIpAddressesNumber;
    uint8_t     linkUtilization;
    char      **apnList;
    uint16_t    apnNumber;
    uint64_t    cellId;
    uint16_t    smnc;
    uint16_t    smcc;
} iowa_connectivity_monitoring_info_t;

/**************************************************************
 * API
 **************************************************************/

// Add a connectivity monitoring object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - optFlags: flags used to enable optional resources.
// - idP: OUT. ID of the created instance of the connectivity monitoring object.
iowa_status_t iowa_client_add_connectivity_monitoring_object(iowa_context_t contextP,
                                                             uint16_t optFlags,
                                                             iowa_sensor_t *idP);

// Remove a connectivity monitoring object created with iowa_client_add_connectivity_monitoring_object().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the connectivity monitoring object.
iowa_status_t iowa_client_remove_connectivity_monitoring_object(iowa_context_t contextP,
                                                                iowa_sensor_t id);

// Update the connectivity monitoring.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - id: ID of the instance of the connectivity monitoring object.
// - flags: specify resources to update.
// - infoP: the connectivity monitoring information to update.
iowa_status_t iowa_client_connectivity_monitoring_update(iowa_context_t contextP,
                                                         iowa_sensor_t id,
                                                         uint16_t flags,
                                                         iowa_connectivity_monitoring_info_t *infoP);



#ifdef __cplusplus
}
#endif

#endif /*_IOWA_CONNECTIVITY_MONITORING_INCLUDE_*/
