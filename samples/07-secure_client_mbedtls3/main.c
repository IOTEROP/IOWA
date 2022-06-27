/**********************************************
 *
 * Copyright (c) 2016-2022 IoTerop.
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
#include <windows.h>
#else
#include <unistd.h>
#endif

// LwM2M Server details
#define SERVER_SHORT_ID 1234
#define SERVER_LIFETIME   50
#define SERVER_URI      "coaps://iowa-server.ioterop.com"

// This function returns a random vector of the specified size.
int iowa_system_random_vector_generator(uint8_t *randomBuffer,
                                        size_t size,
                                        void *userData)
{
    size_t i;

    (void)userData;

/*****************************************
 *
 *       WARNING !
 *
 * This is not a proper way to generate
 * a random vector, it only serves as
 * an example here.
 *
 *****************************************/

    for (i = 0; i < size; i++)
    {
        randomBuffer[i] = rand() % 256;
    }

    return 0;
}

iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP)
{
    uint8_t keyIdentity[] = { /*Enter your Pre-Shared Key identity here*/ };
    uint8_t keyValue[] = { /*Enter your Pre-Shared Key value here*/ };
    size_t keyIdentityLength;
    size_t keyValueLength;

    (void)userDataP;

    if (0 == sizeof(keyIdentity)
        || 0 == sizeof(keyValue))
    {
        fprintf(stderr, "\r\n\n-------------------------\r\n\n"
                "This sample requires some editing before running.\r\n"
                "Please refer to its README."
                "\r\n\n-------------------------\r\n\n");
        return  IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    if (peerIdentityLen != strlen(SERVER_URI)
        || memcmp(peerIdentity, SERVER_URI, peerIdentityLen) != 0)
    {
        fprintf(stderr, "Unknown security credentials for \"%.*s\".\r\n", (int)peerIdentityLen, peerIdentity);
        return IOWA_COAP_404_NOT_FOUND;
    }

/*********************************************
 *
 *       WARNING !
 *
 * This is not a proper way to store security
 * credentials, it only serves as an example
 * here.
 *
 *********************************************/

    keyIdentityLength = sizeof(keyIdentity);
    keyValueLength = sizeof(keyValue);

    switch (securityOp)
    {
    case IOWA_SEC_READ:
        if (IOWA_SEC_PRE_SHARED_KEY == securityDataP->securityMode)
        {
            securityDataP->protocol.pskData.identity = (uint8_t *)malloc(keyIdentityLength);
            if (NULL == securityDataP->protocol.pskData.identity)
            {
                fprintf(stderr, "Memory allocation failure.\r\n");
                return  IOWA_COAP_501_NOT_IMPLEMENTED;
            }
            memcpy(securityDataP->protocol.pskData.identity, keyIdentity, keyIdentityLength);
            securityDataP->protocol.pskData.identityLen = keyIdentityLength;

            securityDataP->protocol.pskData.privateKey = (uint8_t *)malloc(keyValueLength);
            if (NULL == securityDataP->protocol.pskData.privateKey)
            {
                fprintf(stderr, "Memory allocation failure.\r\n");
                return  IOWA_COAP_501_NOT_IMPLEMENTED;
            }
            memcpy(securityDataP->protocol.pskData.privateKey, keyValue, keyValueLength);
            securityDataP->protocol.pskData.privateKeyLen = keyValueLength;

            return IOWA_COAP_NO_ERROR;
        }
        // Implementation for other modes is not done in this sample.
        break;

    case IOWA_SEC_FREE:
        if (IOWA_SEC_PRE_SHARED_KEY == securityDataP->securityMode)
        {
            free(securityDataP->protocol.pskData.identity);
            free(securityDataP->protocol.pskData.privateKey);

            return IOWA_COAP_NO_ERROR;
        }
        // Implementation for other modes is not done in this sample.
        break;

    default:
        // Implementation for other operations is not done in this sample.
        break;
    }

    return IOWA_COAP_501_NOT_IMPLEMENTED;
}

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    iowa_device_info_t devInfo;

    (void)argc;
    (void)argv;

    printf("This a simple LwM2M Client using Mbed TLS 3.1.0 to secure its exchanges with the LwM2M Server.\r\n\n");

    // Initialize the IOWA stack.
    iowaH = iowa_init(NULL);
    if (iowaH == NULL)
    {
        fprintf(stderr, "IOWA context initialization failed.\r\n");
        goto cleanup;
    }

    // Set the information presented in Device Object (ID: 3)
    // all of these are optional
    memset(&devInfo, 0, sizeof(iowa_device_info_t));
    devInfo.manufacturer = "https://ioterop.com";
    devInfo.deviceType = "IOWA sample from https://github.com/IOTEROP/IOWA";
    devInfo.modelNumber = "secure_client_mbedtls3";

    // Configure the LwM2M Client
    result = iowa_client_configure(iowaH, ""/*Enter your client name here*/, &devInfo, NULL);
    if (result != IOWA_COAP_NO_ERROR)
    {
        if (result == IOWA_COAP_400_BAD_REQUEST)
        {
            fprintf(stderr, "\r\n\n-------------------------\r\n\n"
                    "This sample requires some editing before running.\r\n"
                    "Please refer to its README."
                    "\r\n\n-------------------------\r\n\n");
        }
        fprintf(stderr, "IOWA Client configuration failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    // Add a LwM2M Server to connect to
    result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_URI, SERVER_LIFETIME, 0, IOWA_SEC_PRE_SHARED_KEY);
    if (result != IOWA_COAP_NO_ERROR)
    {
        fprintf(stderr, "Adding a server failed (%u.%02u).\r\n", (result & 0xFF) >> 5, (result & 0x1F));
        goto cleanup;
    }

    printf("Registering to the LwM2M server at \"" SERVER_URI "\".\r\nUse Ctrl-C to stop.\r\n\n");

    // Let IOWA run for two minutes
    (void)iowa_step(iowaH, 120);

cleanup:
    iowa_client_remove_server(iowaH, SERVER_SHORT_ID);
    iowa_close(iowaH);

    return 0;
}
