/**********************************************
*
* Copyright (c) 2016-2020 IoTerop.
* All rights reserved.
*
**********************************************/

/**********************************************
 *
 * This file implements the IOWA system
 * abstraction functions for Linux and Windows.
 *
 * This is tailored for the LwM2M Server.
 *
 **********************************************/

// IOWA header
#include "iowa_config.h"
#include "iowa_platform.h"
#include "iowa_server.h"
#include "baseline_server.h"

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


#define LWM2M_SERVER_PORT_UDP  "5683"

// For POSIX platforms, we use BSD sockets. The peer address is stored in a single structure.
typedef struct
{
    struct sockaddr_storage addr;
    socklen_t               len;
} sample_connection_desc_t;

typedef struct
{
    // We need the iowa context for calling iowa_* API inside iowa_system_connection_select
    iowa_context_t contextP;

    // an UDP socket to listen for incoming packets for unsecure connections
    int udpSocket;

    // a socket to interrupt the select()
    int sysSocket;
} sample_server_data_t;


// This function creates a self-connected UDP socket which only purpose
// is to interrupt the select
static int prv_createSysSocket()
{
    int sysSock;
    int len;
    struct sockaddr_in sysAddr, realAddr;
    int addrLen;

    sysSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sysSock == -1)
    {
        return -1;
    }

    memset((char *)& sysAddr, 0, sizeof(sysAddr));

    sysAddr.sin_family = AF_INET;
    sysAddr.sin_port = 0;
    sysAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    //bind socket to port
    if (bind(sysSock, (struct sockaddr *) &sysAddr, sizeof(sysAddr)) == -1)
    {
        goto error;
    }

    addrLen = sizeof(realAddr);
    if (getsockname(sysSock, (struct sockaddr *)&realAddr, (socklen_t *)&addrLen) == -1)
    {
        goto error;
    }

    if (connect(sysSock, (struct sockaddr *)&realAddr, addrLen) == -1)
    {
        goto error;
    }

    return sysSock;

error:
#ifdef _WIN32
    closesocket(sysSock);
#else
    close(sysSock);
#endif
    return -1;
}

// Global struct containing both UDP and interrupt socket and iowa context.
sample_server_data_t g_serverData = {0};

// Thid function close both UDP and interrupt sockets
void close_udp_socket()
{
#ifdef _WIN32
    closesocket(g_serverData.udpSocket);
    closesocket(g_serverData.sysSocket);
#else
    close(g_serverData.udpSocket);
    close(g_serverData.sysSocket);
#endif
}

// This function open UDP and interrupt sockets and store iowa context inside platform variable.

int open_udp_socket(void *iowaContextP)
{
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    struct addrinfo *p;
#ifdef _WIN32
    unsigned long l;
#endif

#ifdef _WIN32
    {
        WORD wVersionRequested;
        WSADATA wsaData;

        wVersionRequested = MAKEWORD(2, 2);

        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            return NULL;
        }
    }
#endif

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // for unsecured UDP connections on the default port
    if (0 != getaddrinfo("0.0.0.0", LWM2M_SERVER_PORT_UDP, &hints, &servinfo)
        || servinfo == NULL)
    {
        return -1;
    }

    // we test the various addresses
    g_serverData.udpSocket = -1;
    for (p = servinfo; p != NULL && g_serverData.udpSocket == -1; p = p->ai_next)
    {
        g_serverData.udpSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (g_serverData.udpSocket >= 0)
        {
            if (-1 == bind(g_serverData.udpSocket, p->ai_addr, p->ai_addrlen))
            {
#ifdef _WIN32
                closesocket(g_serverData.udpSocket);
#else
                close(g_serverData.udpSocket);
#endif
                g_serverData.udpSocket = -1;
            }
        }
    }

    if (NULL != servinfo)
    {
        freeaddrinfo(servinfo);
    }

    if (g_serverData.udpSocket == -1)
    {
        return -1;
    }

#ifdef _WIN32
    ioctlsocket(g_serverData.udpSocket, FIONREAD, &l);

    {
        BOOL bNewBehavior = FALSE;
        DWORD dwBytesReturned = 0;
        WSAIoctl(g_serverData.udpSocket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
    }
#endif

    g_serverData.sysSocket = prv_createSysSocket();
    if (g_serverData.sysSocket == -1)
    {
        return -1;
    }

    g_serverData.contextP = (iowa_context_t) iowaContextP;

    return 0;
}

// We use sendto() with the remote address stored in the sample_connection_desc_t.
int iowa_system_connection_send(void *connP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData)
{
    sample_connection_desc_t *connDescP;
    int nbSent;

    (void)userData;

    connDescP = (sample_connection_desc_t *)connP;

    nbSent = sendto(g_serverData.udpSocket, buffer, length, 0, (struct sockaddr *) & (connDescP->addr), connDescP->len);

    return nbSent;
}

int iowa_system_connection_recv(void *connP,
                                uint8_t *buffer,
                                size_t length,
                                void *userData)
{
    int numBytes;
    sample_connection_desc_t *connDescP;
    struct sockaddr_storage addr;
    socklen_t len;

    (void)userData;

    connDescP = (sample_connection_desc_t *)connP;

    len = sizeof(addr);

    numBytes = recvfrom(g_serverData.udpSocket, buffer, length, 0, (struct sockaddr *) & addr, &len);

#ifdef _WIN32
    if (numBytes == -1
        && WSAGetLastError() == WSAEMSGSIZE)
    {
        numBytes = length;
    }
#endif

    if (numBytes == -1)
    {
        return -1;
    }

    // check that the sender address matches the connP
    // retrieve the peer address.
    // Drawback of this sample implementation is that some packets may be lost.
    if (len != connDescP->len
        || memcmp(&addr, &connDescP->addr, len) != 0)
    {
        numBytes = 0;
    }

    return numBytes;
}

// In this function, we use select on the sockets in the sample_server_data_t
// and on the sample_server_data_t::pipeArray to be able to interrupt the
// select() if required.
int iowa_system_connection_select(void **connArray,
                                  size_t connCount,
                                  int32_t timeout,
                                  void *userData)
{
    int result;
    sample_connection_desc_t *connDescP;
    struct sockaddr_storage addr;
    socklen_t len;
    struct timeval tv;
    fd_set readfds;
    size_t i;
    int res;
    int maxFd;

    (void)userData;

    len = 0;

    FD_ZERO(&readfds);

    // we do the select on the UDP server socket
    FD_SET(g_serverData.udpSocket, &readfds);
    maxFd = g_serverData.udpSocket;

    // we add an dummy socket to be able to interrupt the select()
    FD_SET(g_serverData.sysSocket, &readfds);
    if (g_serverData.sysSocket > maxFd)
    {
        maxFd = g_serverData.sysSocket;
    }

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    result = select(maxFd + 1, &readfds, NULL, NULL, &tv);

    if (result > 0)
    {
        result = 0;

        if (FD_ISSET(g_serverData.sysSocket, &readfds))
        {
            // this is triggered by iowa_system_connection_interrupt_select()
            uint8_t buffer[1];

            (void)recv(g_serverData.sysSocket, buffer, 1, 0);
        }

        if (FD_ISSET(g_serverData.udpSocket, &readfds))
        {
            // There are some incoming data on the UDP server socket

            // Retrieve the peer address
            len = sizeof(addr);
            res = recvfrom(g_serverData.udpSocket, NULL, 0, MSG_PEEK, (struct sockaddr *) &addr, &len);
#ifdef _WIN32
            if (res == -1
                && WSAGetLastError() == WSAEMSGSIZE)
            {
                res = 0;
            }
#endif
            if (res == -1)
            {
                len = 0;
            }
        }

        if (len != 0)
        {
            size_t i;

            // Keep the connections with data to be read in the array and set the other ones to NULL.
            for (i = 0; i < connCount; i++)
            {
                connDescP = (sample_connection_desc_t *)connArray[i];

                if (len == 0)
                {
                    // No data on the server socket or we already found the matching connection
                    connArray[i] = NULL;
                }
                else
                {
                    if (len != connDescP->len || memcmp(&addr, &connDescP->addr, len) != 0)
                    {
                        // This is not the matching connection
                        connArray[i] = NULL;
                    }
                    else
                    {
                        // We found the matching connection
                        len = 0;
                        result++;
                    }
                }
            }
            if (len != 0)
            {
                // There are data on the server socket but we did not find a matching connection,
                // thus we need a new connection.
                connDescP = (sample_connection_desc_t *)iowa_system_malloc(sizeof(sample_connection_desc_t));
                if (connDescP != NULL)
                {
                    memset(connDescP, 0, sizeof(sample_connection_desc_t));
                    connDescP->len = len;
                    memcpy(&connDescP->addr, &addr, len);

                    if (iowa_server_new_incoming_connection(g_serverData.contextP, IOWA_CONN_DATAGRAM, (void *)connDescP, false) != IOWA_COAP_NO_ERROR)
                    {
                        iowa_system_connection_close(connDescP, userData);
                    }
                }
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
    sample_connection_desc_t *connDescP;

    connDescP = (sample_connection_desc_t *)connP;

    (void)userData;

    // Discarding incoming data on closing connection if any
    // Drawback of this sample implementation is that some packets may be lost.

#ifdef _WIN32
    recvfrom(g_serverData.udpSocket, NULL, 0, 0, NULL, NULL);
#else
    recvfrom(g_serverData.udpSocket, NULL, 0, MSG_DONTWAIT, NULL, NULL);
#endif

    // Freeing the allocated memory
    iowa_system_free(connP);
}

// To make the call to select() in iowa_system_connection_select() stops,
// we write data to the sysSocket if it's empty.
void iowa_system_connection_interrupt_select(void *userData)
{
    uint8_t buffer[1];
    int len;

    (void)userData;

    if (g_serverData.sysSocket != -1)
    {
#ifdef _WIN32
        ioctlsocket(g_serverData.sysSocket, FIONREAD, &len);
        if (len == 0)
#else
        len = recv(g_serverData.sysSocket, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
        if (len == -1)
#endif
        {
            buffer[0] = 'n';
            len = send(g_serverData.sysSocket, buffer, 1, 0);
        }
    }
}

// As the peer identifier, we return the address
size_t iowa_system_connection_get_peer_identifier(void *connP,
                                                  uint8_t *addrP,
                                                  size_t length,
                                                  void *userData)
{
    sample_connection_desc_t *connDescP;

    (void)userData;

    connDescP = (sample_connection_desc_t *)connP;

    if (connDescP->len > length)
    {
        return connDescP->len;
    }

    memcpy(addrP, &(connDescP->addr), connDescP->len);

    return connDescP->len;
}
