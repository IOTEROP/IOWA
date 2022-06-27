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
*
**********************************************/

#include "iowa_prv_coap_internals.h"

void coapUtilsfreeApplicationData(application_coap_data_t *dataP)
{
    if (dataP != NULL)
    {
        if (dataP->messageP != NULL)
        {
            iowa_coap_message_free(dataP->messageP);
        }
        iowa_system_free(dataP);
    }
}
