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


#define OBJECT_INSTANCE_ID_TO_SENSOR(objectId, instanceId) (iowa_sensor_t)((objectId << 16) + instanceId)
#define GET_OBJECT_ID_FROM_SENSOR(sensor)                  (uint16_t)(sensor >> 16)
#define GET_INSTANCE_ID_FROM_SENSOR(sensor)                (uint16_t)(sensor & 0x0000FFFF)

/**************************************************************
*  Objects
*/

/* Device object */






iowa_status_t objectDeviceInit(iowa_context_t contextP, iowa_device_info_t *deviceInfo);





iowa_status_t objectDeviceClose(iowa_context_t contextP);




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





iowa_status_t objectServerInit(iowa_context_t contextP);






iowa_status_t objectServerCreate(iowa_context_t contextP, uint16_t id);






iowa_status_t objectServerRemove(iowa_context_t contextP, uint16_t id);





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







iowa_status_t objectSoftwareComponentSetActivationStatus(iowa_context_t contextP,
                                                         bool activate,
                                                         iowa_sensor_t *swCmpIdP,
                                                         size_t swCmpIdCount);





iowa_status_t objectSoftwareComponentActivation(iowa_context_t contextP);

/*******************************
 * Software management object
 */





void objectSoftwareManagementDownload(iowa_context_t contextP);





void objectSoftwareManagementInstall(iowa_context_t contextP);





void objectSoftwareManagementActivate(iowa_context_t contextP);

#endif
