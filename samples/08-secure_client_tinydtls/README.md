# Secure Client with tinydtls

This is the Baseline Client using [tinydtls](https://github.com/eclipse/tinydtls) to secure its exchanges with the LwM2M Server.

## Configuration

Before compiling and running, this sample requires some configuration to setup the security credentials.

This sample uses Pre-Shared Key security. You will need to decide on a key to use. Any byte stream will do. You will also need a key identity. For convenience, this identity is usually an human-readable string.

### Client-side Setup

You need to edit the file *samples/07-secure_client_mbedtls3/sample_env.h* to configure your client:

(PSK credentials) enter your key and its identity:
```c
// PSK security credentials
    #define SAMPLE_PSK_IDENTITY     " /*Enter your Pre-Shared Key identity here as String */"
    #define SAMPLE_PSK_KEY          {/*Enter your Pre-Shared Key value here as bytes array*/ }
```
For instance:
```c
// PSK security credentials
    #define SAMPLE_PSK_IDENTITY  "MyTestID"
    #define SAMPLE_PSK_KEY  {'T','e','s','t','K','E','Y'}
```

Client Name (should be unique)
```c
    #define SAMPLE_ENDPOINT_NAME    "/*Unique endpoint name */"
```

### Server-side Setup

```👓```  *Please refers to [ALASKA](https://alaska.ioterop.com/) documentation, to provision your device with security credentials.*

## Usage

The usage is the same as the [Baseline Client](baseline_client.md) sample.

> If the Client fails to connect to the Server, it is possible that the key identity and/or the client name you chose are alredy in use on the Server.
