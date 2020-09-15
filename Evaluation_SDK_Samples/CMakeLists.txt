##########################################
#
# Copyright (c) 2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 2.8)

project(IOWA_Evaluation_SDK_samples C)

# The list of samples.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/01-baseline_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/02-IPSO_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/03-custom_object_baseline)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/04-custom_object_dynamic)
