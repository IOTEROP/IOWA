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

typedef struct
{
    // We need the iowa context for calling iowa_connection_closed()
    iowa_context_t contextP;

    // a mutex for iowa_system_mutex_* functions
#ifdef _WIN32
    HANDLE mutex;
#else
    pthread_mutex_t mutex;
#endif

    // a socket to interrupt the select()
    int sysSocket;
} sample_platform_data_t;

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

// to set the iowa context once initialized.
void platform_data_set_iowa_context(void *userData,
                                    iowa_context_t contextP)
{
    sample_platform_data_t *dataP;

    dataP = (sample_platform_data_t *)userData;

    dataP->contextP = contextP;
}

void free_platform_data(void *userData)
{
    sample_platform_data_t *dataP;

    dataP = (sample_platform_data_t *)userData;

#ifdef _WIN32
    CloseHandle(dataP->mutex);
    if (dataP->sysSocket != -1)
    {
        closesocket(dataP->sysSocket);
    }
#else
    pthread_mutex_destroy(&(dataP->mutex));
    if (dataP->sysSocket != -1)
    {
        close(dataP->sysSocket);
    }
#endif

    free(dataP);
}

// This function initializes data used in the system abstraction functions.
// The allocated stucture will be past as userData to the other functions in this file.
void * get_platform_data()
{
    sample_platform_data_t *dataP;

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

    dataP = (sample_platform_data_t *)malloc(sizeof(sample_platform_data_t));
    if (dataP == NULL)
    {
        return NULL;
    }

#ifdef _WIN32
    dataP->mutex = CreateMutex(NULL, FALSE, NULL);
    if (dataP->mutex == NULL)
    {
#else
    if (pthread_mutex_init(&(dataP->mutex), NULL) != 0)
    {
#endif
        goto error;
    }

    dataP->sysSocket = prv_createSysSocket();
    if (dataP->sysSocket == -1)
    {
        goto error;
    }

    return (void *)dataP;

error:
    free_platform_data(dataP);
    return NULL;
}

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



// We consider only TCP connections.
// We open an TCP socket binded to the the remote address.
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

    // let's consider only TCP connection in this sample
    if (type != IOWA_CONN_STREAM)
    {
        return NULL;
    }

#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
    {
        return NULL;
    }
#endif
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

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

#ifdef _WIN32
    nbSent = send(sock, buffer, length, 0);
#else
    nbSent = send(sock, buffer, length, MSG_NOSIGNAL);
#endif

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
    sample_platform_data_t *dataP;
    int maxFd;
    int fd;

    dataP = (sample_platform_data_t *)userData;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);

    // we add an dummy socket to be able to interrupt the select()
    FD_SET(dataP->sysSocket, &readfds);
    maxFd = dataP->sysSocket;

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
        result = 0;

        if (FD_ISSET(dataP->sysSocket, &readfds))
        {
            uint8_t buffer[1];

            (void)recv(dataP->sysSocket, buffer, 1, 0);
        }

        for (i = 0; i < connCount; i++)
        {
           if (FD_ISSET(prv_pointerToSock(connArray[i]), &readfds))
            {
                int res;
                char unused;

                res = recv(prv_pointerToSock(connArray[i]), &unused, 1, MSG_PEEK);
                if (res <= 0)
                {
                    iowa_connection_closed(dataP->contextP, connArray[i]);
                    connArray[i] = NULL;
                }
                else
                {
                    result++;
                }
            }
            else
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

// To make the call to select() in iowa_system_connection_select() stops,
// we write data to the sysSocket if it's empty.
void iowa_system_connection_interrupt_select(void *userData)
{
    sample_platform_data_t *dataP;
    uint8_t buffer[1];
    int len;

    dataP = (sample_platform_data_t *)userData;

    if (dataP->sysSocket != -1)
    {
#ifdef _WIN32
        ioctlsocket(dataP->sysSocket, FIONREAD, &len);
        if (len == 0)
#else
        len = recv(dataP->sysSocket, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
        if (len == -1)
#endif
        {
            buffer[0] = 'n';
            len = send(dataP->sysSocket, buffer, 1, 0);
        }
    }
}

void iowa_system_mutex_lock(void *userData)
{
    sample_platform_data_t *dataP;

    dataP = (sample_platform_data_t *)userData;

#ifdef _WIN32
    WaitForSingleObject(dataP->mutex, INFINITE);
#else
    pthread_mutex_lock(&(dataP->mutex));
#endif
}

void iowa_system_mutex_unlock(void *userData)
{
    sample_platform_data_t *dataP;

    dataP = (sample_platform_data_t *)userData;

#ifdef _WIN32
    ReleaseMutex(dataP->mutex);
#else
    pthread_mutex_unlock(&(dataP->mutex));
#endif
}

// This is not a proper way to generate a random vector, it only serves as an example here
int iowa_system_random_vector_generator(uint8_t *randomBuffer,
                                        size_t size,
                                        void *userData)
{
    size_t i;

    (void)userData;

    for (i = 0; i < size; i++)
    {
        randomBuffer[i] = rand() % 256;
    }

    return 0;
}

// In these samples, we return hard coded values.
#define SERVER_URI           "coaps+tcp://127.0.0.1:5684"
#define BOOTSTRAP_SERVER_URI "coaps+tcp://127.0.0.1:5784"
#define PSK_IDENTITY         "IOWA"
#define PSK_KEY              "123456"

iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP)
{
    (void)userDataP;

    switch (securityOp)
    {
    case IOWA_SEC_READ:
        switch (securityDataP->securityMode)
        {
        case IOWA_SEC_PRE_SHARED_KEY:
            if ((peerIdentityLen == strlen(SERVER_URI) && memcmp(peerIdentity, SERVER_URI, peerIdentityLen) == 0)
                || (peerIdentityLen == strlen(BOOTSTRAP_SERVER_URI) && memcmp(peerIdentity, BOOTSTRAP_SERVER_URI, peerIdentityLen) == 0))
            {
                securityDataP->protocol.pskData.identityLen = strlen(PSK_IDENTITY);
                securityDataP->protocol.pskData.identity = (uint8_t *)PSK_IDENTITY;

                securityDataP->protocol.pskData.privateKeyLen = strlen(PSK_KEY);
                securityDataP->protocol.pskData.privateKey = (uint8_t *)PSK_KEY;

                return IOWA_COAP_NO_ERROR;
            }

            // The peer does not match known Server URIs
            return IOWA_COAP_404_NOT_FOUND;

        default:
            // Implementation for other modes is not done in this example.
            break;
        }
        break;

    case IOWA_SEC_FREE:
        // Nothing to do here
        return IOWA_COAP_NO_ERROR;

    default:
        // Implementation for other operations is not done in this example.
        break;
    }

    return IOWA_COAP_501_NOT_IMPLEMENTED;
}
