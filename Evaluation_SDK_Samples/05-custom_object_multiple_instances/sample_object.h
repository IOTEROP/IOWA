/**********************************************
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

/********************************************************************************
 *
 * This is the header file for a simple custom
 * LwM2M Object with the following definition:
 *
 * ID: 3200
 * Single instance
 *
 * Resources:
 * |  ID  | Type    | Operations | Multiple | Description                         |
 * +------+---------+------------+----------+-------------------------------------+
 * | 5500 | Boolean |   R        | Single   | A read-only boolean value           |
 * | 5750 | String  |   R/W      | Single   | A writable string                   |
 * | 5503 | Integer |   R/W      | Single   | A writable integer value            |
 *
 *********************************************************************************/

// IOWA headers
#include "iowa_client.h"

#define SAMPLE_OBJECT_ID 3200

// The Sample Object Resources description
#define SAMPLE_RES_COUNT 3

#define SAMPLE_RES_DESCRIPTION {                                                                              \
    {5500, IOWA_LWM2M_TYPE_BOOLEAN, IOWA_OPERATION_READ,                        IOWA_RESOURCE_FLAG_NONE},     \
    {5750, IOWA_LWM2M_TYPE_STRING,  IOWA_OPERATION_READ | IOWA_OPERATION_WRITE, IOWA_RESOURCE_FLAG_NONE},     \
    {5503, IOWA_LWM2M_TYPE_INTEGER, IOWA_OPERATION_READ | IOWA_OPERATION_WRITE, IOWA_RESOURCE_FLAG_NONE}      \
}

// A structure containing the values of the writable resources of the object
typedef struct
{
    bool  booleanValue;
    int   integerValue;
    char *stringValue;
} sample_instance_values_t;

typedef struct _sample_instance__t
{
    struct _sample_instance__t *next;
    uint16_t id;
    sample_instance_values_t *instanceValuesP;
} sample_instance_t;

typedef struct
{
    sample_instance_t *instanceList;
} sample_object_t;

// The function called by IOWA when an operation is to be performed on the Object resources
iowa_status_t sample_object_dataCallback(iowa_dm_operation_t operation,
                                         iowa_lwm2m_data_t *dataP,
                                         size_t numData,
                                         void *userData,
                                         iowa_context_t iowaH);

// The function called by IOWA when an operation is to be performed on the Object instances
iowa_status_t sample_object_InstanceCallback(iowa_dm_operation_t operation,
                                             uint16_t objectID,
                                             uint16_t instanceID,
                                             void *userData,
                                             iowa_context_t contextP);

// The function to add a new instance to the sample object
iowa_status_t sample_object_add_instance(sample_object_t *objectP,
                                         sample_instance_values_t *instanceValuesP,
                                         uint16_t id);

// The function to free sample object memory
iowa_status_t sample_object_remove_object(sample_object_t *objectP);
