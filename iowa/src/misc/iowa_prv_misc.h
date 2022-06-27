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
* Copyright (c) 2017-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_MISC_INCLUDE_
#define _IOWA_PRV_MISC_INCLUDE_

#include "iowa_config.h"
#include "iowa_utils.h"
#include "iowa_prv_core.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/**************************************************************
* Base64 API
**************************************************************/

#define BASE64_PADDING '='
#define BASE64_MODE_CLASSIC  (uint8_t)0
#define BASE64_MODE_URI_SAFE (uint8_t)1

/**************************************************************
* Sorted linked list API
**************************************************************/

typedef struct _list_16_bits_id_t
{
    struct _list_16_bits_id_t *nextP;
    uint16_t                   id;
} list_16_bits_id_t;

typedef struct _list_32_bits_id_t
{
    struct _list_32_bits_id_t *nextP;
    uint32_t                   id;
} list_32_bits_id_t;

// Callback to find a node by its ID (client side).
// Returned value: true if the node is found, false otherwise.
// Parameters:
// - nodeP: the node to check.
// - criteriaP: the ID.
bool listFindCallbackBy16bitsId(void *nodeP, void *criteriaP);

// Return the lowest unused ID in the list 'headP' (client side).
// Returned value: The lowest unused ID.
// Parameters:
// - headP: the ordered list.
uint16_t listNew16bitsId(list_16_bits_id_t *headP);

// Callback to find a node by its ID (server side).
// Returned value: true if the node is found, false otherwise.
// Parameters:
// - nodeP: the node to check.
// - criteriaP: the ID.
bool listFindCallbackBy32bitsId(void *nodeP, void *criteriaP);

// Return the lowest unused ID in the list 'headP' (server side).
// Returned value: The lowest unused ID.
// Parameters:
// - headP: the ordered list.
uint32_t listNew32bitsId(list_32_bits_id_t *headP);

#define LIST_NEW_16_BITS_ID(H) listNew16bitsId((list_16_bits_id_t *)H)
#define LIST_NEW_32_BITS_ID(H) listNew32bitsId((list_32_bits_id_t *)H)

/**************************************************************
* Utils API
**************************************************************/

// Duplicate the source string.
// Returned value: a pointer to a string on case of success or a NULL pointer.
// Parameters:
// - str: pointer to a string to duplicate.
char * utilsStrdup(const char *str);

// Change the size of the memory block passed as parameter
// Returned value: A pointer to the memory block in case of success or a NULL pointer.
// Parameters:
// - src: pointer to a memory block.
// - sizeSrc: size of the source memory block.
// - sizeDst: new size for the memory block.
void * utilsRealloc(void *src,
                    size_t sizeSrc,
                    size_t sizeDst);

// Allocates a block of memory and initializes all its bits to zero.
// Returned value: A pointer to the memory block in case of success or a NULL pointer.
// Parameters:
// - number: number of element to allocate.
// - size: size of each element.
void * utilsCalloc(size_t number,
                   size_t size);

// Allocate string memory, copy buffer to string and add the end character.
// Returned value: string wanted.
// Parameters:
// - buffer, size: buffer to convert.
char * utilsBufferToString(const uint8_t *buffer,
                           size_t size);

// Compare buffer to string.
// Returned value: true if buffer equals string, else false.
// Parameters:
// - buffer, size: buffer to compare.
// - str: string to compare.
bool utilsCmpBufferWithString(const uint8_t *buffer,
                              size_t size,
                              const char *str);

// Get string length
// Returned value: string length.
// Parameters:
// - str: string to analyze. can be null.
size_t utilsStrlen(const char *str);

// Copy a string into an other string
// Returned value: string copied size or 0 in case of error.
// Parameters:
// - buffer, length: string to fill.
// - str: string to copy. can be null.
size_t utilsStringCopy(char *buffer, size_t length, const char *str);

// Convert an Integer from network-byte order to host byte order and vice versa.
// Returned value: none.
// Parameters:
// - dst: pointer to the destination integer.
// - src: pointer to the source integer.
// - len: size in bytes of dst and src.
// Note: usually a buffer to pointer to float/integer or a pointer to float/integer to a buffer
void utilsCopyValue(void *dst, const void *src, size_t len);

// Encode a raw buffer using Base64
// Parameters:
// - rawBuffer: The raw buffer.
// - rawBufferLen: The length of the raw buffer.
// - base64Buffer: The Base64 buffer.
// - base64BufferLenP: Contains the length of the Base64 buffer. May be updated in BASE64_MODE_URI_SAFE mode.
// - mode: Either BASE64_MODE_CLASSIC or BASE64_MODE_URI_SAFE
void utils_b64Encode(uint8_t *rawBuffer, size_t rawBufferLen, uint8_t *base64Buffer, size_t *base64BufferLenP, uint8_t mode);

// Decode a Base64 buffer into a raw buffer.
// Returned value: The length of the raw buffer or 0 in case of error.
// Parameters:
// - base64Buffer: The Base64 buffer
// - base64BufferLen: The length of the Base64 buffer
// - rawBuffer: The raw buffer
// - mode: Either BASE64_MODE_CLASSIC or BASE64_MODE_URI_SAFE
size_t utils_b64Decode(uint8_t *base64Buffer, size_t base64BufferLen, uint8_t *rawBuffer, uint8_t mode);

// Calculate the length of a raw buffer based on a Base64 buffer.
// Returned value: The length of the raw buffer or 0 in case of error.
// Parameters:
// - base64Buffer: The Base64 buffer.
// - base64BufferLen: The length of the Base64 buffer.
// - withPadding: true if the base64 buffer may include padding.
size_t utils_b64GetDecodedSize(uint8_t *base64Buffer, size_t base64BufferLen, bool withPadding);

#endif // _IOWA_PRV_MISC_INCLUDE_
