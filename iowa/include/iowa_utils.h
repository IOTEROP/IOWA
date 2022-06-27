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

#ifndef _IOWA_UTILS_
#define _IOWA_UTILS_

#ifdef __cplusplus
extern "C" {
#endif

#include "iowa.h"

/**************************************************************
 * Base64 encoding / decoding APIs
 **************************************************************/

// Calculate the length of a Base64 buffer based on a raw buffer represented by its length.
// Returned value: The length of the Base64 buffer.
// Parameters:
// - rawBufferLen: The length of the raw buffer.
size_t iowa_utils_base64_get_encoded_size(size_t rawBufferLen);

// Calculate the length of a raw buffer based on a Base64 buffer.
// Returned value: The length of the raw buffer or 0 in case of error.
// Parameters:
// - base64Buffer: The Base64 buffer.
// - base64BufferLen: The length of the Base64 buffer.
size_t iowa_utils_base64_get_decoded_size(uint8_t *base64Buffer,
                                          size_t base64BufferLen);

// Encode a raw buffer using Base64
// Returned value: The length of the Base64 buffer or 0 in case of error.
// Parameters:
// - rawBuffer: The raw buffer.
// - rawBufferLen: The length of the raw buffer.
// - base64Buffer: The Base64 buffer.
// - base64BufferLen: The length of the Base64 buffer.
size_t iowa_utils_base64_encode(uint8_t * rawBuffer,
                                size_t rawBufferLen,
                                uint8_t * base64Buffer,
                                size_t base64BufferLen);

// Decode a Base64 buffer into a raw buffer.
// Returned value: The length of the raw buffer or 0 in case of error.
// Parameters:
// - base64Buffer: The Base64 buffer
// - base64BufferLen: The length of the Base64 buffer
// - rawBuffer: The raw buffer
// - rawBufferLen: The length of the raw buffer
size_t iowa_utils_base64_decode(uint8_t * base64Buffer,
                                size_t base64BufferLen,
                                uint8_t * rawBuffer,
                                size_t rawBufferLen);

/**************************************************************
 * List APIs
 **************************************************************/

// List structure.
// nextP: pointer to the next element in the list.
typedef struct _iowa_list_t
{
    struct _iowa_list_t *nextP;
} iowa_list_t;

// Callback to free a node.
// nodeP: the node to free.
typedef void(*iowa_list_node_free_callback_t) (void *nodeP);

// Callback to find a node.
// Returned value: true if this is the node, false otherwise.
// nodeP: the node to check.
// criteriaP: the criteria used to determine if this is the corresponding node.
typedef bool(*iowa_list_node_find_callback_t) (void *nodeP,
                                               void *criteriaP);

/****************************
 * Functions
 */

// Add a node to a list.
// Returned value: the list with the new element.
// Parameters:
// - headP: head of the current list.
// - nodeP: node to add to the list.
iowa_list_t * iowa_utils_list_add(iowa_list_t *headP,
                                  iowa_list_t *nodeP);

// Remove a node from a list.
// Returned value: the list updated.
// Parameters:
// - headP: head of the current list.
// - nodeP: node to remove from the list.
iowa_list_t * iowa_utils_list_remove(iowa_list_t *headP,
                                     iowa_list_t *nodeP);

// Free a list.
// Returned value: none.
// Parameters:
// - headP: List to free.
// - freeCb: Callback used to free the list.
void iowa_utils_list_free(iowa_list_t *headP,
                          iowa_list_node_free_callback_t freeCb);

// Find a node in a list.
// Returned value: the node if found.
// Parameters:
// - headP: List to search on.
// - findCb: Callback used to find in the list.
// - criteriaP: Criteria used in the find callback to search the node.
iowa_list_t * iowa_utils_list_find(iowa_list_t *headP,
                                   iowa_list_node_find_callback_t findCb,
                                   void *criteriaP);

// Remove and return the first node of a list matching the criteria.
// Returned value: the list updated.
// Parameters:
// - headP: head of the current list.
// - findCb: Callback used to find in the list.
// - criteriaP: Criteria used in the find callback to search the node.
// - nodeP: OUT. Node removed from the list. Can be nil.
iowa_list_t * iowa_utils_list_find_and_remove(iowa_list_t *headP,
                                              iowa_list_node_find_callback_t findCb,
                                              void *criteriaP,
                                              iowa_list_t **nodeP);

/****************************
 * Macros
 */

// Equivalent macros functions of the list APIs. Used to automatically cast the user list type to `iowa_list_t`

#define IOWA_UTILS_LIST_ADD(H, N)                   iowa_utils_list_add((iowa_list_t *)(H), (iowa_list_t *)(N))
#define IOWA_UTILS_LIST_REMOVE(H, N)                iowa_utils_list_remove((iowa_list_t *)(H), (iowa_list_t *)(N))
#define IOWA_UTILS_LIST_FREE(H, F)                  iowa_utils_list_free((iowa_list_t *)(H), (iowa_list_node_free_callback_t)(F))
#define IOWA_UTILS_LIST_FIND(H, F, C)               iowa_utils_list_find((iowa_list_t *)(H), (iowa_list_node_find_callback_t)(F), (void *)(C))
#define IOWA_UTILS_LIST_FIND_AND_REMOVE(H, F, C, N) iowa_utils_list_find_and_remove((iowa_list_t *)(H), (iowa_list_node_find_callback_t)(F), (void *)(C), (iowa_list_t **)(N))

#ifdef __cplusplus
}
#endif

#endif // _IOWA_UTILS_
