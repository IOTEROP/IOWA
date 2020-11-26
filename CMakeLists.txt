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

# The list of samples.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Evaluation_SDK_Samples)

#The following samples are excluded from the build because they are based on the full version
# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Full_SDK_Samples)
