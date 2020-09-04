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

/**********************************************
 *
 * This file implements simple IOWA connection
 * abstraction functions for Linux and Windows.
 *
 **********************************************/

// IOWA header
#include "iowa_platform.h"

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#endif

// For POSIX platforms, we use BSD sockets.
typedef struct
{
    int sock;
} sample_connection_t;

// We consider only UDP connections.
// We open an UDP socket binded to the the remote address.
void * iowa_system_connection_open(iowa_connection_type_t type,
                                   char *hostname,
                                   char *port,
                                   void *userData)
{
#ifdef _WIN32
    struct WSAData wd;
#endif
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    struct addrinfo *p;
    int s;
    sample_connection_t *connectionP;

    (void)userData;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    switch (type)
    {
    case IOWA_CONN_DATAGRAM:
        hints.ai_socktype = SOCK_DGRAM;
        break;

    case IOWA_CONN_STREAM:
        hints.ai_socktype = SOCK_STREAM;
        break;

    default:
        // let's consider only UDP and TCP connections in this sample
        return NULL;
    }

    if (0 != getaddrinfo(hostname, port, &hints, &servinfo)
        || servinfo == NULL)
    {
        return NULL;
    }

    // we test the various addresses
    s = -1;
    for (p = servinfo; p != NULL && s == -1; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s >= 0)
        {
            if (-1 == connect(s, p->ai_addr, p->ai_addrlen))
            {
#ifdef _WIN32
                closesocket(s);
#else
                close(s);
#endif
                s = -1;
            }
        }
    }

    if (NULL != servinfo)
    {
        freeaddrinfo(servinfo);
    }

    if (s < 0)
    {
        // failure
        return NULL;
    }

    connectionP = (sample_connection_t *)malloc(sizeof(sample_connection_t));
    if (connectionP == NULL)
    {
#ifdef _WIN32
        closesocket(s);
#else
        close(s);
#endif
    }
    else
    {
        connectionP->sock = s;
    }
    

    return connectionP;
}

// Since the socket is binded, we can use send() directly.
int iowa_system_connection_send(void *connP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData)
{
    int nbSent;
    sample_connection_t *connectionP;

    (void)userData;

    connectionP = (sample_connection_t *)connP;

    nbSent = send(connectionP->sock, buffer, length, 0);

    return nbSent;
}

// Since the socket is binded, it receives datagrams only from the binded address.
int iowa_system_connection_recv(void *connP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData)
{
    int numBytes;
    sample_connection_t *connectionP;

    (void)userData;

    connectionP = (sample_connection_t *)connP;

    numBytes = recv(connectionP->sock, buffer, length, 0);
#ifdef _WIN32
    if (numBytes == -1
        && WSAGetLastError() == WSAEMSGSIZE)
    {
        numBytes = length;
    }
#endif

    return numBytes;
}

void iowa_system_connection_close(void *connP,
                                  void *userData)
{
    sample_connection_t *connectionP;

    (void)userData;

    connectionP = (sample_connection_t *)connP;

#ifdef _WIN32
    closesocket(connectionP->sock);
#else
    close(connectionP->sock);
#endif

    free(connectionP);
}

// In this function, we use select on the sockets provided by IOWA.
int iowa_system_connection_select(void **connArray,
                                  size_t connCount,
                                  int32_t timeout,
                                  void *userData)
{
    struct timeval tv;
    fd_set readfds;
    size_t i;
    int result;
    int maxFd;
    int fd;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    maxFd = 0;

    // We monitor the sockets requested by IOWA
    for (i = 0; i < connCount; i++)
    {
        sample_connection_t *connectionP;

        connectionP = (sample_connection_t *)connArray[i];

        FD_SET(connectionP->sock, &readfds);
        if (connectionP->sock > maxFd)
        {
            maxFd = connectionP->sock;
        }
    }

    result = select(maxFd + 1, &readfds, NULL, NULL, &tv);

    if (result > 0)
    {
        for (i = 0; i < connCount; i++)
        {
            sample_connection_t *connectionP;

            connectionP = (sample_connection_t *)connArray[i];

            if (!FD_ISSET(connectionP->sock, &readfds))
            {
                connArray[i] = NULL;
            }
        }
    }

    return result;
}
