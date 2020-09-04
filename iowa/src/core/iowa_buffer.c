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

#include "iowa_prv_core_internals.h"

/*************************************************************************************
** Functions
*************************************************************************************/

iowa_buffer_t coreBufferNew(const uint8_t *dataP,
                            size_t length)
{
    iowa_buffer_t buffer;

    buffer = (iowa_buffer_t){NULL, 0};

    buffer.data = (uint8_t *)iowa_system_malloc(length);
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (buffer.data == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(length);
        return buffer;
    }
#endif

    if (dataP == NULL)
    {
        memset(buffer.data, 0, length);
    }
    else
    {
        memcpy(buffer.data, dataP, length);
    }

    buffer.length = length;

    return buffer;
}
