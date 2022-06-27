###############################################
#
#  _________ _________ ___________ _________
# |         |         |   |   |   |         |
# |_________|         |   |   |   |    _    |
# |         |    |    |   |   |   |         |
# |         |    |    |           |         |
# |         |    |    |           |    |    |
# |         |         |           |    |    |
# |_________|_________|___________|____|____|
#
# Copyright (c) 2016-2018 IoTerop.
# All rights reserved.
#
# This program and the accompanying materials
# are made available under the terms of
# IoTerop’s IOWA License (LICENSE.TXT) which
# accompany this distribution.
#
###############################################

cmake_minimum_required (VERSION 3.5)

set(TINYDTLS_SOURCES_DIR ${tinydtls_SOURCE_DIR})
set(TINYDTLS_INCLUDE_DIR ${tinydtls_SOURCE_DIR})

set(TINYDTLS_SOURCES
    ${TINYDTLS_SOURCES_DIR}/ccm.c
    ${TINYDTLS_SOURCES_DIR}/crypto.c
    ${TINYDTLS_SOURCES_DIR}/dtls.c
    ${TINYDTLS_SOURCES_DIR}/dtls_debug.c
    ${TINYDTLS_SOURCES_DIR}/dtls_time.c
    ${TINYDTLS_SOURCES_DIR}/hmac.c
    ${TINYDTLS_SOURCES_DIR}/netq.c
    ${TINYDTLS_SOURCES_DIR}/peer.c
    ${TINYDTLS_SOURCES_DIR}/session.c
    ${TINYDTLS_SOURCES_DIR}/aes/rijndael.c
    ${TINYDTLS_SOURCES_DIR}/ecc/ecc.c
    ${TINYDTLS_SOURCES_DIR}/sha2/sha2.c)

set(TINYDTLS_HEADERS
    ${TINYDTLS_INCLUDE_DIR}/alert.h
    ${TINYDTLS_INCLUDE_DIR}/ccm.h
    ${TINYDTLS_INCLUDE_DIR}/crypto.h
    ${TINYDTLS_INCLUDE_DIR}/dtls.h
    ${TINYDTLS_INCLUDE_DIR}/dtls_debug.h
    ${TINYDTLS_INCLUDE_DIR}/dtls_time.h
    ${TINYDTLS_INCLUDE_DIR}/global.h
    ${TINYDTLS_INCLUDE_DIR}/hmac.h
    ${TINYDTLS_INCLUDE_DIR}/netq.h
    ${TINYDTLS_INCLUDE_DIR}/numeric.h
    ${TINYDTLS_INCLUDE_DIR}/peer.h
    ${TINYDTLS_INCLUDE_DIR}/prng.h
    ${TINYDTLS_INCLUDE_DIR}/state.h
    ${TINYDTLS_INCLUDE_DIR}/tinydtls.h
    ${TINYDTLS_INCLUDE_DIR}/uthash.h
    ${TINYDTLS_INCLUDE_DIR}/utlist.h
    ${TINYDTLS_INCLUDE_DIR}/aes/rijndael.h
    ${TINYDTLS_INCLUDE_DIR}/ecc/ecc.h
    ${TINYDTLS_INCLUDE_DIR}/sha2/sha2.h)

SOURCE_GROUP(tinydtls FILES ${TINYDTLS_SOURCES} ${TINYDTLS_HEADERS})
