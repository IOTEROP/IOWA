##########################################
#
# Copyright (c) 2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 2.8)

project(iowa_samples C)

# The samples expect the IOWA SDK to be present in the iowa folder of this repo.
# You can modify the following line to point to a different location.
set_property(GLOBAL PROPERTY iowa_sdk_folder "${CMAKE_CURRENT_LIST_DIR}/iowa")

# The list of samples.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/1-simple_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/2-IPSO_client)
