##########################################
#
# Copyright (c) 2016-2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 2.8)

project(simple_client C)

get_property(IOWA_DIR GLOBAL PROPERTY iowa_sdk_folder)

include(${IOWA_DIR}/src/iowa.cmake)

############################################
# Build project
#
add_executable(${PROJECT_NAME}
               ${CMAKE_CURRENT_LIST_DIR}/main.c
               ${CMAKE_CURRENT_LIST_DIR}/iowa_config.h
               ${CMAKE_CURRENT_LIST_DIR}/../abstraction_layer/core_abstraction.c
               ${CMAKE_CURRENT_LIST_DIR}/../abstraction_layer/connection_abstraction.c
               ${IOWA_CLIENT_SOURCES}
               ${IOWA_CLIENT_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
                           ${IOWA_INCLUDE_DIR}
                           ${CMAKE_CURRENT_LIST_DIR})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

## Compilation flags

if (WIN32)
    set(CMAKE_C_FLAGS_DEBUG "/ZI")
endif()
