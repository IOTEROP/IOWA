/**********************************************
*
* Copyright (c) 2016-2018 IoTerop.
* All rights reserved.
*
**********************************************/

// IOWA headers
#include "iowa_client.h"
#include "iowa_mqtt_objects.h"

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

    sprintf(name, "IOWA_sample_client_%ld", id);
}

// The callback called by IOWA when there's an operation by a Server on an MQTT Broker
void brokerCB(iowa_dm_operation_t operation, iowa_sensor_t brokerId, iowa_mqtt_broker_t *brokerDetailsP, void *userData, iowa_context_t contextP)
{
    // Display information on the LwM2M Client
    printf("Broker callback called.\r\n");
    printf("Operation: ");
    switch (operation)
    {
    case IOWA_DM_CREATE:
        printf("IOWA_DM_CREATE\r\n");
        break;
    case IOWA_DM_WRITE:
        printf("IOWA_DM_WRITE\r\n");
        break;
    case IOWA_DM_DELETE:
        printf("IOWA_DM_DELETE\r\n");
        break;
    default:
        printf("unexpected (%d)\r\n", (int)operation);
        break;
    }

    printf("Broker Id %u \r\n", brokerId);
    printf("Broker client Id \"%s\" \r\n", brokerDetailsP->clientId);

    return;
}

// The callback called by IOWA when there's an operation by a Server on an MQTT Publication
void publicationCB(iowa_dm_operation_t operation, iowa_sensor_t publicationId, iowa_mqtt_publication_t *publicationDetailsP, void *userData, iowa_context_t contextP)
{
    iowa_mqtt_broker_t *brokerDetailsP;

    // Display information on the LwM2M Client
    printf("Publication callback called.\r\n");
    printf("Operation: ");
    switch (operation)
    {
    case IOWA_DM_CREATE:
        printf("IOWA_DM_CREATE\r\n");
        break;
    case IOWA_DM_WRITE:
        printf("IOWA_DM_WRITE\r\n");
        break;
    case IOWA_DM_DELETE:
        printf("IOWA_DM_DELETE\r\n");
        break;
    default:
        printf("unexpected (%d)\r\n", (int)operation);
        break;
    }

    printf("Publication Id %u \r\n", publicationId);
    brokerDetailsP = iowa_client_get_mqtt_broker(contextP, publicationDetailsP->brokerId);

    if (brokerDetailsP != NULL)
    {
        printf("Publication's Broker client Id \"%s\". \r\n", brokerDetailsP->clientId);
    }
    else
    {
        printf("Publication is not related to any available Broker.\r\n");
    }

    return;
}

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    char endpoint_name[64];
    iowa_device_info_t devInfo;
    iowa_sensor_t brokerId;
    iowa_sensor_t publicationId;
    iowa_mqtt_broker_t brokerInfo;
    iowa_mqtt_publication_t publicationInfo;
    int i;

    (void)argc;
    (void)argv;

    printf("This a simple LwM2M Client featuring an MQTT Object.\r\n\n");

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

    // Enable MQTT Broker management
    result = iowa_client_enable_mqtt_broker(iowaH, brokerCB, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Enabling MQTT Broker failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    memset(&brokerInfo, 0, sizeof(iowa_mqtt_broker_t));

    brokerInfo.uri = "tcp://127.0.0.1";
    brokerInfo.clientId = "MQTT_Sample";
    brokerInfo.keepAlive = 120;
    brokerInfo.securityMode = IOWA_SEC_NONE;

    // Add an MQTT Broker object
    result = iowa_client_add_mqtt_broker(iowaH, 0x00, &brokerInfo, &brokerId);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding an MQTT Broker failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Enable MQTT Publication management
    result = iowa_client_enable_mqtt_publication(iowaH, publicationCB, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Enabling MQTT Publication failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    memset(&publicationInfo, 0, sizeof(iowa_mqtt_publication_t));

    publicationInfo.topic = "/sensors/tmp0";
    publicationInfo.source = "</3303/0>";
    publicationInfo.brokerId = brokerId;
    publicationInfo.encoding = IOWA_CONTENT_FORMAT_CBOR;

    // Add an MQTT Publication object.
    result = iowa_client_add_mqtt_publication(iowaH, IOWA_MQTT_PUBLICATION_RSC_ENCODING, &publicationInfo, &publicationId);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding an MQTT Publication failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
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

    // Let IOWA run for two minutes.
    for (i = 0; i < 40 && result == IOWA_COAP_NO_ERROR; i++)
    {
        result = iowa_step(iowaH, 3);
    }

cleanup:
    iowa_client_disable_mqtt_broker(iowaH);
    iowa_client_disable_mqtt_publication(iowaH);
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    return 0;
}
