![IOWA Logo](.images/IoTerop_logo.jpg)

# IOWA Samples

This repository contains sample applications illustrating how to use IOWA.

Two kinds of samples are available depending on your SDK. One set of samples for the Evaluation SDK and a second set for the Full SDK.

The Evaluation SDK is available in the repository below.

The full SDK and additional information is available from [https://ioterop.com/iowa/](https://ioterop.com/iowa/).

Additional technical contents and how-to's may be found at [https://ioterop.com/iot-dev-24-7/](https://ioterop.com/iot-dev-24-7/).

Please note: **Neither the SDK nor the samples are open source software but commercial software.**

Whatever SDK you are using, start with the sample 01-baseline_client

Each sample has its own README.md explaining it.

## IOWA Evaluation SDK Samples

### 01-baseline_client

For creating a working LwM2M Client with IOWA.

### 02-IPSO_client

How to add an IPSO temperature sensor Object.

### Custom Object Samples

These samples explain how to create, define, and add custom LwM2M Objects to your LwM2M Client.

#### 03-custom_object_baseline_client

How to add a simple custom LwM2M Object.

#### 04-custom_object_dynamic

How to make the values exposed by the custom Object dynamic.

### COMING SOON

* **custom_object_multiple_instances**

  A LwM2M Object with multiple Instances.

* **custom_object_multiple_resources**

  A LwM2M Object containing a multiple Resource.

* **event_callback**

  How to monitor the LwM2M Operations from the application.

* **sleeping_client**

  How to make the device enter sleep state without disturbing the LwM2M session.

## IOWA Full SDK Samples

### Preparation

To use these samples, you need the [IOWA Full SDK](https://ioterop.com/iowa/).

You can either copy the IOWA Full SDK to the **iowa** folder at the root of the repo, or you can edit the root **CMakeLists.txt** to indicate the path to the IOWA Full SDK by modifying the line 14:

```
set_property(GLOBAL PROPERTY iowa_sdk_folder "PATH_TO_THE_IOWA_SDK")
```

### 01-multithread_IPSO_client

This sample demonstrate the multi-thread support.

### COMING SOON

* **secure_client**

  How to use secure communications.

* **fwupdate_push_client**

  How to receive Firmware Updates in push mode.

* **fwupdate_pull_client**

  How to receive Firmware Updates in pull mode. This sample demonstrates the CoAP APIs.

* **timestamp_IPSO**

  How to timestamp values in IPSO sensors.

* **timestamp_custom_objects**

  How to timestamp values in custom LwM2M Objects.

* **streamable_resources**

  How to work with large values like images or file contents.

* **asynchronous_resources**

  How to work with time consuming value retrieval.

* **low_MTU_client**

  How to use adapt to limited network MTU.

* **adaptive_client**

  How to adapt to degraded network conditions by modifying the CoAP timers.

## Compile Samples

### On Linux

**Prerequisites:** An x86-64 computer with a Linux distribution installed, the `cmake` utility, the `make` utility and a C compiler.

1. Create a build folder

   `mkdir build`

2. Go to this folder

   `cd build`

3. Launch cmake in debug mode

   `cmake -DCMAKE_BUILD_TYPE=Debug ..`

   (the last parameter point to the folder containing the CMakeLists.txt file of your target. In this case the one at the root of the repo including all the samples)

4. Build the client and the server

   `make -j 4`

   ( the `-j 4` parameter enables four parallel compilations)

After making some modifications to the code, only the step 4 is required.

### On Windows

#### Using Visual Studio Code

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

#### Using Visual Studio with C++ support.

Visual Studio version must be at least 2017 for the CMake support.

1. In the Visual Studio menu bar, go to "File", "Open", "Folder". Select the IOWA Samples folder.
1. In the "Solution Explorer" windows, right-click on "CMakeList.txt" and choose "Set as Startup Item".
1. In the Visual Studio menu bar, go to "Build", "Build All"

## Let's Get Programming

------------------------------------------------------------

![IoTerop Logo](.images/IoTerop_logo.jpg)
