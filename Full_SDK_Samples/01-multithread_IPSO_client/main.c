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

/**************************************************
 *
 * This is a very simple LwM2M Client featuring an
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
#include <strsafe.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

// global variables
static iowa_context_t iowaH = NULL;
static iowa_sensor_t sensorId = IOWA_INVALID_SENSOR_ID;

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

// A simulated measure task launched in a separate thread
#ifdef _WIN32
DWORD WINAPI measure_routine(LPVOID arg)
#else
void * measure_routine(void *arg)
#endif
{
    int i;
    iowa_status_t result;

    i = 0;
    do
    {
#ifdef _WIN32
        Sleep(3000);
#else
        usleep(3000000);
#endif

        result = iowa_client_IPSO_update_value(iowaH, sensorId, 20 + i%4);

        i++;
    } while (i < 40 && result == IOWA_COAP_NO_ERROR);

    iowa_stop(iowaH);

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int main(int argc,
         char *argv[])
{
    iowa_status_t result;
    char endpoint_name[64];
    iowa_device_info_t devInfo;
#ifdef _WIN32
    DWORD  threadId;
    HANDLE thread;
    HANDLE mutex;
#else
    pthread_t thread;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
#endif

    (void)argc;
    (void)argv;

    printf("This a multithreaded LwM2M Client featuring an IPSO Temperature Object.\r\n\n");

    // Create a mutex for the iowa_system_mutex_* functions
#ifdef _WIN32
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL)
#else
    if (pthread_mutex_init(&mutex, NULL) != 0)
#endif
    {
        fprintf(stderr, "Mutex creation failed.\r\n");
        return 1;
    }

    // Initialize the IOWA stack using the mutex as the system abstraction functions user data.
    iowaH = iowa_init(&mutex);
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
    devInfo.modelNumber = "multithread_IPSO_client";

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

    // Start a thread for the measure task
#ifdef _WIN32
    thread = CreateThread(NULL, 0, measure_routine, NULL, 0, &threadId);
    if (thread == NULL)
#else
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (pthread_create(&thread, &attr, measure_routine, NULL) != 0)
#endif
    {
        fprintf(stderr, "Thread creation failed.");
    }
#ifndef _WIN32
    pthread_attr_destroy(&attr);
#endif

    printf("Registering to the LwM2M server at \"" SERVER_URI "\" under the Endpoint name \"%s\".\r\nUse Ctrl-C to stop.\r\n\n", endpoint_name);

    // Let IOWA run indefinitely, the measure routine will stop it.
    result = iowa_step(iowaH, -1);

    // Application specific: wait for the measure thread to finish
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
#else
    (void)pthread_join(thread, NULL);
#endif


cleanup:
    iowa_client_IPSO_remove_sensor(iowaH, sensorId);
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    // Close the mutex
#ifdef _WIN32
    CloseHandle(mutex);
#else
    pthread_mutex_destroy(&mutex);
#endif

    return 0;
}
