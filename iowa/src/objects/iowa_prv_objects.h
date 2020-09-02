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
* Copyright (c) 2016-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_OBJECTS_INCLUDE_
#define _IOWA_PRV_OBJECTS_INCLUDE_

#include "iowa_client.h"

/**************************************************************
* Common API
**************************************************************/

// iowa_sensor_t <-> object / instance IDs conversion
#define OBJECT_INSTANCE_ID_TO_SENSOR(objectId, instanceId) (iowa_sensor_t)((objectId << 16) + instanceId)
#define GET_OBJECT_ID_FROM_SENSOR(sensor)                  (uint16_t)(sensor >> 16) // Both syntax are equals: ((sensor & 0xFFFF0000) >> 16), ((sensor >> 16) & 0x0000FFFF)
#define GET_INSTANCE_ID_FROM_SENSOR(sensor)                (uint16_t)(sensor & 0x0000FFFF)

/**************************************************************
*  Objects
*/

/* Device object */

// Add device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - deviceInfo: optional information of the LwM2M Client.
iowa_status_t objectDeviceInit(iowa_context_t contextP, iowa_device_info_t *deviceInfo);

// Remove device object.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
iowa_status_t objectDeviceClose(iowa_context_t contextP);

// Call factoryResetCallback given in the information of the LwM2M Client to objectDeviceInit().
// Parameters:
// - contextP: returned by iowa_init().
void objectDeviceFactoryReset(iowa_context_t contextP);

/* Firmware object */

iowa_status_t objectFirmwareContextClose(iowa_context_t contextP);
void objectFirmwareDownload(iowa_context_t contextP);
void objectFirmwareUpdate(iowa_context_t contextP);

/* Security object */

iowa_status_t objectSecurityInit(iowa_context_t contextP);
iowa_status_t objectSecurityCreate(iowa_context_t contextP, uint16_t id);
iowa_status_t objectSecurityRemove(iowa_context_t contextP, uint16_t id);
iowa_status_t objectSecurityClose(iowa_context_t contextP);

/* Server object */

#ifndef IOWA_SERVER_RSC_DISABLE_TIMEOUT_DEFAULT_VALUE
#define IOWA_SERVER_RSC_DISABLE_TIMEOUT_DEFAULT_VALUE 86400
#endif

#ifndef IOWA_SERVER_RSC_STORING_DEFAULT_VALUE
#define IOWA_SERVER_RSC_STORING_DEFAULT_VALUE false
#endif

#ifndef IOWA_SERVER_RSC_MUTE_SEND_DEFAULT_VALUE
#define IOWA_SERVER_RSC_MUTE_SEND_DEFAULT_VALUE false
#endif

// This function will initialize and create the object Server in iowa context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
iowa_status_t objectServerInit(iowa_context_t contextP);

// This function will create an instance in the object Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
// - id: the instance ID (the id must be different than IOWA_LWM2M_ID_ALL).
iowa_status_t objectServerCreate(iowa_context_t contextP, uint16_t id);

// This function will remove an instance of the object Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
// - activate: the id of the instance to be removed (if the id equal to IOWA_LWM2M_ALL, all the available instances of the object server will be removed).
iowa_status_t objectServerRemove(iowa_context_t contextP, uint16_t id);

// This function will remove the object Server from iowa context.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context.
iowa_status_t objectServerClose(iowa_context_t contextP);

/* Oscore object */

iowa_status_t objectOscoreInit(iowa_context_t contextP);
iowa_status_t objectOscoreClose(iowa_context_t contextP);

/* ACL object */

iowa_status_t objectAclInit(iowa_context_t contextP);
iowa_status_t objectAclClose(iowa_context_t contextP);

/*******************************
 * Software component object
 */

// This function will set the software component activation status.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context on which iowa_client_enable_software_component() was called.
// - activate: activated state asked.
// - swCmpIdP, swCmpIdCount: software component id to change status.
iowa_status_t objectSoftwareComponentSetActivationStatus(iowa_context_t contextP,
                                                         bool activate,
                                                         iowa_sensor_t *swCmpIdP,
                                                         size_t swCmpIdCount);

// This is the software component activation function, it will check each instance which need an activation operation and call corresponding callback.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: the IOWA context on which iowa_client_enable_software_component() was called.
iowa_status_t objectSoftwareComponentActivation(iowa_context_t contextP);

/*******************************
 * Software management object
 */

// This is the software management download function, it will check each instance which need a download operation and call corresponding callback.
// Returned value: None.
// Parameters:
// - contextP: the IOWA context on which iowa_client_enable_software_management() was called.
void objectSoftwareManagementDownload(iowa_context_t contextP);

// This is the software management install function, it will check each instance which need an install operation and call corresponding callback.
// Returned value: None.
// Parameters:
// - contextP: the IOWA context on which iowa_client_enable_software_management() was called.
void objectSoftwareManagementInstall(iowa_context_t contextP);

// This is the software management activate function, it will check each instance which need an activate operation and call corresponding callback.
// Returned value: None.
// Parameters:
// - contextP: the IOWA context on which iowa_client_enable_software_management() was called.
void objectSoftwareManagementActivate(iowa_context_t contextP);

#endif // _IOWA_PRV_OBJECTS_INCLUDE_
