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
* Copyright (c) 2017-2019 IoTerop.
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

/*************************************************************************************
** Internal functions
*************************************************************************************/

void * objectGetData(iowa_context_t contextP,
                     uint16_t objectId)
{
    lwm2m_object_t *objectP;

    objectP = (lwm2m_object_t *)IOWA_UTILS_LIST_FIND(contextP->lwm2mContextP->objectList, listFindCallbackBy16bitsId, &objectId);
    if (objectP == NULL)
    {
        return NULL;
    }

    return objectP->userData;
}

void * objectGetInstanceData(iowa_context_t contextP,
                             uint16_t objectId,
                             uint16_t instanceId)
{
    object_data_t *dataP;

    dataP = (object_data_t *)objectGetData(contextP, objectId);
    if (dataP == NULL)
    {
        return NULL;
    }

    return IOWA_UTILS_LIST_FIND(dataP->instanceList, listFindCallbackBy16bitsId, &instanceId);
}

void objectSetRscDesc(iowa_lwm2m_resource_desc_t *rscDescP, int *ptP, uint16_t resId, iowa_lwm2m_data_type_t resType, uint8_t resOp, uint8_t flags)
{
    rscDescP[*ptP].id = resId;
    rscDescP[*ptP].type = resType;
    rscDescP[*ptP].operations = resOp;
    rscDescP[*ptP].flags = flags;
    (*ptP)++;
}

#endif // LWM2M_CLIENT_MODE
