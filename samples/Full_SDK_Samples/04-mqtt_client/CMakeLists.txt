##########################################
#
# Copyright (c) 2016-2020 IoTerop.
# All rights reserved.
#
##########################################

cmake_minimum_required(VERSION 3.5)

project(mqtt_client C)

get_property(IOWA_DIR GLOBAL PROPERTY iowa_sdk_folder)

include(${IOWA_DIR}/src/iowa.cmake)

set(PAHO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/paho.mqtt.c)

add_custom_target(
    pahoLib
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/config.sh ${CMAKE_CURRENT_LIST_DIR}
)

############################################
# Build project
#
add_executable(${PROJECT_NAME}
               ${CMAKE_CURRENT_LIST_DIR}/main.c
               ${CMAKE_CURRENT_LIST_DIR}/mqtt_user_functions.c
               ${CMAKE_CURRENT_LIST_DIR}/iowa_config.h
               ${CMAKE_CURRENT_LIST_DIR}/../00-abstraction_layer/core_abstraction.c
               ${CMAKE_CURRENT_LIST_DIR}/../00-abstraction_layer/connection_abstraction.c
               ${IOWA_CLIENT_SOURCES}
               ${IOWA_CLIENT_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
                           ${IOWA_INCLUDE_DIR}
                           ${PAHO_DIR}/src
                           ${CMAKE_CURRENT_LIST_DIR})

add_dependencies(${PROJECT_NAME} pahoLib)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32 ${PAHO_DIR}/build/src/libpaho-mqtt3c.so)
else()
    target_link_libraries(${PROJECT_NAME} ${LIB_NAME} pthread ${PAHO_DIR}/build/src/libpaho-mqtt3c.so)
endif()

## Compilation flags

if (WIN32)
    set(CMAKE_C_FLAGS_DEBUG "/ZI")
endif()
