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
* Copyright (c) 2016-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_core_internals.h"
#include "iowa_prv_lwm2m_internals.h"

#define PRV_BOOTSTRAP_SERVER_SECURITY_OBJECT_RSC_NB 9
#define PRV_SERVER_SECURITY_OBJECT_RSC_NB           7
#define PRV_SERVER_SERVER_OBJECT_RSC_NB             4

#define PRV_ID_BUFFER_LEN (size_t)5

#define RES_ID_TO_TYPE(O, R) case IOWA_LWM2M_##O##_ID_##R : return IOWA_LWM2M_##O##_TYPE_##R

#define PRV_LWM2M_UDP_BINDING    "U"
#define PRV_LWM2M_TCP_BINDING    "T"
#define PRV_LWM2M_SMS_BINDING    "S"
#define PRV_LWM2M_NON_IP_BINDING "N"

/*************************************************************************************
** Private functions
*************************************************************************************/

/*************************************************************************************
** Public functions
*************************************************************************************/

