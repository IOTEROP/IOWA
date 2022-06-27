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
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_lwm2m_internals.h"

void valueFree(lwm2m_value_t *valueP)
{
    if (valueP != NULL)
    {
        IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "counter: %u.", valueP->counter);
        iowa_system_free(valueP->buffer);
        iowa_system_free(valueP);
    }
}
