##########################################
#
# Copyright (c) 2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 2.8)

project(IOWA_Full_SDK_samples C)

# The list of samples.a

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/01-multithread_IPSO_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/05-streamable_resource_client)
