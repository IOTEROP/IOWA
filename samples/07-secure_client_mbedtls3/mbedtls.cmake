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
# Copyright (c) 2016-2022 IoTerop.
# All rights reserved.
#
# This program and the accompanying materials
# are made available under the terms of
# IoTerop’s IOWA License (LICENSE.TXT) which
# accompany this distribution.
#
###############################################

cmake_minimum_required(VERSION 3.5)

set(MBEDTLS_SOURCE_DIR ${mbedtls3_SOURCE_DIR}/library)
set(MBEDTLS_INCLUDE_DIR ${mbedtls3_SOURCE_DIR}/include)

set(MBEDTLS_SOURCES
    ${MBEDTLS_SOURCE_DIR}/aes.c
    ${MBEDTLS_SOURCE_DIR}/aesni.c
    ${MBEDTLS_SOURCE_DIR}/aria.c
    ${MBEDTLS_SOURCE_DIR}/asn1parse.c
    ${MBEDTLS_SOURCE_DIR}/asn1write.c
    ${MBEDTLS_SOURCE_DIR}/base64.c
    ${MBEDTLS_SOURCE_DIR}/bignum.c
    ${MBEDTLS_SOURCE_DIR}/camellia.c
    ${MBEDTLS_SOURCE_DIR}/ccm.c
    ${MBEDTLS_SOURCE_DIR}/chacha20.c
    ${MBEDTLS_SOURCE_DIR}/chachapoly.c
    ${MBEDTLS_SOURCE_DIR}/cipher_wrap.c
    ${MBEDTLS_SOURCE_DIR}/cipher.c
    ${MBEDTLS_SOURCE_DIR}/cmac.c
    ${MBEDTLS_SOURCE_DIR}/constant_time.c
    ${MBEDTLS_SOURCE_DIR}/ctr_drbg.c
    ${MBEDTLS_SOURCE_DIR}/debug.c
    ${MBEDTLS_SOURCE_DIR}/des.c
    ${MBEDTLS_SOURCE_DIR}/dhm.c
    ${MBEDTLS_SOURCE_DIR}/ecdh.c
    ${MBEDTLS_SOURCE_DIR}/ecdsa.c
    ${MBEDTLS_SOURCE_DIR}/ecjpake.c
    ${MBEDTLS_SOURCE_DIR}/ecp_curves.c
    ${MBEDTLS_SOURCE_DIR}/ecp.c
    ${MBEDTLS_SOURCE_DIR}/entropy_poll.c
    ${MBEDTLS_SOURCE_DIR}/entropy.c
    ${MBEDTLS_SOURCE_DIR}/error.c
    ${MBEDTLS_SOURCE_DIR}/gcm.c
    ${MBEDTLS_SOURCE_DIR}/hkdf.c
    ${MBEDTLS_SOURCE_DIR}/hmac_drbg.c
    ${MBEDTLS_SOURCE_DIR}/md.c
    ${MBEDTLS_SOURCE_DIR}/md5.c
    ${MBEDTLS_SOURCE_DIR}/memory_buffer_alloc.c
    ${MBEDTLS_SOURCE_DIR}/mps_reader.c
    ${MBEDTLS_SOURCE_DIR}/mps_trace.c
    ${MBEDTLS_SOURCE_DIR}/net_sockets.c
    ${MBEDTLS_SOURCE_DIR}/nist_kw.c
    ${MBEDTLS_SOURCE_DIR}/oid.c
    ${MBEDTLS_SOURCE_DIR}/padlock.c
    ${MBEDTLS_SOURCE_DIR}/pem.c
    ${MBEDTLS_SOURCE_DIR}/pk_wrap.c
    ${MBEDTLS_SOURCE_DIR}/pk.c
    ${MBEDTLS_SOURCE_DIR}/pkcs5.c
    ${MBEDTLS_SOURCE_DIR}/pkcs12.c
    ${MBEDTLS_SOURCE_DIR}/pkparse.c
    ${MBEDTLS_SOURCE_DIR}/pkwrite.c
    ${MBEDTLS_SOURCE_DIR}/platform_util.c
    ${MBEDTLS_SOURCE_DIR}/platform.c
    ${MBEDTLS_SOURCE_DIR}/poly1305.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_aead.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_cipher.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_client.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_driver_wrappers.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_ecp.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_hash.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_mac.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_rsa.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_se.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_slot_management.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto_storage.c
    ${MBEDTLS_SOURCE_DIR}/psa_crypto.c
    ${MBEDTLS_SOURCE_DIR}/psa_its_file.c
    ${MBEDTLS_SOURCE_DIR}/ripemd160.c
    ${MBEDTLS_SOURCE_DIR}/rsa_alt_helpers.c
    ${MBEDTLS_SOURCE_DIR}/rsa.c
    ${MBEDTLS_SOURCE_DIR}/sha1.c
    ${MBEDTLS_SOURCE_DIR}/sha256.c
    ${MBEDTLS_SOURCE_DIR}/sha512.c
    ${MBEDTLS_SOURCE_DIR}/ssl_cache.c
    ${MBEDTLS_SOURCE_DIR}/ssl_ciphersuites.c
    ${MBEDTLS_SOURCE_DIR}/ssl_cli.c
    ${MBEDTLS_SOURCE_DIR}/ssl_cookie.c
    ${MBEDTLS_SOURCE_DIR}/ssl_debug_helpers_generated.c
    ${MBEDTLS_SOURCE_DIR}/ssl_msg.c
    ${MBEDTLS_SOURCE_DIR}/ssl_srv.c
    ${MBEDTLS_SOURCE_DIR}/ssl_ticket.c
    ${MBEDTLS_SOURCE_DIR}/ssl_tls.c
    ${MBEDTLS_SOURCE_DIR}/ssl_tls13_client.c
    ${MBEDTLS_SOURCE_DIR}/ssl_tls13_generic.c
    ${MBEDTLS_SOURCE_DIR}/ssl_tls13_keys.c
    ${MBEDTLS_SOURCE_DIR}/ssl_tls13_server.c
    ${MBEDTLS_SOURCE_DIR}/threading.c
    ${MBEDTLS_SOURCE_DIR}/timing.c
    ${MBEDTLS_SOURCE_DIR}/version_features.c
    ${MBEDTLS_SOURCE_DIR}/version.c
    ${MBEDTLS_SOURCE_DIR}/x509_create.c
    ${MBEDTLS_SOURCE_DIR}/x509_crl.c
    ${MBEDTLS_SOURCE_DIR}/x509_crt.c
    ${MBEDTLS_SOURCE_DIR}/x509_csr.c
    ${MBEDTLS_SOURCE_DIR}/x509.c
    ${MBEDTLS_SOURCE_DIR}/x509write_crt.c
    ${MBEDTLS_SOURCE_DIR}/x509write_csr.c
)

set(MBEDTLS_HEADERS
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/aes.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/aria.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/asn1.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/asn1write.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/base64.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/bignum.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/build_info.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/camellia.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ccm.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/chacha20.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/chachapoly.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/check_config.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/cipher.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/cmac.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/compat-2.x.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/config_psa.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/constant_time.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ctr_drbg.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/debug.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/des.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/dhm.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ecdh.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ecdsa.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ecjpake.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ecp.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/entropy.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/error.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/gcm.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/hkdf.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/hmac_drbg.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/mbedtls_config.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/md.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/md5.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/memory_buffer_alloc.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/net_sockets.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/nist_kw.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/oid.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/pem.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/pk.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/pkcs12.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/pkcs5.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/platform.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/platform_time.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/platform_util.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/poly1305.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/private_access.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/psa_util.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ripemd160.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/rsa.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/sha1.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/sha256.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/sha512.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ssl.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ssl_cache.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ssl_ciphersuites.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ssl_cookie.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/ssl_ticket.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/threading.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/timing.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/x509.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/x509_crl.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/x509_crt.h
    ${MBEDTLS_INCLUDE_DIR}/mbedtls/x509_csr.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_builtin_composites.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_builtin_primitives.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_compat.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_config.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_driver_common.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_driver_contexts_composites.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_driver_contexts_primitives.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_extra.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_platform.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_se_driver.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_sizes.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_struct.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_types.h
    ${MBEDTLS_INCLUDE_DIR}/psa/crypto_values.h
)

SOURCE_GROUP(mbedtls FILES ${MBEDTLS_SOURCES} ${MBEDTLS_HEADERS})
