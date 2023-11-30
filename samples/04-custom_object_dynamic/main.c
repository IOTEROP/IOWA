/**********************************************
 *
 * Copyright (c) 2016-2023 IoTerop.
 * All rights reserved.
 *
 * This program and the accompanying materials
 * are made available under the terms of
 * IoTerop’s IOWA License (LICENSE.TXT) which
 * accompany this distribution.
 *
 **********************************************/

/**************************************************
 *
 * This is a very simple LwM2M Client featuring a
 * custom LwM2M Object with changing values.
 *
 **************************************************/

// IOWA headers
#include "iowa_client.h"

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Header file containing the definition of the sample custom Object
#include "sample_object.h"

// LwM2M Server details
#include "sample_env.h" // Sample Config file

#define SERVER_URI SAMPLE_SERVER_URI
#define SERVER_SHORT_ID SAMPLE_SERVER_SHORT_ID
#define SERVER_LIFETIME SAMPLE_SERVER_LIFETIME
#define IOWA_DEVICE_NAME SAMPLE_ENDPOINT_NAME

#define MAX_ENDPOINT_NAME_SIZE 64

// If the IOWA_DEVICE_NAME is not defined, this unction will generates
// a unique one from your computer ID on Linux or from
// your "C:" volume serial number on Windows.
static void prv_generate_unique_name(char *name)
{
#ifdef _WIN32
    DWORD id;

    GetVolumeInformation("C:\\", 0, 0, &id, 0, 0, 0, 0);
#else
    long id;

    id = gethostid();
#endif

#ifdef IOWA_DEVICE_NAME
    strncpy(name, IOWA_DEVICE_NAME, MAX_ENDPOINT_NAME_SIZE);
#else
    sprintf(name, "IOWA_sample_client_%ld", id);
#endif
}

int main(int argc, char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    char endpoint_name[64];
    iowa_device_info_t devInfo;
    iowa_lwm2m_resource_desc_t sample_object_resources[SAMPLE_RES_COUNT] = SAMPLE_RES_DESCRIPTION;
    sample_object_values_t objectValues;
    int i;

    (void)argc;
    (void)argv;

    printf("This a simple LwM2M Client featuring a custom LwM2M Object.\r\n\n");

    // Initialize the IOWA stack.
    iowaH = iowa_init(NULL);
    if (iowaH == NULL)
    {
        fprintf(stderr, "IOWA context initialization failed.\r\n");
        goto cleanup;
    }

    // Generate an unique name
    prv_generate_unique_name(endpoint_name);

    // Set the information presented in Device Object (ID: 3)
    // all of these are optional
    memset(&devInfo, 0, sizeof(iowa_device_info_t));
    devInfo.manufacturer = "https://ioterop.com";
    devInfo.deviceType = "IOWA sample from https://github.com/IOTEROP/IOWA";
    devInfo.modelNumber = "custom_object_dynamic_client";

    // Configure the LwM2M Client
    result = iowa_client_configure(iowaH, endpoint_name, &devInfo, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "IOWA Client configuration failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Create of a custom object with a single instance and 3 resources
    // Actual values are stored in a struct sample_object_values_t
    objectValues.booleanValue = true;
    objectValues.integerValue = 10;
    objectValues.stringValue = NULL;

    result = iowa_client_add_custom_object(iowaH,
                                           SAMPLE_OBJECT_ID,                          // The ID of our custom object
                                           0, NULL,                                   // This is a single instance object
                                           SAMPLE_RES_COUNT, sample_object_resources, // the object's resources description
                                           sample_object_dataCallback,                // the callback to handle operations on Resources
                                           NULL,                                      // the server can not create new instances
                                           NULL,                                      // there are no multiple instances Resources
                                           &objectValues);                            // to access our values from the callback
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding a custom object failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Add a LwM2M Server to connect to
    result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_URI, SERVER_LIFETIME, 0, IOWA_SEC_NONE);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding a server failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    printf("Registering to the LwM2M server at \"" SERVER_URI "\" under the Endpoint name \"%s\".\r\nUse Ctrl-C to stop.\r\n\n", endpoint_name);

    // Let IOWA run for two minutes, updating the boolean and integer values each 3 seconds
    for (i = 0; i < 40 && result == IOWA_COAP_NO_ERROR; i++)
    {
        result = iowa_step(iowaH, 3);
        objectValues.booleanValue = !objectValues.booleanValue;
        objectValues.integerValue = objectValues.integerValue + 1;
        iowa_client_notification_lock(iowaH, true);
        iowa_client_object_resource_changed(iowaH, SAMPLE_OBJECT_ID, 0, SAMPLE_OBJECT_BOOLEAN_RES_ID);
        iowa_client_object_resource_changed(iowaH, SAMPLE_OBJECT_ID, 0, SAMPLE_OBJECT_INTEGER_RES_ID);
        iowa_client_notification_lock(iowaH, false);
    }

cleanup:
    iowa_client_remove_custom_object(iowaH, SAMPLE_OBJECT_ID);
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    // free our string value in case the LwM2M wrote something
    free(objectValues.stringValue);

    return 0;
}
