#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

#include "myWifi.h"

const char* ssid = SSID;
const char* password = PASSWORD;

// MQTT broker address and port
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASS;

const char* MQTT_PUB_TOPIC[] = { "student/ucfnuax/test",
                                 "student/ucfnuax/test2" };
const char* MQTT_SUB_TOPIC[] = { "student/ucfnuax/test",
                                 "student/ucfnuax/test2" };

int status = WL_IDLE_STATUS;

WiFiClient WiFiclient;
PubSubClient client(WiFiclient);
long lastReconnectAttempt = 0;
long now = 0;
long lastMsg = 0;

char msg[50];
int value = 0;

void doSubscriptions();
void reconnect();
void handlePeopleMoving(byte* payload, int length);

void WiFiAndMQTTConnection(void (*callback)(char*, unsigned char*, unsigned int)) {
  WiFi.setPins(8, 7, 4, 2);

  // 连接到 WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // you're connected now, so print out a success message:
  Serial.println("You're connected to the network");

  // print your Feather's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Device IP Address: ");
  Serial.println(ip);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  doSubscriptions();
  // print info on MQTT broker
  Serial.print("Attempting to connect to MQTT Broker: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);
  lastReconnectAttempt = 0;

  reconnect();
}



void MQTTloop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //publishMessage();
}

void publishMessage() {
  // 每隔一定时间发布一次消息
  now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, 50, "hello world #%d", value);
    Serial.print("Publishing message: ");
    Serial.println(msg);
    client.publish(MQTT_PUB_TOPIC[0], msg);
  }
}




void doSubscriptions() {
  for (int i = 0; i < sizeof(MQTT_SUB_TOPIC) / sizeof(MQTT_SUB_TOPIC[0]); i++) {
    client.subscribe(MQTT_SUB_TOPIC[i]);
    Serial.print("Subscribed to: ");
    Serial.println(MQTT_SUB_TOPIC[i]);
  }
}

void reconnect() {
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print("Using server: ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(mqtt_port);

    // 尝试连接，使用用户名和密码
    if (client.connect("FeatherM0Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // 订阅主题
      doSubscriptions();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 等待 5 秒钟再尝试连接
      delay(5000);
    }
  }
}

