/**********************************************
*
* Copyright (c) 2016-2018 IoTerop.
* All rights reserved.
*
**********************************************/

// IOWA headers
#include "iowa_client.h"
#include "iowa_bearer_selection.h"

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

#ifdef IOWA_DEVICE_NAME
    sprintf(name, IOWA_DEVICE_NAME "_%ld", id);
#else
    sprintf(name, "IOWA_sample_client_%ld", id);
#endif

}
// The callback called by IOWA when there's a write operation by a Server on the bearer selection object
iowa_status_t resultCb(iowa_sensor_t id, iowa_bearer_selection_info_t *infoP, void *userDataCallback, iowa_context_t contextP)
{
    return IOWA_COAP_NO_ERROR;
}

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    char endpoint_name[64];
    iowa_device_info_t devInfo;
    iowa_bearer_selection_info_t info;
    char *randomCharListP[4] = {"IoTerop", "IoWa", "Alaska", "2020"};
    uint8_t randomIntList[4] = {1, 2, 3, 4};
    iowa_sensor_t sensorId;
    int i;

    (void)argc;
    (void)argv;

    printf("This a simple LwM2M Client featuring an Bearer Selection Object.\r\n\n");

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
    devInfo.modelNumber = "bear_selection_client";

    // Configure the LwM2M Client
    result = iowa_client_configure(iowaH, endpoint_name, &devInfo, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "IOWA Client configuration failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Add a bearer selection object with some optional resources
    result = iowa_client_add_bearer_selection_object(iowaH, IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_NB_IOT | IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST | IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST_MODE | IOWA_BEARER_SELECTION_RSC_ATTACH_WO_PDN_CONNECTION, resultCb, NULL, &sensorId);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding a sensor failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Add a LwM2M Server to connect to
    result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_URI, SERVER_LIFETIME, 0, IOWA_SEC_NONE);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding a server failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    memset(&info, 0, sizeof(iowa_bearer_selection_info_t));

    printf("Registering to the LwM2M server at \"" SERVER_URI "\" under the Endpoint name \"%s\".\r\nUse Ctrl-C to stop.\r\n\n", endpoint_name);

    // Let IOWA run for two minutes, updating the bearer selection value each 3 seconds
    for (i = 0; i < 40 && result == IOWA_COAP_NO_ERROR; i++)
    {
        info.acceptableRsrpNbIot = (uint16_t)(-70+ i%4);
        info.operatorList = randomCharListP[i%4];
        info.attachWoPdnConnection = (bool)(i%2);
        info.operatorListMode = (bool)(i%2);

        result = iowa_step(iowaH, 3);
        result = iowa_client_bearer_selection_update(iowaH, sensorId, IOWA_BEARER_SELECTION_RSC_ACCEPTABLE_RSRP_NB_IOT | IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST | IOWA_BEARER_SELECTION_RSC_OPERATOR_LIST_MODE | IOWA_BEARER_SELECTION_RSC_ATTACH_WO_PDN_CONNECTION, &info);
    }

cleanup:
    iowa_client_remove_bearer_selection_object(iowaH, sensorId);
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    return 0;
}
