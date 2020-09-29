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
 * This is a very simple LwM2M Client demonstrating
 * IOWA ease-of-use.
 *
 **************************************************/

// IOWA headers
#include "iowa_server.h"
#include "baseline_server.h"
#include "iowa_platform.h"

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// The callback called by IOWA when a LwM2M Client register or deregister to the LwM2M Server
void client_monitor(const iowa_client_t *clientP,
                    iowa_state_t state,
                    void *monitorUserData,
                    iowa_context_t contextP)
{
    size_t i;

    (void)monitorUserData;

    // Display information on the LwM2M Client
    printf("Client monitoring callback called.\r\n");
    printf("State: ");
    switch (state)
    {
    case IOWA_STATE_REGISTERED:
        printf("IOWA_STATE_REGISTERED\r\n");
        break;
    case IOWA_STATE_UPDATING:
        printf("IOWA_STATE_UPDATING\r\n");
        break;
    case IOWA_STATE_UNREGISTERED:
        printf("IOWA_STATE_UNREGISTERED\r\n");
        break;
    default:
        printf("unexpected (%d)\r\n", (int)state);
        break;
    }
    if (clientP->name != NULL)
    {
        printf("Client \"%s\", id: %u\r\n", clientP->name, clientP->id);
    }
    else
    {
        printf("Client id: %u\r\n", clientP->id);
    }

    printf("\r\n");
}

int main(int argc,
         char *argv[])
{
    iowa_status_t result;
    iowa_context_t iowaH;
    int udpSocket;
    uint8_t i;

    (void)argc;
    (void)argv;

    /******************
     * Initialization
     */

    // Initialize the IOWA stack
    iowaH = iowa_init(NULL);
    if (iowaH == NULL)
    {
        fprintf(stderr, "IOWA context initialization failed.\r\n");
        goto cleanup;
    }

    //Open UDP Socket
    udpSocket = open_udp_socket(iowaH);
    if (udpSocket < 0)
    {
        fprintf(stderr, "UDP Socket opening failed.\r\n");
        goto cleanup;
    }

    // Configure the LwM2M Server
    result = iowa_server_configure(iowaH, client_monitor, NULL, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Configuring the server failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    printf("Server started.\r\n");

    /******************
     * "Main loop"
     */
    for (i = 0; i < 40; i++)
    {
        result = iowa_step(iowaH, 3);
    }

cleanup:
    iowa_close(iowaH);
    close_udp_socket();

    return 0;
}
