# IOWA Samples

This repository contains sample applications illustrating how to use IOWA.

These samples require the IOWA SDK. This SDK is available from https://ioterop.com/iowa/

## Content

Numbered samples increased in complexity from the simplest LwM2M Client. Other samples demonstrate a particular feature.

Each sample has its own README.md providing more details.

### 1-simple_client

A minimal LwM2M Client with one LwM2M Server and only the required LwM2M Objects in 4 API calls.

### 2-IPSO_client

A LwM2M Client with one LwM2M Server, the required LwM2M Objects, and an IPSO temperature sensor Object.

### 3-IPSO_dynamic_client

A LwM2M Client with one LwM2M Server, the required LwM2M Objects, and an IPSO temperature sensor Object with changing values. This sample demonstrate the multithread support.

### 4-custom_object_client

A LwM2M Client with one LwM2M Server, the required LwM2M Objects, and a custom LwM2M Object.

### fwupdate_pull_client

A LwM2M Client supporting the reception of Firmware Updates in pull mode. This sample demonstrate the CoAP APIs.

### fwupdate_push_client

A LwM2M Client supporting the reception of Firmware Updates in push mode.

### secure_client

A LwM2M Client using mbedtls to secure the communication with the LwM2M Server.

## Usage

This samples can be used either on Linux or Windows.

### Preparation

First either copy the IOWA SDK to the `iowa` folder.

Alternatively, you can edit the root CMakeLists.txt to indicate the path to the IOWA SDK by modifying the line 12:

```
set_property(GLOBAL PROPERTY iowa_sdk_folder "PATH_TO_THE_IOWA_SDK")
```

### Compilation

#### On Linux

**Prerequisites:** The `cmake` utility, the `make` utility and a C compiler.

Go to the 'samples' folder and type the commands 'cmake .' then 'make'.

#### On Windows

**Prerequisites:** Visual Studio with C++ support.

1. In the Visual Studio menu bar, go to "File", "Open", "Folder". Select the IOWA SDK folder.
1. In the "Solution Explorer" windows, expand the "samples" folder.
1. Right-click on "CMakeList.txt" and choose "Set as Startup Item".
1. In the Visual Studio menu bar, go to "Build", "Build All"





