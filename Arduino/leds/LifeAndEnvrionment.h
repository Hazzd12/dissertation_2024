#ifndef __LIFEANDENVRIONMENT_H
#define __LIFEANDENVRIONMENT_H

#include <ArduinoHttpClient.h>
#include "MQTTClient.h"
#include <ArduinoJson.h>
#include <TimeLib.h>
#include "common.h"
extern HttpClient httpclient;

enum Weather {
  THUNDERSTORM = 0,
  DRIZZLE = 1,
  RAIN = 2,
  CLEAR = 3,
  CLOUD = 4
};

extern HttpClient httpTimeclient;
static time_t currentTime = 0;

extern time_t sunrise;
extern time_t sunset;
extern Weather currentWea;
void sendGetRequest();


#endif