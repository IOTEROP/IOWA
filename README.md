![IOWA Logo](.images/IoTerop_logo.jpg)

[![Website](https://img.shields.io/website-up-down-green-red/http/shields.io.svg?label=Documentation&style=for-the-badge)](https://ioterop.github.io/IOWA/)

# IOWA public code

This repository contains a public version of [IOWA LwM2M stack](https://ioterop.com/iowa/), illustrating how to use the LwM2M library.

**Documentation** could be found here: [https://ioterop.github.io/IOWA/](https://ioterop.github.io/IOWA/)

_This code is provided *as-is*, under the associated licence (./LICENSE.txt).
(This code should not be used in a commercial product and is for personal evaluation ONLY)_

With this code, you can jump into LwM2M and validate the build and the execution of the IOWA stack on your device, but with some limitations:

|                                    | IOWA Eval SDK                   | [IOWA Full SDK](https://ioterop.com/iowa/)              |
| ---------------------------------- | ------------------------------- | ------------------------------------------------------- |
|                                    | Free for **Evaluation** purpose | [Contact us](https://info.ioterop.com/contact-products) |
| IOWA-based LwM2M Client            | :heavy_check_mark:              | :heavy_check_mark:                                      |
| IOWA-based LwM2M Server            | :x:                             | :heavy_check_mark:                                      |
| IOWA-based Bootstrap Server        | :x:                             | :heavy_check_mark:                                      |
| LwM2M 1.0​                         | :heavy_check_mark:              | :heavy_check_mark:                                      |
| LwM2M 1.1​                         | :x:                             | :heavy_check_mark:                                      |
| LwM2M 1.2                          | :x:                             | :heavy_check_mark:                                      |
| TLV Data Format                    | :heavy_check_mark:              | :heavy_check_mark:                                      |
| LwM2M 1.1 Data Formats​            | :x:                             | :heavy_check_mark:                                      |
| LwM2M 1.2 Data Formats​            | :x:                             | :heavy_check_mark:                                      |
| Bootstrap ​Support                 | :x:                             | :heavy_check_mark:                                      |
| Mandatory LwM2M Objects​           | :heavy_check_mark:              | :heavy_check_mark:                                      |
| CTO IPSO Objects​                  | :heavy_check_mark:              | :heavy_check_mark:                                      |
| Other IPSO Objects​                | :x:                             | :heavy_check_mark:                                      |
| Custom Objects Creation ​          | :heavy_check_mark:              | :heavy_check_mark:                                      |
|                                    |                                 |                                                         |
| UDP Transport​                     | :heavy_check_mark:              | :heavy_check_mark:                                      |
| TCP Transport​                     | :x:                             | :heavy_check_mark:                                      |
| WebSockets Transport​              | :x:                             | :heavy_check_mark:                                      |
| SMS Transport​                     | :x:                             | :heavy_check_mark:                                      |
| COAP Block​-Wise Transfer          | :x:                             | :heavy_check_mark:                                      |
| DTLS Support ​                     | :heavy_check_mark:              | :heavy_check_mark:                                      |
| TLS Support ​                      | :x:                             | :heavy_check_mark:                                      |
| OSCORE                             | :x:                             | :heavy_check_mark:                                      |
|                                    |                                 |                                                         |
| Muti Server Management  ​          | :heavy_check_mark:              | :heavy_check_mark:                                      |
| Registration Rules Configuration ​ | :x:                             | :heavy_check_mark:                                      |
| Server Access Control              | :x:                             | :heavy_check_mark:                                      |
| Firmware Update​                   | :x:                             | :heavy_check_mark:                                      |
| MQTT Channels Configuration      ​ | :x:                             | :heavy_check_mark:                                      |
| Context Storage                    | :x:                             | :heavy_check_mark:                                      |
| Power Cycle Management             | :x:                             | :heavy_check_mark:                                      |
| Connectivity Management            | :x:                             | :heavy_check_mark:                                      |
| Notification Storage               | :x:                             | :heavy_check_mark:                                      |
| Asynchronous Response Support      | :x:                             | :heavy_check_mark:                                      |
| Payload Streaming                  | :x:                             | :heavy_check_mark:                                      |
|                                    |                                 |                                                         |
| Source Code & C-Make​              | :heavy_check_mark:              | :heavy_check_mark:                                      |
| Full Documentation​                | :x:                             | :heavy_check_mark:                                      |
| Logs                               | :heavy_check_mark:              | :heavy_check_mark:                                      |
| Multithread​ Support               | :x:                             | :heavy_check_mark:                                      |
| Python Binding                     | :x:                             | :heavy_check_mark:                                      |
| LwM2M Object Code Generation Tool  | :x:                             | :heavy_check_mark:                                      |

### Sdk samples available on this repository

| Category | Sample name | Purpose |
| --- | --- | --- |
| Basic samples | **01-baseline_client** | For creating a working LwM2M Client with IOWA |
| Basic samples | **02-IPSO_client** | How to add an IPSO temperature sensor Object |
| Custom Object | **03-custom_object_baseline_client** | How to add a simple custom LwM2M Object |
| Custom Object | **04-custom_object_dynamic** | How to make the values exposed by the custom Object dynamic |
| | |  |
| _Full SDK only_ | __01-multithread_IPSO_client__ |_This sample demonstrates the multi-thread support (full SDK only)_ |


### Extra IOWA Sdk samples (available on request)
| Category | Sample name | Purpose |
| --- | --- | --- |
| _Full SDK only_ | secure_client |  How to use secure communications. |
| _Full SDK only_ | fwupdate_push_client |  How to receive Firmware Updates in push mode. |
| _Full SDK only_ | fwupdate_pull_client |  How to receive Firmware Updates in pull mode. This sample demonstrates the CoAP APIs. |
| _Full SDK only_ | timestamp_IPSO |  How to timestamp values in IPSO sensors. |
| _Full SDK only_ | timestamp_custom_objects |  How to timestamp values in custom LwM2M Objects. |
| _Full SDK only_ | streamable_resources |  How to work with large values like images or file contents. |
| _Full SDK only_ | asynchronous_resources |  How to work with time consuming value retrieval. |
| _Full SDK only_ | low_MTU_client  |  How to use adapt to limited network MTU. |
| _Full SDK only_ | adaptive_client |  How to adapt to degraded network conditions by modifying the CoAP timers. |

## QuickStart Guide

All the samples can be built/run on Windows and Linux. Other platforms and OS are available in [IoTerop's GitHub](https://github.com/IOTEROP).

If you prefer, a Docker version is available in the *docker* folder.

### Tutorials

You can find some interesting tutorials on [Hackster.io](https://www.hackster.io/):

- [Device management with LwM2M IOWA stack - Tutorial 1](https://www.hackster.io/ioterop/device-management-with-lwm2m-iowa-stack-tutorial-1-e5aaf8)

- [Device management with LwM2M IOWA stack #2: ESP32](https://www.hackster.io/ioterop/device-management-with-lwm2m-iowa-stack-2-esp32-fcb294)

### Compile Samples

Initial setup: Don't forget to clone this repository:

`git clone https://github.com/IOTEROP/IOWA.git`

#### On Linux

**Prerequisites:** An x86-64 computer with a Linux distribution installed, the `cmake` utility, the `make` utility and a C compiler.

To compile the *08-secure_client_tinydtls* sample, you will also need `git` to be installed.

1. Inside the IOWA repository, create a build folder
   
   `mkdir build`

2. Go to this folder
   
   `cd build`

3. Launch cmake in debug mode
   
   `cmake -DCMAKE_BUILD_TYPE=Debug ..`
   
   The last parameter point to the folder containing the CMakeLists.txt file of your target. In this case the one at the root of the repo.

4. Compile the samples.
   
   `make -j 4`
   
   (The `-j 4` parameter enables four parallel compilations, could be replaced with `make -j$(nproc)`)
   
   (After making some modifications to the code, only the step 4 is required)

5. Jump into the associated directory. E.g:
   
   `cd samples/01-baseline_client `

6. Run the sample. E.g:
   
   `./baseline_client`

#### On Windows

##### Using Visual Studio Code

1. Install the Microsoft C++ compiler as explained here: https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019
    1. Select the "Build Tools for Visual Studio 2019".
    2. In the Installer, make sure the following optional features are checked:
        * MSVC v142 - VS 2019 C++ x64/x86 build tools (Note that the version may differ)
        * Windows 10 SDK
        * C++ CMake tools for Windows
2. Install Visual Studio Code from https://code.visualstudio.com/
3. Launch Visual Studio Code.
4. Go to the "Extensions" panel (Ctrl+Shift+X) on the left side.
5. Install the "C/C++", "CMake", and "CMake Tools" extensions
6. Open the folder containing the IOWA SDK ("File" menu -> "Open Folder..." or "Ctrl+K Ctrl+O")
7. Open the CMake panel on the left side.
8. On the top bar of the CMake panel, click on the icon "Configure All Projects".
9. When prompted to select a kit, choose one of the Visual Studio Build Tools.
10. On the top bar of the CMake panel, click on the icon "Build All Projects".
11. Click on the sample of your choice.
12. Right-click on the application and select "Run in terminal"

##### Using Visual Studio with C++ support.

Visual Studio version must be at least 2017 for the CMake support.

1. In the Visual Studio menu bar, go to "File", "Open", "Folder". Select the IOWA folder.
2. In the "Solution Explorer" windows, right-click on "CMakeList.txt" and choose "Set as Startup Item".
3. In the Visual Studio menu bar, go to "Build", "Build All"

### IOWA Connecticut Server

You can interact with the samples or your device using the [Ioterop Connecticut server](https://iowa-server.ioterop.com). This server can get/send commands/data with your device and validate the way your code runs.
(*Connecticut* is the LwM2M Ioterop test server where your device will be connected. This is one solution among others (e.g.: [ALASKA platform](https://ioterop.com/alaska/), ...).

## Let's Get Programming

------------------------------------------------------------

![IoTerop Logo](.images/IoTerop_logo.jpg)
