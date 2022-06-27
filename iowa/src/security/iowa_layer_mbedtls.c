/**********************************************
*
*  _________ _________ ___________ _________
* |         |         |   |   |   |         |
* |_________|         |   |   |   |    _    |
* |         |    |    |   |   |   |         |
* |         |    |    |           |         |
* |         |    |    |           |    |    |
* |         |         |           |    |    |
* |_________|_________|___________|____|____|
*
* Copyright (c) 2017-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_security_internals.h"

#if (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY)

#pragma message("The configuration flags IOWA_SECURITY_LAYER_MBEDTLS and IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY will be deprecated in the next IOWA release. It is advised to set the flag IOWA_SECURITY_LAYER_USER instead and link with functions in samples/secure_client_mbedtls3/user_security_mbedtls3.c.")

#define PRV_MBEDTLS_SUCCESSFUL           0
#define PRV_MBEDTLS_ERROR_MESSAGE_LENGTH 100

#define PRV_MBEDTLS_TIMER_CANCELLED   -1
#define PRV_MBEDTLS_TIMER_NOT_EXPIRED 0
#define PRV_MBEDTLS_TIMER_EXPIRED     2

#define PRV_ADD_CIPHERSUITE(ciphersuites, id, ciphersuite) ciphersuites[id++] = ciphersuite;

#if IOWA_LOG_LEVEL > IOWA_LOG_LEVEL_NONE
#define PRV_PRINT_MBEDTLS_ERROR(funcName, res) prv_printMbedtlsError(funcName,  (unsigned int)res)
#else
#define PRV_PRINT_MBEDTLS_ERROR(funcName, res)
#endif

/*************************************************************************************
** Private functions
*************************************************************************************/

#if IOWA_LOG_LEVEL == IOWA_LOG_LEVEL_TRACE
static void prv_debug(void *userData,
                      int level,
                      const char *file,
                      int line,
                      const char *str)
{
    const char *strSearch;

    (void)userData;
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

static int prv_mbedtlsSendFunc(void *userData,
                               const unsigned char *buf,
                               size_t len)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    securityS = (iowa_security_session_t)userData;

    return commSend(securityS->contextP, securityS->channelP, (uint8_t *)buf, len);
}

static int prv_mbedtlsRecvFunc(void *userData,
                               unsigned char *buf,
                               size_t len)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;
    int result;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    securityS = (iowa_security_session_t)userData;

    switch (securityS->state)
    {
    case SECURITY_STATE_HANDSHAKING:
        if (securityS->channelP->type != IOWA_CONN_STREAM
            && securityS->dataAvailable == false)
        {
            return MBEDTLS_ERR_SSL_TIMEOUT;
        }

        // Reset the structure
        securityS->startTime = -1;
        securityS->timeout = 0;
        securityS->dataAvailable = false;
        break;

    default:
        // Do nothing
        break;
    }

    result = commRecv(securityS->contextP, securityS->channelP, (uint8_t *)buf, (int)len);
    if (result == 0)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "No message has been received.");
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    // 'result' can be either negative (error) or positive (data available)
    return result;
}

static int prv_mbedtlsRandomVectorGenerator(void *userData,
                                            uint8_t *randomBuffer,
                                            size_t size)
{
    // WARNING: This function is called in a critical section
    int result;
    iowa_security_session_t securityS;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    securityS = (iowa_security_session_t)userData;

    CRIT_SECTION_LEAVE(securityS->contextP);
    result = iowa_system_random_vector_generator(randomBuffer, size, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    return result;
}

#ifdef IOWA_SECURITY_SERVER_MODE
static int prv_mbedtlsPskCallback(void *userData,
                                  mbedtls_ssl_context *ssl,
                                  const unsigned char *identity,
                                  size_t identityLen)
{
    // WARNING: This function is called in a critical section
    iowa_security_session_t securityS;
    iowa_security_data_t securityData;
    iowa_status_t result;
    int res;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    securityS = (iowa_security_session_t)userData;

    memset(&securityData, 0, sizeof(iowa_security_data_t));
    securityData.securityMode = IOWA_SEC_PRE_SHARED_KEY;
    securityData.protocol.pskData.identity = (uint8_t *)identity;
    securityData.protocol.pskData.identityLen = identityLen;

    CRIT_SECTION_LEAVE(securityS->contextP);
    result = iowa_system_security_data(identity, identityLen, IOWA_SEC_READ, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Failed to retrieve the PSK key (%u.%02u)", (result & 0xFF) >> 5, (result & 0x1F));
        return -1;
    }

    // MbedTLS is already checking the private key buffer. No need to do it here.
    res = mbedtls_ssl_set_hs_psk(ssl, securityData.protocol.pskData.privateKey, securityData.protocol.pskData.privateKeyLen);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_set_hs_psk", res);
        // Do not exit the callback here since the security data need to be free
    }

    CRIT_SECTION_LEAVE(securityS->contextP);
    (void)iowa_system_security_data(identity, identityLen, IOWA_SEC_FREE, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    return res;
}

static int prv_mbedtlsCookieRNG(void *userData,
                                uint8_t *randomBuffer,
                                size_t size)
{
    // WARNING: This function is called in a critical section
    static size_t gSize = 0;
    // Should be COOKIE_MD_OUTLEN but it is not exposed by mbedtls. We use 48 which is the maximum possible value.
    static uint8_t gKey[48] = {0};
    int result;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    if (gSize != size)
    {
        result = prv_mbedtlsRandomVectorGenerator(userData, randomBuffer, size);
        if (result == 0)
        {
            memcpy(gKey, randomBuffer, size);
            gSize = size;
        }
    }
    else
    {
        memcpy(randomBuffer, gKey, size);
        result = 0;
    }

    return result;
}
#endif // IOWA_SECURITY_SERVER_MODE

static void prv_mbedtlsSetDelay(void *userData,
                                uint32_t intMs,
                                uint32_t finMs)
{
    iowa_security_session_t securityS;

    (void)intMs;

    securityS = (iowa_security_session_t)userData;

    // Check if the timer must be stopped
    if (finMs == 0)
    {
        securityS->startTime = -1;
        return;
    }

    // Set the timer values
    securityS->timeout = finMs / 1000;

    // Get the time when the timer begins
    securityS->startTime = securityS->contextP->currentTime;
}

static int prv_mbedtlsGetDelay(void *userData)
{
    // Function must return one of the following values:
    // * -1 if timer is cancelled
    // * 0 if none of the delays is expired
    // * 1 if the final delay is expired

    iowa_security_session_t securityS;

    securityS = (iowa_security_session_t)userData;

    if (securityS->startTime < 0
        || securityS->contextP->currentTime < securityS->startTime)
    {
        return PRV_MBEDTLS_TIMER_CANCELLED;
    }

    if ((securityS->contextP->currentTime - securityS->startTime) >= securityS->timeout)
    {
        return PRV_MBEDTLS_TIMER_EXPIRED;
    }

    return PRV_MBEDTLS_TIMER_NOT_EXPIRED;
}

#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
static iowa_status_t prv_initCertificate(iowa_security_session_t securityS)
{
    // WARNING: This function is called in a critical section
    int res;
    iowa_security_data_t securityData;
    iowa_status_t result;
    uint8_t *peerIdentity;
    size_t peerIdentityLen;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    // Only verify certificate on client side
    // Client need to check if it's connecting to the right server
    if (securityS->conf.endpoint == 0) // 0: client, 1: server
    {
        mbedtls_ssl_conf_authmode(&securityS->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }
    else
    {
        mbedtls_ssl_conf_authmode(&securityS->conf, MBEDTLS_SSL_VERIFY_NONE);
    }

    // Retrieve the certificate structure
    memset(&securityData, 0, sizeof(iowa_security_data_t));
    securityData.securityMode = IOWA_SEC_CERTIFICATE;

    if (securityS->conf.endpoint == 0) // 0: client, 1: server
    {
        peerIdentity = (uint8_t *)securityS->uri;
        peerIdentityLen = strlen(securityS->uri);
    }
    else
    {
        peerIdentity = NULL;
        peerIdentityLen = 0;
    }

    CRIT_SECTION_LEAVE(securityS->contextP);
    result = iowa_system_security_data(peerIdentity, peerIdentityLen, IOWA_SEC_READ, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);
    if (result != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ARG_ERROR(IOWA_PART_SYSTEM, "Failed to retrieve the certificate (%u.%02u)", (result & 0xFF) >> 5, (result & 0x1F));
        return result;
    }

    // Configure and parse the certificate
    securityS->cert = (mbedtls_x509_crt *)iowa_system_malloc(sizeof(mbedtls_x509_crt));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->cert == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(mbedtls_x509_crt));
        res = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
        goto error;
    }
#endif
    mbedtls_x509_crt_init(securityS->cert);

    // MbedTLS is already checking the certificate buffer. No need to do it here.
    res = mbedtls_x509_crt_parse(securityS->cert,
                                 securityData.protocol.certData.certificate,
                                 securityData.protocol.certData.certificateLen);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_x509_crt_parse", res);
        goto error;
    }

    // Configure and parse the private key
    securityS->privateKey = (mbedtls_pk_context *)iowa_system_malloc(sizeof(mbedtls_pk_context));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->privateKey == NULL)
    {
        IOWA_LOG_ERROR_MALLOC(sizeof(mbedtls_pk_context));
        res = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
        goto error;
    }
#endif
    mbedtls_pk_init(securityS->privateKey);

    // MbedTLS is already checking the private key buffer. No need to do it here.
    res = mbedtls_pk_parse_key(securityS->privateKey,
                               securityData.protocol.certData.privateKey,
                               securityData.protocol.certData.privateKeyLen,
                               NULL,
                               0);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_pk_parse_key", res);
        goto error;
    }

    res = mbedtls_ssl_conf_own_cert(&securityS->conf,
                                    securityS->cert,
                                    securityS->privateKey);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_conf_own_cert", res);
        goto error;
    }

    // Check if a CA certificate has been provided
    if (securityData.protocol.certData.caCertificate != NULL)
    {
        securityS->caCert = (mbedtls_x509_crt *)iowa_system_malloc(sizeof(mbedtls_x509_crt));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (securityS->caCert == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(mbedtls_x509_crt));
            res = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
            goto error;
        }
#endif
        mbedtls_x509_crt_init(securityS->caCert);

        // MbedTLS is already checking the CA certificate buffer. No need to do it here.
        res = mbedtls_x509_crt_parse(securityS->caCert,
                                     securityData.protocol.certData.caCertificate,
                                     securityData.protocol.certData.caCertificateLen);
        if (res != PRV_MBEDTLS_SUCCESSFUL)
        {
            PRV_PRINT_MBEDTLS_ERROR("mbedtls_x509_crt_parse", res);
            goto error;
        }

        mbedtls_ssl_conf_ca_chain(&securityS->conf, securityS->caCert, NULL);
    }

error:
    CRIT_SECTION_LEAVE(securityS->contextP);
    (void)iowa_system_security_data(peerIdentity, peerIdentityLen, IOWA_SEC_FREE, &securityData, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    return res;
}
#endif

static iowa_status_t prv_addCiphersuites(iowa_security_session_t securityS,
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

    securityS->ciphersuites = (int *)iowa_system_malloc(ciphersuitesArrayLength*sizeof(int));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
    if (securityS->ciphersuites == NULL)
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
        PRV_ADD_CIPHERSUITE(securityS->ciphersuites, pos, MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8);
        PRV_ADD_CIPHERSUITE(securityS->ciphersuites, pos, MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);
    }

    if (psk == true)
    {
        IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Adding pre-shared key ciphersuites");

        PRV_ADD_CIPHERSUITE(securityS->ciphersuites, pos, MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8);
        PRV_ADD_CIPHERSUITE(securityS->ciphersuites, pos, MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256);
    }

    PRV_ADD_CIPHERSUITE(securityS->ciphersuites, pos, 0);

    mbedtls_ssl_conf_ciphersuites(&securityS->conf, securityS->ciphersuites);

    return IOWA_COAP_NO_ERROR;
}

static void prv_connectionFailing(iowa_security_session_t securityS)
{
    mbedtlsDisconnect(securityS);
    securityS->contextP->timeout = 0;
    securityS->state = SECURITY_STATE_CONNECTION_FAILED;
    // After the session event callback, don't try to access 'securityS' pointer since the callback could have removed it.
    SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DISCONNECTED);
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t mbedtlsStep(iowa_security_session_t securityS)
{
    iowa_status_t result;

    switch (securityS->state)
    {
    case SECURITY_STATE_DISCONNECTING:
        mbedtlsDisconnect(securityS);
        securityS->state = SECURITY_STATE_DISCONNECTED;
        // After the session event callback, don't try to access 'securityS' pointer since the callback could have removed it.
        SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_DISCONNECTED);
        break;

    case SECURITY_STATE_INIT_HANDSHAKE:
        if (securityS->sslContext.state != MBEDTLS_SSL_HELLO_REQUEST)
        {
            IOWA_LOG_WARNING(IOWA_PART_SECURITY, "Session state is not Hello Request.");
            return IOWA_COAP_412_PRECONDITION_FAILED;
        }

        if (securityS->conf.endpoint == 0) // 0: client, 1: server
        {
            result = mbedtlsConnect(securityS);
            if (result != IOWA_COAP_NO_ERROR)
            {
                IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to initiate the handshake.");
                prv_connectionFailing(securityS);
                break;
            }

            if (securityS->sslContext.state != MBEDTLS_SSL_HANDSHAKE_OVER)
            {
                securityS->state = SECURITY_STATE_HANDSHAKING;
            }
        }
        break;

    case SECURITY_STATE_HANDSHAKING:
        result = mbedtlsConnect(securityS);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Handshaking error.");
            prv_connectionFailing(securityS);
        }
        break;

    default:
        break;
    }

    return IOWA_COAP_NO_ERROR;
}

#ifdef IOWA_SECURITY_CLIENT_MODE
iowa_status_t mbedtlsCreateClientSession(iowa_security_session_t securityS)
{
    // WARNING: This function is called in a critical section
    int res;
    int transport;
    uint8_t maxFragmentLengthCode;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    mbedtls_ssl_init(&securityS->sslContext);
    mbedtls_ssl_config_init(&securityS->conf);

    switch (securityS->type)
    {
    case IOWA_CONN_STREAM:
        transport = MBEDTLS_SSL_TRANSPORT_STREAM;
        break;

    default:
        transport = MBEDTLS_SSL_TRANSPORT_DATAGRAM;
    }

    res = mbedtls_ssl_config_defaults(&securityS->conf,
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
    mbedtls_ssl_conf_dbg(&securityS->conf, prv_debug, NULL);
#endif

    // Set the random vector generator function to use
    mbedtls_ssl_conf_rng(&securityS->conf, prv_mbedtlsRandomVectorGenerator, securityS);

    // Set the Maximum Fragment Length extension
    if (IOWA_BUFFER_SIZE <= 512)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_512;
    }
    else if (IOWA_BUFFER_SIZE <= 1024)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_1024;
    }
    else if (IOWA_BUFFER_SIZE <= 2048)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_2048;
    }
    else if (IOWA_BUFFER_SIZE <= 4096)
    {
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_4096;
    }
    else
    {
        // Do not set the maximum fragment length
        maxFragmentLengthCode = MBEDTLS_SSL_MAX_FRAG_LEN_NONE;
    }

    (void)mbedtls_ssl_conf_max_frag_len(&securityS->conf, maxFragmentLengthCode);

    // Set up based on the security mode
    switch (securityS->securityMode)
    {
    case IOWA_SEC_PRE_SHARED_KEY:
    {
        iowa_security_data_t securityData;
        iowa_status_t result;

        if (prv_addCiphersuites(securityS, false, true) != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to add the ciphersuites");
            goto error;
        }

        memset(&securityData, 0, sizeof(iowa_security_data_t));
        securityData.securityMode = IOWA_SEC_PRE_SHARED_KEY;

        CRIT_SECTION_LEAVE(securityS->contextP);
        result = iowa_system_security_data((uint8_t *)securityS->uri, strlen(securityS->uri), IOWA_SEC_READ, &securityData, securityS->contextP->userData);
        CRIT_SECTION_ENTER(securityS->contextP);
        if (result != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SYSTEM, "No PSK key-identity pair found");
            goto error;
        }

        // MbedTLS is already checking the PSK identity/key buffer. No need to do it here.
        res = mbedtls_ssl_conf_psk(&securityS->conf, securityData.protocol.pskData.privateKey, securityData.protocol.pskData.privateKeyLen,
                                   securityData.protocol.pskData.identity, securityData.protocol.pskData.identityLen);

        CRIT_SECTION_LEAVE(securityS->contextP);
        (void)iowa_system_security_data((uint8_t *)securityS->uri, strlen(securityS->uri), IOWA_SEC_FREE, &securityData, securityS->contextP->userData);
        CRIT_SECTION_ENTER(securityS->contextP);

        // Check previous 'mbedtls_ssl_conf_psk' result here since the security data is now free
        if (res != PRV_MBEDTLS_SUCCESSFUL)
        {
            PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_conf_psk", res);
            goto error;
        }

        break;
    }

#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
    case IOWA_SEC_CERTIFICATE:
    {
        if (prv_addCiphersuites(securityS, true, false) != IOWA_COAP_NO_ERROR)
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
        IOWA_LOG_ARG_ERROR(IOWA_PART_SECURITY, "Unknown security mode: %d", securityS->securityMode);
        goto error;
    }

#ifdef MBEDTLS_SSL_DTLS_CONNECTION_ID
    CRIT_SECTION_LEAVE(securityS->contextP);
    res = iowa_system_random_vector_generator(securityS->connId, MBEDTLS_CONN_ID_LENGTH, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    if (res != 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to generate the connection ID.");
        goto error;
    }

    res = mbedtls_ssl_conf_cid(&securityS->conf, MBEDTLS_CONN_ID_LENGTH, MBEDTLS_SSL_UNEXPECTED_CID_IGNORE);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }
#endif

    res = mbedtls_ssl_setup(&securityS->sslContext, &securityS->conf);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }

#ifdef MBEDTLS_SSL_DTLS_CONNECTION_ID
    res = mbedtls_ssl_set_cid(&securityS->sslContext, MBEDTLS_SSL_CID_ENABLED, securityS->connId, MBEDTLS_CONN_ID_LENGTH);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }
#endif

    // Use cases are:
    // - non blocking I/O: f_recv != NULL and f_recv_timeout == NULL
    // - blocking I/O: f_recv == NULL and f_recv_timout != NULL
    mbedtls_ssl_set_bio(&securityS->sslContext, securityS, prv_mbedtlsSendFunc, prv_mbedtlsRecvFunc, NULL);

    mbedtls_ssl_set_timer_cb(&securityS->sslContext, securityS, prv_mbedtlsSetDelay, prv_mbedtlsGetDelay);

    return IOWA_COAP_NO_ERROR;

error:
    mbedtlsDeleteSession(securityS);

    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}
#endif

#ifdef IOWA_SECURITY_SERVER_MODE
iowa_status_t mbedtlsCreateServerSession(iowa_security_session_t securityS)
{
    // WARNING: This function is called in a critical section
    int res;
    int transport;
    uint8_t idBuffer[IOWA_PEER_IDENTIFIER_SIZE];
    size_t idLength;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

    mbedtls_ssl_init(&securityS->sslContext);
    mbedtls_ssl_config_init(&securityS->conf);

    switch (securityS->channelP->type)
    {
    case IOWA_CONN_STREAM:
        transport = MBEDTLS_SSL_TRANSPORT_STREAM;
        break;

    default:
        transport = MBEDTLS_SSL_TRANSPORT_DATAGRAM;
    }

    res = mbedtls_ssl_config_defaults(&securityS->conf,
                                      MBEDTLS_SSL_IS_SERVER,
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
    mbedtls_ssl_conf_dbg(&securityS->conf, prv_debug, NULL);
#endif

    mbedtls_ssl_conf_rng(&securityS->conf, prv_mbedtlsRandomVectorGenerator, securityS);

    // PSK mode
    mbedtls_ssl_conf_psk_cb(&securityS->conf, prv_mbedtlsPskCallback, securityS);

#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
    // Certificate mode
    if (prv_initCertificate(securityS) != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_INFO(IOWA_PART_SECURITY, "Do not expose ciphersuites related to certificate");

        if (prv_addCiphersuites(securityS, false, true) != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to add the ciphersuites");
            goto error;
        }
    }
    else
    {
        if (prv_addCiphersuites(securityS, true, true) != IOWA_COAP_NO_ERROR)
        {
            IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to add the ciphersuites");
            goto error;
        }
    }
#else
    if (prv_addCiphersuites(securityS, false, true) != IOWA_COAP_NO_ERROR)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to add the ciphersuites");
        goto error;
    }
#endif

#ifdef MBEDTLS_SSL_DTLS_CONNECTION_ID
    CRIT_SECTION_LEAVE(securityS->contextP);
    res = iowa_system_random_vector_generator(securityS->connId, MBEDTLS_CONN_ID_LENGTH, securityS->contextP->userData);
    CRIT_SECTION_ENTER(securityS->contextP);

    if (res != 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to generate the connection ID.");
        goto error;
    }

    res = mbedtls_ssl_conf_cid(&securityS->conf, MBEDTLS_CONN_ID_LENGTH, MBEDTLS_SSL_UNEXPECTED_CID_IGNORE);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }
#endif

    res = mbedtls_ssl_setup(&securityS->sslContext, &securityS->conf);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }

#ifdef MBEDTLS_SSL_DTLS_CONNECTION_ID
    res = mbedtls_ssl_set_cid(&securityS->sslContext, MBEDTLS_SSL_CID_ENABLED, securityS->connId, MBEDTLS_CONN_ID_LENGTH);
    if (res != PRV_MBEDTLS_SUCCESSFUL)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_setup", res);
        goto error;
    }
#endif

    idLength = securityGetIdentity(securityS->contextP, securityS, idBuffer, IOWA_PEER_IDENTIFIER_SIZE);
    if (idLength == 0)
    {
        IOWA_LOG_ERROR(IOWA_PART_SECURITY, "Failed to retrieve the client network address.");
        goto error;
    }
    res = mbedtls_ssl_set_client_transport_id(&securityS->sslContext, idBuffer, idLength);
    if (res != 0)
    {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_set_client_transport_id", res);
        goto error;
    }

    mbedtls_ssl_cookie_init(&securityS->cookieContext);
    res = mbedtls_ssl_cookie_setup(&securityS->cookieContext,
                                   prv_mbedtlsCookieRNG, securityS);
    if (res != 0) {
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_cookie_setup", res);
        goto error;
    }

    mbedtls_ssl_conf_dtls_cookies(&securityS->conf, mbedtls_ssl_cookie_write,
                                  mbedtls_ssl_cookie_check,
                                  &securityS->cookieContext);

    // Use cases are:
    // - non blocking I/O: f_recv != NULL and f_recv_timeout == NULL
    // - blocking I/O: f_recv == NULL and f_recv_timout != NULL
    mbedtls_ssl_set_bio(&securityS->sslContext, securityS, prv_mbedtlsSendFunc, prv_mbedtlsRecvFunc, NULL);

    mbedtls_ssl_set_timer_cb(&securityS->sslContext, securityS, prv_mbedtlsSetDelay, prv_mbedtlsGetDelay);

    return IOWA_COAP_NO_ERROR;

error:
    mbedtlsDeleteSession(securityS);

    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
}
#endif

void mbedtlsDeleteSession(iowa_security_session_t securityS)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering");

#ifdef IOWA_SECURITY_CERTIFICATE_SUPPORT
    if (securityS->caCert != NULL)
    {
        mbedtls_x509_crt_free(securityS->caCert);
        iowa_system_free(securityS->caCert);
    }

    if (securityS->cert != NULL)
    {
        mbedtls_x509_crt_free(securityS->cert);
        iowa_system_free(securityS->cert);
    }

    if (securityS->privateKey != NULL)
    {
        mbedtls_pk_free(securityS->privateKey);
        iowa_system_free(securityS->privateKey);
    }
#endif

    mbedtls_ssl_free(&securityS->sslContext);
    mbedtls_ssl_config_free(&securityS->conf);

    if (securityS->ciphersuites != NULL)
    {
        iowa_system_free(securityS->ciphersuites);
    }
}

iowa_status_t mbedtlsHandleHandshakePacket(iowa_security_session_t securityS)
{
    int res;

next_state:
    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "MbedTLS state: %s and timeout: %u.", STR_MBEDTLS_STATE(securityS->sslContext.state), securityS->contextP->timeout);

    if (securityS->conf.endpoint == 0) // 0: client, 1: server
    {
        switch (securityS->sslContext.state)
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
    }
    else
    {
        switch (securityS->sslContext.state)
        {
        case MBEDTLS_SSL_HELLO_REQUEST:
        case MBEDTLS_SSL_CLIENT_HELLO:
        case MBEDTLS_SSL_SERVER_HELLO:
        case MBEDTLS_SSL_CLIENT_CERTIFICATE:
        case MBEDTLS_SSL_CLIENT_KEY_EXCHANGE:
        case MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC:
        case MBEDTLS_SSL_CLIENT_FINISHED:
        case MBEDTLS_SSL_FLUSH_BUFFERS:
        case MBEDTLS_SSL_HANDSHAKE_WRAPUP:
        case MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT:
            // Do nothing
            break;

        default:
            goto exit;
        }
    }

    res = mbedtls_ssl_handshake_step(&securityS->sslContext);
    switch (res)
    {
    case PRV_MBEDTLS_SUCCESSFUL:
        switch (securityS->sslContext.state)
        {
        case MBEDTLS_SSL_HANDSHAKE_OVER:
            IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake done.");
            securityS->state = SECURITY_STATE_CONNECTED;
            // After the session event callback, don't try to access 'securityS' pointer since the callback could have removed it.
            SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_CONNECTED);
            break;

        default:
            if (securityS->timeout == 0)
            {
                goto next_state;
            }
            else if (securityS->timeout < securityS->contextP->timeout)
            {
                securityS->contextP->timeout = securityS->timeout;
            }
        }
        break;

    case MBEDTLS_ERR_SSL_WANT_READ:
        if (securityS->contextP->timeout > securityS->timeout)
        {
            securityS->contextP->timeout = securityS->timeout;
        }
        break;

    default:
        PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_handshake_step", res);
        prv_connectionFailing(securityS);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

exit:

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with MbedTLS state: %s and timeout: %u.", STR_MBEDTLS_STATE(securityS->sslContext.state), securityS->contextP->timeout);

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t mbedtlsConnect(iowa_security_session_t securityS)
{
    int res;
    int timeout;

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Entering with MbedTLS state: %s and timeout: %u.", STR_MBEDTLS_STATE(securityS->sslContext.state), securityS->contextP->timeout);

    timeout = prv_mbedtlsGetDelay((void *)securityS);
    switch (timeout)
    {
    case PRV_MBEDTLS_TIMER_CANCELLED:
    case PRV_MBEDTLS_TIMER_EXPIRED:
next_state:
        res = mbedtls_ssl_handshake_step(&securityS->sslContext);
        switch (res)
        {
        case PRV_MBEDTLS_SUCCESSFUL:
            switch (securityS->sslContext.state)
            {
            case MBEDTLS_SSL_HANDSHAKE_OVER:
                IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Handshake done.");
                securityS->state = SECURITY_STATE_CONNECTED;
                // After the session event callback, don't try to access 'securityS' pointer since the callback could have removed it.
                SESSION_CALL_EVENT_CALLBACK(securityS, SECURITY_EVENT_CONNECTED);
                break;

            default:
                if (securityS->timeout == 0)
                {
                    goto next_state;
                }
            }
            break;

        case MBEDTLS_ERR_SSL_WANT_READ:
            break;

#ifdef IOWA_SECURITY_SERVER_MODE
        case MBEDTLS_ERR_SSL_UNKNOWN_IDENTITY:
#endif
        default:
            PRV_PRINT_MBEDTLS_ERROR("mbedtls_ssl_handshake_step", res);
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
        break;

    default:
        // Do nothing
        break;
    }

    if (securityS->contextP->timeout > securityS->timeout)
    {
        securityS->contextP->timeout = securityS->timeout;
    }

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with MbedTLS state: %s and timeout: %u.", STR_MBEDTLS_STATE(securityS->sslContext.state), securityS->contextP->timeout);

    return IOWA_COAP_NO_ERROR;
}

void mbedtlsDisconnect(iowa_security_session_t securityS)
{
    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    (void)mbedtls_ssl_close_notify(&securityS->sslContext);
    mbedtls_ssl_session_reset(&securityS->sslContext);
}

int mbedtlsSend(iowa_security_session_t securityS,
                uint8_t *buffer,
                size_t length)
{
    int res;
    size_t writtenBufferLength;

    if (securityS->type == IOWA_CONN_DATAGRAM)
    {
        // Datagram
        res = mbedtls_ssl_get_max_out_record_payload(&securityS->sslContext);
        if (res < (int)length)
        {
            // 'res' can be negative (error) or if the current buffer size exceeds the maximum payload, we inform the upper layer
            return res;
        }
    }

    for (writtenBufferLength = 0; writtenBufferLength < length; writtenBufferLength += res)
    {
        do
        {
            res = mbedtls_ssl_write(&securityS->sslContext, buffer + writtenBufferLength, length - writtenBufferLength);
        } while (res == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (res <= 0)
        {
            // Got an error when sending the message, we inform the upper layer
            return res;
        }
    }

    // Return the number of bytes written
    return writtenBufferLength;
}

int mbedtlsRecv(iowa_security_session_t securityS,
                uint8_t *buffer,
                size_t length)
{
    int res;

    res = mbedtls_ssl_read(&securityS->sslContext, buffer, length);

    if (res == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
        IOWA_LOG_INFO(IOWA_PART_SECURITY, "Close notify message received.");

        securityS->contextP->timeout = 0;
        securityS->state = SECURITY_STATE_DISCONNECTING;

        res = 0;
    }
    else if (res == MBEDTLS_ERR_SSL_CLIENT_RECONNECT)
    {
        IOWA_LOG_INFO(IOWA_PART_SECURITY, "Peer is reconnecting.");

        securityS->contextP->timeout = 0;
        securityS->state = SECURITY_STATE_HANDSHAKING;

        res = 0;
    }

    return res;
}

#endif // (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY)

/*************************************************************************************
** OSCORE related functions
*************************************************************************************/

#if (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY)

iowa_status_t iowa_user_security_HKDF(iowa_security_hash_t hash,
                                      uint8_t *IKM, size_t IKMLength,
                                      uint8_t *salt, size_t saltLength,
                                      uint8_t *info, size_t infoLength,
                                      uint8_t *OKM, size_t OKMLength)
{
    mbedtls_md_type_t md_type;
    int res;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    switch (hash)
    {
    case SECURITY_HMAC_SHA256:
        md_type = MBEDTLS_MD_SHA256;
        break;

    case SECURITY_HMAC_SHA384:
        md_type = MBEDTLS_MD_SHA384;
        break;

    case SECURITY_HMAC_SHA512:
        md_type = MBEDTLS_MD_SHA512;
        break;

    default:
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "Unsupported hash function: %d.", hash);
        return IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    res = mbedtls_hkdf(mbedtls_md_info_from_type(md_type),
                       salt, saltLength, IKM, IKMLength, info, infoLength, OKM, OKMLength);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "mbedtls_hkdf() returned 0x%04X.", -res);

    if (res != 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "mbedtls_hkdf() failed with error 0x%04X.", -res);
        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
    }

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Exiting on success.");

    return IOWA_COAP_NO_ERROR;
}

iowa_status_t iowa_user_security_AEAD_encrypt(iowa_security_aead_t aead,
                                              uint8_t *key, size_t keyLength,
                                              uint8_t *nonce, size_t nonceLength,
                                              uint8_t *aad, size_t aadLength,
                                              uint8_t *plainData, size_t plainDataLength,
                                              uint8_t *encryptedData, size_t *encryptedDataLengthP,
                                              uint8_t *tag, size_t tagLength)
{
    iowa_status_t result;
    mbedtls_cipher_type_t cipher_type;
    int res;
    mbedtls_cipher_context_t ctx;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    switch (aead)
    {
    case SECURITY_AEAD_AES_CCM_16_64_128:
        cipher_type = MBEDTLS_CIPHER_AES_128_CCM;
        break;

    case SECURITY_AEAD_AES_CCM_16_64_256:
        cipher_type = MBEDTLS_CIPHER_AES_256_CCM;
        break;

    default:
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "Unsupported AEAD algorithm: %d.", aead);
        return IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    mbedtls_cipher_init(&ctx);
    res = mbedtls_cipher_setup(&ctx, mbedtls_cipher_info_from_type(cipher_type));
    if (res != 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "mbedtls_cipher_setup() returned 0x%04X.", -res);
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto exit;
    }

    res = mbedtls_cipher_setkey(&ctx, key, 8 * keyLength, MBEDTLS_ENCRYPT);
    if (res != 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "mbedtls_cipher_setkey() returned 0x%04X.", -res);
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto exit;
    }

    res = mbedtls_cipher_auth_encrypt(&ctx, nonce, nonceLength, aad, aadLength, plainData, plainDataLength, encryptedData, encryptedDataLengthP, tag, tagLength);
    if (res != 0)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "mbedtls_cipher_auth_encrypt() returned 0x%04X.", -res);
        result = IOWA_COAP_401_UNAUTHORIZED;
    }
    else
    {
        result = IOWA_COAP_NO_ERROR;
    }

exit:

    mbedtls_cipher_free(&ctx);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

iowa_status_t iowa_user_security_AEAD_decrypt(iowa_security_aead_t aead,
                                              uint8_t *key, size_t keyLength,
                                              uint8_t *nonce, size_t nonceLength,
                                              uint8_t *aad, size_t aadLength,
                                              uint8_t *tag, size_t tagLength,
                                              uint8_t *encryptedData, size_t encryptedDataLength,
                                              uint8_t *plainData, size_t *plainDataLengthP)
{
    iowa_status_t result;
    mbedtls_cipher_type_t cipher_type;
    int res;
    mbedtls_cipher_context_t ctx;

    IOWA_LOG_TRACE(IOWA_PART_SECURITY, "Entering.");

    switch (aead)
    {
    case SECURITY_AEAD_AES_CCM_16_64_128:
        cipher_type = MBEDTLS_CIPHER_AES_128_CCM;
        break;

    case SECURITY_AEAD_AES_CCM_16_64_256:
        cipher_type = MBEDTLS_CIPHER_AES_256_CCM;
        break;

    default:
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "Unsupported AEAD algorithm: %d.", aead);
        return IOWA_COAP_501_NOT_IMPLEMENTED;
    }

    mbedtls_cipher_init(&ctx);
    res = mbedtls_cipher_setup(&ctx, mbedtls_cipher_info_from_type(cipher_type));
    if (res != 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "mbedtls_cipher_setup() returned 0x%04X.", -res);
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto exit;
    }

    res = mbedtls_cipher_setkey(&ctx, key, 8 * keyLength, MBEDTLS_DECRYPT);
    if (res != 0)
    {
        IOWA_LOG_ARG_WARNING(IOWA_PART_SECURITY, "mbedtls_cipher_setkey() returned 0x%04X.", -res);
        result = IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        goto exit;
    }

    res = mbedtls_cipher_auth_decrypt(&ctx, nonce, nonceLength, aad, aadLength, encryptedData, encryptedDataLength, plainData, plainDataLengthP, tag, tagLength);
    if (res != 0)
    {
        IOWA_LOG_ARG_INFO(IOWA_PART_SECURITY, "mbedtls_cipher_auth_encrypt() returned 0x%04X.", -res);
        result = IOWA_COAP_401_UNAUTHORIZED;
    }
    else
    {
        result = IOWA_COAP_NO_ERROR;
    }

exit:

    mbedtls_cipher_free(&ctx);

    IOWA_LOG_ARG_TRACE(IOWA_PART_SECURITY, "Exiting with result %u.%02u.", (result & 0xFF) >> 5, (result & 0x1F));

    return result;
}

#endif // (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY) || (IOWA_SECURITY_LAYER == IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY)
