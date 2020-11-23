# Security

To enable the Security, [`IOWA_SECURITY_LAYER`][IOWA_SECURITY_LAYER] must not be set to **IOWA_SECURITY_LAYER_NONE**.

IOWA supports two security layers and add the ability to provide your security layer:

- mbed TLS: https://tls.mbed.org/
- tinydtls: https://projects.eclipse.org/projects/iot.tinydtls
- Custom implementation

Additionally, IOWA supports the following security mode with their possible cipher suites (by priority order):

- OSCORE: AES128-CCM8 and AES128-CBCSHA256,
- Pre-Shared Key: AES128-CCM8 and AES128-CBCSHA256,
- Raw Public Key: AES128-CCM8 and AES128-CBCSHA256,
- X.509 (Certificate): AES128-CCM8 and AES128-CBCSHA256.

## Security layers

> **Warning:** There is currently an incompatibility between tinydtls and mbed TLS. Thus A tinydtls client can not connect to mbed TLS server.
>
> The recommended security layer to use is mbed TLS.

### mbed TLS

To use mbed TLS as the security layer, [`IOWA_SECURITY_LAYER`][IOWA_SECURITY_LAYER] can be set with the following defines:

- *IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY* supports only OSCORE mode,
- *IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY* supports Pre-Shared Key mode. Imply supporting OSCORE.
- *IOWA_SECURITY_LAYER_MBEDTLS* supports OSCORE, Pre-Shared Key and X.509 modes,

These defines exist mainly for code size reduction if the application doesn't need to embed the whole mbed TLS stack.

Note that mbed TLS does not support Raw Public Key security mode.

### tinydtls

To use tinydtls as the security layer, [`IOWA_SECURITY_LAYER`][IOWA_SECURITY_LAYER] has to bet set to **IOWA_SECURITY_LAYER_TINYDTLS**.

Note that tinydtls does not support X.509 security mode.

### Custom implementation

To use your security implementation, [`IOWA_SECURITY_LAYER`][IOWA_SECURITY_LAYER] has to bet set to **IOWA_SECURITY_LAYER_USER**.

Please refer to the [Providing your security implementation][Providing your security implementation] section for details.

## How to use the security in IOWA

Find below two examples that could help to understand how to enable encrypted connection between a LwM2M Client and a LwM2M Server.

### Client side

The following example shows how to use a certificate loaded from a file with the security layer mbed TLS. This is in complement of the Client sample using Pre-Shared Key.

The structure to contain the certificates:

```c
typedef struct
{
    iowa_certificate_data_t certData;
} application_data_t;
```

The main implementation:

```c
#include "iowa_client.h"

#include <stdio.h>
#include <stdlib.h>

#define SERVER_URI "coaps://localhost:5684"

#define CERTIFICATES_FOLDER          "certs"
#define CA_CERTIFICATE_FILE          "server.crt"
#define CERTIFICATE_FILE             "client.crt"
#define CERTIFICATE_PRIVATE_KEY_FILE "client.key"

size_t read_certificate_file(char *certFolder,
                             char *certFile,
                             uint8_t **bufferP)
{
    size_t fileLength;
    char *certPath;
    FILE *fileP;

    fileLength = 0;
    certPath = malloc(strlen(certFolder) + strlen(certFile) + 2);
    sprintf(certPath, "%s/%s", certFolder, certFile);

    fileP = fopen(certPath, "rb");
    if (fileP == NULL)
    {
        fprintf(stderr, "Error while opening the file: %s.\r\n", certPath);
        goto function_exit;
    }

    fseek(fileP, 0, SEEK_END);
    fileLength = ftell(fileP);
    if (fileLength == 0)
    {
        fprintf(stderr, "File length is 0.\r\n");
        goto function_exit;
    }
    fseek(fileP, 0, SEEK_SET);

    *bufferP = iowa_system_malloc(fileLength);
    if (*bufferP == NULL)
    {
        fprintf(stderr, "Memory allocation error.\r\n");
        goto function_exit;
    }

    fread(*bufferP, fileLength, 1, fileP);

function_exit:
    if (fileP != NULL)
    {
        fclose(fileP);
    }

    iowa_system_free(certPath);

    return fileLength;
}

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    application_data_t clientData;

    /******************
     * Initialization
     */

    clientData.certData.caCertificateLength = read_certificate_file(CERTIFICATES_FOLDER, CA_CERTIFICATE_FILE, &clientData.certData.caCertificate);
    if (clientData.certData.caCertificateLength == 0)
    {
        return -1;
    }

    clientData.certData.certificateLength = read_certificate_file(CERTIFICATES_FOLDER, CERTIFICATE_FILE, &clientData.certData.certificate);
    if (clientData.certData.certificateLength == 0)
    {
        iowa_system_free(clientData.certData.caCertificate);
        return -1;
    }

    clientData.certData.certificatePrivateKeyLength = read_certificate_file(CERTIFICATES_FOLDER, CERTIFICATE_PRIVATE_KEY_FILE, &clientData.certData.certificatePrivateKey);
    if (clientData.certData.certificatePrivateKeyLength == 0)
    {
        iowa_system_free(clientData.certData.caCertificate);
        iowa_system_free(clientData.certData.certificate);
        return -1;
    }

    // Initialize IOWA
    iowaH = iowa_init(&clientData);

    iowa_client_configure(iowaH, "IOWA_Client", NULL, NULL);

    iowa_client_add_server(iowaH, 1234, SERVER_URI, 0, 0, IOWA_SEC_CERTIFICATE);

    /******************
     * "Main loop"
     */

    do
    {
        // Run for 4 seconds
        result = iowa_step(iowaH, 4);
    } while (result == IOWA_COAP_NO_ERROR)

    iowa_client_remove_server(iowaH, 1234);
    iowa_close(iowaH);

    iowa_system_free(clientData.certData.caCertificate);
    iowa_system_free(clientData.certData.certificate);
    iowa_system_free(clientData.certData.certificatePrivateKey);

    return 0;
}
```

The Platform implementation:

```c
iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP)
{
    application_data_t *clientDataP;

    clientDataP = (application_data_t *)userDataP;

    switch (securityOp)
    {
    case IOWA_SEC_READ:
    {
        switch (securityDataP->securityMode)
        {
        case IOWA_SEC_CERTIFICATE:
            securityDataP->securityMode = IOWA_SEC_CERTIFICATE;

            // Copy the CA certificate
            securityDataP->protocol.certData.caCertificate = clientDataP->certData.caCertificate;
            securityDataP->protocol.certData.caCertificateLen = clientDataP->certData.caCertificateLen;

            // Copy the certificate
            securityDataP->protocol.certData.certificate = clientDataP->certData.certificate;
            securityDataP->protocol.certData.certificateLen = clientDataP->certData.certificateLen;

            // Copy the private key
            securityDataP->protocol.certData.privateKey = clientDataP->certData.privateKey;
            securityDataP->protocol.certData.privateKeyLen = clientDataP->certData.privateKeyLen;
            break;

        default:
            break;
        }

        break;
    }

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}
```

### Server side

The following example shows how to use a certificate loaded from a file with the security layer mbed TLS. This is in complement of the Server sample using Pres-Shared Key.

The example is nearly identical to the Client one, except that [`iowa_system_security_data`](AbstractionLayer.md#iowa_system_security_data) reads only the Server certificate and do not verify Client certificates.

The same structure and the same loading functions can be reused

```c
typedef struct
{
    iowa_certificate_data_t certData;
} application_data_t;
```

The main implementation:

```c
#include "iowa_server.h"

#include <stdio.h>
#include <stdlib.h>

#define CERTIFICATES_FOLDER          "certs"
#define CERTIFICATE_FILE             "server.crt"
#define CERTIFICATE_PRIVATE_KEY_FILE "server.key"

size_t read_certificate_file(char *certFolder,
                             char *certFile,
                             uint8_t **bufferP)
{
    ...
}

int main(int argc,
         char *argv[])
{
    iowa_context_t iowaH;
    iowa_status_t result;
    application_data_t clientData;

    /******************
     * Initialization
     */

    clientData.certData.certificateLength = read_certificate_file(CERTIFICATES_FOLDER, CERTIFICATE_FILE, &clientData.certData.certificate);
    if (clientData.certData.certificateLength == 0)
    {
        iowa_system_free(clientData.certData.caCertificate);
        return -1;
    }

    clientData.certData.certificatePrivateKeyLength = read_certificate_file(CERTIFICATES_FOLDER, CERTIFICATE_PRIVATE_KEY_FILE, &clientData.certData.certificatePrivateKey);
    if (clientData.certData.certificatePrivateKeyLength == 0)
    {
        iowa_system_free(clientData.certData.caCertificate);
        iowa_system_free(clientData.certData.certificate);
        return -1;
    }

    // Initialize IOWA
    iowaH = iowa_init(&clientData);

    iowa_server_configure(iowaH, NULL, NULL, NULL);

    /******************
     * "Main loop"
     */

    do
    {
        // Run for 4 seconds
        result = iowa_step(iowaH, 4);
    } while (result == IOWA_COAP_NO_ERROR)

    iowa_close(iowaH);

    iowa_system_free(clientData.certData.certificate);
    iowa_system_free(clientData.certData.certificatePrivateKey);

    return 0;
}
```

The Platform implementation:

```c
iowa_status_t iowa_system_security_data(const uint8_t *peerIdentity,
                                        size_t peerIdentityLen,
                                        iowa_security_operation_t securityOp,
                                        iowa_security_data_t *securityDataP,
                                        void *userDataP)
{
    application_data_t *clientDataP;

    clientDataP = (application_data_t *)userDataP;

    switch (securityOp)
    {
    case IOWA_SEC_READ:
    {
        switch (securityDataP->securityMode)
        {
        case IOWA_SEC_CERTIFICATE:
            securityDataP->securityMode = IOWA_SEC_CERTIFICATE;

            // Copy the certificate
            securityDataP->protocol.certData.certificate = clientDataP->certData.certificate;
            securityDataP->protocol.certData.certificateLen = clientDataP->certData.certificateLen;

            // Copy the private key
            securityDataP->protocol.certData.privateKey = clientDataP->certData.privateKey;
            securityDataP->protocol.certData.privateKeyLen = clientDataP->certData.privateKeyLen;
            break;

        default:
            break;
        }

        break;
    }

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}
```

## Providing your security implementation

You can replace the provided mbed TLS or tinydtls with your TLS/DTLS implementation.

There are two ways to provide your security implementation: either you re-implement the whole Security component of IOWA as described in [IOWA Components][IOWA Components], or you can use the Security Implementation Abstraction described here.

To do this, first [`IOWA_SECURITY_LAYER`][IOWA_SECURITY_LAYER] must be set to **IOWA_SECURITY_LAYER_USER**. Then you must implement the abstraction functions defined in the file *include/iowa_security_user.h* and documented below.

Depending of your security implementation, you can set the two following defines in *iowa_config.h* to specify if the layer supports RPK and/or Certificate modes. By default, when **IOWA_SECURITY_LAYER_USER** is set, we considered that the two modes are not supported:

- `#define IOWA_SECURITY_RAW_PUBLIC_KEY_SUPPORT`
- `#define IOWA_SECURITY_CERTIFICATE_SUPPORT`

An example of such implementation is provided in the sample *user_security_client*.

## Security Implementation Abstraction Functions

### iowa_user_security_create_client_session

** Prototype **

```c
iowa_status_t iowa_user_security_create_client_session(iowa_security_session_t securityS);
```

** Description **

`iowa_user_security_create_client_session()` initializes implementation internals data of a new client-side security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to initialize.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

\clearpage

### iowa_user_security_create_server_session

** Prototype **

```c
iowa_status_t iowa_user_security_create_server_session(iowa_security_session_t securityS);
```

** Description **

`iowa_user_security_create_server_session()` initializes implementation internals data of a new server-side security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to initialize.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

\clearpage

### iowa_user_security_delete_session

** Prototype **

```c
void iowa_user_security_delete_session(iowa_security_session_t securityS);
```

** Description **

`iowa_user_security_delete_session()` deletes the implementation internals data of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to delete data from.

** Return Value **

None.

\clearpage

### iowa_user_security_handle_handshake_packet

** Prototype **

```c
iowa_status_t iowa_user_security_handle_handshake_packet(iowa_security_session_t securityS);
```

** Description **

`iowa_user_security_handle_handshake_packet()` is called by IOWA when there is incoming data to read while the security session is in the **SECURITY_STATE_HANDSHAKING** state.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session receiving data.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

\clearpage

### iowa_user_security_step

** Prototype **

```c
iowa_status_t iowa_user_security_step(iowa_security_session_t securityS);
```

** Description **

`iowa_user_security_step()` performs the required operations on the security session: updating state, handling timeouts, etc...

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to update.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

** Notes **

This function should update the IOWA context global timeout. See [`iowa_security_session_set_step_delay()`](Security.md#iowa_security_session_set_step_delay).

\clearpage

### iowa_user_security_send

** Prototype **

```c
int iowa_user_security_send(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length);
```

** Description **

`iowa_user_security_send()` encrypts and sends data on a connected security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to send data on.

*buffer*
: The data to send.

*length*
: The length of the data in bytes.

** Return Value **

The number of bytes sent or a negative number in case of error.

\clearpage

### iowa_user_security_recv

** Prototype **

```c
int iowa_user_security_recv(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length);
```

** Description **

`iowa_user_security_recv()` reads and decrypts data received on a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to receives data from.

*buffer*
: A buffer to store the decrypted data.

*length*
: The length of the buffer in bytes.

** Return Value **

The number of received bytes or a negative number in case of error.

\clearpage

### iowa_user_security_disconnect

** Prototype **

```c
void iowa_user_security_disconnect(iowa_security_session_t securityS);
```

** Description **

`iowa_user_security_disconnect()` disconnects a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session to disconnect.

** Return Value **

None.

\clearpage

### iowa_user_security_HKDF

** Prototype **

```c
iowa_status_t iowa_user_security_HKDF(iowa_security_hash_t hash,
                                      uint8_t *IKM, size_t IKMLength,
                                      uint8_t *salt, size_t saltLength,
                                      uint8_t *info, size_t infoLength,
                                      uint8_t *OKM, size_t OKMLength);
```

** Description **

`iowa_user_security_HKDF()` derives a new security key using HMAC-based Extract-and-Expand Key Derivation Function from [RFC 5869](https://tools.ietf.org/html/rfc5869).

** Arguments **

*hash*
: The hash algorithm to use.

*IKM*
: The input keying material.

*IKMLength*
: The length in bytes of *IKM*.

*salt*
: The salt value. This may be nil.

*saltLength*
: The length in bytes of *salt*. This may be 0.

*info*
: The context and application specific information. This may be nil.

*infoLength*
: The length in bytes of *info*. This may be 0.

*OKM*
: The buffer to store the compted output keying material.

*OKMLength*
: The length in bytes of *OKM*.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

\clearpage

### iowa_user_security_AEAD_encrypt

** Prototype **

```c
iowa_status_t
    iowa_user_security_AEAD_encrypt(iowa_security_aead_t aead,
                                    uint8_t *key, size_t keyLength,
                                    uint8_t *nonce, size_t nonceLength,
                                    uint8_t *aad, size_t aadLength,
                                    uint8_t *plainData,
                                    size_t plainDataLength,
                                    uint8_t *encryptedData,
                                    size_t *encryptedDataLengthP,
                                    uint8_t *tag,
                                    size_t tagLength);
```

** Description **

`iowa_user_security_AEAD_encrypt()` encrypts data.

** Arguments **

*aead*
: The AEAD algorithm to use.

*key*
: The key to use to encrypt.

*keyLength*
: The length in bytes of *key*.

*nonce*
: The nonce to use. This may be nil.

*nonceLength*
: The length in bytes of *nonce*. This may be 0.

*aad*
: The additional data to authenticate. This may be nil.

*aadLength*
: The length in bytes of *aad*. This may be 0.

*plainData*
: The data to encrypt.

*plainDataLength*
: The length in bytes of *plainData*.

*encryptedData*
: The buffer to store the encrypted data.

*encryptedDataLengthP*
: IN/OUT. The length in bytes of *encryptedData*. This may be modified by `iowa_user_security_AEAD_encrypt()`.

*tag*
: A buffer to store the authentication tag. This may be nil.

*tagLength*
: The length in bytes of *tag*. This may be 0.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

\clearpage

### iowa_user_security_AEAD_decrypt

** Prototype **

```c
iowa_status_t
    iowa_user_security_AEAD_decrypt(iowa_security_aead_t aead,
                                    uint8_t *key, size_t keyLength,
                                    uint8_t *nonce, size_t nonceLength,
                                    uint8_t *aad, size_t aadLength,
                                    uint8_t *tag, size_t tagLength,
                                    uint8_t *encryptedData,
                                    size_t encryptedDataLength,
                                    uint8_t *plainData,
                                    size_t *plainDataLengthP);
```

** Description **

`iowa_user_security_AEAD_decrypt()` decrypts data.

** Arguments **

*aead*
: The AEAD algorithm to use.

*key*
: The key to use to encrypt.

*keyLength*
: The length in bytes of *key*.

*nonce*
: The nonce to use. This may be nil.

*nonceLength*
: The length in bytes of *nonce*. This may be 0.

*aad*
: The additional data to authenticate. This may be nil.

*aadLength*
: The length in bytes of *aad*. This may be 0.

*tag*
: The authentication tag. This may be nil.

*tagLength*
: The length in bytes of *tag*. This may be 0.

*encryptedData*
: The data to decrypt.

*encryptedDataLength*
: The length in bytes of *encryptedData*.

*plainData*
: The buffer to store the decrypted data.

*plainDataLengthP*
: IN/OUT. The length in bytes of *plainData*. This may be modified by `iowa_user_security_AEAD_decrypt()`.

** Return Value **

**IOWA_COAP_NO_ERROR** in case of success or an error status.

\clearpage

## Data types

### iowa_security_session_t

This type is used to store the session of the IOWA security stack engine. Multiple sessions can exist at the same time. This can be treated as an opaque type and be used through the helper functions below.

### iowa_security_mode_t

```c
typedef uint8_t iowa_security_mode_t;
```

This contains the possible security mode for the Security layer. The following values are defined:

IOWA_SEC_PRE_SHARED_KEY
: Use the Pre-Shared Key mode of the security layer.

IOWA_SEC_RAW_PUBLIC_KEY
: Use the Private/Public Key mode of the security layer.

IOWA_SEC_CERTIFICATE
: Use the X.509 Certificate mode of the security layer.

IOWA_SEC_NONE
: The security layer is not involved in the communication with this peer.

IOWA_SEC_CERTIFICATE_WITH_EST
: Use the X.509 Certificate with EST mode of the security layer.

IOWA_SEC_OSCORE
: Use the OSCORE security of the CoAP layer. This value can be used as a flag to be combined with the other values, e.g. `IOWA_SEC_PRE_SHARED_KEY | IOWA_SEC_OSCORE`.

### iowa_cert_usage_mode_t

```c
typedef uint8_t iowa_cert_usage_mode_t;
```

This contains the possible semantic of the certificate or raw public key, which is used to match the certificate presented in the TLS/DTLS handshake. It is used by the Security layer when the security is **IOWA_SEC_RAW_PUBLIC_KEY**, **IOWA_SEC_CERTIFICATE**, or **IOWA_SEC_CERTIFICATE_WITH_EST**. The following values are defined:

IOWA_CERTIFICATE_USAGE_CA_CONSTRAINT
: CA constraint.

IOWA_CERTIFICATE_USAGE_SERVICE_CERTIFICATE_CONSTRAINT
: Service certificate constraint.

IOWA_CERTIFICATE_USAGE_TRUST_ANCHOR_ASSERTION
: Trust anchor assertion.

IOWA_CERTIFICATE_USAGE_DOMAIN_ISSUED_CERTIFICATE
: Domain-issued certificate.

### iowa_security_state_t

```c
typedef enum
{
    SECURITY_STATE_DISCONNECTED = 0,
    SECURITY_STATE_DISCONNECTING,
    SECURITY_STATE_INIT_HANDSHAKE,
    SECURITY_STATE_HANDSHAKING,
    SECURITY_STATE_HANDSHAKE_DONE,
    SECURITY_STATE_CONNECTED,
    SECURITY_STATE_CONNECTION_FAILING,
    SECURITY_STATE_CONNECTION_FAILED,
} iowa_security_state_t;
```

This reflects the state of the security session. It is an enumeration of the following values:

SECURITY_STATE_DISCONNECTED
: The security session is disconnected.

SECURITY_STATE_DISCONNECTING
: The security session is currently disconnecting.

SECURITY_STATE_INIT_HANDSHAKE
: The security session is starting the handshake with the peer.

SECURITY_STATE_HANDSHAKING
: The security session is in the middle of the handshake with the peer.

SECURITY_STATE_HANDSHAKE_DONE
: The handshake with the peer is done.

SECURITY_STATE_CONNECTED
: The security session is connected to the peer.

SECURITY_STATE_CONNECTION_FAILING
: An error occurred during the security session connection to the peer.

SECURITY_STATE_CONNECTION_FAILED
: The security session failed to connect to the peer.

### iowa_security_event_t

```c
typedef enum
{
    SECURITY_EVENT_UNDEFINED = 0,
    SECURITY_EVENT_DISCONNECTED,
    SECURITY_EVENT_CONNECTED,
    SECURITY_EVENT_DATA_AVAILABLE,
} iowa_security_event_t;
```

This contains the possible events to report to the upper layers. It is an enumeration of the following values:

SECURITY_EVENT_UNDEFINED
: No specific event. This should never be used and only serves as a non default event.

SECURITY_EVENT_DISCONNECTED
: The security session disconnected.

SECURITY_EVENT_CONNECTED
: The security session is now connected.

SECURITY_EVENT_DATA_AVAILABLE
: Decrypted data are available to read on the security session.

### iowa_security_hash_t

```c
typedef enum
{
    SECURITY_HMAC_UNDEFINED = 0,
    SECURITY_HMAC_SHA256_64 = 4,
    SECURITY_HMAC_SHA256    = 5,
    SECURITY_HMAC_SHA384    = 6,
    SECURITY_HMAC_SHA512    = 7
} iowa_security_hash_t;
```

This contains the possible HMAC algorithms used by OSCORE. See table 7 of [RFC 8152](https://tools.ietf.org/html/rfc8152).

SECURITY_HMAC_UNDEFINED
: No specific algorithm. This should never be used and only serves as a non default value.

SECURITY_HMAC_SHA256_64
: HMAC with SHA-256 truncated to 64 bits.

SECURITY_HMAC_SHA256
: HMAC with SHA-256.

SECURITY_HMAC_SHA384
: HMAC with SHA-384.

SECURITY_HMAC_SHA512
: HMAC with SHA-512.

### iowa_security_aead_t

```c
typedef enum
{
    SECURITY_AEAD_UNDEFINED          = 0,
    SECURITY_AEAD_AES_CCM_16_64_128  = 10,
    SECURITY_AEAD_AES_CCM_16_64_256  = 11,
    SECURITY_AEAD_AES_CCM_64_64_128  = 12,
    SECURITY_AEAD_AES_CCM_64_64_256  = 13,
    SECURITY_AEAD_AES_CCM_16_128_128 = 30,
    SECURITY_AEAD_AES_CCM_16_128_256 = 31,
    SECURITY_AEAD_AES_CCM_64_128_128 = 32,
    SECURITY_AEAD_AES_CCM_64_128_256 = 33
} iowa_security_aead_t;
```

This contains the possible AEAD algorithms used by OSCORE. See table 10 of [RFC 8152](https://tools.ietf.org/html/rfc8152).

SECURITY_AEAD_UNDEFINED
: No specific algorithm. This should never be used and only serves as a non default value.

SECURITY_AEAD_AES_CCM_16_64_128
: AES-CCM mode 128-bit key, 64-bit tag, 13-byte nonce.

SECURITY_AEAD_AES_CCM_16_64_256
: AES-CCM mode 256-bit key, 64-bit tag, 13-byte nonce.

SECURITY_AEAD_AES_CCM_64_64_128
: AES-CCM mode 128-bit key, 64-bit tag, 7-byte nonce.

SECURITY_AEAD_AES_CCM_64_64_256
: AES-CCM mode 256-bit key, 64-bit tag, 7-byte nonce.

SECURITY_AEAD_AES_CCM_16_128_128
: AES-CCM mode 128-bit key, 128-bit tag, 13-byte nonce.

SECURITY_AEAD_AES_CCM_16_128_256
: AES-CCM mode 256-bit key, 128-bit tag, 13-byte nonce.

SECURITY_AEAD_AES_CCM_64_128_128
: AES-CCM mode 128-bit key, 128-bit tag, 7-byte nonce.

SECURITY_AEAD_AES_CCM_64_128_256
: AES-CCM mode 256-bit key, 128-bit tag, 7-byte nonce.

\clearpage

## Helper Functions

### iowa_security_session_set_user_internals

** Prototype **

```c
void iowa_security_session_set_user_internals(iowa_security_session_t securityS,
                                              void *internalsP);
```

** Description **

`iowa_security_session_set_user_internals()` stores opaque data in a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session where to store the opaque data.

*internalsP*
: A pointer to the opaque data to store.

** Return Value **

None.

\clearpage

### iowa_security_session_get_user_internals

** Prototype **

```c
void * iowa_security_session_get_user_internals(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_user_internals()` retrieves the opaque data stored in a security session with [`iowa_security_session_set_user_internals()`](Security.md#iowa_security_session_set_user_internals).

** Arguments **

*securityS*
: An iowa_security_session_t pointing to the security session where to store the opaque data.

** Return Value **

A pointer to the stored opaque data.

\clearpage

### iowa_security_session_get_state

** Prototype **

```c
iowa_security_state_t
iowa_security_session_get_state(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_state()` retrieves the state of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

** Return Value **

The state of the security session.

\clearpage

### iowa_security_session_set_state

** Prototype **

```c
void iowa_security_session_set_state(iowa_security_session_t securityS,
                                     iowa_security_state_t state);
```

** Description **

`iowa_security_session_set_state()` sets the state of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

*state*
: The new state of the security session.

** Return Value **

None.

\clearpage

### iowa_security_get_server_session

** Prototype **

```c
iowa_security_session_t iowa_security_get_server_session(iowa_context_t contextP,
                                                         uint16_t shortServerId);

```

** Description **

`iowa_security_get_server_session()` retrieves the security session associated to a Server.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*shortServerId*
: The Short ID assigned to the server.

** Return Value **

The security session if found, otherwise NULL.

\clearpage

### iowa_security_get_client_session

** Prototype **

```c
iowa_security_session_t iowa_security_get_client_session(iowa_context_t contextP,
                                                         uint32_t clientId);

```

** Description **

`iowa_security_get_client_session()` retrieves the security session associated to a Client.

** Arguments **

*contextP*
: An [`iowa_context_t`](CommonAPI.md#iowa_context_t) as returned by [`iowa_init()`](CommonAPI.md#iowa_init). Not checked at runtime.

*clientId*
: The ID assigned to the Client.

** Return Value **

The security session if found, otherwise NULL.

\clearpage

### iowa_security_session_generate_event

** Prototype **

```c
void iowa_security_session_generate_event(iowa_security_session_t securityS,
                                          iowa_security_event_t event);
```

** Description **

`iowa_security_session_generate_event()` calls the event callback of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

*event*
: The event to be passed as parameter to the event callback.

** Return Value **

None.

\clearpage

### iowa_security_session_get_connection_type

** Prototype **

```c
iowa_connection_type_t
iowa_security_session_get_connection_type(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_connection_type()` retrieves the type of the underlying connection of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

** Return Value **

The type of the underlying connection as an [`iowa_connection_type_t`](AbstractionLayer.md#iowa_connection_type_t).

\clearpage

### iowa_security_session_get_security_mode

** Prototype **

```c
iowa_security_mode_t
iowa_security_session_get_security_mode(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_security_mode()` retrieves the security mode of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

** Return Value **

The security mode as an [`iowa_security_mode_t`](Security.md#iowa_security_mode_t).

\clearpage

### iowa_security_session_get_uri

** Prototype **

```c
const char * iowa_security_session_get_uri(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_uri()` retrieves the URI of the peer of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

** Return Value **

The URI of the peer.

\clearpage

### iowa_security_session_set_step_delay

** Prototype **

```c
void iowa_security_session_set_step_delay(iowa_security_session_t securityS,
                                          int32_t delay);
```

** Description **

`iowa_security_session_set_step_delay()` sets the delay before the next scheduled operation on a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

*securityS*
: the time in seconds before [`iowa_user_security_step()`](Security.md#iowa_user_security_step) needs to be call again for this security session.

** Return Value **

None.

** Notes **

If *delay* is a negative number, it is ignored.

As IOWA maintains only one global delay, [`iowa_user_security_step()`](Security.md#iowa_user_security_step) may be called before the delay expires. The delay must be updated at each call.

\clearpage

### iowa_security_session_get_context

** Prototype **

```c
iowa_context_t iowa_security_session_get_context(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_context()` retrieves the IOWA context in which a security session exists.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

** Return Value **

The IOWA context.

\clearpage

### iowa_security_session_get_context_user_data

** Prototype **

```c
void * iowa_security_session_get_context_user_data(iowa_security_session_t securityS);
```

** Description **

`iowa_security_session_get_context_user_data()` retrieves the pointer to application-specific data of the IOWA context in which a security session exists.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

** Return Value **

The pointer to application-specific data as passed to [`iowa_init()`](CommonAPI.md#iowa_init).

\clearpage

### iowa_security_connection_send

** Prototype **

```c
int iowa_security_connection_send(iowa_security_session_t securityS,
                                  uint8_t *buffer,
                                  size_t length);
```

** Description **

`iowa_security_connection_send()` sends data on the underlying connection of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

*buffer*
: The data to send.

*length*
: The length of the data in bytes.

** Return Value **

The number of bytes sent or a negative number in case of error.

\clearpage

### iowa_security_connection_recv

** Prototype **

```c
int iowa_security_connection_recv(iowa_security_session_t securityS,
                                  uint8_t *buffer,
                                  size_t length);
```

** Description **

`iowa_security_connection_recv()` reads data on the underlying connection of a security session.

** Arguments **

*securityS*
: An iowa_security_session_t pointing to a security session.

*buffer*
: A buffer to store the received data.

*length*
: The number of bytes to read.

** Return Value **

The number of bytes read or a negative number in case of error.
