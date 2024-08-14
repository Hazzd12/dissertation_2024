#define S10 25
#define S11 33  // 修改为有效的引脚号
#define S12 32
#define Z1 26  // 将Z设置为数字引脚，而不是模拟引脚
#define PIR8 27
#define S20 19
#define S21 18
#define S22 5
#define Z2 21

#define LED 4
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


const char* publish_topic = "student/ucfnuax/moving";

const char* subscribe_topics[] = { "student/ucfnuax/status",
                                   "student/ucfnuax/GesStatus" };

unsigned long lastPublish = 0;
unsigned long lastLed = 0;
/// Create WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

bool status = true;
bool ledSt = false;

int PIRs[17];

void setup() {
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // RX2默认为16, TX2默认为17

  Serial.begin(9600);  // 用于调试的串口

  setup_wifi();


  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  String macAddress = WiFi.macAddress();
  // 输出MAC地址
  Serial.println("MAC Address: " + macAddress);
  set_PIRs();
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
}

void loop() {

  unsigned long nowTime = millis();

  if (!client.connected()) {
    mqtt_connect_and_subscribe();
  }

  if (Serial2.available() && !ledSt) {
    digitalWrite(LED, HIGH);
    ledSt = true;
    lastLed = nowTime;
    String received = Serial2.readString();
    mqtt_publish_Ges(received);
    Serial.println("Received from Nano 33 BLE: " + received);  // 输出到PC
  }

  if (ledSt && nowTime - lastLed > 3000) {
    ledSt = false;
    String received = Serial2.readString();
    digitalWrite(LED, LOW);

  }

  client.loop();




  // 每隔10秒发布一次数组
  if (nowTime - lastPublish > 1000 && status) {
    loop_PIRs();
    lastPublish =nowTime;
    mqtt_publish_PIR(PIRs, 17);
  }
}

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


void set_PIRs() {
  pinMode(S10, OUTPUT);
  pinMode(S11, OUTPUT);
  pinMode(S12, OUTPUT);
  Serial.begin(9600);
  pinMode(Z1, INPUT_PULLDOWN);
  pinMode(S20, OUTPUT);
  pinMode(S21, OUTPUT);
  pinMode(S22, OUTPUT);
  pinMode(Z2, INPUT_PULLDOWN);
  pinMode(PIR8, INPUT);
}


void mqtt_connect_and_subscribe() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe to the topic
      for (int i = 0; i < sizeof(subscribe_topics) / sizeof(subscribe_topics); i++) {
        client.subscribe(subscribe_topics[i]);
        Serial.print("Subscribed to: ");
        Serial.println(subscribe_topics[i]);
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqtt_publish_PIR(int* array, size_t length) {
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

void mqtt_publish_Ges(String mes) {
  const char* c_mes = mes.c_str();
  client.publish("student/ucfnuax/Gesture", c_mes);
}

int count = 0;
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  Serial.println(count++);
  // 解析JSON消息
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  if (strcmp(topic, "student/ucfnuax/status") == 0) {
    if (message == "0") {
      status = false;
    } else if (message == "1") {
      status = true;
    }
  } else if (strcmp(topic, "student/ucfnuax/GesStatus") == 0) {
    if (message == "0") {

      for (int i = 0; i < 5; i++) {
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);
        delay(50);
      }
    }
  }
  Serial.println();
}


void loop_PIRs() {
  for (int i = 0; i < 8; i++) {
    // 设置选择引脚的状态

    digitalWrite(S10, (i & 0x01));
    digitalWrite(S11, (i & 0x02) >> 1);
    digitalWrite(S12, (i & 0x04) >> 2);

    digitalWrite(S20, (i & 0x01));
    digitalWrite(S21, (i & 0x02) >> 1);
    digitalWrite(S22, (i & 0x04) >> 2);

    delay(5);  // 增加延迟，确保通道切换完成

    int value1 = digitalRead(Z1);  // 读取选定通道的数字值
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(value1);
    PIRs[i] = value1;

    int value2 = digitalRead(Z2);  // 读取选定通道的数字值
    // Serial.print("Channel ");
    // Serial.print(i + 9);
    // Serial.print(": ");
    // Serial.println(value2);
    PIRs[i + 9] = value2;
  }

  int value = digitalRead(PIR8);
  PIRs[8] = value;
  Serial.print("PIR8: ");
  Serial.println(value);
}
