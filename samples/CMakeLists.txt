##########################################
#
# Copyright (c) 2016-2021 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 3.5)

project(IOWA_Evaluation_SDK_samples C)

get_property(IOWA_DIR GLOBAL PROPERTY iowa_sdk_folder)
if (NOT IOWA_DIR)
    # The samples expect the IOWA SDK to be present in the parent folder.
    # You can modify the following line to point to a different location.
    set_property(GLOBAL PROPERTY iowa_sdk_folder "${CMAKE_CURRENT_LIST_DIR}/../iowa")
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/01-baseline_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/02-IPSO_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/03-custom_object_baseline)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/04-custom_object_dynamic)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/05-custom_object_multiple_instances)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/06-custom_object_multiple_resource_instances)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/07-secure_client_mbedtls3)
if (NOT WIN32)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/08-secure_client_tinydtls)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
