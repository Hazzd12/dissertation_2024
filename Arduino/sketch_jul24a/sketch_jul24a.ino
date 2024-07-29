#include <ArduinoJson.h>
#include "MQTTClient.h"

int movingArray[32]={0};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFiAndMQTTConnection(callback);
  // Feather M0-specific WiFi pins
}

void loop(){
  MQTTloop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // 处理接收到的消息
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(length);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strcmp(topic, "student/ucfnuax/test") == 0){
    handlePeopleMoving(payload, length);
  }
}

void handlePeopleMoving(byte* payload, int length) {
  // 解析JSON消息
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray jsonArray = doc.as<JsonArray>();
  Serial.print("Received array: ");
  int i = 0;
  for (JsonVariant v : jsonArray) {
    Serial.print(v.as<int>());
    Serial.print(" ");
    movingArray[i] = v.as<int>();
    i++;
    if(i >31) break;
  }
  Serial.println();
  for(int isa: movingArray){
    Serial.print(isa);
    Serial.print(" ");
  }
  Serial.println();

}
