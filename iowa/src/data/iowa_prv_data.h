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

#define HALF_FLT_EPSILON 0.0009765625 // 2^(-10)

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

// The resource data type callback. Used to know the type of a resource to validate deserialization.
// Returned value: the data resource type.
// objectID: the object the resource belongs to.
// resourceID: the resource.
// callbackUserData: passed through the deserialization function as parameter with the callback reference.
typedef iowa_lwm2m_data_type_t(*data_resource_type_callback_t) (uint16_t objectID,
                                                                uint16_t resourceID,
                                                                void *callbackUserData);

/**************************************************************
 * Function to serialize / deserialize data
 * Defined in iowa_data.c
 **************************************************************/

// Serialize LwM2M data.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: IN. the base URI of the serialized data. This can be nil.
// - dataP, dataCount: IN. data to serialize.
// - contentFormatP: IN/OUT. required content format to serialize to. It can be changed to a default content format if using the required one is not possible.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note:
// - TEXT, OPAQUE and CBOR formats can not serialize several data nor data at instance or object level
// - OPAQUE format can only serialize data with Opaque type
// - baseUriP is only used for the TLV and JSON formats
iowa_status_t dataLwm2mSerialize(iowa_lwm2m_uri_t *baseUriP, iowa_lwm2m_data_t *dataP, size_t dataCount, iowa_content_format_t *contentFormatP, uint8_t **bufferP, size_t *bufferLengthP);

// Deserialize LwM2M data.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: IN. the base URI of the serialized data. This can be nil.
// - bufferP, bufferLength: IN. payload to deserialize.
// - contentFormatP: IN. content format expected.
// - dataP, dataCount: OUT. deserialized, dynamically allocated data.
// - resTypeCb: resource data type callback called to get the type of the deserialized data. This can be nil.
// - userDataP: user data passed to resTypeCb. This can be nil.
// Note:
// - baseUriP is mandatory for TEXT, OPAQUE and CBOR format, optional for TLV and JSON format and not needed for the others.
// - data are sorted by object and instance levels
iowa_status_t dataLwm2mDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_content_format_t contentFormat, iowa_lwm2m_data_t **dataP, size_t *dataCountP, data_resource_type_callback_t resTypeCb, void *userDataP);

// Free allocated data
// Returned value: None.
// Parameters:
// - dataCount: IN. data count.
// - dataArrayP: IN. data to free.
// Note: free each allocation made in data array and the data array itself.
void dataLwm2mFree(size_t dataCount, iowa_lwm2m_data_t *dataArrayP);

/**************************************************************
 * Function which give an access to the data
 **************************************************************/

// Defined in utils.c
// Get the result of number^power
// Returned value: number^power
// Parameters:
// - number: the number
// - power: the power
double dataUtilsPower(double number, int64_t power);
bool dataUtilsIsInt(double value);
size_t dataUtilsBufferToInt(uint8_t *buffer, size_t length, int64_t *dataP);
size_t dataUtilsIntToBuffer(int64_t data, uint8_t *buffer, size_t length, bool withExponent);
size_t dataUtilsIntToBufferLength(int64_t data, bool withExponent);

// Convert a buffer to a float
// Returned value: '1' if the buffer has been converted, '0' otherwise
// Parameters:
// - buffer: the buffer to convert
// - length: the length of the buffer
// - dataP: OUT. the resulted float number
size_t dataUtilsBufferToFloat(uint8_t *buffer, size_t length, double *dataP);

// Convert a float to a buffer
// Returned value: the length of the buffer
// Parameters:
// - data: the float to convert
// - buffer: the buffer
// - length: the length of the buffer
// - withExponent: boolean to use the exponent form
size_t dataUtilsFloatToBuffer(double data, uint8_t *buffer, size_t length, bool withExponent);

// Get the buffer length of a float number
// Returned value: the length of the buffer
// Parameters:
// - data: the float to convert
// - withExponent: boolean to use the exponent form
size_t dataUtilsFloatToBufferLength(double data, bool withExponent);
size_t dataUtilsBufferToObjectLink(uint8_t *buffer, size_t bufferLength, iowa_lwm2m_data_t *dataP);
size_t dataUtilsObjectLinkToBuffer(iowa_lwm2m_data_t *dataP, uint8_t *buffer, size_t bufferLength);
size_t dataUtilsObjectLinkToBufferLength(iowa_lwm2m_data_t *dataP);

size_t dataUtilsBufferToUri(const char *buffer, size_t bufferLength, iowa_lwm2m_uri_t *uriP);
size_t dataUtilsUriToBuffer(iowa_lwm2m_uri_t *uriP, uint8_t *buffer, size_t bufferLength);
size_t dataUtilsUriToBufferLength(iowa_lwm2m_uri_t *uriP);

// Get Base Uri
// Returned value: true if each Uri information from LwM2M data have an Uri Depth lower than uriDepthP, else return false.
// Parameters:
// - dataP: an array of resources targeted by the operation along with their values.
// - size: the number of elements in dataP.
// - uriP: OUT. base Uri result.
// - uriDepthP: OUT. Uri depth.
bool dataUtilsGetBaseUri(iowa_lwm2m_data_t *dataP, size_t size, iowa_lwm2m_uri_t *uriP, lwm2m_uri_depth_t *uriDepthP);

// Get the URI depth
// Returned value: the URI depth
// Parameters:
// - uriP: the URI used to retrieve the URI depth
lwm2m_uri_depth_t dataUtilsGetUriDepth(iowa_lwm2m_uri_t *uriP);

// Set the buffer of a data
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - buffer: the buffer to use for the data
// - bufferLength: the length of the buffer
// - dataP: the data where the buffer has to be set
// - type: the data type to set
iowa_status_t dataUtilsSetBuffer(uint8_t *buffer, size_t bufferLength, iowa_lwm2m_data_t *dataP, iowa_lwm2m_data_type_t type);

// Inform if the data is in the base URI
// Returned value: true if the data is in the base URI, false otherwise
// Parameters:
// - dataP: the data to check in the base URI
// - baseUriP: the base URI
// - uriDepth: the URI depth of the base URI
bool dataUtilsIsInBaseUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *baseUriP, lwm2m_uri_depth_t uriDepth);

// Convert an undefined depending of a content format
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - bufferP: the buffer to convert
// - bufferLength: the length of the buffer
// - dataP: the resulted data
// - format: the content format to use to interpret the undefiend value
// - resTypeCb: the callback used to retrieve a resource type
// - userDataP: the user data sent to the resource type callback
iowa_status_t dataUtilsConvertUndefinedValue(uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t *dataP, iowa_content_format_t format, data_resource_type_callback_t resTypeCb, void *userDataP);

// Get Base time of dataP's timestamp
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP: an array of resources targeted by the operation along with their values.
// - size: the number of elements in dataP.
// - basetimeP: OUT. base time result in second.
iowa_status_t dataUtilsGetBaseTime(iowa_lwm2m_data_t *dataP, size_t size, int32_t *basetimeP);

// Compare LwM2M Uri information with LwM2M Data.
// Returned value: if LwM2M Uri information is the same than in LwM2M Data return true, else return false.
// Parameters:
// - dataP: IN. LwM2M Data.
// - uriP: IN. LwM2M Uri.
bool dataUtilsIsEqualUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP);

// Get LwM2M Uri information from LwM2M Data.
// Returned value: none.
// Parameters:
// - dataP: IN. LwM2M Data.
// - uriP: OUT. LwM2M Uri.
void dataUtilsGetUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP);

// Set LwM2M Uri information in LwM2M Data.
// Returned value: none.
// Parameters:
// - dataP: OUT. LwM2M Data.
// - uriP: IN. LwM2M Uri.
void dataUtilsSetUri(iowa_lwm2m_data_t *dataP, iowa_lwm2m_uri_t *uriP);

// Convert half float data into float data
// Returned value: float conversion
// Parameters:
// - halfFloat: 16bits of an half float data
float dataUtilsConvertHalfFloatToFloat(uint16_t halfFloat);

// Search in a buffer the first character location which is not a space a.k.a. skip space
// Returned value: The first character location which is not a space
// Parameters:
// - bufferP: The buffer to search the next element
// - bufferP: The size of the buffer
size_t dataSkipBufferSpace(const uint8_t *bufferP, size_t bufferLength);

// Compare floating point numbers
// Returned value: if numbers are nearly equal return true, else return false
// Parameters:
// - num1: the first floating point number to compare
// - num2: the second floating point number to compare
// - precision: floating number precision
bool dataUtilsCompareFloatingPointNumbers(double num1, double num2, double epsilon);

#endif // _IOWA_PRV_DATA_INCLUDE_
