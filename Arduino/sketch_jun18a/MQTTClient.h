#ifndef __MQTT_H
#define __MQTT_H
#include <PubSubClient.h>

void WiFiAndMQTTConnection(MQTT_CALLBACK_SIGNATURE);
void MQTTloop();
void publishMessage();

#endif