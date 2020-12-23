![IOWA Logo](.images/IoTerop_logo.jpg)

[![Website](https://img.shields.io/website-up-down-green-red/http/shields.io.svg?label=Documentation&style=for-the-badge)](https://ioterop.github.io/IOWA/)

# IOWA public code

This repository contains a public version of [IOWA LwM2M stack](https://ioterop.com/iowa/), illustrating how to use the LwM2M library.

[![IOWA Docs](.images/documentation.png)](https://ioterop.github.io/IOWA/) **Documentation** could be found here: [https://ioterop.github.io/IOWA/](https://ioterop.github.io/IOWA/)

_This code is provided *as-is*, under the associated licence (./LICENSE.txt).
(This code should not be used in a commercial product and is for personal evaluation ONLY)_

With this code, you can jump into LwM2M and validate the build and the execution of the IOWA stack on your device, but with some limitations:

|         | IOWA Eval SDK           | IOWA Full SDK  |
| --- | --- | --- |
| | Free for **Non Commercial** Usage | [Contact us](https://ioterop.com/iowa/)|
| Source Code & C-Make​ | :heavy_check_mark: | :heavy_check_mark: |
| LwM2M 1.0​ | :heavy_check_mark: | :heavy_check_mark: |
| LwM2M 1.1​ | :x: | :heavy_check_mark: |
| Mandatory LwM2M Objects​ | :heavy_check_mark: | :heavy_check_mark: |
| Full IPSO Objects​ |  :x:  | :heavy_check_mark: |
| UDP Transport​ | :heavy_check_mark: | :heavy_check_mark: |
| TCP Transport​ |  :x:  | :heavy_check_mark: |
| COAP Block​ | :heavy_check_mark: | :heavy_check_mark: |
| LwM2M 1.1 data formats​ |  :x:  | :heavy_check_mark: |
| Bootstrap ​ |  :x:  | :heavy_check_mark: |
| Firmware Update​ |  :x:  | :heavy_check_mark: |
| Multithread​ |  :x:  | :heavy_check_mark: |


Two kinds of samples are available on this repository. The first set of samples can be used as a reference for evaluating this code. The second set (refered as _Full_SDK_Samples_) is for information only, as it is *NOT BUILDABLE* without the *Full SDK*

The *Full SDK" and additional information is available from [https://ioterop.com/iowa/](https://ioterop.com/iowa/).

Additional technical contents and how-to's may be found at [https://ioterop.com/iot-dev-24-7/](https://ioterop.com/iot-dev-24-7/).

Please note: **Neither the SDK nor the samples are open source software but commercial software.**

Whatever SDK (Eval or Full) you are using, start with the sample **01-baseline_client**.

# IOWA SDK samples

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

# Quickstart guide

All the samples can be built/run on Windows and Linux. Other platforms and OS (FreeRTOS, Zephyr, Android, ...) are available on request.

## Tutorials

You can find some interesting tutorials on [Hackster.io](https://www.hackster.io/):

- [Device management with LwM2M IOWA stack - Tutorial 1](https://www.hackster.io/ioterop/device-management-with-lwm2m-iowa-stack-tutorial-1-e5aaf8)

- [Device management with LwM2M IOWA stack #2: ESP32](https://www.hackster.io/ioterop/device-management-with-lwm2m-iowa-stack-2-esp32-fcb294)

## Compile Samples

Initial setup: Don't forget to clone this repository:

   `git clone --recurse-submodules https://github.com/IOTEROP/IOWA.git`

### On Linux

**Prerequisites:** An x86-64 computer with a Linux distribution installed, the `cmake` utility, the `make` utility and a C compiler.

1. Inside the IOWA repositiory, create a build folder

   `mkdir build`

2. Go to this folder

   `cd build`

3. Launch cmake in debug mode

   `cmake -DCMAKE_BUILD_TYPE=Debug ..`

   The last parameter point to the folder containing the CMakeLists.txt file of your target. In this case the one at the root of the repo including all the samples.

   If you want, you can define the IOWA Client name with -DIOWA_DEV_NAME:"..."

   (e.g.: `cmake -DCMAKE_BUILD_TYPE=Debug -DIOWA_DEV_NAME="MyDevice" ..`)

4. Build the client and the server.

   `make -j 4`

   ( the `-j 4` parameter enables four parallel compilations, could be replaced with `make -j$(nproc)`)

  (After making some modifications to the code, only the step 4 is required)

5. Jump into the associated directory. E.g:

  `cd samples/01-baseline_client `

6. Run the sample. E.g:

  `./baseline_client`

### On Windows

#### Using Visual Studio Code

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
6. Open the folder containing the IOWA Samples ("File" menu -> "Open Folder..." or "Ctrl+K Ctrl+O")
7. Open the CMake panel on the left side.
8. On the top bar of the CMake panel, click on the icon "Configure All Projects".
9. When prompted to select a kit, choose one of the Visual Studio Build Tools.
10. On the top bar of the CMake panel, click on the icon "Build All Projects".
11. Click on the sample of your choice.
12. Right-click on the application and select "Run in terminal"

#### Using Visual Studio with C++ support.

Visual Studio version must be at least 2017 for the CMake support.

1. In the Visual Studio menu bar, go to "File", "Open", "Folder". Select the IOWA Samples folder.
1. In the "Solution Explorer" windows, right-click on "CMakeList.txt" and choose "Set as Startup Item".
1. In the Visual Studio menu bar, go to "Build", "Build All"

# To go further with your code...

By default, sample codes are built with the "verbose" option, so don't be afraid if your terminal is filled with many traces ! The important line to find is the reference name of your client (based on hardware values). You can customize it in the code. E.g:
```
...
[info:lwm2m:lwm2m_configure:137] endpointName: "IOWA_sample_client_8323329", msisdn: "NULL"
...
```
## IOWA Connecticut server
You can interact with your device using the [Ioterop Connecticut server](https://iowa-server.ioterop.com) (https://iowa-server.ioterop.com). This server can get/send commands/datas with your device and validate the way your code runs.
(* Connecticut* is the LwM2M Ioterop test server where your device will be connected. This is one solution among others (e.g.: [Alaska platform](https://ioterop.com/alaska/), ...).

![Connecticut](.images/connecticut.png?raw=true "Title")


## Let's Get Programming

------------------------------------------------------------

![IoTerop Logo](.images/IoTerop_logo.jpg)
