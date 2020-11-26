/**********************************************
 *
 * Copyright (c) 2016-2020 IoTerop.
 * All rights reserved.
 *
 * This program and the accompanying materials
 * are made available under the terms of
 * IoTerop’s IOWA License (LICENSE.TXT) which
 * accompany this distribution.
 *
 **********************************************/

#include "mqtt_user_functions.h"
#include <string.h>

#define MQTT_TEMP_SENSOR_SOURCE "</3303>"

// Connect to an MQTT Broker.
int user_mqtt_connect(MQTTClient **mqttClientP, iowa_mqtt_broker_t *brokerDetailsP)
{
    MQTTClient pahoClient;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    printf("Opening the MQTT Broker connection...\r\n");

    MQTTClient_create(&pahoClient, brokerDetailsP->uri, brokerDetailsP->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = brokerDetailsP->keepAlive;
    if (brokerDetailsP->cleanSession == true)
    {
        conn_opts.cleansession = 1;
    }
    else
    {
        conn_opts.cleansession = 0;
    }
    conn_opts.username = brokerDetailsP->userName;
    conn_opts.binarypwd.data = brokerDetailsP->password;
    conn_opts.binarypwd.len = brokerDetailsP->passwordLength;
    rc = MQTTClient_connect(pahoClient, &conn_opts);

    if (rc != MQTTCLIENT_SUCCESS)
    {
      printf("Failed to connect to connect to the MQTT Broker. Paho error code: %d.\n", rc);
    }
    printf("Successful.\r\n");

    *mqttClientP = pahoClient;
    return rc;
}

// Disconnect from an MQTT Broker.
int user_mqtt_disconnect(MQTTClient **mqttClientP)
{
    int rc;
    MQTTClient *pahoClient;
    rc = 0;

    pahoClient = (MQTTClient *) mqttClientP;

    if (*mqttClientP != NULL)
    {
        printf("Closing the MQTT Broker connection.\r\n");
        rc = MQTTClient_disconnect(*pahoClient, 10000);

        MQTTClient_destroy(pahoClient);
        pahoClient = NULL;
    }

    return rc;
}

int user_mqtt_publish(user_mqtt_info *mqttUserData)
{
    int rc;
    rc = 0;

    if (mqttUserData->mqttClientP == NULL)
    {
        printf("MQTT Broker is disconnected\r\n");
        return 0;
    }

    if (mqttUserData->active == true)
    {
        MQTTClient_message pubmsg = MQTTClient_message_initializer;
        MQTTClient_deliveryToken token;
        uint8_t payload[20];

        if (strcmp(mqttUserData->source, MQTT_TEMP_SENSOR_SOURCE) != 0)
        {
            printf("This sample work only with a topic source equal to </3303>.\r\n");
            return 0;
        }
        memset(payload, 0, 20*sizeof(uint8_t));

        pubmsg.payloadlen = sprintf(payload, "data: %d °C", mqttUserData->sensorValue);
        // If we have something to publish, use paho.
        pubmsg.payload = (void*)payload;
        pubmsg.qos = 1;
        pubmsg.retained = 1;

        rc = MQTTClient_publishMessage(mqttUserData->mqttClientP, mqttUserData->topic, &pubmsg, &token);
    }

    return rc;
}

// The callback called by IOWA when there's an operation by a Server on an MQTT Broker
void mqttBrokerCallback(iowa_dm_operation_t operation, iowa_sensor_t brokerId, iowa_mqtt_broker_t *brokerDetailsP, void *userData, iowa_context_t contextP)
{
    user_mqtt_info *mqttUserData;
    // Display information on the LwM2M Client
    printf("Broker callback called.\r\n");
    printf("Operation: ");
    switch (operation)
    {
    case IOWA_DM_CREATE:
        printf("IOWA_DM_CREATE\r\n");
        break;
    case IOWA_DM_WRITE:
        printf("IOWA_DM_WRITE\r\n");
        break;
    case IOWA_DM_DELETE:
        printf("IOWA_DM_DELETE\r\n");
        break;
    default:
        printf("unexpected (%d)\r\n", (int)operation);
        break;
    }

    printf("Broker Id %u \r\n", brokerId);
    printf("Broker client Id \"%s\" \r\n", brokerDetailsP->clientId);

    mqttUserData = (user_mqtt_info *)userData;

    user_mqtt_disconnect(&(mqttUserData->mqttClientP));

    if (operation != IOWA_DM_DELETE)
    {
        user_mqtt_connect(&(mqttUserData->mqttClientP), brokerDetailsP);
    }

    return;
}

// The callback called by IOWA when there's an operation by a Server on an MQTT Publication
void mqttPublicationCallback(iowa_dm_operation_t operation, iowa_sensor_t publicationId, iowa_mqtt_publication_t *publicationDetailsP, void *userData, iowa_context_t contextP)
{
    iowa_mqtt_broker_t *brokerDetailsP;
    user_mqtt_info *mqttUserData;

    // Display information on the LwM2M Client
    printf("Publication callback called.\r\n");
    printf("Operation: ");
    switch (operation)
    {
    case IOWA_DM_CREATE:
        printf("IOWA_DM_CREATE\r\n");
        break;
    case IOWA_DM_WRITE:
        printf("IOWA_DM_WRITE\r\n");
        break;
    case IOWA_DM_DELETE:
        printf("IOWA_DM_DELETE\r\n");
        break;
    default:
        printf("unexpected (%d)\r\n", (int)operation);
        break;
    }

    printf("Publication Id %u \r\n", publicationId);
    brokerDetailsP = iowa_client_get_mqtt_broker(contextP, publicationDetailsP->brokerId);

    if (brokerDetailsP != NULL)
    {
        printf("Publication's Broker client Id \"%s\". \r\n", brokerDetailsP->clientId);
    }
    else
    {
        printf("Publication is not related to any available Broker.\r\n");
    }

    mqttUserData = (user_mqtt_info *)userData;

    mqttUserData->source = (const char*)publicationDetailsP->source; // This sample work only with a topic source equal to </3303>
    mqttUserData->topic = (const char*)publicationDetailsP->topic;
    mqttUserData->active = true;

    return;
}
