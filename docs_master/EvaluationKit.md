# IOWA Evaluation Kit

## Content

doc
: The IOWA APIs reference document.

externals
: Open source code used by IOWA.

include
: The IOWA header files.

lib
: The IOWA prebuilt libraries.

samples
: Sample applications.

## Samples

IOWA SDK comes with several sample applications:

samples/client
: A sample LwM2M Client using IOWA. This client features two IPSO Objects.

samples/client_1_1
: A sample LwM2M 1.1 Client over TCP using IOWA. This client features two IPSO Objects. When values are updated, the client performs a data push operation with the IPSO Voltage Object values.

samples/custom_object_client
: A sample LwM2M Client using IOWA. This client implements a custom LwM2M Object.

samples/fw_update_client
: A sample LwM2M Client using IOWA with (simulated) Firmware Update capability.

samples/bootstrap_client
: A sample LwM2M Client using IOWA. This client is configured with only a LwM2M Bootstrap Server.

samples/user_security_client
: A sample LwM2M Client using IOWA but implementing its security layer.

samples/server
: A sample LwM2M Server using IOWA. This server listens for client registrations on UDP ports 5683 and 5684 (for DTLS connections). When a LwM2M Client registers, the Server reads the Device Object of the Client and, if possible, sets observations on the IPSO Temperature Object Instances.

samples/server_1_1
: A sample LwM2M 1.1 Server using IOWA. This server listens for client registrations on TCP ports 5683 and 5684 (for TLS connections). When a LwM2M Client registers, the Server makes a Read-Composite on the Device and Server Objects of the Client and, if possible, sets observations on the IPSO Temperature Object Instances.

samples/bootstrap_server
: A sample LwM2M Bootstrap Server using IOWA. This bootstrap server listens for client bootstrap requests on UDP ports 5783 and 5784 (for DTLS connections). When receiving a Bootstrap Request from a LwM2M Client, it provisions a LwM2M Server Account with URL "coap://127.0.0.1:5683".

## Compilation

**Prerequisites:** An x86-64 computer with a Linux distribution installed. The `make` utility and a C compiler.

Go to each sample folder and type the command 'make'. This builds each of the sample programs.

## Testing

### Without Security

- Launch the previously built 'server'.
- Launch the previously built 'client' on the same computer.

### With PSK security

- Launch the 'server' previously built.
- Edit the *samples/client/main.c* file to go from:
```c
    // Add the information of a LwM2M Server to connect to
    result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_URI, 300, 0, IOWA_SEC_NONE);
    // or if you want to use a secure connection
    // result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_SECURE_URI, 300, 0, IOWA_SEC_PRE_SHARED_KEY);
```
to:
```c
    // Add the information of a LwM2M Server to connect to
    // result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_URI, 300, 0, IOWA_SEC_NONE);
    // or if you want to use a secure connection
    result = iowa_client_add_server(iowaH, SERVER_SHORT_ID, SERVER_SECURE_URI, 300, 0, IOWA_SEC_PRE_SHARED_KEY);
```
- Rebuild and launch 'client'.
