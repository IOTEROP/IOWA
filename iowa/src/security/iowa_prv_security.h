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
* Copyright (c) 2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#ifndef _IOWA_PRV_SECURITY_INCLUDE_
#define _IOWA_PRV_SECURITY_INCLUDE_

#include "iowa_security.h"
#include "iowa_config.h"

#include <stddef.h>
#include <stdint.h>

/**************************************************************
* Defines
*/

#define IOWA_SECURITY_LAYER_NONE                0
#define IOWA_SECURITY_LAYER_MBEDTLS             1
#define IOWA_SECURITY_LAYER_MBEDTLS_PSK_ONLY    2
#define IOWA_SECURITY_LAYER_MBEDTLS_OSCORE_ONLY 3
#define IOWA_SECURITY_LAYER_TINYDTLS            4
#define IOWA_SECURITY_LAYER_USER                5

#define STR_SECURITY_STATE(M)                                                     \
((M) == SECURITY_STATE_DISCONNECTED ? "SECURITY_STATE_DISCONNECTED" :             \
((M) == SECURITY_STATE_DISCONNECTING ? "SECURITY_STATE_DISCONNECTING" :           \
((M) == SECURITY_STATE_INIT_HANDSHAKE ? "SECURITY_STATE_INIT_HANDSHAKE" :         \
((M) == SECURITY_STATE_HANDSHAKING ? "SECURITY_STATE_HANDSHAKING" :               \
((M) == SECURITY_STATE_HANDSHAKE_DONE ? "SECURITY_STATE_HANDSHAKE_DONE" :         \
((M) == SECURITY_STATE_CONNECTED ? "SECURITY_STATE_CONNECTED" :                   \
((M) == SECURITY_STATE_CONNECTION_FAILING ? "SECURITY_STATE_CONNECTION_FAILING" : \
((M) == SECURITY_STATE_CONNECTION_FAILED ? "SECURITY_STATE_CONNECTION_FAILED" :   \
"Unknown"))))))))

/**************************************************************
* Types
*/


typedef struct _iowa_security_context_t *iowa_security_context_t;






typedef void(*security_event_callback_t)(iowa_security_session_t securityS,
                                         iowa_security_event_t event,
                                         void *userData,
                                         iowa_context_t contextP);

/*************************************
* API
*/





iowa_status_t securityInit(iowa_context_t contextP);




void securityClose(iowa_context_t contextP);





iowa_status_t securityStep(iowa_context_t contextP);







iowa_security_session_t securityClientNewSession(iowa_context_t contextP,
                                                 const char *uri,
                                                 iowa_security_mode_t securityMode);








iowa_security_session_t securityServerNewSession(iowa_context_t contextP,
                                                 iowa_connection_type_t type,
                                                 void *connP,
                                                 bool isSecure);





void securityDeleteSession(iowa_context_t contextP,
                           iowa_security_session_t securityS);







void securitySetEventCallback(iowa_context_t contextP,
                              iowa_security_session_t securityS,
                              security_event_callback_t eventCb,
                              void *userDataCb);






iowa_status_t securityConnect(iowa_context_t contextP,
                              iowa_security_session_t securityS);





void securityDisconnect(iowa_context_t contextP,
                        iowa_security_session_t securityS);







int securitySend(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);








int securityRecv(iowa_context_t contextP,
                 iowa_security_session_t securityS,
                 uint8_t *buffer,
                 size_t length);






bool securityGetIsSecure(iowa_context_t contextP,
                         iowa_security_session_t securityS);








size_t securityGetIdentity(iowa_context_t contextP,
                           iowa_security_session_t securityS,
                           uint8_t *buffer,
                           size_t length);





void * securityGetConnP(iowa_security_session_t securityS);







iowa_status_t securityAddKey(iowa_context_t contextP,
                             const char *uri,
                             iowa_security_data_t *securityDataP);






iowa_status_t securityRemoveKey(iowa_context_t contextP,
                                const char *uri);

#endif
