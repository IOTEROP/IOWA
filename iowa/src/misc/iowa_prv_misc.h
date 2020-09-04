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
#include "iowa_prv_logger.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/**************************************************************
* Base64 API
**************************************************************/

#define BASE64_PADDING '='

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






bool listFindCallbackBy16bitsId(void *nodeP, void *criteriaP);





uint16_t listNew16bitsId(list_16_bits_id_t *headP);






bool listFindCallbackBy32bitsId(void *nodeP, void *criteriaP);





uint32_t listNew32bitsId(list_32_bits_id_t *headP);

#define LIST_NEW_16_BITS_ID(H) listNew16bitsId((list_16_bits_id_t *)H)
#define LIST_NEW_32_BITS_ID(H) listNew32bitsId((list_32_bits_id_t *)H)

/**************************************************************
* Utils API
**************************************************************/





char * utilsStrdup(const char *str);







void * utilsRealloc(void *src,
                    size_t sizeSrc,
                    size_t sizeDst);






void * utilsCalloc(size_t number,
                   size_t size);





char * utilsBufferToString(const uint8_t *buffer,
                           size_t size);






bool utilsCmpBufferWithString(const uint8_t *buffer,
                              size_t size,
                              const char *str);





size_t utilsStrlen(const char *str);






size_t utilsStringCopy(char *buffer, size_t length, const char *str);








void utilsCopyValue(void *dst, const void *src, size_t len);

#endif
