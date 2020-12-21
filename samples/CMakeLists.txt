##########################################
#
# Copyright (c) 2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 3.5)

project(IOWA_Evaluation_SDK_samples C)

# The list of samples (Lite version)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/01-baseline_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/02-IPSO_client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/03-custom_object_baseline)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/04-custom_object_dynamic)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/05-custom_object_multiple_instances)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/06-custom_object_multiple_resource_instances)

## *****************************************************************
## The following directory is for reference only (Full Evaluation SDK only)
## !!! Uncomment the following line to build the  addtionnal samples with full version of IOWA
## *****************************************************************
## add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Full_SDK_Samples)
