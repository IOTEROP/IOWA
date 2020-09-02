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

#ifndef _IOWA_PRV_DATA_INCLUDE_
#define _IOWA_PRV_DATA_INCLUDE_

#include "iowa_config.h"
#include "iowa.h"
#include "iowa_prv_lwm2m.h"

/**************************************************************
 * Data Structures and Constants
 **************************************************************/

#define DATA_IS_BLOCK(T) ((T) == IOWA_LWM2M_TYPE_STRING_BLOCK || (T) == IOWA_LWM2M_TYPE_OPAQUE_BLOCK || (T) == IOWA_LWM2M_TYPE_CORE_LINK_BLOCK)
#define DATA_BLOCK_DETAIL_TO_OPTION(D) ((D) & 0x003FFFFF)

typedef enum
{
    LWM2M_URI_DEPTH_ROOT,
    LWM2M_URI_DEPTH_OBJECT,
    LWM2M_URI_DEPTH_OBJECT_INSTANCE,
    LWM2M_URI_DEPTH_RESOURCE,
    LWM2M_URI_DEPTH_RESOURCE_INSTANCE
} lwm2m_uri_depth_t;

/**************************************************************
 * Callbacks
 **************************************************************/

typedef iowa_lwm2m_data_type_t(*data_resource_type_callback_t) (uint16_t objectID,
                                                                uint16_t resourceID,
                                                                void *callbackUserData);

/**************************************************************
 * Function to serialize / deserialize data
 * Defined in iowa_data.c
 **************************************************************/

iowa_status_t dataLwm2mSerialize(iowa_lwm2m_uri_t *baseUriP, iowa_lwm2m_data_t *dataP, size_t dataCount, iowa_content_format_t *contentFormatP, uint8_t **bufferP, size_t *bufferLengthP);

iowa_status_t dataLwm2mDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_content_format_t contentFormat, iowa_lwm2m_data_t **dataP, size_t *dataCountP, data_resource_type_callback_t resTypeCb, void *userDataP);

void dataLwm2mFree(size_t dataCount, iowa_lwm2m_data_t *dataArrayP);

/**************************************************************
 * Function which give an access to the data
 **************************************************************/

double dataUtilsPower(double number, int64_t power);
bool dataUtilsIsInt(double value);
size_t dataUtilsBufferToInt(uint8_t *buffer, size_t length, int64_t *dataP);
size_t dataUtilsIntToBuffer(int64_t data, uint8_t *buffer, size_t length, bool withExponent);
size_t dataUtilsIntToBufferLength(int64_t data, bool withExponent);

size_t dataUtilsBufferToFloat(uint8_t *buffer, size_t length, double *dataP);

size_t dataUtilsFloatToBuffer(double data, uint8_t *buffer, size_t length, bool withExponent);

size_t dataUtilsFloatToBufferLength(double data, bool withExponent);
size_t dataUtilsBufferToObjectLink(uint8_t *buffer, size_t bufferLength, iowa_lwm2m_data_t *dataP);
size_t dataUtilsObjectLinkToBuffer(iowa_lwm2m_data_t *dataP, uint8_t *buffer, size_t bufferLength);
size_t dataUtilsObjectLinkToBufferLength(iowa_lwm2m_data_t *dataP);

size_t dataUtilsBufferToUri(const char *buffer, size_t bufferLength, iowa_lwm2m_uri_t *uriP);
size_t dataUtilsUriToBuffer(iowa_lwm2m_uri_t *uriP, uint8_t *buffer, size_t bufferLength);
size_t dataUtilsUriToBufferLength(iowa_lwm2m_uri_t *uriP);

bool dataUtilsGetBaseUri(iowa_lwm2m_data_t *dataP, size_t size, iowa_lwm2m_uri_t *uriP, lwm2m_uri_depth_t *uriDepthP);

lwm2m_uri_depth_t dataUtilsGetUriDepth(iowa_lwm2m_uri_t *uriP);

iowa_status_t dataUtilsSetBuffer(uint8_t *buffer, size_t bufferLength, iowa_lwm2m_data_t *dataP, iowa_lwm2m_data_type_t type);

bool dataUtilsIsInBaseUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *baseUriP, lwm2m_uri_depth_t uriDepth);

iowa_status_t dataUtilsConvertUndefinedValue(uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t *dataP, iowa_content_format_t format, data_resource_type_callback_t resTypeCb, void *userDataP);

iowa_status_t dataUtilsGetBaseTime(iowa_lwm2m_data_t *dataP, size_t size, int32_t *basetimeP);

bool dataUtilsIsEqualUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP);

void dataUtilsGetUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP);

void dataUtilsSetUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP);

float dataUtilsConvertHalfFloatToFloat(uint16_t halfFloat);

size_t dataSkipBufferSpace(const uint8_t *bufferP, size_t bufferLength);

bool dataUtilsCompareFloatingPointNumbers(double num1, double num2, double epsilon);

#endif
