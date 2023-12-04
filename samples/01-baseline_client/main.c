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
 * This is a very simple LwM2M Client demonstrating
 * IOWA ease-of-use.
 *
 **************************************************/

// IOWA headers
#include "iowa_client.h"

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#pragma warning(disable : 4996)
#include <windows.h>
#else
#include <unistd.h>
#endif

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

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    char endpoint_name[MAX_ENDPOINT_NAME_SIZE + 1];
    iowa_device_info_t devInfo;

    (void)argc;
    (void)argv;

    printf("This a simple LwM2M Client.\r\n\n");

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
    devInfo.modelNumber = "baseline_client";

    // Configure the LwM2M Client
    result = iowa_client_configure(iowaH, endpoint_name, &devInfo, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "IOWA Client configuration failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
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

    // Let IOWA run for two minutes
    result = iowa_step(iowaH, 120);

cleanup:
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    return 0;
}
