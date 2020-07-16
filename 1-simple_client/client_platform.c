/**********************************************
*
* Copyright (c) 2016-2019 IoTerop.
* All rights reserved.
*
**********************************************/

/**********************************************
 *
 * This file implements the IOWA system
 * abstraction functions for Linux and Windows.
 *
 * This is tailored for the LwM2M Client.
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
#include <io.h>
#else
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#endif


// We bind this function directly to malloc().
void * iowa_system_malloc(size_t size)
{
    return malloc(size);
}

// We bind this function directly to free().
void iowa_system_free(void *pointer)
{
    free(pointer);
}

// We return the number of seconds since Epoch.
int32_t iowa_system_gettime(void)
{
#ifdef _WIN32
    return (int32_t)(GetTickCount() / 1000);
#else
    return (int32_t)time(NULL);
#endif
}

// We fake a reboot by exiting the application.
void iowa_system_reboot(void *userData)
{
    (void)userData;

    fprintf(stdout, "\n\tFaking a reboot.\r\n\n");
    exit(0);
}

// Traces are output on stderr.
void iowa_system_trace(const char *format,
                       va_list varArgs)
{
    vfprintf(stderr, format, varArgs);
}

// For POSIX platforms, we use BSD sockets.
// The socket number (incremented by 1, 0 being a valid socket number) is cast as a void *.
static void * prv_sockToPointer(int sock)
{
    void *pointer;

    pointer = NULL;
    sock += 1;

    memcpy(&pointer, &sock, sizeof(int));

    return pointer;
}

static int prv_pointerToSock(void *pointer)
{
    int sock;

    memcpy(&sock, &pointer, sizeof(int));

    return sock - 1;
}

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

    (void)userData;

    // let's consider only UDP connection in this sample
    if (type != IOWA_CONN_DATAGRAM)
    {
        return NULL;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

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

    return prv_sockToPointer(s);
}

// Since the socket is binded, we can use send() directly.
int iowa_system_connection_send(void *connP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData)
{
    int nbSent;
    int sock;

    (void)userData;

    sock = prv_pointerToSock(connP);

    nbSent = send(sock, buffer, length, 0);

    return nbSent;
}

// Since the socket is binded, it receives datagrams only from the binded address.
int iowa_system_connection_recv(void *connP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData)
{
    int numBytes;
    int sock;

    (void)userData;

    sock = prv_pointerToSock(connP);

    numBytes = recv(sock, buffer, length, 0);
#ifdef _WIN32
    if (numBytes == -1
        && WSAGetLastError() == WSAEMSGSIZE)
    {
        numBytes = length;
    }
#endif

    return numBytes;
}

// In this function, we use select on the sockets provided by IOWA
// and on the sample_platform_data_t::pipeArray to be able to
// interrupt the select() if required.
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

    // Then the sockets requested by IOWA
    for (i = 0; i < connCount; i++)
    {
        fd = prv_pointerToSock(connArray[i]);
        FD_SET(fd, &readfds);
        if (fd > maxFd)
        {
            maxFd = fd;
        }
    }

    result = select(maxFd + 1, &readfds, NULL, NULL, &tv);

    if (result > 0)
    {
        for (i = 0; i < connCount; i++)
        {
            if (!FD_ISSET(prv_pointerToSock(connArray[i]), &readfds))
            {
                connArray[i] = NULL;
            }
        }
    }
    else if (result < 0)
    {
        if (errno == EINTR)
        {
            result = 0;
        }
    }

    return result;
}

void iowa_system_connection_close(void *connP,
                                  void *userData)
{
    int sock;

    (void)userData;

    sock = prv_pointerToSock(connP);

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}
