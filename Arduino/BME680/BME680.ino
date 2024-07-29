#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// Create an instance of the sensor
Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println(F("BME680 test"));

  // Initialize the sensor
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}
void loop() {
  if (!bme.performReading()) {
    Serial.println(F("Failed to perform reading :("));
    return;
  }
  
  float temperature = bme.temperature;
  float humidity = bme.humidity;
  float pressure = bme.pressure / 100.0;
  float gas = bme.gas_resistance / 1000.0;
  float altitude = bme.readAltitude(1013.25);
  
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

  Serial.print(F("Approx. Altitude = "));
  Serial.print(altitude);
  Serial.println(F(" m"));

  // Calculate Air Quality Index (simple estimation based on gas resistance)
  float IAQ = calculateIAQ(temperature, humidity, gas);
  Serial.print(F("Estimated IAQ = "));
  Serial.println(IAQ);

  // Calculate Heat Index
  float heatIndex = calculateHeatIndex(temperature, humidity);
  Serial.print(F("Heat Index = "));
  Serial.print(heatIndex);
  Serial.println(F(" *C"));

  // Calculate Dew Point
  float dewPoint = calculateDewPoint(temperature, humidity);
  Serial.print(F("Dew Point = "));
  Serial.print(dewPoint);
  Serial.println(F(" *C"));

  Serial.println();
  delay(2000);
}

float calculateIAQ(float temperature, float humidity, float gas) {
  // Simplified IAQ calculation for demonstration purposes
  // Temperature and humidity can affect gas sensor readings
  float gas_baseline = 500.0; // Baseline value for gas resistance in KOhms

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

float calculateDewPoint(float temperature, float humidity) {
  // Calculate dew point using the formula from NOAA
  float A = 17.27;
  float B = 237.7;
  float alpha = ((A * temperature) / (B + temperature)) + log(humidity / 100.0);
  float dewPoint = (B * alpha) / (A - alpha);
  return dewPoint;
}