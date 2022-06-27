##########################################
## Copyright (c) 2016-2022 IoTerop.
# All rights reserved.
###########################################

cmake_minimum_required(VERSION 3.11)

project(secure_client_mbedtls3 C)

get_property(IOWA_DIR GLOBAL PROPERTY iowa_sdk_folder)
if (NOT IOWA_DIR)
    set(IOWA_DIR ${CMAKE_CURRENT_LIST_DIR}/../../iowa)
endif()

include(FetchContent)


###################################################
# Retrieve mbedtls 3.1.0 sources
#

FetchContent_Declare(
    mbedtls3
    URL https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v3.1.0.zip
)

message("Retrieving Mbed TLS v3.1.0 release from https://github.com/Mbed-TLS/mbedtls...")

FetchContent_Populate(mbedtls3)

############################################
# Build project
#

include(${CMAKE_CURRENT_LIST_DIR}/mbedtls.cmake)
include(${IOWA_DIR}/src/iowa.cmake)

add_compile_definitions(MBEDTLS_CONFIG_FILE="${CMAKE_CURRENT_LIST_DIR}/mbedtls_config.h")

add_executable(${PROJECT_NAME}
               ${CMAKE_CURRENT_LIST_DIR}/main.c
               ${CMAKE_CURRENT_LIST_DIR}/user_security_mbedtls3.c
               ${CMAKE_CURRENT_LIST_DIR}/iowa_config.h
               ${CMAKE_CURRENT_LIST_DIR}/../abstraction_layer/core_abstraction.c
               ${CMAKE_CURRENT_LIST_DIR}/../abstraction_layer/connection_abstraction.c
               ${IOWA_CLIENT_SOURCES}
               ${IOWA_CLIENT_HEADERS}
               ${MBEDTLS_SOURCES}
               ${MBEDTLS_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
                           ${IOWA_INCLUDE_DIR}
                           ${MBEDTLS_INCLUDE_DIR}
                           ${CMAKE_CURRENT_LIST_DIR})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

## Compilation flags

if (WIN32)
    set(CMAKE_C_FLAGS_DEBUG "/ZI")
endif()
