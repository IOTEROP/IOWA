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
* APN connection profile
*
** Description
*** This object specifies resources to enable a device to connect to an APN.
*
** Object Definition
*** Object Id: 11
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_APN_CONNECTION_PROFILE_INCLUDE_
#define _IOWA_APN_CONNECTION_PROFILE_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/
#define IOWA_APN_CONNECTION_PROFILE_RSC_PROFILE_NAME                            (1<<0)
#define IOWA_APN_CONNECTION_PROFILE_RSC_APN                                     (1<<1)
#define IOWA_APN_CONNECTION_PROFILE_RSC_AUTO_SELECT_APN_DEVICE                  (1<<2)
#define IOWA_APN_CONNECTION_PROFILE_RSC_ENABLE_STATUS                           (1<<3)
#define IOWA_APN_CONNECTION_PROFILE_RSC_AUTHENTICATION_TYPE                     (1<<4)
#define IOWA_APN_CONNECTION_PROFILE_RSC_USER_NAME                               (1<<5)
#define IOWA_APN_CONNECTION_PROFILE_RSC_SECRET                                  (1<<6)
#define IOWA_APN_CONNECTION_PROFILE_RSC_RECONNECT_SCHEDULE                      (1<<7)
#define IOWA_APN_CONNECTION_PROFILE_RSC_VALIDITY                                (1<<8)
#define IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_TIME                 (1<<9)
#define IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_RESULT               (1<<10)
#define IOWA_APN_CONNECTION_PROFILE_RSC_CONN_ESTABLISHMENT_REJECT_CAUSE         (1<<11)
#define IOWA_APN_CONNECTION_PROFILE_RSC_CONNECTION_END_TIME                     (1<<12)
#define IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_BYTES_SENT                        (1<<13)
#define IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_BYTES_RECEIVED                    (1<<14)
#define IOWA_APN_CONNECTION_PROFILE_RSC_IP_ADDRESS                              (1<<15)
#define IOWA_APN_CONNECTION_PROFILE_RSC_PREFIX_LENGTH                           (1<<16)
#define IOWA_APN_CONNECTION_PROFILE_RSC_SUBNET_MASK                             (1<<17)
#define IOWA_APN_CONNECTION_PROFILE_RSC_GATEWAY                                 (1<<18)
#define IOWA_APN_CONNECTION_PROFILE_RSC_PRIMARY_DNS_ADDRESS                     (1<<19)
#define IOWA_APN_CONNECTION_PROFILE_RSC_SECONDARY_DNS_ADDRESS                   (1<<20)
#define IOWA_APN_CONNECTION_PROFILE_RSC_QCI                                     (1<<21)
#define IOWA_APN_CONNECTION_PROFILE_RSC_TOTAL_PACKETS_SENT                      (1<<22)
#define IOWA_APN_CONNECTION_PROFILE_RSC_PDN_TYPE                                (1<<23)
#define IOWA_APN_CONNECTION_PROFILE_RSC_APN_RATE_CONTROL                        (1<<24)

// APN connection profile details
// Elements:
// - apn: APN of the APN connection profile.
// - autoSelect: it enables the device to choose an APN according to a device specific algorithm.
// - enableStatus: allows the profile to be remotely activated or deactivated.
// - authenticationType: 0: PAP, 1: CHAP, 2: PAP or CHAP, 3: None
// - userName: used with e.g. PAP.
// - secret: used with e.g. PAP.
// - reconnectSchedule: list of retry delay values in seconds to be used in case of unsuccessful connection establishment attempts.
// - validity: coma separated mobile country code, then mobile network code.
// - connectionEstablishmentTime: UTC time of connection request.
// - connectionEstablishmentResult: 0 = accepted, 1 = rejected.
// - connectionEstablishmentRejectCause: Reject cause.
// - connectionEndTime: UTC time of connection end.
// - totalBytesSent: rolling counter for total number of bytes sent via this interface since last device reset.
// - totalBytesReceived: rolling counter for total number of bytes sent via this interface since last device reset.
// - ipAddress: may be IPv4 or IPv6 address.
// - prefixLength: associated with IPv6 address.
// - subnetMask: subnet mask.
// - gateway: gateway.
// - primaryDnsAddress: primary DNS address.
// - secondaryDnsAddress: secondary DNS address.
// - qci: Quality of service Class Identifier. For LTE and NB-IoT only.
// - totalPacketsSent: rolling counter for total number of packets sent via this interface since last device reset.
// - pdnType: 0 = Non-IP, 1 = IPv4, 2 = IPv6, 3 = IPv4v6.
// - apnRateControl: number of allowed uplink PDU transmissions per time interval per APN.
typedef struct
{
    char       *apn;
    bool        autoSelect;
    bool        enableStatus;
    int         authenticationType;
    char       *userName;
    char       *secret;
    char       *reconnectSchedule;
    char      **validityList;
    uint16_t    validityNumber;
    int        *connectionEstablishmentTimeList;
    uint16_t    connectionEstablishmentTimeNumber;
    int        *connectionEstablishmentResultList;
    uint16_t    connectionEstablishmentResultNumber;
    int        *connectionEstablishmentRejectCauseList;
    uint16_t    connectionEstablishmentRejectCauseNumber;
    int        *connectionEndTimeList;
    uint16_t    connectionEndTimeNumber;
    int         totalBytesSent;
    int         totalBytesReceived;
    char      **ipAddressList;
    uint16_t    ipAddressNumber;
    char      **prefixLengthList;
    uint16_t    prefixLengthNumber;
    char      **subnetMaskList;
    uint16_t    subnetMaskNumber;
    char      **gatewayList;
    uint16_t    gatewayNumber;
    char      **primaryDnsAddressList;
    uint16_t    primaryDnsAddressNumber;
    char      **secondaryDnsAddressList;
    uint16_t    secondaryDnsAddressNumber;
    int         qci;
    int         totalPacketsSent;
    int         pdnType;
    int         apnRateControl;
} iowa_apn_connection_profile_details_t;

// The callback called to change the state of the APN connection profile.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - profileName: unique name of the APN connection profile. This may be new.
// - operation: the operation performed by the Server on this APN connection profile (creation, deletion, or write)
// - flags: specify values set in detailsP.
// - detailsP: APN connection profile details. This may be nil.
// - userDataCallback: user data callback.
// - contextP: the IOWA context.
typedef iowa_status_t(*iowa_apn_connection_profile_update_callback_t)(char *profileName,
                                                                      iowa_dm_operation_t operation,
                                                                      uint32_t flags,
                                                                      iowa_apn_connection_profile_details_t *detailsP,
                                                                      void *userDataCallback,
                                                                      iowa_context_t contextP);

/**************************************************************
 * API
 **************************************************************/

// Enable APN connection profiles management.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - updateCallback: callback used when a Server modify APN connection profiles.
// - userDataCallback: user data callback.
iowa_status_t iowa_client_enable_apn_connection_profile_object(iowa_context_t contextP,
                                                               iowa_apn_connection_profile_update_callback_t updateCallback,
                                                               void *userDataCallback);

// Disable APN connection profiles management.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t iowa_client_disable_apn_connection_profile_object(iowa_context_t contextP);

// Add an APN connection profile.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - profileName: unique name of the APN connection profile.
// - optFlags: specify values set in detailsP.
// - detailsP: apn connection profile details.
iowa_status_t iowa_client_add_apn_connection_profile(iowa_context_t contextP,
                                                     const char *profileName,
                                                     uint32_t optFlags,
                                                     iowa_apn_connection_profile_details_t *detailsP);

// Remove an APN connection profile object created with iowa_client_add_apn_connection_profile().
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - profileName: unique name of the APN connection profile.
iowa_status_t iowa_client_remove_apn_connection_profile(iowa_context_t contextP,
                                                        const char *profileName);

// Update an APN connection profile.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - profileName: unique name of the APN connection profile.
// - flags: specify resources to update.
// - detailsP: apn connection profile details.
iowa_status_t iowa_client_update_apn_connection_profile(iowa_context_t contextP,
                                                        const char *profileName,
                                                        uint32_t flags,
                                                        iowa_apn_connection_profile_details_t *detailsP);

// Retrieve the LwM2M Object Link to an APN connection profile.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - profileName: unique name of the APN connection profile.
// - objectLinkP: OUT. LwM2M Object Link to the APN connection profile.
iowa_status_t iowa_client_get_apn_connection_profile_object_link(iowa_context_t contextP,
                                                                 const char *profileName,
                                                                 iowa_lwm2m_object_link_t *objectLinkP);

#ifdef __cplusplus
}
#endif

#endif /*_IOWA_APN_CONNECTION_PROFILE_INCLUDE_*/
