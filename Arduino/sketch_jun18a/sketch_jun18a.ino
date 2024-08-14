#include <cppQueue.h>
#include <ArduinoJson.h>
#include "strip.h"
#include "myWifi.h"
#include "MQTTClient.h"
#include "myMusic.h"
#include "LifeAndEnvrionment.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "common.h"
#include "DFRobot_DF2301Q.h"

#define Led 9

DFRobot_DF2301Q_UART* asr;


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;  // I2C

enum State {
  TEST_MODE,
  MOVING_MODE,
  MUSIC_MODE,
  ENVIRONMENT_MODE,
  LIFE_MODE,
  WAKE_MODE
};

State currentState = MOVING_MODE;
State lastState = MOVING_MODE;
int movingArray[17] = { 0 };
int oldMovingHeight[32] = { 0 };

cppQueue myQueue(sizeof(Mes*), 11, FIFO, true);
unsigned long previousMillis = 0;
const long interval = 2000;  // 2 seconds
String segments[10];
int spaces[10];

float temperature = 0;
float humidity = 0;
float pressure = 0;
float gas = 0;
float IAQ = 0;
float heatIndex = 0;

unsigned long lastStringTime = 0;
unsigned long VoiceTime = 0;
unsigned long EnvTime = 0;
int envIndex = 0;
int envDuration = 5000;

int lastTimeCheck = 0;
int lastUpdateTime = 0;
bool ifRiseSet = false;
bool ifWake = false;
bool ifCancelWake = false;

unsigned long lastWake = 0;
void setup() {
  // 初始化灯带
  Serial.begin(9600);
  Serial.println("start");
  delay(1000);

  //BME setup
  Serial.println(F("BME680 test"));

  // Initialize the sensor
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320*C for 150 ms

  delay(2000);

  //strip setup
  setupStrip();

  //matrix setup
  ledSetup();
  // initMatrix();
  strip.begin();

  char st1[] = "AUDIO";
  drawString(st1, strip.Color(255, 255, 255), 1, 7);
  strip.show();  // 初始化所有像素为'关'

  asr = new DFRobot_DF2301Q_UART(&Serial1);

  // Init the sensor
  while (!(asr->begin())) {
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
  }
  Serial.println("Begin ok!");

  asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_MUTE, 1);
  asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_VOLUME, 7);
  asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_WAKE_TIME, 10);
  //asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_ENTERWAKEUP, 0);

  asr->playByCMDID(23);
  Serial.println("IN");
  strip.clear();

  char st2[] = "SETUP";
  drawString(st2, strip.Color(255, 255, 255), 1, 7);
  strip.show();  // 初始化所有像素为'关'
  WiFiAndMQTTConnection(callback);


  httpclient.setTimeout(5000);
  // Setup MQTT subscription
  sendGetRequest();
  strip.clear();


  randomSeed(analogRead(2));
}



void loop() {
  MQTTloop();

  unsigned long nowT = millis();
  if (nowT - VoiceTime >= 300) {
    VoiceTime = nowT;
    uint8_t CMDID = asr->getCMDID();

    if (CMDID != 0) {
      Serial.print("CMDID = ");  //打印命令ID
      Serial.println(CMDID);
    }

    switch (CMDID) {
      case 5:  //music

        currentState = MUSIC_MODE;
        break;

      case 6:  //moving
        currentState = MOVING_MODE;
        break;

      case 7:  //life - indoor
        currentState = LIFE_MODE;
        break;

      case 8:  //environment - outdoor
        currentState = ENVIRONMENT_MODE;
        break;

      case 117:
        currentState = TEST_MODE;
        break;
      case 1:
        lastState = currentState;
        currentState = WAKE_MODE;
        ifWake = true;
        lastWake = nowT;
        break;
      default:
        break;
    }
  }

  int queueCount = myQueue.getCount();
  if (queueCount > 0) {
    if (nowT - lastStringTime > 700) {
      drawStrings();
      lastStringTime = nowT;
    }
  } else {
    switch (currentState) {
      case WAKE_MODE:
        if (ifWake) {
          ifWake = false;
          drawHorizontalLine(7);
        }

        if (nowT - lastWake > 5000 ) {
          currentState = lastState;
          strip.clear();
          strip.show();
          Serial.println("12356");
        }
        break;
      case MUSIC_MODE:
        // 在这里持续执行功能A

        drawRHYTHM();
        delay(5);
        break;

      case MOVING_MODE:
        // 在这里持续执行功能B
        drawMoving(movingArray, oldMovingHeight);
        delay(5);
        break;

      case LIFE_MODE:
        Lifeloop(nowT);
        break;

      case ENVIRONMENT_MODE:

        if (nowT - EnvTime >= envDuration) {
          Serial.println("Executing ENVIRONMENT_MODE...");
          Envrionmentloop();
          drawEnvironment();
          envIndex = (++envIndex) % 10;
          Serial.print("envIndex");
          Serial.println(envIndex);
          EnvTime = nowT;
        }
        break;

      case TEST_MODE:
        test_loop();

        break;

      default:
        break;
    }
  }
}

void test_loop() {
  strip.clear();
  drawRSun(0, -15);
  strip.show();
  delay(5000);
  for (int j = -14; j <= 0; j++) {
    strip.clear();
    drawRSun((int)(3 - j / (-5)), j);
    delay(1000);
    strip.show();
  }
  for (int x = 0; x < 5; x++) {
    for (int i = 0; i < 10; i++) {
      drawWeather(x, millis());
      delay(1000);
    }
  }
  currentState = MUSIC_MODE;
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

  if (strcmp(topic, "student/ucfnuax/moving") == 0) {
    handlePeopleMoving(payload, length);
  }
  if (strcmp(topic, "student/ucfnuax/Gesture") == 0) {
    String message;
    message = handleGesture((char)payload[0]);
    handleString(message);
  }

  if (strcmp(topic, "student/ucfnuax/Message") == 0) {
    String message;
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    handleString(message);
    Serial.println(myQueue.getCount());
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
    if (i > 16) break;
  }
  Serial.println();
  for (int isa : movingArray) {
    Serial.print(isa);
    Serial.print(" ");
  }
  Serial.println();
  handleMoving(movingArray, oldMovingHeight);
}

String handleGesture(char message) {
  if (message == 'u')
    return "GOOD MORNING";
  else if (message == 'd')
    return "GOOD AFTERNOON";
  else if (message == 'l')
    return "HELLO";
  else if (message == 'r')
    return "GOOD EVENING";
  else
    return "";
}

void handleString(String message) {

  int count = splitString(message);

  if (count + myQueue.getCount() > 10) {
    client.publish("student/ucfnuax/GesStatus", "0");
  } else {
    client.publish("student/ucfnuax/GesStatus", "1");
    for (int i = 0; i < count; i++) {
      String met = segments[i];


      Mes* temp = new Mes();
      const char* c_mes = met.c_str();
      strcpy(temp->message, c_mes);

      int queueCount = myQueue.getCount();

      temp->yLocation = queueCount * 8 + 19 - spaces[i];
      myQueue.push(&temp);
    }
  }
}

void drawStrings() {
  strip.clear();
  int count = myQueue.getCount();
  for (int i = 0; i < count; i++) {

    Mes* temp;
    myQueue.pop(&temp);

    drawString(temp->message, strip.Color(255, 255, 255), 1, temp->yLocation);
    Mes* temp2 = new Mes();
    temp2->yLocation = temp->yLocation - 1;
    strcpy(temp2->message, temp->message);

    if (temp2->yLocation > -7) {
      myQueue.push(&temp2);
    } else {
      delete temp2;
    }
    delete temp;
  }

  strip.show();
}


//test function, test which led may be broken or disconnected
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < 32; i++) {
    drawLine(i, color);
  }
  delay(wait);
  strip.show();
}


int splitString(String str) {
  String segment = "";
  int count = 0;
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c == ' ') {
      if (segment.length() > 0) {
        spaces[count] = 1;
        segments[count++] = segment;
        segment = "";
      }
    } else {
      segment += c;
      if (segment.length() == 5) {
        spaces[count] = 0;
        segments[count++] = segment;
        segment = "";
      }
    }
  }
  if (segment.length() > 0) {
    spaces[count] = 0;
    segments[count++] = segment;
  }
  return count;
}

void drawEnvironment() {
  String info;
  strip.clear();
  switch (envIndex) {
    case 0:
      info = "TEMPE";
      envDuration = 2000;
      break;
    case 1:
      info = "T:" + String(temperature, 1);
      envDuration = 30000;
      break;
    case 2:
      info = "HUMID";
      envDuration = 2000;
      break;
    case 3:
      info = "H:" + String(humidity, 1);
      envDuration = 3000;
      break;
    case 4:
      info = "PRESS";
      envDuration = 2000;
      break;
    case 5:
      info = "P:" + String(pressure, 2);
      envDuration = 3000;
      break;
    case 6:
      info = "IAQ:" + String((int)IAQ);
      envDuration = 5000;
      break;
    case 7:
      info = "HEAT";
      envDuration = 1000;
      break;
    case 8:
      info = "INDEX";
      envDuration = 1000;
      break;
    case 9:
      info = "HI:" + String(heatIndex, 1);
      envDuration = 1000;
      break;
    default:
      break;
  }
  const char* charArray = info.c_str();
  char* mes = const_cast<char*>(charArray);
  drawString(mes, strip.Color(255, 255, 255), 0, 7);
  strip.show();
}


void Lifeloop(unsigned long nowT) {

  if (nowT - lastUpdateTime > 30 * 60 * 1000) {
    sendGetRequest();
    lastUpdateTime = nowT;
  }


  if (nowT - lastTimeCheck > 2 * 60 * 1000) {
    time_t currentTime = now();
    int minutesLeft = (currentTime - sunrise) / 60;  // 计算到日出的分钟数

    if (minutesLeft < 30 && minutesLeft >= -5) {  // 如果距离日出30分钟或更少
      int index = (int)(minutesLeft / 10) + 1;
      if (index < 0) index = 0;
      int y = (int)(minutesLeft / 2) - 15;
      if (y > 0) y = 0;
      drawRSun(index, y);  // 调用特定的函数
      ifRiseSet = true;
    }

    minutesLeft = (sunset - currentTime) / 60;  // 计算到日出的分钟数

    Serial.print("Minutes to sunset: ");
    Serial.println(minutesLeft);

    if (minutesLeft < 30 && minutesLeft >= -5) {  // 如果距离日出30分钟或更少
      int index = (int)(minutesLeft / 10) + 1;
      if (index < 0) index = 0;
      int y = (int)(minutesLeft / 2) - 15;
      if (y > 0) y = 0;
      drawRSun(index, y);  // 调用特定的函数
      ifRiseSet = true;
    }
    lastTimeCheck = nowT;
  }

  if (ifRiseSet && nowT - lastTimeCheck > 60 * 1000) {
    ifRiseSet = false;
  }

  if (!ifRiseSet) {
    drawWeather(currentWea, nowT);
  }
}

void Envrionmentloop() {

  if (!bme.performReading()) {
    Serial.println(F("Failed to perform reading :("));
    delay(1000);
    return;
  }

  temperature = bme.temperature;
  humidity = bme.humidity;
  pressure = bme.pressure / 100.0;
  gas = bme.gas_resistance / 1000.0;



  Serial.print(F("Temperature = "));
  Serial.print(temperature);
  Serial.println(F(" *C"));

  Serial.print(F("Pressure = "));
  Serial.print(pressure);
  Serial.println(F(" hPa"));

  Serial.print(F("Humidity = "));
  Serial.print(humidity);
  Serial.println(F(" %"));

  Serial.print(F("Gas = "));
  Serial.print(gas);
  Serial.println(F(" KOhms"));


  // Calculate Air Quality Index (simple estimation based on gas resistance)
  IAQ = calculateIAQ(temperature, humidity, gas);
  Serial.print(F("Estimated IAQ = "));
  Serial.println(IAQ);

  // Calculate Heat Index
  heatIndex = calculateHeatIndex(temperature, humidity);
  Serial.print(F("Heat Index = "));
  Serial.print(heatIndex);
  Serial.println(F(" *C"));


  Serial.println();
}


float calculateIAQ(float temperature, float humidity, float gas) {
  // Simplified IAQ calculation for demonstration purposes
  // Temperature and humidity can affect gas sensor readings
  float gas_baseline = 500.0;  // Baseline value for gas resistance in KOhms

  // Calculate a humidity score
  float humidity_score;
  if (humidity >= 30 && humidity <= 60) {
    humidity_score = 0.25 * 100;
  } else {
    if (humidity < 30) {
      humidity_score = 0.25 * (humidity / 30.0) * 100;
    } else {
      humidity_score = 0.25 * ((100 - humidity) / 40.0) * 100;
    }
  }

  // Calculate a gas score
  float gas_score;
  if (gas > gas_baseline) {
    gas_score = 0.75 * (gas_baseline / gas) * 100;
  } else {
    gas_score = 0.75 * 100;
  }

  // Calculate air quality index (IAQ)
  float IAQ = humidity_score + gas_score;
  return IAQ;
}

float calculateHeatIndex(float temperature, float humidity) {
  // Calculate heat index using the formula from NOAA
  float T = temperature;
  float R = humidity;
  float HI = 0.5 * (T + 61.0 + ((T - 68.0) * 1.2) + (R * 0.094));

  if (HI >= 80) {
    HI = -42.379 + 2.04901523 * T + 10.14333127 * R
         - 0.22475541 * T * R - 6.83783e-3 * T * T
         - 5.481717e-2 * R * R + 1.22874e-3 * T * T * R
         + 8.5282e-4 * T * R * R - 1.99e-6 * T * T * R * R;
  }
  return HI;
}