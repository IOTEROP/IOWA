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
* Copyright (c) 2018-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_objects_internals.h"

#ifdef LWM2M_CLIENT_MODE

#define PRV_RSC_NUMBER 11

/*************************************************************************************
** Private functions
*************************************************************************************/

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t objectSecurityInit(iowa_context_t contextP)
{
    iowa_status_t result;
    iowa_lwm2m_resource_desc_t resources[PRV_RSC_NUMBER];
    int currentPt;
    const uint16_t nbrRes = PRV_RSC_NUMBER
                            - 1
                            ;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Init security object.");

    // Get the resources list
    currentPt = 0;

    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, URI, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, BOOTSTRAP, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, SECURITY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, PUBLIC_KEY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, SERVER_PUBLIC_KEY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, SECRET_KEY, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, SMS_SERVER_NUMBER, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, SHORT_SERVER_ID, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, HOLD_OFF, resources, currentPt);
    SET_LWM2M_DESC_T_TO_OBJECT_RSC(IOWA_LWM2M_SECURITY, BOOTSTRAP_TIMEOUT, resources, currentPt);

    // Inform the stack
    result = customObjectAdd(contextP,
                             IOWA_LWM2M_SECURITY_OBJECT_ID,
                             OBJECT_MULTIPLE,
                             0, NULL,
                             nbrRes, resources,
                             NULL,
                             NULL,
                             NULL,
                             NULL);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectSecurityCreate(iowa_context_t contextP,
                                   uint16_t id)
{
    iowa_status_t result;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Adding new security object.");

    result = objectAddInstance(contextP,
                               IOWA_LWM2M_SECURITY_OBJECT_ID,
                               id,
                               0, NULL);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectSecurityRemove(iowa_context_t contextP,
                                   uint16_t id)
{
    iowa_status_t result;

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Removing security object (instance: %d).", id);

    result = objectRemoveInstance(contextP,
                                  IOWA_LWM2M_SECURITY_OBJECT_ID,
                                  id);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t objectSecurityClose(iowa_context_t contextP)
{
    // WARNING: This function is called in a critical section
    iowa_status_t result;

    IOWA_LOG_INFO(IOWA_PART_OBJECT, "Closing security object.");

    result = customObjectRemove(contextP, IOWA_LWM2M_SECURITY_OBJECT_ID);

    IOWA_LOG_ARG_INFO(IOWA_PART_OBJECT, "Exiting with code %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

#endif
