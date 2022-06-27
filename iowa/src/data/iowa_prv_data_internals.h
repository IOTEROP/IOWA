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

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa_prv_data.h"
#include "iowa_prv_lwm2m_internals.h"

/*************************************************************************************
** Internal functions
*************************************************************************************/

/****************************************
* CBOR and SenML CBOR Serialization/Deserialization
* Defined in iowa_cbor.c
*****************************************/

// Constants needed for CBOR serialization/Deserialization
#define CBOR_ERROR                     -1
#define CBOR_NO_ERROR                  0
#define CBOR_NO_ERROR_INDEFINITE       1

#define CBOR_NUMBER_MAX                UINT64_MAX
#define CBOR_BYTE_1_SIZE               1
#define CBOR_BYTE_2_SIZE               2
#define CBOR_BYTE_4_SIZE               4
#define CBOR_BYTE_8_SIZE               8

// Constants for CBOR Add information
#define CBOR_ADD_INFO_MASK             0x1FU    // 00011111
#define CBOR_ADD_INFO_1_BYTE           24       // next byte is value with uint8_t format (between 32 and 255)
#define CBOR_ADD_INFO_2_BYTES          25       // next bytes are a 2-byte value
#define CBOR_ADD_INFO_4_BYTES          26       // next bytes are a 4-byte value
#define CBOR_ADD_INFO_8_BYTES          27       // next bytes are a 8-byte value
#define CBOR_ADD_INFO_VALUE_BREAK      31       // break
#define CBOR_ADD_INFO_DECIMAL_FRAC     4

#define CBOR_DECIMAL_FRAC_ARRAY_LENGTH 2

// Constants for CBOR Major Type
#define CBOR_MAJOR_TYPE_MASK           0xE0U // 111000000
#define CBOR_MAJOR_TYPE_BIT_SHIFT      5U    // b7b6b5b4b3b2b1b0 => 111000000

typedef enum
{
    CBOR_MAJOR_TYPE_NONE                     = -1,
    CBOR_MAJOR_TYPE_UNSIGNED_INTEGER         = 0,
    CBOR_MAJOR_TYPE_NEGATIVE_INTEGER         = 1,
    CBOR_MAJOR_TYPE_BYTE_STRING              = 2,
    CBOR_MAJOR_TYPE_TEXT_STRING              = 3,
    CBOR_MAJOR_TYPE_ARRAY_OF_ITEMS           = 4,
    CBOR_MAJOR_TYPE_MAP_OF_PAIRS_OF_ITEMS    = 5,
    CBOR_MAJOR_TYPE_OPTIONAL_SEMANTIC        = 6,
    CBOR_MAJOR_TYPE_FLOAT_OR_SIMPLE_DATA     = 7,
}major_type_t;

#define CBOR_GET_ITEM_INITIAL_BYTE(MAJOR_TYPE,ADD_INFO) \
(uint8_t)((((uint8_t)(MAJOR_TYPE)<<CBOR_MAJOR_TYPE_BIT_SHIFT)&CBOR_MAJOR_TYPE_MASK)+((ADD_INFO)&CBOR_ADD_INFO_MASK))

#define CBOR_SERIALIZATION_TEST_FUNCTION_RESULT(RES)        \
{                                                           \
    if (RES <= CBOR_ERROR)                                  \
    {                                                       \
        IOWA_LOG_WARNING(IOWA_PART_DATA, "test failed.");   \
        goto exit_error;                                    \
    }                                                       \
}

/****************************
* Used for CBOR and SenML CBOR serialization
*/

// Add Text string to a cbor buffer.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - stringP, stringSize: the string to add.
// - bufferP: buffer in which the number will be added.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
// - isByteString: indicate if major type is Text string or Byte string
int8_t cborAddStringToBuffer(uint8_t *stringP, size_t stringSize, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP, bool isByteString);

// Get size that the number could take on cbor buffer.
// Returned value: size that the number could take on cbor buffer.
// Parameters:
// - number: the number to put in a buffer.
size_t cborGetNumberToBufferLength(uint64_t number);

// Add number to a cbor buffer.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - majorType: major type of the number.
// - number: the number to add.
// - bufferP: buffer in which the number will be added.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborAddNumberToBuffer(major_type_t majorType, uint64_t number, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

// Get size that the float number could take on cbor buffer.
// Returned value: size that the number could take on cbor buffer.
// Parameters:
// - number: the number to put in a buffer.
size_t cborGetFloatToBufferLength(double number);

// Add float number to a cbor buffer.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - number: the number to add.
// - bufferP: buffer in which the number will be added.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborAddFloatToBuffer(double number, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

// Get size that the LwM2M data could take on cbor buffer.
// Returned value: size that the number could take on cbor buffer.
// Parameters:
// - dataP: the data to put in a buffer.
size_t cborGetDataToBufferLength(iowa_lwm2m_data_t *dataP);

// Add LwM2M Data to a cbor buffer.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - dataP: the data to add.
// - bufferP: buffer in which the data will be added.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborAddDataToBuffer(iowa_lwm2m_data_t *dataP, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

/****************************
* Used for CBOR and SenML CBOR deserialization
*/

// Put cbor buffer to number.
// Returned value: major type of the number in case of success else CBOR_MAJOR_TYPE_NONE if any error.
// Parameters:
// - number: the number in which the buffer will be put.
// - bufferNumberP: the buffer to get number.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
major_type_t cborPutBufferToNumber(uint64_t *numberP, uint8_t *bufferNumberP, size_t bufferLength, size_t *bufferIndexP);

// Get size that the string could take on cbor buffer.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - majorType: major type of the string (CBOR_MAJOR_TYPE_BYTE_STRING or CBOR_MAJOR_TYPE_TEXT_STRING).
// - convertResult: initial byte of cbor item.
// - stringLengthP: the string length.
// - bufferP: the buffer to get string.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborGetBufferToStringLength(major_type_t majorType, uint64_t convertResult, size_t *stringLengthP, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

// Put cbor buffer to a string.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - majorType: major type of the string (CBOR_MAJOR_TYPE_BYTE_STRING or CBOR_MAJOR_TYPE_TEXT_STRING).
// - convertResult: initial byte of cbor item.
// - stringP, stringLengthP: the string in which the buffer will be put.
// - bufferP: the buffer to get string.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborPutBufferToString(major_type_t majorType, uint64_t convertResult, uint8_t **stringP, size_t *stringLengthP, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

// Get value number from float major type cbor buffer.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - convertResult: initial byte of cbor item.
// - dataP: data in which the buffer will be put.
// - bufferP: the buffer to get data.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborGetValueFromFloatMajorType(uint64_t convertResult, iowa_lwm2m_data_t *dataP, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

// Put cbor buffer to a LwM2M Data.
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - majorType: major type of the value
// - convertResult: initial byte of cbor item.
// - dataP: data in which the buffer will be put.
// - bufferP: the buffer to get data.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
int8_t cborPutBufferToData(major_type_t majorType, uint64_t convertResult, iowa_lwm2m_data_t *dataP, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

// Handle decimal fraction, put buffer decimal fraction into data.value.asFloat
// Returned value: CBOR_NO_ERROR in case of success else CBOR_ERROR if any error.
// Parameters:
// - dataP: data in which the buffer will be put.
// - bufferP: the buffer to get data.
// - bufferLength: maximal size of the buffer.
// - bufferIndexP: current buffer index.
// Note: only if majorType is CBOR_MAJOR_TYPE_OPTIONAL_SEMANTIC with tag CBOR_ADD_INFO_DECIMAL_FRAC
int8_t cborHandleDecimalFraction(iowa_lwm2m_data_t *dataP, uint8_t *bufferP, size_t bufferLength, size_t *bufferIndexP);

/*************************************************************************************
** External functions
*************************************************************************************/


// Convert data values to the correct type.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataCount: IN. data count.
// - dataArray: IN. data to consolidate.
// - contentFormat: the original content format.
// - resTypeCb: resource data type callback called to get the type of the deserialized data. This can be nil.
// - userDataP: user data passed to resTypeCb. This can be nil.
iowa_status_t dataLwm2mConsolidate(size_t dataCount, iowa_lwm2m_data_t *dataArray, iowa_content_format_t contentFormat, data_resource_type_callback_t resTypeCb, void *userDataP);

/**************************************************************
 * Function to serialize / deserialize data in TEXT
 * Defined in iowa_text_opaque.c
 **************************************************************/

// Convert LwM2M data into TEXT buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note: Support string, opaque, integer, float, boolean, core link, object link, time and unsigned integer type
iowa_status_t textSerialize(iowa_lwm2m_data_t *dataP, uint8_t **bufferP, size_t *bufferLengthP);

// Convert TEXT buffer into LwM2M data.
// The LwM2M data type is set to IOWA_LWM2M_TYPE_STRING.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: URI of the data. Can not be at instance nor object level. Can not be the Root path.
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
iowa_status_t textDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);


/**************************************************************
 * Function to serialize / deserialize data in OPAQUE
 * Defined in iowa_text_opaque.c
 **************************************************************/

// Convert LwM2M data into OPAQUE buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note: Support opaque, undefined type
iowa_status_t opaqueSerialize(iowa_lwm2m_data_t *dataP, uint8_t **bufferP, size_t *bufferLengthP);

// Convert OPAQUE buffer into  data.
// The LwM2M data type is set to IOWA_LWM2M_TYPE_OPAQUE.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: URI of the data. Can not be at instance nor object level. Can not be the Root path.
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
iowa_status_t opaqueDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);


/**************************************************************
 * Function to serialize / deserialize data in CBOR
 * Defined in iowa_cbor.c
 **************************************************************/

// Convert LwM2M data into CBOR buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note: Support string, opaque, integer, float, boolean, core link, object link, time and unsigned integer type
iowa_status_t cborSerialize(iowa_lwm2m_data_t *dataP, uint8_t **bufferP, size_t *bufferLengthP);

// Convert CBOR buffer into LwM2M data.
// The LwM2M data type is set to the CBOR data type.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: URI of the data. Can not be at instance nor object level. Can not be the Root path.
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
// Note:
// - Support Half float, decimal fraction, null and undefined value
iowa_status_t cborDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);


/**************************************************************
 * Function to serialize / deserialize data in SenML JSON
 * Defined in iowa_senml_json.c
 **************************************************************/

// Convert LwM2M data into SenML JSON buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP, size: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note:
// - Support string, opaque, integer, float, boolean, core link, object link, time, unsigned integer type
// - Base name is only present when there are several data
// - Support timestamp, URI only
iowa_status_t senmlJsonSerialize(iowa_lwm2m_data_t *dataP, size_t size, uint8_t **bufferP, size_t *bufferLengthP);

// Convert SenML JSON buffer into LwM2M data.
// The LwM2M data type is set to the SenML data type.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
// Note:
// - Support floating timestamp, URI only
// - Handle excessive space
iowa_status_t senmlJsonDeserialize(uint8_t *buffer, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);


/**************************************************************
 * Function to serialize / deserialize data in SenML CBOR
 * Defined in iowa_senml_cbor.c
 **************************************************************/

// Convert LwM2M data into SenML CBOR buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP, size: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note:
// - Support string, opaque, integer, float, boolean, core link, object link, time, unsigned integer type
// - Base name is only present when there are several data
// - Support timestamp, URI only
iowa_status_t senmlCborSerialize(iowa_lwm2m_data_t *dataP, size_t size, uint8_t **bufferP, size_t *bufferLengthP);

// Convert SenML CBOR buffer into LwM2M data.
// The LwM2M data type is set to the SenML data type.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
// Note:
// - Support floating timestamp, URI only
// - Support Half float, decimal fraction, null and undefined value
// - Support Indefinite size of array, map, text string and byte string
iowa_status_t senmlCborDeserialize(uint8_t *buffer, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);

/**************************************************************
 * Function to serialize / deserialize data in LwM2M CBOR
 * Defined in iowa_lwm2m_cbor.c
 **************************************************************/

// Convert LwM2M data into LwM2M CBOR buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: URI targeted by the operation
// - dataP, size: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
iowa_status_t lwm2mCborSerialize(iowa_lwm2m_uri_t *baseUriP, iowa_lwm2m_data_t *dataP, size_t size, uint8_t **bufferP, size_t *bufferLengthP);

// Convert LwM2M CBOR buffer into LwM2M data.
// The LwM2M data type is set to the CBOR data type.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: URI targeted by the operation
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
iowa_status_t lwm2mCborDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);

/**************************************************************
 * Function to serialize / deserialize data in TLV
 * Defined in iowa_tlv.c
 **************************************************************/

// Convert LwM2M data into TLV buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: the base URI of the serialized data. Can not be the Root path.
// - dataP, size: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note:
// - Support string, opaque, integer, float, boolean, core link, object link, time, unsigned integer type
// - If a data is not corresponding to the base uri, it is ignored
// - data should be at resource or resource instance level
iowa_status_t tlvSerialize(iowa_lwm2m_uri_t *baseUriP, iowa_lwm2m_data_t *dataP, size_t size, uint8_t **bufferP, size_t *bufferLengthP);

// Convert TLV buffer into LwM2M data.
// The LwM2M data type is set to IOWA_LWM2M_TYPE_UNDEFINED.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: the base URI of the serialized data. This can be nil.
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
iowa_status_t tlvDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);


/**************************************************************
 * Function to serialize / deserialize data in JSON
 * Defined in iowa_json.c
 **************************************************************/

// Convert LwM2M data into JSON buffer.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: the base URI of the serialized data. Can not be the Root path.
// - dataP, size: data to serialize.
// - bufferP, bufferLengthP: OUT. serialized, dynamically allocated payload.
// Note:
// - Support string, opaque, integer, float, boolean, core link, object link, time, unsigned integer type
// - Support timestamp
// - data should be at resource or resource instance level
iowa_status_t jsonSerialize(iowa_lwm2m_uri_t *baseUriP, iowa_lwm2m_data_t *dataP, size_t size, uint8_t **bufferP, size_t *bufferLengthP);

// Convert JSON buffer into LwM2M data.
// The LwM2M data type is set to the SenML data type.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - baseUriP: the base URI of the serialized data. This can be nil.
// - bufferP, bufferLength: payload to deserialize.
// - dataP, dataCount: OUT. data deserialized, dynamically allocated.
// Note:
// - Support floating timestamp
// - Handle excessive space
// - if no base name found in buffer and no base URI set, an error occurred
// - if a base name has been found, the base URI is ignored
iowa_status_t jsonDeserialize(iowa_lwm2m_uri_t *baseUriP, uint8_t *bufferP, size_t bufferLength, iowa_lwm2m_data_t **dataP, size_t *dataCountP);

#ifdef __cplusplus
}
#endif

#endif // _IOWA_PRV_DATA_INTERNALS_INCLUDE_
