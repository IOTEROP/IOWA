# IOWA Samples

This repository contains sample applications illustrating how to use IOWA.

These samples require the IOWA SDK. This SDK is available from https://ioterop.com/iowa/

## Content

Numbered samples increase in complexity from the simplest LwM2M Client. Other samples demonstrate a particular feature.

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

First, copy the IOWA SDK to the `iowa` folder.

Alternatively, you can edit the root CMakeLists.txt to indicate the path to the IOWA SDK by modifying the line 12:

```
set_property(GLOBAL PROPERTY iowa_sdk_folder "PATH_TO_THE_IOWA_SDK")
```

### Compilation

#### On Linux

**Prerequisites:** The `cmake` utility, the `make` utility and a C compiler.

Go to the 'samples' folder and type the commands 'cmake .' then 'make'.

#### On Windows

##### Using Visual Studio Code

1. Install the Microsoft C++ compiler as explained here: https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019
   1. Select the "Build Tools for Visual Studio 2019".
   2. In the Installer, make sure the following optional features are checked:
      * MSVC v142 - VS 2019 C++ x64/x86 build tools (Note that the version may differ)
      * Windows 10 SDK
      * C++ CMake tools for Windows
1. Install Visual Studio Code from https://code.visualstudio.com/
1. Launch Visual Studio Code.
1. Go to the "Extensions" panel (Ctrl+Shift+X) on the left side.
1. Install the "C/C++", "CMake", and "CMake Tools" extensions
1. Open the folder containing the IOWA Samples ("File" menu -> "Open Folder..." or "Ctrl+K Ctrl+O")
1. Open the CMake panel on the left side.
1. On the top bar of the CMake panel, click on the icon "Configure All Projects".
1. When prompted to select a kit, choose one of the Visual Studio Build Tools.
1. On the top bar of the CMake panel, click on the icon "Build All Projects".
1. Click on the sample of your choice.
1. Right-click on the application and select "Run in terminal"

##### Using Visual Studio with C++ support.

Visual Studio version must be at least 2017 for the CMake support.

1. In the Visual Studio menu bar, go to "File", "Open", "Folder". Select the IOWA Samples folder.
1. In the "Solution Explorer" windows, right-click on "CMakeList.txt" and choose "Set as Startup Item".
1. In the Visual Studio menu bar, go to "Build", "Build All"





