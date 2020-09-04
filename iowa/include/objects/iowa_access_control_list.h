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
* Access Control List
*
** Description
*** This object is used to check whether the LwM2M Server has access right for performing an operation.
*
** Object Definition
*** Object Id: 2
*** Instances: Multiple
*** Optional
*************************************************************************************/

#ifndef _IOWA_ACCESS_CONTROL_LIST_INCLUDE_
#define _IOWA_ACCESS_CONTROL_LIST_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_client.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

#define IOWA_ACL_DEFAULT_ID 0

#define IOWA_ACL_NONE_RIGHT    0x00
#define IOWA_ACL_READ_RIGHT    0x01
#define IOWA_ACL_WRITE_RIGHT   0x02
#define IOWA_ACL_EXECUTE_RIGHT 0x04
#define IOWA_ACL_DELETE_RIGHT  0x08
#define IOWA_ACL_CREATE_RIGHT  0x10

/**************************************************************
 * API
 **************************************************************/

// Set the access rights for a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectId: ID of the Object.
// - instanceId: ID of the Object Instance.
// - serverId: Short Server ID or IOWA_ACL_DEFAULT_ID.
// - accessRights: New access rights to set.
iowa_status_t iowa_client_acl_rights_server_set(iowa_context_t contextP,
                                                uint16_t objectId,
                                                uint16_t instanceId,
                                                uint16_t serverId,
                                                uint8_t accessRights);

// Unset the access rights for a LwM2M Server.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectId: ID of the Object.
// - instanceId: ID of the Object Instance.
// - serverId: Short Server ID or IOWA_ACL_DEFAULT_ID.
iowa_status_t iowa_client_acl_rights_server_clear(iowa_context_t contextP,
                                                  uint16_t objectId,
                                                  uint16_t instanceId,
                                                  uint16_t serverId);

// Clear the access rights for an Object/Object Instance.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - contextP: returned by iowa_init().
// - objectId: ID of the Object.
// - instanceId: ID of the Object Instance.
iowa_status_t iowa_client_acl_rights_object_clear(iowa_context_t contextP,
                                                  uint16_t objectId,
                                                  uint16_t instanceId);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_ACCESS_CONTROL_LIST_INCLUDE_
