##########################################
#
# Copyright (c) 2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 2.8)

project(IOWA_samples C)

# The samples expect the IOWA SDK to be present in the iowa folder of this repo.
# You can modify the following line to point to a different location.
set_property(GLOBAL PROPERTY iowa_sdk_folder "${CMAKE_CURRENT_LIST_DIR}/iowa")

# Will use the default name (IOWA_sample_client) or
# the specified with the config line:
# e.g.: cmake -DCMAKE_BUILD_TYPE=Debug -DIOWA_DEV_NAME="TestDevice" ..
#
if(DEFINED IOWA_DEV_NAME)
    add_definitions(-DIOWA_DEVICE_NAME="${IOWA_DEV_NAME}")
endif()

# The list of samples.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/samples)

