##########################################
#
# Copyright (c) 2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 3.5)

project(IOWA_Full_SDK_samples C)

# The list of samples.a
MESSAGE(WARNING "********************************************************************\n"
		"** The Full SDK samples *NEED* the full SDK version to build !!! ***\n"
                "********************************************************************")								

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/01-multithread_IPSO_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/02-bearer_selection_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/03-baseline_server)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/04-mqtt_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/05-lwm2m_1_1_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/06-bootstrap_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/07-streamable_resource_client)
