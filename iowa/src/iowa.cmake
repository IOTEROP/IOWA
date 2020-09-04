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
# Copyright (c) 2016-2019 IoTerop.
# All rights reserved.
#
# This program and the accompanying materials
# are made available under the terms of
# IoTerop’s IOWA License (LICENSE.TXT) which
# accompany this distribution.
#
###############################################

############################################
# Define the internal IOWA header/source files
############################################

## Base files

set(BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/core)

set(BASE_HEADERS
    ${BASE_DIR}/iowa_prv_core.h
    ${BASE_DIR}/iowa_prv_timer.h
    ${BASE_DIR}/iowa_prv_core_internals.h
    ${BASE_DIR}/iowa_prv_core_check_config.h)

set(BASE_SOURCES
    ${BASE_DIR}/iowa_base.c
    ${BASE_DIR}/iowa_buffer.c
    ${BASE_DIR}/iowa_context.c
    ${BASE_DIR}/iowa_timer.c
    ${BASE_DIR}/iowa_comms.c)

set(BASE_CLIENT_SOURCES
    ${BASE_DIR}/iowa_client.c)

set(BASE_SERVER_SOURCES
    ${BASE_DIR}/iowa_server.c)

SOURCE_GROUP(Iowa\\Core FILES ${BASE_HEADERS} ${BASE_SOURCES} ${BASE_CLIENT_SOURCES} ${BASE_SERVER_SOURCES})

## Comm files

set(COMM_DIR ${CMAKE_CURRENT_LIST_DIR}/comm)

set(COMM_HEADERS
    ${COMM_DIR}/iowa_prv_comm.h)

set(COMM_SOURCES
    ${COMM_DIR}/iowa_comm.c)

SOURCE_GROUP(Iowa\\Comm FILES ${COMM_HEADERS} ${COMM_SOURCES})

## CoAP files

set(COAP_DIR ${CMAKE_CURRENT_LIST_DIR}/coap)

set(COAP_HEADERS
    ${COAP_DIR}/iowa_prv_coap.h
    ${COAP_DIR}/iowa_prv_coap_internals.h)

set(COAP_SOURCES
    ${COAP_DIR}/iowa_block.c
    ${COAP_DIR}/iowa_coap.c
    ${COAP_DIR}/iowa_coap_lorawan.c
    ${COAP_DIR}/iowa_coap_sms.c
    ${COAP_DIR}/iowa_coap_tcp.c
    ${COAP_DIR}/iowa_coap_udp.c
    ${COAP_DIR}/iowa_coap_utils.c
    ${COAP_DIR}/iowa_message.c
    ${COAP_DIR}/iowa_option.c
    ${COAP_DIR}/iowa_peer.c
    ${COAP_DIR}/iowa_transaction.c)

SOURCE_GROUP(Iowa\\CoAP FILES ${COAP_HEADERS} ${COAP_SOURCES})

## Data files

set(DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/data)

set(DATA_HEADERS
    ${DATA_DIR}/iowa_prv_data_internals.h
    ${DATA_DIR}/iowa_prv_data.h)

set(DATA_SOURCES
    ${DATA_DIR}/iowa_data.c
    ${DATA_DIR}/iowa_json.c
    ${DATA_DIR}/iowa_senml_json.c
    ${DATA_DIR}/iowa_cbor.c
    ${DATA_DIR}/iowa_senml_cbor.c
    ${DATA_DIR}/iowa_text_opaque.c
    ${DATA_DIR}/iowa_tlv.c
    ${DATA_DIR}/iowa_data_utils.c)

SOURCE_GROUP(Iowa\\Data FILES ${DATA_HEADERS} ${DATA_SOURCES})

## Logger files

set(LOGGER_DIR ${CMAKE_CURRENT_LIST_DIR}/logger)

set(LOGGER_HEADERS
    ${LOGGER_DIR}/iowa_prv_logger.h)

set(LOGGER_SOURCES
    ${LOGGER_DIR}/iowa_logger.c)

SOURCE_GROUP(Iowa\\Logger FILES ${LOGGER_HEADERS} ${LOGGER_SOURCES})

## LwM2M files

set(LWM2M_DIR ${CMAKE_CURRENT_LIST_DIR}/lwm2m)

set(LWM2M_HEADERS
    ${LWM2M_DIR}/iowa_prv_lwm2m.h
    ${LWM2M_DIR}/iowa_prv_lwm2m_internals.h)

set(LWM2M_SOURCES
    ${LWM2M_DIR}/iowa_acl.c
    ${LWM2M_DIR}/iowa_attributes.c
    ${LWM2M_DIR}/iowa_bootstrap.c
    ${LWM2M_DIR}/iowa_core_link.c
    ${LWM2M_DIR}/iowa_lwm2m.c
    ${LWM2M_DIR}/iowa_management.c
    ${LWM2M_DIR}/iowa_objects.c
    ${LWM2M_DIR}/iowa_observe.c
    ${LWM2M_DIR}/iowa_packet.c
    ${LWM2M_DIR}/iowa_registration.c
    ${LWM2M_DIR}/iowa_send.c
    ${LWM2M_DIR}/iowa_uri.c
    ${LWM2M_DIR}/iowa_lwm2m_utils.c
    ${LWM2M_DIR}/iowa_value.c)

SOURCE_GROUP(Iowa\\LwM2M FILES ${LWM2M_HEADERS} ${LWM2M_SOURCES})

## Misc files

set(MISC_DIR ${CMAKE_CURRENT_LIST_DIR}/misc)

set(MISC_HEADERS
    ${MISC_DIR}/iowa_prv_misc.h)

set(MISC_SOURCES
    ${MISC_DIR}/iowa_list.c
    ${MISC_DIR}/iowa_utils.c)

SOURCE_GROUP(Iowa\\Misc FILES ${MISC_HEADERS} ${MISC_SOURCES})

## Objects files

set(OBJECTS_DIR ${CMAKE_CURRENT_LIST_DIR}/objects)

set(OBJECTS_HEADERS
    ${OBJECTS_DIR}/iowa_prv_objects.h
    ${OBJECTS_DIR}/iowa_prv_objects_internals.h)

set(OBJECTS_SOURCES
    ${OBJECTS_DIR}/iowa_object_accelerometer.c
    ${OBJECTS_DIR}/iowa_object_acl.c
    ${OBJECTS_DIR}/iowa_object_apn_connection_profile.c
    ${OBJECTS_DIR}/iowa_object_at_command.c
    ${OBJECTS_DIR}/iowa_object_bearer_selection.c
    ${OBJECTS_DIR}/iowa_object_cellular_connectivity.c
    ${OBJECTS_DIR}/iowa_object_connectivity_monitoring.c
    ${OBJECTS_DIR}/iowa_object_connectivity_stats.c
    ${OBJECTS_DIR}/iowa_object_device.c
    ${OBJECTS_DIR}/iowa_object_digital_output.c
    ${OBJECTS_DIR}/iowa_object_firmware.c
    ${OBJECTS_DIR}/iowa_object_gps.c
    ${OBJECTS_DIR}/iowa_object_gyrometer.c
    ${OBJECTS_DIR}/iowa_object_internals.c
    ${OBJECTS_DIR}/iowa_object_ipso.c
    ${OBJECTS_DIR}/iowa_object_light_control.c
    ${OBJECTS_DIR}/iowa_object_location.c
    ${OBJECTS_DIR}/iowa_object_magnetometer.c
    ${OBJECTS_DIR}/iowa_object_oscore.c
    ${OBJECTS_DIR}/iowa_object_security.c
    ${OBJECTS_DIR}/iowa_object_server.c
    ${OBJECTS_DIR}/iowa_object_software_component.c
    ${OBJECTS_DIR}/iowa_object_software_management.c)

SOURCE_GROUP(Iowa\\Objects FILES ${OBJECTS_HEADERS} ${OBJECTS_SOURCES})

## OSCORE files

set(OSCORE_DIR ${CMAKE_CURRENT_LIST_DIR}/oscore)

set(OSCORE_HEADERS
    ${OSCORE_DIR}/iowa_prv_oscore_internals.h)

set(OSCORE_SOURCES
    ${OSCORE_DIR}/iowa_cose.c
    ${OSCORE_DIR}/iowa_oscore.c)

SOURCE_GROUP(Iowa\\OSCORE FILES ${OSCORE_HEADERS} ${OSCORE_SOURCES})

## Security files

set(SECURITY_DIR ${CMAKE_CURRENT_LIST_DIR}/security)

set(SECURITY_HEADERS
    ${SECURITY_DIR}/iowa_prv_security.h
    ${SECURITY_DIR}/iowa_prv_security_internals.h)

set(SECURITY_SOURCES
    ${SECURITY_DIR}/iowa_layer_mbedtls.c
    ${SECURITY_DIR}/iowa_layer_tinydtls.c
    ${SECURITY_DIR}/iowa_security_helper.c
    ${SECURITY_DIR}/iowa_security.c)

SOURCE_GROUP(Iowa\\Security FILES ${SECURITY_HEADERS} ${SECURITY_SOURCES})

############################################
# Define the exposed IOWA header/source files
############################################

set(IOWA_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/../include)
set(IOWA_OBJECT_HEADERS_DIR ${IOWA_HEADERS_DIR}/objects)

## Common files

set(IOWA_COMMON_HEADERS
    ${IOWA_HEADERS_DIR}/iowa.h
    ${IOWA_HEADERS_DIR}/iowa_IPSO_ID.h
    ${IOWA_HEADERS_DIR}/iowa_logger.h
    ${IOWA_HEADERS_DIR}/iowa_LwM2M_ID.h
    ${IOWA_HEADERS_DIR}/iowa_platform.h
    ${IOWA_HEADERS_DIR}/iowa_security.h
    ${IOWA_HEADERS_DIR}/iowa_utils.h
    ${BASE_HEADERS}
    ${COMM_HEADERS}
    ${COAP_HEADERS}
    ${DATA_HEADERS}
    ${LOGGER_HEADERS}
    ${LWM2M_HEADERS}
    ${MISC_HEADERS}
    ${OSCORE_HEADERS}
    ${SECURITY_HEADERS})

set(IOWA_COMMON_OBJECT_HEADERS
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_accelerometer.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_access_control_list.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_apn_connection_profile.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_at_command.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_bearer_selection.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_cellular_connectivity.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_connectivity_monitoring.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_connectivity_stats.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_digital_output.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_firmware_update.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_gps.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_gyrometer.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_ipso.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_light_control.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_location.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_magnetometer.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_software_component.h
    ${IOWA_OBJECT_HEADERS_DIR}/iowa_software_management.h)

set(IOWA_COMMON_SOURCES
    ${BASE_SOURCES}
    ${COMM_SOURCES}
    ${COAP_SOURCES}
    ${DATA_SOURCES}
    ${LOGGER_SOURCES}
    ${LWM2M_SOURCES}
    ${MISC_SOURCES}
    ${OSCORE_SOURCES}
    ${SECURITY_SOURCES})

set(IOWA_COMMON_DIR
    ${BASE_DIR}
    ${COMM_DIR}
    ${COAP_DIR}
    ${DATA_DIR}
    ${IOWA_HEADERS_DIR}
    ${LOGGER_DIR}
    ${LWM2M_DIR}
    ${MISC_DIR}
    ${OSCORE_DIR}
    ${SECURITY_DIR})

## Client files

set(IOWA_CLIENT_HEADERS
    ${IOWA_COMMON_HEADERS}
    ${IOWA_HEADERS_DIR}/iowa_client.h
    ${OBJECTS_HEADERS}
    ${IOWA_COMMON_OBJECT_HEADERS})

set(IOWA_CLIENT_SOURCES
    ${IOWA_COMMON_SOURCES}
    ${BASE_CLIENT_SOURCES}
    ${OBJECTS_SOURCES})

set(IOWA_CLIENT_DIR
    ${IOWA_COMMON_DIR}
    ${IOWA_OBJECT_HEADERS_DIR}
    ${OBJECTS_DIR})

## Server files

set(IOWA_SERVER_HEADERS
    ${IOWA_COMMON_HEADERS}
    ${IOWA_HEADERS_DIR}/iowa_server.h)

set(IOWA_SERVER_SOURCES
    ${IOWA_COMMON_SOURCES}
    ${BASE_SERVER_SOURCES})

set(IOWA_SERVER_DIR
    ${IOWA_COMMON_DIR})

## IOWA files

set(IOWA_HEADERS
    ${IOWA_CLIENT_HEADERS}
    ${IOWA_SERVER_HEADERS})

SOURCE_GROUP(Iowa\\Headers FILES ${IOWA_HEADERS})

set(IOWA_SOURCES
    ${IOWA_CLIENT_SOURCES}
    ${IOWA_SERVER_SOURCES})

set(IOWA_INCLUDE_DIR
    ${IOWA_CLIENT_DIR}
    ${IOWA_SERVER_DIR})
