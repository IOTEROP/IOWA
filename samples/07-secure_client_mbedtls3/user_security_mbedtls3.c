/**********************************************
*
* Copyright (c) 2018-2022 IoTerop.
* All rights reserved.
*
**********************************************/

/**********************************************
 *
 * This file implements the IOWA platform
 * abstraction callbacks for User Security with MBEDTLS 3.
 * Client side is used here, MBEDTLS_SSL_CLI_C must be defined
 *
 **********************************************/


#include "iowa_security.h"
#include "iowa_prv_logger.h"

// #define SECURITY_CERTIFICATE_SUPPORT

// mbedtls headers
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cache.h"
#include "mbedtls/timing.h"

#ifdef SECURITY_CERTIFICATE_SUPPORT
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#endif

#include <string.h>

#ifndef MBEDTLS_SSL_CLI_C
#error "MBEDTLS_SSL_CLI_C must be defined in mbedtls_config.h"
#endif

/*************************************************************************************
** Private types
*************************************************************************************/

#define PRV_MBEDTLS_SUCCESSFUL           0
#define PRV_MBEDTLS_ERROR_MESSAGE_LENGTH 100

#define PRV_MBEDTLS_TIMER_CANCELLED   -1
#define PRV_MBEDTLS_TIMER_NOT_EXPIRED 0
#define PRV_MBEDTLS_TIMER_EXPIRED     2

#define PRV_ADD_CIPHERSUITE(ciphersuites, id, ciphersuite) ciphersuites[id++] = ciphersuite;

#if IOWA_LOG_LEVEL > IOWA_LOG_LEVEL_NONE
#define PRV_PRINT_MBEDTLS_ERROR(funcName, res) prv_printMbedtlsError(funcName, (unsigned int)res)
#else
#define PRV_PRINT_MBEDTLS_ERROR(funcName, res)
#endif

// Internal structure holding the security data
typedef struct
{
    // Common
    mbedtls_ssl_context         sslContext;
    mbedtls_ssl_config          sslConfig;
    mbedtls_ssl_cache_context   sslCache;
    int                         *ciphersuites;
    int32_t                     startTime;
    uint32_t                    timeout;
    bool                        handshakeDataAvailable;
#ifdef SECURITY_CERTIFICATE_SUPPORT
    // Certificate
    mbedtls_x509_crt            *caCert;
    mbedtls_x509_crt            *cert;
    mbedtls_pk_context          *privateKey;
#endif
} user_security_internal_t;

#define PRV_STR_MBEDTLS_STATE(M)                                                                        \
((M) == MBEDTLS_SSL_HELLO_REQUEST ? "MBEDTLS_SSL_HELLO_REQUEST" :                                       \
((M) == MBEDTLS_SSL_CLIENT_HELLO ? "MBEDTLS_SSL_CLIENT_HELLO" :                                         \
((M) == MBEDTLS_SSL_SERVER_HELLO ? "MBEDTLS_SSL_SERVER_HELLO" :                                         \
((M) == MBEDTLS_SSL_SERVER_CERTIFICATE ? "MBEDTLS_SSL_SERVER_CERTIFICATE" :                             \
((M) == MBEDTLS_SSL_SERVER_KEY_EXCHANGE ? "MBEDTLS_SSL_SERVER_KEY_EXCHANGE" :                           \
((M) == MBEDTLS_SSL_CERTIFICATE_REQUEST ? "MBEDTLS_SSL_CERTIFICATE_REQUEST" :                           \
((M) == MBEDTLS_SSL_SERVER_HELLO_DONE ? "MBEDTLS_SSL_SERVER_HELLO_DONE" :                               \
((M) == MBEDTLS_SSL_CLIENT_CERTIFICATE ? "MBEDTLS_SSL_CLIENT_CERTIFICATE" :                             \
((M) == MBEDTLS_SSL_CLIENT_KEY_EXCHANGE ? "MBEDTLS_SSL_CLIENT_KEY_EXCHANGE" :                           \
((M) == MBEDTLS_SSL_CERTIFICATE_VERIFY ? "MBEDTLS_SSL_CERTIFICATE_VERIFY" :                             \
((M) == MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC ? "MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC" :               \
((M) == MBEDTLS_SSL_CLIENT_FINISHED ? "MBEDTLS_SSL_CLIENT_FINISHED" :                                   \
((M) == MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC ? "MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC" :               \
((M) == MBEDTLS_SSL_SERVER_FINISHED ? "MBEDTLS_SSL_SERVER_FINISHED" :                                   \
((M) == MBEDTLS_SSL_FLUSH_BUFFERS ? "MBEDTLS_SSL_FLUSH_BUFFERS" :                                       \
((M) == MBEDTLS_SSL_HANDSHAKE_WRAPUP ? "MBEDTLS_SSL_HANDSHAKE_WRAPUP" :                                 \
((M) == MBEDTLS_SSL_HANDSHAKE_OVER ? "MBEDTLS_SSL_HANDSHAKE_OVER" :                                     \
((M) == MBEDTLS_SSL_SERVER_NEW_SESSION_TICKET ? "MBEDTLS_SSL_SERVER_NEW_SESSION_TICKET" :               \
((M) == MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT ? "MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT" : \
"Unknown")))))))))))))))))))

#define STR_MBEDTLS_ERROR(M)                                                    \
((M) == MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE ? "MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE":   \
((M) == MBEDTLS_ERR_SSL_BAD_INPUT_DATA ? "MBEDTLS_ERR_SSL_BAD_INPUT_DATA":    \
((M) == MBEDTLS_ERR_SSL_INVALID_MAC ? "MBEDTLS_ERR_SSL_INVALID_MAC":   \
((M) == MBEDTLS_ERR_SSL_INVALID_RECORD ? "MBEDTLS_ERR_SSL_INVALID_RECORD":    \
((M) == MBEDTLS_ERR_SSL_CONN_EOF ? "MBEDTLS_ERR_SSL_CONN_EOF":  \
((M) == MBEDTLS_ERR_SSL_UNKNOWN_CIPHER ? "MBEDTLS_ERR_SSL_UNKNOWN_CIPHER":    \
((M) == MBEDTLS_ERR_SSL_NO_CIPHER_CHOSEN ? "MBEDTLS_ERR_SSL_NO_CIPHER_CHOSEN":  \
((M) == MBEDTLS_ERR_SSL_NO_RNG ? "MBEDTLS_ERR_SSL_NO_RNG":    \
((M) == MBEDTLS_ERR_SSL_NO_CLIENT_CERTIFICATE ? "MBEDTLS_ERR_SSL_NO_CLIENT_CERTIFICATE": \
((M) == MBEDTLS_ERR_SSL_CERTIFICATE_TOO_LARGE ? "MBEDTLS_ERR_SSL_CERTIFICATE_TOO_LARGE": \
((M) == MBEDTLS_ERR_SSL_CERTIFICATE_REQUIRED ? "MBEDTLS_ERR_SSL_CERTIFICATE_REQUIRED":  \
((M) == MBEDTLS_ERR_SSL_PRIVATE_KEY_REQUIRED ? "MBEDTLS_ERR_SSL_PRIVATE_KEY_REQUIRED":  \
((M) == MBEDTLS_ERR_SSL_CA_CHAIN_REQUIRED ? "MBEDTLS_ERR_SSL_CA_CHAIN_REQUIRED": \
((M) == MBEDTLS_ERR_SSL_UNEXPECTED_MESSAGE ? "MBEDTLS_ERR_SSL_UNEXPECTED_MESSAGE":    \
((M) == MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE ? "MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE":   \
((M) == MBEDTLS_ERR_SSL_PEER_VERIFY_FAILED ? "MBEDTLS_ERR_SSL_PEER_VERIFY_FAILED":    \
((M) == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY ? "MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY": \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CLIENT_HELLO ? "MBEDTLS_ERR_SSL_BAD_HS_CLIENT_HELLO":   \
((M) == MBEDTLS_ERR_SSL_BAD_HS_SERVER_HELLO ? "MBEDTLS_ERR_SSL_BAD_HS_SERVER_HELLO":   \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE ? "MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE":    \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE_REQUEST ? "MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE_REQUEST":    \
((M) == MBEDTLS_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE ? "MBEDTLS_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE":    \
((M) == MBEDTLS_ERR_SSL_BAD_HS_SERVER_HELLO_DONE ? "MBEDTLS_ERR_SSL_BAD_HS_SERVER_HELLO_DONE":  \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE ? "MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE":    \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_RP ? "MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_RP": \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_CS ? "MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_CS": \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE_VERIFY ? "MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE_VERIFY": \
((M) == MBEDTLS_ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC ? "MBEDTLS_ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC": \
((M) == MBEDTLS_ERR_SSL_BAD_HS_FINISHED ? "MBEDTLS_ERR_SSL_BAD_HS_FINISHED":   \
((M) == MBEDTLS_ERR_SSL_ALLOC_FAILED ? "MBEDTLS_ERR_SSL_ALLOC_FAILED":  \
((M) == MBEDTLS_ERR_SSL_HW_ACCEL_FAILED ? "MBEDTLS_ERR_SSL_HW_ACCEL_FAILED":   \
((M) == MBEDTLS_ERR_SSL_HW_ACCEL_FALLTHROUGH ? "MBEDTLS_ERR_SSL_HW_ACCEL_FALLTHROUGH":  \
((M) == MBEDTLS_ERR_SSL_COMPRESSION_FAILED ? "MBEDTLS_ERR_SSL_COMPRESSION_FAILED":    \
((M) == MBEDTLS_ERR_SSL_BAD_HS_PROTOCOL_VERSION ? "MBEDTLS_ERR_SSL_BAD_HS_PROTOCOL_VERSION":   \
((M) == MBEDTLS_ERR_SSL_BAD_HS_NEW_SESSION_TICKET ? "MBEDTLS_ERR_SSL_BAD_HS_NEW_SESSION_TICKET": \
((M) == MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED ? "MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED":    \
((M) == MBEDTLS_ERR_SSL_PK_TYPE_MISMATCH ? "MBEDTLS_ERR_SSL_PK_TYPE_MISMATCH":  \
((M) == MBEDTLS_ERR_SSL_UNKNOWN_IDENTITY ? "MBEDTLS_ERR_SSL_UNKNOWN_IDENTITY":  \
((M) == MBEDTLS_ERR_SSL_INTERNAL_ERROR ? "MBEDTLS_ERR_SSL_INTERNAL_ERROR":    \
((M) == MBEDTLS_ERR_SSL_COUNTER_WRAPPING ? "MBEDTLS_ERR_SSL_COUNTER_WRAPPING":  \
((M) == MBEDTLS_ERR_SSL_WAITING_SERVER_HELLO_RENEGO ? "MBEDTLS_ERR_SSL_WAITING_SERVER_HELLO_RENEGO":   \
((M) == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED ? "MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED": \
((M) == MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL ? "MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL":  \
((M) == MBEDTLS_ERR_SSL_NO_USABLE_CIPHERSUITE ? "MBEDTLS_ERR_SSL_NO_USABLE_CIPHERSUITE": \
((M) == MBEDTLS_ERR_SSL_WANT_READ ? "MBEDTLS_ERR_SSL_WANT_READ": \
((M) == MBEDTLS_ERR_SSL_WANT_WRITE ? "MBEDTLS_ERR_SSL_WANT_WRITE":    \
((M) == MBEDTLS_ERR_SSL_TIMEOUT ? "MBEDTLS_ERR_SSL_TIMEOUT":   \
((M) == MBEDTLS_ERR_SSL_CLIENT_RECONNECT ? "MBEDTLS_ERR_SSL_CLIENT_RECONNECT":  \
((M) == MBEDTLS_ERR_SSL_UNEXPECTED_RECORD ? "MBEDTLS_ERR_SSL_UNEXPECTED_RECORD": \
((M) == MBEDTLS_ERR_SSL_NON_FATAL ? "MBEDTLS_ERR_SSL_NON_FATAL": \
((M) == MBEDTLS_ERR_SSL_INVALID_VERIFY_HASH ? "MBEDTLS_ERR_SSL_INVALID_VERIFY_HASH":   \
((M) == MBEDTLS_ERR_SSL_CONTINUE_PROCESSING ? "MBEDTLS_ERR_SSL_CONTINUE_PROCESSING":   \
((M) == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS ? "MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS": \
((M) == MBEDTLS_ERR_SSL_EARLY_MESSAGE ? "MBEDTLS_ERR_SSL_EARLY_MESSAGE": \
((M) == MBEDTLS_ERR_SSL_UNEXPECTED_CID ? "MBEDTLS_ERR_SSL_UNEXPECTED_CID":    \
((M) == MBEDTLS_ERR_SSL_VERSION_MISMATCH ? "MBEDTLS_ERR_SSL_VERSION_MISMATCH":  \
((M) == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS ? "MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS":    \
"Unknown")))))))))))))))))))))))))))))))))))))))))))))))))))))))))


/*************************************************************************************
** Private functions
*************************************************************************************/

/*******************************
** Security Application Private functions
*******************************/

// Used to free the allocated user_security_internal_t
static void prv_internalsFree(user_security_internal_t *internalsP)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

#ifdef SECURITY_CERTIFICATE_SUPPORT
    if (internalsP->caCert != NULL)
    {
        mbedtls_x509_crt_free(internalsP->caCert);
        iowa_system_free(internalsP->caCert);
    }

    if (internalsP->cert != NULL)
    {
        mbedtls_x509_crt_free(internalsP->cert);
        iowa_system_free(internalsP->cert);
    }

    if (internalsP->privateKey != NULL)
    {
        mbedtls_pk_free(internalsP->privateKey);
        iowa_system_free(internalsP->privateKey);
    }
#endif

    mbedtls_ssl_free(&internalsP->sslContext);
    mbedtls_ssl_config_free(&internalsP->sslConfig);

    if (internalsP->ciphersuites != NULL)
    {
        iowa_system_free(internalsP->ciphersuites);
    }
    iowa_system_free(internalsP);
}

#if IOWA_LOG_LEVEL == IOWA_LOG_LEVEL_TRACE
static void prv_debug(void *userDataP,
                      int level,
                      const char *file,
                      int line,
                      const char *str)
{
    const char *strSearch;

    (void)userDataP;
    (void)level;

    // file: Contains the full path to the file
    strSearch = strrchr(file, '/');
    if (strSearch == NULL)
    {
        strSearch = strrchr(file, '\\');
        if (strSearch == NULL)
        {
            strSearch = file;
        }
        else
        {
            strSearch += 1; // Remove the search character
        }
    }
    else
    {
        strSearch += 1; // Remove the search character
    }

    // str: Contains a '\n' at the end
    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "%s:%d: %.*s", strSearch, line, strlen(str) - 1, str);
}
#endif

#if IOWA_LOG_LEVEL > IOWA_LOG_LEVEL_NONE
static void prv_printMbedtlsError(const char *functionName,
                                  unsigned int res)
{
    char bufferError[PRV_MBEDTLS_ERROR_MESSAGE_LENGTH];

    mbedtls_strerror((int)res, bufferError, PRV_MBEDTLS_ERROR_MESSAGE_LENGTH);
    IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "%s() failed with code %d: %s", functionName, res, bufferError);
}
#endif

static int prv_mbedtlsSendFunc(void *userDataP,
                               const unsigned char *buf,
                               size_t len)
{
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = (iowa_security_session_t)userDataP;

    return iowa_security_connection_send(securityS, (uint8_t *)buf, len);
}

static int prv_mbedtlsRecvFunc(void *userDataP,
                               unsigned char *buf,
                               size_t len)
{
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = (iowa_security_session_t)userDataP;

    switch (iowa_security_session_get_state(securityS))
    {
    case SECURITY_STATE_HANDSHAKING:
    {
        user_security_internal_t *internalsP;

        internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

        if (iowa_security_session_get_connection_type(securityS) != IOWA_CONN_STREAM
            && internalsP->handshakeDataAvailable == false)
        {
            IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Timeout failure.");
            return MBEDTLS_ERR_SSL_TIMEOUT;
        }

        // Reset the structure
        internalsP->startTime = -1;
        internalsP->timeout = 0;
        internalsP->handshakeDataAvailable = false;
        break;
    }

    default:
        // Do nothing
        break;
    }

    return iowa_security_connection_recv(securityS, (uint8_t *)buf, (int)len);
}

static int prv_mbedtlsRandomVectorGenerator(void *userDataP,
                                            uint8_t *randomBuffer,
                                            size_t size)
{
    int res;
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = (iowa_security_session_t)userDataP;

    res = iowa_system_random_vector_generator(randomBuffer, size, iowa_security_session_get_context_user_data(securityS));

    return res;
}

static void prv_mbedtlsSetDelay(void *userDataP,
                                uint32_t intMs,
                                uint32_t finMs)
{
    iowa_security_session_t securityS;
    user_security_internal_t *internalsP;
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    (void)intMs;

    securityS = (iowa_security_session_t)userDataP;
    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    // Check if the timer must be stopped
    if (finMs == 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Stop timer.");
        internalsP->startTime = -1;
        return;
    }

    // Set the timer values
    internalsP->timeout = finMs / 1000;

    // Get the time when the timer begins
    internalsP->startTime = iowa_system_gettime();
    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "startTime: %d.", internalsP->startTime);
}

static int prv_mbedtlsGetDelay(void *userDataP)
{
    // Function must return one of the following values:
    // * -1 if timer is cancelled
    // * 0 if none of the delays is expired
    // * 2 if the final delay is expired

    iowa_security_session_t securityS;
    user_security_internal_t *internalsP;
    int32_t currentTime;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    securityS = (iowa_security_session_t)userDataP;
    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);
    currentTime = iowa_system_gettime();
    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "currentTime: %d.", currentTime);

    if (internalsP->startTime < 0
        || currentTime < internalsP->startTime)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Timer cancelled.");
        return PRV_MBEDTLS_TIMER_CANCELLED;
    }

    if ((currentTime - internalsP->startTime) > internalsP->timeout)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Timer expired.");
        return PRV_MBEDTLS_TIMER_EXPIRED;
    }

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Timer not expired yet.");

    return PRV_MBEDTLS_TIMER_NOT_EXPIRED;
}

#ifdef SECURITY_CERTIFICATE_SUPPORT
static iowa_status_t prv_initCertificate(iowa_security_session_t securityS)
{
    int res;
    iowa_security_data_t securityData;
    user_security_internal_t *internalsP;
    iowa_status_t iowaRes;
    uint8_t *peerIdentity;
    size_t peerIdentityLen;
    const char *uriP;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    // Retrieve the certificate structure
    memset(&securityData, 0, sizeof(iowa_security_data_t));
    securityData.securityMode = IOWA_SEC_CERTIFICATE;

    uriP = iowa_security_session_get_uri(securityS);
    peerIdentity = (uint8_t *)uriP;
    peerIdentityLen = strlen(uriP);

    iowaRes = iowa_system_security_data(peerIdentity, peerIdentityLen, IOWA_SEC_READ, &securityData, iowa_security_session_get_context_user_data(securityS));
    if (iowaRes != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Failed to retrieve the certificate (%u.%02u)", (iowaRes & 0xFF) >> 5, (iowaRes & 0x1F));
        return iowaRes;
    }

    // Only verify certificate on client side
    // Client need to check if it's connecting to the right server
    if (securityData.protocol.certData.caCertificateLen != 0
        && securityData.protocol.certData.caCertificate != NULL)
    {
        mbedtls_ssl_conf_authmode(&internalsP->sslConfig, MBEDTLS_SSL_VERIFY_REQUIRED);
    }
    else
    {
        mbedtls_ssl_conf_authmode(&internalsP->sslConfig, MBEDTLS_SSL_VERIFY_NONE);
    }

    // Configure and parse the certificate
    internalsP->cert = (mbedtls_x509_crt *)iowa_system_malloc(sizeof(mbedtls_x509_crt));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (internalsP->cert == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(mbedtls_x509_crt));
        goto error;
    }
#endif
    mbedtls_x509_crt_init(internalsP->cert);

    // MbedTLS is already checking the certificate buffer. No need to do it here.
    res = mbedtls_x509_crt_parse(internalsP->cert,
                                 securityData.protocol.certData.certificate,
                                 securityData.protocol.certData.certificateLen);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_x509_crt_parse", res);
        goto error;
    }

    // Configure and parse the private key
    internalsP->privateKey = (mbedtls_pk_context *)iowa_system_malloc(sizeof(mbedtls_pk_context));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (internalsP->privateKey == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(mbedtls_pk_context));
        goto error;
    }
#endif
    mbedtls_pk_init(internalsP->privateKey);

    // MbedTLS is already checking the private key buffer. No need to do it here.
    res = mbedtls_pk_parse_key(internalsP->privateKey,
                               securityData.protocol.certData.privateKey,  securityData.protocol.certData.privateKeyLen,
                               NULL, 0,
                               prv_mbedtlsRandomVectorGenerator, NULL);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_pk_parse_key", res);
        goto error;
    }

    res = mbedtls_ssl_conf_own_cert(&internalsP->sslConfig,
                                    internalsP->cert,
                                    internalsP->privateKey);

    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_conf_own_cert", res);
        goto error;
    }

    // Check if a CA certificate has been provided
    if (securityData.protocol.certData.caCertificateLen != 0
        && securityData.protocol.certData.caCertificate != NULL)
    {
        internalsP->caCert = (mbedtls_x509_crt *)iowa_system_malloc(sizeof(mbedtls_x509_crt));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (internalsP->caCert == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(mbedtls_x509_crt));
            goto error;
        }
#endif
        mbedtls_x509_crt_init(internalsP->caCert);

        res = mbedtls_x509_crt_parse(internalsP->caCert,
                                     securityData.protocol.certData.caCertificate,
                                     securityData.protocol.certData.caCertificateLen);
        if (res != PRV_MBEDTLS_SUCCESSFUL)
        {
            PRV_PRINT_MBEDTLS_ERROR("mbedtls_x509_crt_parse", res);
            goto error;
        }

        // MbedTLS is already checking the CA certificate buffer. No need to do it here.
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "\r\nset CA certificate.\r\n");
        mbedtls_ssl_conf_ca_chain(&internalsP->sslConfig, internalsP->caCert, NULL);
    }

error:
    iowaRes = iowa_system_security_data(peerIdentity, peerIdentityLen, IOWA_SEC_FREE, &securityData, iowa_security_session_get_context_user_data(securityS));
    if (iowaRes != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Failed to free the certificate (%u.%02u)", (iowaRes & 0xFF) >> 5, (iowaRes & 0x1F));
    }

    return iowaRes;
}
#endif

static iowa_status_t prv_addCiphersuites(user_security_internal_t *internalsP,
                                         bool certificate,
                                         bool psk)
{
    int pos;
    size_t ciphersuitesArrayLength;

    pos = 0;

    // Calculate the length of the ciphersuites array
    ciphersuitesArrayLength = 1;
    if (certificate == true)
    {
        ciphersuitesArrayLength += 2;
    }
    if (psk == true)
    {
        ciphersuitesArrayLength += 2;
    }

    internalsP->ciphersuites = (int *)iowa_system_malloc(ciphersuitesArrayLength*sizeof(int));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (internalsP->ciphersuites == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(ciphersuitesArrayLength*sizeof(int));
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
#endif

    // Ciphersuites ordered by preference. First in the list has the highest preference
    if (certificate == true)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Adding certificate ciphersuites");

        // MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 should not be used due to security concern.
        // Instead, MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 must be used whenever possible. (from LwM2M 1.1 specification)
        PRV_ADD_CIPHERSUITE(internalsP->ciphersuites, pos, MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8);
        PRV_ADD_CIPHERSUITE(internalsP->ciphersuites, pos, MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);
    }

    if (psk == true)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Adding pre-shared key ciphersuites");

        PRV_ADD_CIPHERSUITE(internalsP->ciphersuites, pos, MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8);
        PRV_ADD_CIPHERSUITE(internalsP->ciphersuites, pos, MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256);
    }

    PRV_ADD_CIPHERSUITE(internalsP->ciphersuites, pos, 0);

    mbedtls_ssl_conf_ciphersuites(&internalsP->sslConfig, internalsP->ciphersuites);

    return IOWA_COAP_NO_ERROR;
}

static iowa_status_t prv_mbedtlsConnect(user_security_internal_t *internalsP,
                                        iowa_security_session_t securityS)
{
    int res;
    int timeout;

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Entering with MbedTLS state: %s.", PRV_STR_MBEDTLS_STATE(internalsP->sslContext.MBEDTLS_PRIVATE(state)));

    timeout = prv_mbedtlsGetDelay((void *)securityS);
    switch (timeout)
    {
    case PRV_MBEDTLS_TIMER_CANCELLED:
    case PRV_MBEDTLS_TIMER_EXPIRED:
        do
        {
            res = mbedtls_ssl_handshake_step(&internalsP->sslContext);
            switch (res)
            {
            case PRV_MBEDTLS_SUCCESSFUL:
                switch (internalsP->sslContext.MBEDTLS_PRIVATE(state))
                {
                case MBEDTLS_SSL_HANDSHAKE_OVER:
                    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake done.");
                    iowa_security_session_set_state(securityS, SECURITY_STATE_HANDSHAKE_DONE);
                    break;

                default:
                    break;
                }
                break;

            case MBEDTLS_ERR_SSL_WANT_READ:
                break;

            default:
                IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake failed.");
                PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_handshake_step", res);
                return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
            }
        } while (res == PRV_MBEDTLS_SUCCESSFUL
                 && internalsP->sslContext.MBEDTLS_PRIVATE(state) != MBEDTLS_SSL_HANDSHAKE_OVER
                 && internalsP->timeout == 0);

        break;

    default:
        // Do nothing
        break;
    }

    iowa_security_session_set_step_delay(securityS, internalsP->timeout);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with MbedTLS state: %s.", PRV_STR_MBEDTLS_STATE(internalsP->sslContext.MBEDTLS_PRIVATE(state)));

    return IOWA_COAP_NO_ERROR;
}

static void prv_mbedtlsDisconnect(user_security_internal_t *internalsP,
                                  iowa_security_session_t securityS)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    (void)mbedtls_ssl_close_notify(&internalsP->sslContext);
    mbedtls_ssl_session_reset(&internalsP->sslContext);
}

static void prv_connectionFailing(user_security_internal_t *internalsP, iowa_security_session_t securityS)
{
    prv_mbedtlsDisconnect(internalsP, securityS);
    iowa_security_session_set_step_delay(securityS, 0);
    iowa_security_session_set_state(securityS, SECURITY_STATE_CONNECTION_FAILED);
    iowa_security_session_generate_event(securityS, SECURITY_EVENT_DISCONNECTED);
}

/*************************************************************************************
** Custom security implementation functions
*************************************************************************************/

// Initialize security data for a new client-side security session.
iowa_status_t iowa_user_security_create_client_session(iowa_security_session_t securityS)
{
    user_security_internal_t *internalsP;
    int res;
    int transport;
    uint8_t maxFragmentLengthCode;

    internalsP = (user_security_internal_t *)iowa_system_malloc(sizeof(user_security_internal_t));
    if (internalsP == NULL)
    {
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }
    memset(internalsP, 0, sizeof(user_security_internal_t));

    // Store the user security data in the IOWA session
    iowa_security_session_set_user_internals(securityS, internalsP);

    mbedtls_ssl_init(&internalsP->sslContext);
    mbedtls_ssl_config_init(&internalsP->sslConfig);
    mbedtls_ssl_cache_init(&internalsP->sslCache);

    switch (iowa_security_session_get_connection_type(securityS))
    {
    case IOWA_CONN_STREAM:
        transport = MBEDTLS_SSL_TRANSPORT_STREAM;
        break;

    default:
        transport = MBEDTLS_SSL_TRANSPORT_DATAGRAM;
    }
    res = mbedtls_ssl_config_defaults(&internalsP->sslConfig,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      transport,
                                      MBEDTLS_SSL_PRESET_DEFAULT);

    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_config_defaults", res);
        goto error;
    }

#if (IOWA_LOG_LEVEL == IOWA_LOG_LEVEL_TRACE)
    // Debug levels for MbedTLS (from the doc):
    // 0: No debug
    // 1: Error
    // 2: State change
    // 3: Informational
    // 4: Verbose
    mbedtls_debug_set_threshold(4);
    mbedtls_ssl_conf_dbg(&internalsP->sslConfig, prv_debug, NULL);
#endif

    // Set the random vector generator function to use
    mbedtls_ssl_conf_rng(&internalsP->sslConfig, prv_mbedtlsRandomVectorGenerator, securityS);

    // Set the Maximum Fragment Length extension
    if (MBEDTLS_SSL_OUT_CONTENT_LEN <= 512)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_512;
    }
    else if (MBEDTLS_SSL_OUT_CONTENT_LEN <= 1024)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_1024;
    }
    else if (MBEDTLS_SSL_OUT_CONTENT_LEN <= 2048)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_2048;
    }
    else if (MBEDTLS_SSL_OUT_CONTENT_LEN <= 4096)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_4096;
    }
    else
    {
        // Do not set the maximum fragment length
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_NONE;
    }

    (void)mbedtls_ssl_conf_max_frag_len(&internalsP->sslConfig, maxFragmentLengthCode);


    // Set up based on the security mode: here we support PSK and Certificate mode
    switch (iowa_security_session_get_security_mode(securityS))
    {
    case IOWA_SEC_PRE_SHARED_KEY:
    {
        iowa_security_data_t securityData;
        const char *uriP;

        if (prv_addCiphersuites(internalsP, false, true) != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to add the ciphersuites");
            goto error;
        }

        memset(&securityData, 0, sizeof(iowa_security_data_t));
        securityData.securityMode = IOWA_SEC_PRE_SHARED_KEY;

        uriP = iowa_security_session_get_uri(securityS);
        res = iowa_system_security_data((uint8_t *)uriP, strlen(uriP), IOWA_SEC_READ, &securityData, iowa_security_session_get_context_user_data(securityS));
        if (res != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "No PSK key-identity pair found");
            goto error;
        }

        // MbedTLS is already checking the PSK identity/key buffer. No need to do it here.
        (void)mbedtls_ssl_conf_psk(&internalsP->sslConfig, securityData.protocol.pskData.privateKey, securityData.protocol.pskData.privateKeyLen,
                                    securityData.protocol.pskData.identity, securityData.protocol.pskData.identityLen);

        res = iowa_system_security_data((uint8_t *)uriP, strlen(uriP), IOWA_SEC_FREE, &securityData, iowa_security_session_get_context_user_data(securityS));
        if (res != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "Failed to free the PSK key-identity pair");
            goto error;
        }

        // Check previous 'mbedtls_ssl_conf_psk' res here since the security data is now free
        if (res != PRV_MBEDTLS_SUCCESSFUL)
        {
            PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_conf_psk", res);
            goto error;
        }
        break;
    }

#ifdef SECURITY_CERTIFICATE_SUPPORT
    case IOWA_SEC_CERTIFICATE:
    {
        if (prv_addCiphersuites(internalsP, true, false) != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to add the ciphersuites");
            goto error;
        }

        if (prv_initCertificate(securityS) != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Certificate configuration failed");
            goto error;
        }

        break;
    }
#endif

    default:
        IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Unhandled security mode: %d", iowa_security_session_get_security_mode(securityS));
        goto error;
    }

    // Save config in context
    res = mbedtls_ssl_setup(&internalsP->sslContext, &internalsP->sslConfig);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }

    // Use cases are:
    // - non blocking I/O: f_recv != NULL and f_recv_timeout == NULL
    // - blocking I/O: f_recv == NULL and f_recv_timout != NULL
    mbedtls_ssl_set_bio(&internalsP->sslContext, securityS, prv_mbedtlsSendFunc, prv_mbedtlsRecvFunc, NULL);

    mbedtls_ssl_set_timer_cb(&internalsP->sslContext, securityS, prv_mbedtlsSetDelay, prv_mbedtlsGetDelay);
    return IOWA_COAP_NO_ERROR;

error:
    prv_internalsFree(internalsP);

    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}

// Delete security data of a security session.
void iowa_user_security_delete_session(iowa_security_session_t securityS)
{
    user_security_internal_t *internalsP;

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    prv_internalsFree(internalsP);
}

// Handle an incoming packet during the TLS/DTLS handshake.
iowa_status_t iowa_user_security_handle_handshake_packet(iowa_security_session_t securityS)
{
    user_security_internal_t *internalsP;
    int res;

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    internalsP->handshakeDataAvailable = true;

next_state:
    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "MbedTLS state: %s.", PRV_STR_MBEDTLS_STATE(internalsP->sslContext.MBEDTLS_PRIVATE(state)));

    switch (internalsP->sslContext.MBEDTLS_PRIVATE(state))
    {
    case MBEDTLS_SSL_SERVER_HELLO:
    case MBEDTLS_SSL_SERVER_CERTIFICATE:
    case MBEDTLS_SSL_SERVER_KEY_EXCHANGE:
    case MBEDTLS_SSL_CERTIFICATE_REQUEST:
    case MBEDTLS_SSL_SERVER_HELLO_DONE:
    case MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC:
    case MBEDTLS_SSL_SERVER_FINISHED:
    case MBEDTLS_SSL_FLUSH_BUFFERS:
    case MBEDTLS_SSL_HANDSHAKE_WRAPUP:
        // Do nothing
        break;

    default:
        goto exit;
    }

    res = mbedtls_ssl_handshake_step(&internalsP->sslContext);
    switch (res)
    {
    case PRV_MBEDTLS_SUCCESSFUL:
        switch (internalsP->sslContext.MBEDTLS_PRIVATE(state))
        {
        case MBEDTLS_SSL_HANDSHAKE_OVER:
            IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake done.");
            iowa_security_session_set_state(securityS, SECURITY_STATE_HANDSHAKE_DONE);
            iowa_security_session_generate_event(securityS, SECURITY_STATE_HANDSHAKE_DONE);
            break;

        default:
            if (internalsP->timeout == 0)
            {
                goto next_state;
            }
        }
        break;

    case MBEDTLS_ERR_SSL_WANT_READ:
        break;

    default:
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake failed.");
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_handshake_step", res);
        prv_connectionFailing(internalsP, securityS);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

exit:
    iowa_security_session_set_step_delay(securityS, (int32_t)internalsP->timeout);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with MbedTLS state: %s.", PRV_STR_MBEDTLS_STATE(internalsP->sslContext.MBEDTLS_PRIVATE(state)));

    return IOWA_COAP_NO_ERROR;
}

// Perform various security operation depending on the security session state.
iowa_status_t iowa_user_security_step(iowa_security_session_t securityS)
{
    user_security_internal_t *internalsP;
    iowa_status_t res;

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    switch (iowa_security_session_get_state(securityS))
    {
    case SECURITY_STATE_DISCONNECTING:
        prv_mbedtlsDisconnect(internalsP, securityS);
        iowa_security_session_set_state(securityS, SECURITY_STATE_DISCONNECTED);
        iowa_security_session_generate_event(securityS, SECURITY_EVENT_DISCONNECTED);
        break;

    case SECURITY_STATE_INIT_HANDSHAKE:

        if (internalsP->sslContext.MBEDTLS_PRIVATE(state) != MBEDTLS_SSL_HELLO_REQUEST)
        {
            IOWA_LOG_WARNING(IOWA_PART_SECURITY, "Session state is not Hello Request.");
            return IOWA_COAP_412_PRECONDITION_FAILED;
        }

        res = prv_mbedtlsConnect(internalsP, securityS);
        if (res != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to initiate the handshake.");
            prv_connectionFailing(internalsP, securityS);
            break;
        }

        if (internalsP->sslContext.MBEDTLS_PRIVATE(state) != MBEDTLS_SSL_HANDSHAKE_OVER)
        {
            iowa_security_session_set_state(securityS, SECURITY_STATE_HANDSHAKING);
        }
        break;

    case SECURITY_STATE_HANDSHAKING:
        res = prv_mbedtlsConnect(internalsP, securityS);
        if (res != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Handshaking error.");
            prv_connectionFailing(internalsP, securityS);
        }
        break;

    case SECURITY_STATE_HANDSHAKE_DONE:
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Connected.");
        iowa_security_session_set_state(securityS, SECURITY_STATE_CONNECTED);
        iowa_security_session_generate_event(securityS, SECURITY_EVENT_CONNECTED);
        break;

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

// Send data on a secure connection.
int iowa_user_security_send(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length)
{
    user_security_internal_t *internalsP;
    int res;

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    if (iowa_security_session_get_connection_type(securityS) == IOWA_CONN_DATAGRAM)
    {
        // Datagram
        res = mbedtls_ssl_get_max_out_record_payload(&internalsP->sslContext);
        if (res < (int)length)
        {
            // 'res' can be negative (error) or if the current buffer size exceeds the maximum payload, we inform the upper layer
            return res;
        }

        do
        {
            res = mbedtls_ssl_write(&internalsP->sslContext, buffer, length);
        } while (res == MBEDTLS_ERR_SSL_WANT_WRITE);
    }
    else
    {
        // Not datagram, stream
        size_t writtenBufferLength;

        for (writtenBufferLength = 0; writtenBufferLength < length; writtenBufferLength += res)
        {
            do
            {
                res = mbedtls_ssl_write(&internalsP->sslContext, buffer + writtenBufferLength, length - writtenBufferLength);
            } while (res == MBEDTLS_ERR_SSL_WANT_WRITE);

            if (res <= 0)
            {
                // Got an error when sending the message
                break;
            }
        }
    }

    return res;

}

// Receive data on a secure connection.
int iowa_user_security_recv(iowa_security_session_t securityS,
                            uint8_t *buffer,
                            size_t length)
{
    user_security_internal_t *internalsP;
    int res;

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    do
    {
        res = mbedtls_ssl_read(&internalsP->sslContext, buffer, length);
    } while (res == MBEDTLS_ERR_SSL_WANT_READ);

    if (res == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
        IOWA_LOG_INFO(IOWA_PART_SECURITY, "Close notify message received.");
        iowa_security_session_set_state(securityS, SECURITY_STATE_DISCONNECTING);
    }

    return res;
}

// Close a secure connection to a peer.
void iowa_user_security_disconnect(iowa_security_session_t securityS)
{
    user_security_internal_t *internalsP;

    internalsP = (user_security_internal_t *)iowa_security_session_get_user_internals(securityS);

    prv_mbedtlsDisconnect(internalsP, securityS);
}
