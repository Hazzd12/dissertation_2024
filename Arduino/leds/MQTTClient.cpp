#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>

#include "myWifi.h"

const char* ssid = WIFISSID;
const char* password = WIFIPASSWORD;

// MQTT broker address and port
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASS;

const char* MQTT_SUB_TOPIC[] = { "student/ucfnuax/moving",
                                 "student/ucfnuax/Gesture",
                                 "student/ucfnuax/GesStatus",
                                 "student/ucfnuax/Message"};

int status = WL_IDLE_STATUS;

WiFiClient WiFiClientInstance;
PubSubClient client(WiFiClientInstance);
long lastReconnectAttempt = 0;

long lastMsg = 0;

char msg[50];
int value = 0;

void doSubscriptions();
void reconnect();
void handlePeopleMoving(byte* payload, int length);

void WiFiAndMQTTConnection(void (*callback)(char*, unsigned char*, unsigned int)) {
  WiFi.setPins(8, 7, 4, 2);

  WiFi.disconnect();
  // 连接到 WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  

    // 将复位引脚拉低
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(retries++ > 10){
      WiFi.disconnect();
      delay(500);
      WiFi.begin(ssid, password);
      Serial.println("Reconnecting");
      delay(500);
    }

  }
  // if(retries>=20){
  //   Serial.println();
  //   Serial.println("reset");
  //   digitalWrite(9, HIGH);
  // }
  // you're connected now, so print out a success message:
  Serial.println("You're connected to the network");

  // print your Feather's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Device IP Address: ");
  Serial.println(ip);

byte mac[6];
  WiFi.macAddress(mac);

  // 输出MAC地址
  Serial.print("MAC Address: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println();

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

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
      WiFi.disconnect();
      delay(500);
      WiFi.begin(ssid, password);
      Serial.println("Reconnecting");
    }


  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //publishMessage();
}


void publishMessage(const char* topic, String message) {
  // 每隔一定时间发布一次消息
  const char* c_mes = message.c_str();
  client.publish(topic, c_mes);
}




void doSubscriptions() {
  for (int i = 0; i < sizeof(MQTT_SUB_TOPIC) / sizeof(MQTT_SUB_TOPIC[0]); i++) {
    client.subscribe(MQTT_SUB_TOPIC[i]);
    Serial.print("Subscribed to: ");
    Serial.println(MQTT_SUB_TOPIC[i]);
  }
}

void reconnect() {
  String clientId = "MyESP32Client-";
  clientId += String(random(0xffff), HEX);
  client.setKeepAlive(3600); 
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

