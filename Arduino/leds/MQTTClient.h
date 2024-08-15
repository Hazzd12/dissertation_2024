#ifndef __MQTT_H
#define __MQTT_H
#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

extern WiFiClient WiFiClientInstance;
extern PubSubClient client;

// 声明函数
void WiFiAndMQTTConnection(MQTT_CALLBACK_SIGNATURE);
void MQTTloop();
void publishMessage(const char* topic, String message);

#endif