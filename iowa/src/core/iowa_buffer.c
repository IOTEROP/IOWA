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
* Copyright (c) 2019-2021 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_core_internals.h"

/*************************************************************************************
** Functions
*************************************************************************************/

iowa_buffer_t coreBufferNew(const uint8_t *dataP,
                            size_t length)
{
    iowa_buffer_t buffer;

    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "dataP: %p, length: %u.", dataP, length);

    buffer = IOWA_BUFFER_EMPTY;

    if (length > 0)
    {
        buffer.memory = (uint8_t *)iowa_system_malloc(length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (buffer.memory == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(length);
            return buffer;
        }
#endif

        buffer.data = buffer.memory;

        if (dataP == NULL)
        {
            memset(buffer.data, 0, length);
        }
        else
        {
            memcpy(buffer.data, dataP, length);
        }

        buffer.length = length;

        IOWA_LOG_TRACE(IOWA_PART_BASE, "Exiting on success.");
    }

    return buffer;
}

void coreBufferSet(iowa_buffer_t *bufferP,
                   uint8_t *dataP,
                   size_t length)
{
    IOWA_LOG_ARG_TRACE(IOWA_PART_BASE, "dataP: %p, length: %u.", dataP, length);

    assert(bufferP != NULL);

    bufferP->memory = (uint8_t *)dataP;
    bufferP->data = (uint8_t *)dataP;
    bufferP->length = length;
}

void coreBufferClear(iowa_buffer_t *bufferP)
{
    iowa_system_free(bufferP->memory);

    memset(bufferP, 0, sizeof(iowa_buffer_t));
}
