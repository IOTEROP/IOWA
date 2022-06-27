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

#include "iowa_prv_core_internals.h"
#include "iowa_prv_lwm2m_internals.h"

/*************************************************************************************
** Data Structures and Constants
*************************************************************************************/

#define USER_CALLBACK_MIN_ID 0xF000

#define IOWA_CONTEXT_VERSION "2019-12"

/**********************
** Data Size
**********************/

#define PRV_LWM2M_URI_SIZE                       2
#define PRV_ATTRIBUTES_NUMERIC_SIZE              8 // sizeof(double)

/**********************
** Data Key
**********************/

#define PRV_VERSION_KEY                         299

#define PRV_SERVER_KEY                          300
#define PRV_SERVER_SHORT_ID_KEY                 301
#define PRV_SERVER_SEC_INST_ID_KEY              302
#define PRV_SERVER_SRV_INST_ID_KEY              303
#define PRV_SERVER_URI_KEY                      304
#define PRV_SERVER_MSISDN_KEY                   305 // optional for serialization
#define PRV_SERVER_LIFETIME_KEY                 306
#define PRV_SERVER_BINDING_KEY                  307
#define PRV_SERVER_DEFAULT_PMIN_KEY             308 // optional for serialization
#define PRV_SERVER_DEFAULT_PMAX_KEY             309 // optional for serialization
#define PRV_SERVER_NOTIF_STORING_KEY            310
#define PRV_SERVER_BOOTSTRAP_KEY                311 // optional for serialization
#define PRV_SERVER_DISABLE_TIMEOUT_KEY          312 // optional for serialization
#define PRV_SERVER_MUTE_KEY                     313 // optional for serialization
#define PRV_SERVER_SECURITY_MODE_KEY            314
#define PRV_SERVER_LWM2M_VERSION_KEY            315

#define PRV_REG_PROCEDURE_KEY                   400 // optional for serialization
#define PRV_REG_PROCEDURE_PRIORITY_ORDER_KEY    401 // optional for serialization
#define PRV_REG_PROCEDURE_INITIAL_DELAY_KEY     402 // optional for serialization
#define PRV_REG_PROCEDURE_BLOCK_FAILURE_KEY     403 // optional for serialization
#define PRV_REG_PROCEDURE_BOOTSTRAP_FAILURE_KEY 404 // optional for serialization
#define PRV_REG_PROCEDURE_RETRY_COUNT_KEY       405 // optional for serialization
#define PRV_REG_PROCEDURE_RETRY_DELAY_KEY       406 // optional for serialization
#define PRV_REG_PROCEDURE_SEQ_RETRY_COUNT_KEY   407 // optional for serialization
#define PRV_REG_PROCEDURE_SEQ_DELAY_KEY         408 // optional for serialization

#define PRV_RUNTIME_KEY                         450
#define PRV_RUNTIME_STATUS_KEY                  451
#define PRV_RUNTIME_LOCATION_KEY                452
#define PRV_RUNTIME_RETRY_COUNT_KEY             453
#define PRV_RUNTIME_SEQ_RETRY_COUNT_KEY         454
#define PRV_RUNTIME_QUEUE_KEY                   455

#define PRV_LWM2M_URI_KEY                       475
#define PRV_LWM2M_URI_TYPE_KEY                  476

#define PRV_ATTRIBUTES_KEY                      500
#define PRV_ATTRIBUTES_MIN_PERIOD_KEY           501
#define PRV_ATTRIBUTES_MAX_PERIOD_KEY           502
#define PRV_ATTRIBUTES_GREATER_KEY              503
#define PRV_ATTRIBUTES_LESS_KEY                 504
#define PRV_ATTRIBUTES_STEP_KEY                 505
#define PRV_ATTRIBUTES_MIN_EVAL_PERIOD_KEY      506 // optional for serialization
#define PRV_ATTRIBUTES_MAX_EVAL_PERIOD_KEY      507 // optional for serialization

#define PRV_OBSERVE_KEY                         550
#define PRV_OBSERVE_URI_KEY                     551
#define PRV_OBSERVE_FORMAT_KEY                  552
#define PRV_OBSERVE_TOKEN_KEY                   553
#define PRV_OBSERVE_LAST_TIME_KEY               554
#define PRV_OBSERVE_COUNTER_KEY                 555

#define PRV_ACL_KEY                             580
#define PRV_ACL_ACL_INSTANCE_ID_KEY             581
#define PRV_ACL_OBJECT_ID_KEY                   582
#define PRV_ACL_INSTANCE_ID_KEY                 583
#define PRV_ACL_OWNER_ID_KEY                    584
#define PRV_ACL_SERVER_FLAG_KEY                 585
#define PRV_ACL_FLAGS_KEY                       586
#define PRV_ACL_SERVER_ID_KEY                   587

#define CONTEXT_ADD_BUFFER_OPTION(messageP, optionP, key, data, dataLength)   \
{                                                                             \
    optionP = iowa_coap_option_new((key));                                    \
    if ((optionP) == NULL)                                                    \
    {                                                                         \
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;                         \
        goto exit_on_error;                                                   \
    }                                                                         \
    optionP->value.asBuffer = (uint8_t *)data;                                \
    optionP->length = (uint16_t)dataLength;                                   \
    iowa_coap_message_add_option((messageP), (optionP));                      \
}

#define CONTEXT_ADD_INTEGER_OPTION(messageP, optionP, key, data)              \
{                                                                             \
    optionP = iowa_coap_option_new((key));                                    \
    if ((optionP) == NULL)                                                    \
    {                                                                         \
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;                         \
        goto exit_on_error;                                                   \
    }                                                                         \
    optionP->value.asInteger = (uint32_t) data;                               \
    iowa_coap_message_add_option((messageP), (optionP));                      \
}

