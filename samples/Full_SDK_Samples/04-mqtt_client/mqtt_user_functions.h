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

#include "MQTTClient.h"
#include "iowa_mqtt_objects.h"

// MQTT user data structure
typedef struct{
    // Publication Info
    const char *topic;
    const char *source;
    bool active;

    //Paho MQTT Client
    MQTTClient *mqttClientP;

    //SensorValue
    int sensorValue;
} user_mqtt_info;

// MQTT user functions
int user_mqtt_connect(MQTTClient **mqttClientP, iowa_mqtt_broker_t *brokerDetailsP);
int user_mqtt_disconnect(MQTTClient **mqttClientP);
int user_mqtt_publish(user_mqtt_info *mqttUserData);
void mqttPublicationCallback(iowa_dm_operation_t operation, iowa_sensor_t publicationId, iowa_mqtt_publication_t *publicationDetailsP, void *userData, iowa_context_t contextP);
void mqttBrokerCallback(iowa_dm_operation_t operation, iowa_sensor_t brokerId, iowa_mqtt_broker_t *brokerDetailsP, void *userData, iowa_context_t contextP);
