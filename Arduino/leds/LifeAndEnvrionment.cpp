#include <ArduinoHttpClient.h>
#include "MQTTClient.h"
#include <ArduinoJson.h>
#include <TimeLib.h>

HttpClient httpclient = HttpClient(WiFiClientInstance, "api.openweathermap.org", 80);

enum Weather {
  THUNDERSTORM = 0,
  DRIZZLE = 1,
  RAIN = 2,
  CLEAR = 3,
  CLOUD = 4
};
Weather currentWea = CLEAR;

time_t sunrise = 0;
time_t sunset = 0;
int risM = 0;
int risH = 0;
int setM = 0;
int setH = 0;
static time_t currentTime = 0;

void setOwnTime(int statusCode, String response) {
  if (statusCode > 0) {
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    time_t c_time = doc["dt"];
    c_time += 3600;
    Serial.print("unixTime: ");
    Serial.println(c_time);
    setTime(c_time);

    time_t nowT = now();
    currentTime = nowT;
    Serial.print("Year: ");
    Serial.println(year(nowT));  // 输出年份

    Serial.print("Month: ");
    Serial.println(month(nowT));  // 输出月份

    Serial.print("Day: ");
    Serial.println(day(nowT));  // 输出日期

    Serial.print("Hour: ");
    Serial.println(hour(nowT));  // 输出小时

    Serial.print("Minute: ");
    Serial.println(minute(nowT));  // 输出分钟
  }
}

void sendGetRequest() {
  Serial.println("Sending GET request...");

  httpclient.get("/data/2.5/weather?lat=51.53&lon=-0.01&appid=821264f83782113ce2caacdf89abbe54");  // 替换为你的RESTful API路径

  int statusCode = httpclient.responseStatusCode();
  String response = httpclient.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  if (statusCode > 0) {
    // 创建一个JSON文档对象
    setOwnTime(statusCode, response);
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      httpclient.stop();
      return;
    }

    // 获取JSON对象的值
    JsonArray sysArray = doc["sys"];
    int value1 = doc["dt"];        // 替换为你的JSON键
    int value2 = doc["timezone"];  // 替换为你的JSON键
    JsonArray weaArray = doc["weather"];

    if (!sysArray.isNull()) {
      JsonObject sys = sysArray[0];
      sunrise = sys["sunrise"];
      sunset = sys["sunset"];
      sunrise += 3600;
      sunset += 3600;
      Serial.println("sunrise");
      Serial.print("Year: ");
      Serial.println(year(sunrise));  // 输出年份

      Serial.print("Month: ");
      Serial.println(month(sunrise));  // 输出月份

      Serial.print("Day: ");
      Serial.println(day(sunrise));  // 输出日期

      Serial.print("Hour: ");
      Serial.println(hour(sunrise));  // 输出小时
      risH = hour(sunrise);
      Serial.print("Minute: ");
      Serial.println(minute(sunrise));  // 输出分钟
      risM = hour(sunrise);
      /////////////////////////
      Serial.println("sunset");
      Serial.print("Year: ");
      Serial.println(year(sunset));  // 输出年份

      Serial.print("Month: ");
      Serial.println(month(sunset));  // 输出月份

      Serial.print("Day: ");
      Serial.println(day(sunset));  // 输出日期

      Serial.print("Hour: ");
      Serial.println(hour(sunset));  // 输出小时
      setH = hour(sunset);

      Serial.print("Minute: ");
      Serial.println(minute(sunset));  // 输出分钟
      setM = minute(sunset);
    }
    // 打印JSON值


    if (weaArray.isNull()) {
      JsonObject weather = weaArray[0];
      String main = weather["main"];

      if (main == "Thunderstorm") {
        currentWea = THUNDERSTORM;
      } else if (main == "Drizzle") {
        currentWea = DRIZZLE;
      } else if (main == "Rain") {
        currentWea = RAIN;
      } else if (main == "Clear") {
        currentWea = CLEAR;
      }
    }
  }
  httpclient.stop();
}
