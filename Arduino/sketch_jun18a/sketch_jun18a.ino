#include <cppQueue.h>
#include <ArduinoJson.h>
#include "common.h"
#include "strip.h"
#include "myWifi.h"
#include "MQTTClient.h"

int movingArray[32] = { 0 };
int oldMovingHeight[32] = {0};

cppQueue myQueue(sizeof(Mes), 10, FIFO, true);
unsigned long previousMillis = 0;
const long interval = 2000;  // 2 seconds


Mes tab[4] = {
  { "X", 16 },
  { "T", 24 },
  { "Z", 32 },
  { "A", 40 }
};

void setup() {
  // 初始化灯带
  Serial.begin(9600);
  setupStrip();

  // initMatrix();
  strip.begin();
  strip.show();  // 初始化所有像素为'关'

  WiFiAndMQTTConnection(callback);
  // Setup MQTT subscription
  //mqtt.subscribe(&subscribeFeed);
}

//uint32_t colors[3] = { strip.Color(0, 255, 0), strip.Color(255, 0, 0), strip.Color(0, 0, 255) };
void loop() {

  MQTTloop();

  //matrix.fillScreen(0);
  // 测试代码：循环显示红、绿、蓝三色
  // unsigned long currentMillis = millis();
  // if (currentMillis - previousMillis >= 1000) {
  //   previousMillis = currentMillis;
  //   strip.clear();
  //   testQueue();
  // }
  //Serial.print(val);
  // for (char s = 'X'; s <= 'Y'; s++) {
  //   strip.clear();
  //   drawCharacter(s, strip.Color(255, 0, 0), 0, 4);
  //   //drawLine(7, strip.Color(255, 0, 0));
  //   strip.show();
  //   delay(1000);
  // }


  delay(500);
  int color = 0;
  for (int i = 0; i < 9; i++) {
    uint32_t colors1 = hsv2rgb(color, 100, 100);
    color += 360 / 9;
    colorWipe(colors1, 1000);
  }
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

  if (strcmp(topic, "student/ucfnuax/test") == 0) {
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
    if (i > 31) break;
  }
  Serial.println();
  for (int isa : movingArray) {
    Serial.print(isa);
    Serial.print(" ");
  }
  Serial.println();
}

void testQueue() {
  if (myQueue.isEmpty()) {
    Serial.println("Queue is  empty initially.");
    for (int i = 0; i < 4; i++) {
      Mes* temp = new Mes();
      strcpy(temp->message, tab[i].message);
      temp->yLocation = tab[i].yLocation;  // 假设有初始的 yLocation
      myQueue.push(temp);
    }
  }

  delay(10);


  for (int i = 0; i < 4; i++) {

    Mes temp;
    myQueue.pop(&temp);
    Serial.print("Current message: ");
    Serial.println(temp.message);

    // Uncomment the next line if you need to use the drawString function
    // drawString(temp.message, strip.Color(255, 0, 0), 0, temp.yLocation);
    drawString(temp.message, strip.Color(255, 0, 0), 0, temp.yLocation);
    Mes temp2;
    temp2.yLocation = --temp.yLocation;
    strcpy(temp2.message, temp.message);
    if (temp2.yLocation > -7) {
      myQueue.push(&temp2);
    }
  }
  strip.show();
}



// 函数：按顺序点亮每个LED，并设置颜色
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < 32; i++) {
    drawLine(i, color);
  }
  delay(wait);
  strip.show();
}


uint32_t hsv2rgb(uint16_t h, uint8_t s, uint8_t v) {
  uint8_t r, g, b;
  int i;
  float f, p, q, t;
  h = max((uint16_t)0, min((uint16_t)360, h));
  s = max((uint8_t)0, min((uint8_t)100, s));
  v = max((uint8_t)0, min((uint8_t)100, v));

  s /= 100.0;
  v /= 100.0;

  if (s == 0) {
    // Achromatic (grey)
    r = g = b = round(v * 255.0);
    return 0;
  }

  h /= 60.0;  // sector 0 to 5
  i = floor(h);
  f = h - i;  // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));

  switch (i) {
    case 0:
      r = round(v * 255);
      g = round(t * 255);
      b = round(p * 255);
      break;
    case 1:
      r = round(q * 255);
      g = round(v * 255);
      b = round(p * 255);
      break;
    case 2:
      r = round(p * 255);
      g = round(v * 255);
      b = round(t * 255);
      break;
    case 3:
      r = round(p * 255);
      g = round(q * 255);
      b = round(v * 255);
      break;
    case 4:
      r = round(t * 255);
      g = round(p * 255);
      b = round(v * 255);
      break;
    case 5:
      r = round(v * 255);
      g = round(p * 255);
      b = round(q * 255);
      break;
  }
  return strip.Color(r, g, b);
}
