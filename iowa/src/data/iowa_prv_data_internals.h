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
* Copyright (c) 2019-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_DATA_INTERNALS_INCLUDE_
#define _IOWA_PRV_DATA_INTERNALS_INCLUDE_

#include "iowa_prv_data.h"
#include "iowa_prv_lwm2m_internals.h"

/*************************************************************************************
** Internal functions
*************************************************************************************/

/*************************************************************************************
** External functions
*************************************************************************************/

/**************************************************************
 * Function to serialize / deserialize data in TEXT
 * Defined in iowa_text_opaque.c
 **************************************************************/

iowa_status_t textSerialize(iowa_lwm2m_data_t *dataP, uint8_t **bufferP, size_t *bufferLengthP);

iowa_status_t textDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP, data_resource_type_callback_t resTypeCb, void *userDataP);


/**************************************************************
 * Function to serialize / deserialize data in OPAQUE
 * Defined in iowa_text_opaque.c
 **************************************************************/

iowa_status_t opaqueSerialize(iowa_lwm2m_data_t *dataP, uint8_t **bufferP, size_t *bufferLengthP);

iowa_status_t opaqueDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP, data_resource_type_callback_t resTypeCb, void *userDataP);

/**************************************************************
 * Function to serialize / deserialize data in TLV
 * Defined in iowa_tlv.c
 **************************************************************/

iowa_status_t tlvSerialize(iowa_lwm2m_uri_t *baseUriP, iowa_lwm2m_data_t *dataP, size_t size, uint8_t **bufferP, size_t *bufferLengthP);

iowa_status_t tlvDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP, data_resource_type_callback_t resTypeCb, void *userDataP);

#endif
