

![IOWA Logo](.images/iowa_logo.png)

# IOWA Samples

This repository contains sample applications illustrating how to use IOWA.

Two kinds of samples depending on your SDK.

Evaluation SDK is included in the repo

Full SDK is available from https://ioterop.com/iowa/

Warning this is not open source code

Whatever your SDK version, start with the sample 01-baseline_client

Each sample has its own README.md explaining it.

## IOWA Evaluation SDK Samples

### 01-baseline_client

The baseline to create a working LwM2M Client with IOWA.

### Custom Object Samples

These samples explain how to add custom LwM2M Objects to your LwM2M Client.

#### 04-custom_object_baseline_client

How to add a simple custom LwM2M Object.

#### 05-custom_object_dynamic

How to make the values exposed by the Object dynamic.

> #### custom_object_multiple_instances
>
> A LwM2M Object with multiple Instances.
>
> #### custom_object_multiple_resources
>
> A LwM2M Object containing a multiple Resource.

## IOWA Full SDK Samples

### 02-IPSO_client

How to add an IPSO temperature sensor Object.

### 03-multithread_IPSO_client

This sample demonstrate the multithread support.

> ### Firmware Update Samples
>
> #### fwupdate_push_client
>
> A LwM2M Client supporting the reception of Firmware Updates in push mode.
>
> #### fwupdate_pull_client
>
> A LwM2M Client supporting the reception of Firmware Updates in pull mode. This sample demonstrate the CoAP APIs.



## Compile Samples

### On Linux

**Prerequisites:** The `cmake` utility, the `make` utility and a C compiler.

Go to the 'samples' folder and type the commands 'cmake .' then 'make'.

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

