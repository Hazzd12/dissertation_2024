#ifndef MQTT_H
#define MQTT_H
#include <PubSubClient.h>

void WiFiAndMQTTConnection(MQTT_CALLBACK_SIGNATURE);
void MQTTloop();
void publishMessage();

#endif