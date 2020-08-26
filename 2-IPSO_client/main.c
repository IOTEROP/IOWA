/**********************************************
*
* Copyright (c) 2020 IoTerop.
* All rights reserved.
*
**********************************************/

/**************************************************
 *
 * This a a very simple LwM2M Client featuring an
 * IPSO Temperature sensor.
 *
 **************************************************/

// IOWA headers
#include "iowa_client.h"
#include "iowa_ipso.h"

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// LwM2M Server details
#define SERVER_SHORT_ID 1234
#define SERVER_LIFETIME   50
#define SERVER_URI      "coap://iowa-server.ioterop.com"      // to connect to IoTerop's Connecticut test server
// #define SERVER_URI   "coap://leshan.eclipseprojects.io"    // to connect to the Eclipse's Leshan server demo
// #define SERVER_URI   "coap://127.0.0.1:5683"               // to connect to a local server

// As this sample does not use security, the LwM2M Server relies only
// on the endpoint name to identify the LwM2M Client. Thus we need an
// unique name. This function generates one from your computer ID on
// Linux or from your "C:" volume serial number on Windows.
static void prv_generate_unique_name(char *name)
{
#ifdef _WIN32
    DWORD id;

    GetVolumeInformation("C:\\", 0, 0, &id, 0, 0, 0, 0);
#else
    long id;

    id = gethostid();
#endif

    sprintf(name, "IOWA_simple_client_%ld", id);
}

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    char endpoint_name[64];
    iowa_device_info_t devInfo;
    iowa_sensor_t sensorId;
    int i;

    (void)argc;
    (void)argv;

    /******************
     * Initialization
     */

    printf("This a simple LwM2M Client featuring an IPSO Temperature Object.\r\n\n");

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
    devInfo.deviceType = "IOWA sample from https://github.com/IOTEROP/IOWA-Samples";
    devInfo.modelNumber = "2-IPSO_client";

    // Configure the LwM2M Client
    result = iowa_client_configure(iowaH, endpoint_name, &devInfo, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "IOWA Client configuration failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Add an IPSO Temperature Object
    result = iowa_client_IPSO_add_sensor(iowaH, IOWA_IPSO_TEMPERATURE, 20, "Cel", "Test Temperature", -20.0, 50.0, &sensorId);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding the temperature sensor failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
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

    // Let IOWA run for two minutes, updating the temperature value each 3 seconds
    for (i = 0; i < 40 && result == IOWA_COAP_NO_ERROR; i++)
    {
        result = iowa_step(iowaH, 3);
        result = iowa_client_IPSO_update_value(iowaH, sensorId, 20 + i%4);
    }

cleanup:
    iowa_client_IPSO_remove_sensor(iowaH, sensorId);
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    return 0;
}
