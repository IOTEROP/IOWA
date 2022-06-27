##########################################
## Copyright (c) 2016-2022 IoTerop.
# All rights reserved.
###########################################

cmake_minimum_required(VERSION 3.11)

project(secure_client_tinydtls C)

get_property(IOWA_DIR GLOBAL PROPERTY iowa_sdk_folder)
if (NOT IOWA_DIR)
    set(IOWA_DIR ${CMAKE_CURRENT_LIST_DIR}/../../iowa)
endif()

include(FetchContent)


###################################################
# Retrieve tinydtls sources
#
if (WIN32)
     message("Tinydtls does not build on Windows platforms, skipping this sample build.")
     return()
endif()

FetchContent_Declare(
    tinydtls
    URL        https://github.com/eclipse/tinydtls/archive/706888256c3e03d9fcf1ec37bb1dd6499213be3c.zip
)

message("Retrieving tinydtls from https://github.com/eclipse/tinydtls...")

FetchContent_Populate(tinydtls)

############################################
# Build project
#

include(${CMAKE_CURRENT_LIST_DIR}/tinydtls.cmake)
include(${IOWA_DIR}/src/iowa.cmake)

add_executable(${PROJECT_NAME}
               ${CMAKE_CURRENT_LIST_DIR}/main.c
               ${CMAKE_CURRENT_LIST_DIR}/user_security_tinydtls.c
               ${CMAKE_CURRENT_LIST_DIR}/iowa_config.h
               ${CMAKE_CURRENT_LIST_DIR}/../abstraction_layer/core_abstraction.c
               ${CMAKE_CURRENT_LIST_DIR}/../abstraction_layer/connection_abstraction.c
               ${IOWA_CLIENT_SOURCES}
               ${IOWA_CLIENT_HEADERS}
               ${TINYDTLS_SOURCES}
               ${TINYDTLS_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
                           ${IOWA_INCLUDE_DIR}
                           ${TINYDTLS_INCLUDE_DIR}
                           ${CMAKE_CURRENT_LIST_DIR})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

## Compilation flags

if (WIN32)
    set(CMAKE_C_FLAGS_DEBUG "/ZI")
endif()
