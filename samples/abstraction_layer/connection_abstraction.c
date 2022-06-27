/**********************************************
 *
 * Copyright (c) 2016-2021 IoTerop.
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
#include "iowa_config.h"
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

// We consider only UDP and TCP connections.
// For UDP, we open an UDP socket binded to the the remote address.
void * iowa_system_connection_open(iowa_connection_type_t type,
                                   char *hostname,
                                   char *port,
                                   void *userData)
{
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
#endif
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    struct addrinfo *p;
    int s;
    sample_connection_t *connectionP;

    (void)userData;

#ifdef _WIN32
    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        return NULL;
    }
#endif

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

#ifndef IOWA_THREAD_SUPPORT

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

#ifdef _WIN32
    // On Windows systems, a select() with no sockets will return an error.
    // We do a sleep instead.
    if (0 == connCount)
    {
        (void)Sleep(timeout * 1000);

        return 0;
    }
#endif

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

#else // IOWA_THREAD_SUPPORT is defined

// A socket used only to interrupt the select()
int g_interruptSocket = -1;

// In this function, we use select on the sockets provided by IOWA
// and on our socket to be able to interrupt the select() if required.
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

    if (g_interruptSocket == -1)
    {
#ifdef _WIN32
    {
        WSADATA wsaData;

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            return -1;
        }
    }
#endif
        // we create the socket used to interrupt the select()
        g_interruptSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (g_interruptSocket != -1)
        {
            int res;
            struct sockaddr_in sysAddr;

            memset((char *)&sysAddr, 0, sizeof(sysAddr));

            sysAddr.sin_family = AF_INET;
            sysAddr.sin_port = 0;
            sysAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

            // bind socket to port
            res = bind(g_interruptSocket, (struct sockaddr *) &sysAddr, sizeof(sysAddr));
            if (res != -1)
            {
                struct sockaddr_in realAddr;
                int addrLen;

                addrLen = sizeof(realAddr);
                res = getsockname(g_interruptSocket, (struct sockaddr *)&realAddr, (socklen_t *)&addrLen);
                if (res != -1)
                {
                    res = connect(g_interruptSocket, (struct sockaddr *)&realAddr, addrLen);
                }
            }
            if (res == -1)
            {
#ifdef _WIN32
                closesocket(g_interruptSocket);
#else
                close(g_interruptSocket);
#endif
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    // we add our socket to be able to interrupt the select()
    FD_SET(g_interruptSocket, &readfds);
    maxFd = g_interruptSocket;

    // Then the sockets requested by IOWA
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
        if (FD_ISSET(g_interruptSocket, &readfds))
        {
            uint8_t buffer[1];
            result--;

            // flush the summy data
            (void)recv(g_interruptSocket, buffer, 1, 0);
        }
    }

    return result;
}

// To make the call to select() in iowa_system_connection_select() stops,
// we write data to the dummy socket if it's empty.
void iowa_system_connection_interrupt_select(void *userData)
{
    uint8_t buffer[1];
    int len;

    if (g_interruptSocket != -1)
    {
#ifdef _WIN32
        ioctlsocket(g_interruptSocket, FIONREAD, &len);
        if (len == 0)
#else
        len = recv(g_interruptSocket, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
        if (len == -1)
#endif
        {
            buffer[0] = 'S';
            len = send(g_interruptSocket, buffer, 1, 0);
        }
    }
}

#endif // IOWA_THREAD_SUPPORT
