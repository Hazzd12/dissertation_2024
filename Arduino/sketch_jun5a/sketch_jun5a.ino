#include <WiFi.h>  // 使用ESP32的WiFi库
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "myWifi.h"
const char* ssid = SSID;
const char* password = PASSWORD;

// MQTT broker address and port
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASS;


const char* subscribe_topic = "student/ucfnuax";
const char* publish_topic = "student/ucfnuax";


/// Create WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_connect_and_subscribe() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe to the topic
      client.subscribe(subscribe_topic);
      Serial.print("Subscribed to: ");
      Serial.println(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqtt_publish(int* array, size_t length) {
  // 创建一个动态JSON文档
  DynamicJsonDocument doc(1024);

  // 将数组添加到JSON文档中
  JsonArray jsonArray = doc.to<JsonArray>();
  for (size_t i = 0; i < length; i++) {
    jsonArray.add(array[i]);
  }

  // 将JSON文档序列化为字符串
  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  // 发布JSON字符串到MQTT主题
  client.publish(publish_topic, buffer, n);
  Serial.print("Published array to topic: ");
  Serial.println(publish_topic);
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // 解析JSON消息
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // 获取JSON数组
  JsonArray jsonArray = doc.as<JsonArray>();

  Serial.print("Received array: ");
  for (JsonVariant v : jsonArray) {
    Serial.print(v.as<int>());
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // RX2默认为16, TX2默认为17

  Serial.begin(115200);  // 用于调试的串口

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    mqtt_connect_and_subscribe();
  }

  if (Serial2.available()) {
    String received = Serial2.readString();
    Serial.println("Received from Nano 33 BLE: " + received);  // 输出到PC
  }

  //client.loop();

  // 示例数组
  int myArray[] = { 1, 2, 3, 4, 5 };
  size_t arrayLength = sizeof(myArray) / sizeof(myArray[0]);

  // 每隔10秒发布一次数组
  static unsigned long lastPublish = 0;
  // if (millis() - lastPublish > 10000) {
  //   lastPublish = millis();
  //   mqtt_publish(myArray, arrayLength);
  // }
}