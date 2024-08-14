#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 15
#define MATRIX_F_HEIGHT 19
#define LED_PIN 13

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "common.h"

int brightness = 50;
int matrixArray[32][19];
unsigned long movingPeekTime = 0;
unsigned long movingDownTime = 0;
int newMovingHeight[32] = { 0 };

Adafruit_NeoPixel strip = Adafruit_NeoPixel(MATRIX_WIDTH * MATRIX_HEIGHT, LED_PIN, NEO_GRB + NEO_KHZ800);

void drawThunders();
void drawThunder(int offsetX, int offsetY, uint32_t color);
void drawRain(int intensity);
void drawClear(int offsetX, int offsetY, uint32_t color);
void drawCloud(int offsetX, int offsetY, uint32_t color);
void drawSmallCloud(int offsetX, int offsetY, uint32_t color);

void handleSetup(int i, int type);
void setPixel(int x, int y, bool state, uint32_t color = strip.Color(255, 0, 0));
void setPixel(int x, int y, uint32_t color);
int getLength(char str[]);

int lastClear = 0;
int lastCloudX = 0;
int lastSmallCloudX = 0;
int ThunderIntensity = 0;
unsigned long lastRainTime = 0;
unsigned long lastThunderTime = 0;
unsigned long lastClearTime = 0;
unsigned long lastCloudTime = 0;
unsigned long lastSmallCloudTime = 0;

uint32_t rains[32];

int randomThunderDelay = random(18, 28);

int getType(int line) {
  int temp = line % 8;
  int result = 0;
  switch (temp) {
    case 0:
    case 4:
      result = 2;
      break;
    case 1:
    case 3:
      result = 3;
      break;
    case 2:
      result = 4;
      break;
    case 5:
    case 7:
      result = 1;
      break;
    case 6:
      result = 0;
      break;
  }
  return result;
}


void setupStrip() {
  for (int i = 0; i < MATRIX_WIDTH; i++) {
    int temp = i % 8;
    int type = getType(i);
    handleSetup(i, type);
  }
  //strip.setBrightness(1);
}

//NEED TO TEST
void drawLine(int line, uint32_t color1, int height = MATRIX_HEIGHT) {
  int type = getType(line);
  for (int i = type; i < type + height; i++) {
    setPixel(line, i, color1);
  }
}


void drawCharacter(char str, uint32_t color, int offsetX, int offsetY, int index) {

  for (int x = 0; x < 5; x++) {
    uint8_t column = font5x7[index][x];
    for (int y = 0; y < 7; y++) {
      bool state = column & (1 << y);
      setPixel(offsetX + x, offsetY + y, state, color);
    }
  }
}

void drawNums(char str, uint32_t color, int offsetX, int offsetY, int index) {

  for (int x = 0; x < 3; x++) {
    uint8_t column = num3x7[index][x];
    for (int y = 0; y < 7; y++) {
      bool state = column & (1 << y);
      setPixel(offsetX + x, offsetY + y, state, color);
    }
  }
}

void drawSymbols(char str, uint32_t color, int offsetX, int offsetY) {
  int index;
  if (str == ':') {
    index = 1;
  } else if (str == '.') {
    index = 0;
  } else {
    return;
  }
  uint8_t column = symbols1x7[index];
  for (int y = 0; y < 7; y++) {
    bool state = column & (1 << y);
    setPixel(offsetX, offsetY + y, state, color);
  }
}

void drawString(char str[], uint32_t color, int offsetX, int offsetY) {
  int index;
  for (int i = 0; i < getLength(str); i++) {
    Serial.println(str[i]);
    if (str[i] >= 'A' && str[i] <= 'Z') {
      index = str[i] - 'A';
      drawCharacter(str[i], color, offsetX, offsetY, index);
      offsetX += 6;
    } else if (str[i] >= '0' && str[i] <= '9') {
      index = str[i] - '0';
      drawNums(str[i], color, offsetX, offsetY, index);
      offsetX += 4;
    } else if (str[i] == ':' || str[i] == '.') {
      Serial.println("IN2");
      Serial.println(str[i]);
      drawSymbols(str[i], color, offsetX, offsetY);
      offsetX += 3;
    }
  }
}

void optimizer(int movingArray[]) {
  if (movingArray[1] == 1 || movingArray[2] == 1) {
    movingArray[0] = 1;
    movingArray[1] = 1;
  }
  if (movingArray[15] == 1 || movingArray[14] == 1) {
    movingArray[16] = 1;
    movingArray[15] = 1;
  }
  int temp[17] = { 0 };
  for (int i = 2; i < 15; i++) {
    temp[i] = movingArray[i];
  }
  for (int i = 2; i < 15; i++) {
    if (temp[i] == 1) {
      movingArray[i - 1] = 1;
      movingArray[i - 2] = 1;
      movingArray[i + 1] = 1;
      movingArray[i + 2] = 1;
    }
  }
}
void handleMoving(int movingArray[], int oldMovingHeight[]) {
  byte i = 0;
  optimizer(movingArray);
  uint32_t white = strip.Color(255, 255, 255);
  for (; i < 9; i++) {
    if (movingArray[i]) {
      newMovingHeight[i * 2] = MATRIX_HEIGHT;
      if (i > 0 && movingArray[i - 1]) {
        newMovingHeight[i * 2 - 1] = MATRIX_HEIGHT;
      } else if (i > 0 && movingArray[i - 1] == 0) {
        newMovingHeight[i * 2 - 1] = MATRIX_HEIGHT / 2;
        newMovingHeight[i * 2 + 1] = MATRIX_HEIGHT / 2;
      } else {
        newMovingHeight[i * 2 + 1] = MATRIX_HEIGHT / 2;
      }
    }
  }
  for (; i < 17; i++) {
    if (movingArray[i]) {
      newMovingHeight[i * 2 - 1] = MATRIX_HEIGHT;
      if (i > 9 && movingArray[i - 1]) {
        newMovingHeight[(i - 1) * 2] = MATRIX_HEIGHT;
      }
    }
  }
}

void drawMoving(int movingArray[], int oldMovingHeight[]) {
  unsigned long nowTime = millis();

  if (nowTime - movingPeekTime >= 50) {
    bool ifUpdate = false;
    for (int i = 0; i < 32; i++) {
      if (newMovingHeight[i] > oldMovingHeight[i]) {
        ifUpdate = true;
        oldMovingHeight[i]++;
        drawLine(i, strip.Color(255, 255, 255), oldMovingHeight[i]);
      } else if (newMovingHeight[i] <= oldMovingHeight[i] && newMovingHeight != 0) {
        newMovingHeight[i] = 0;
      }
    }
    movingPeekTime = nowTime;
    if (ifUpdate) strip.show();
  }

  if (nowTime - movingDownTime >= 100) {
    strip.clear();
    for (int band = 0; band < 32; band++) {

      drawLine(band, strip.Color(255, 255, 255), oldMovingHeight[band]);
      //handle the lines directly controlled by PIR
      if (newMovingHeight[band] != 0) {
        Serial.print(band);
        Serial.print(", ");
        Serial.println(newMovingHeight[band]);
      }
      if (newMovingHeight[band] == 0) {
        if (band <= 16 && band % 2 == 0 || band > 16 && band % 2 == 1) {
          if (oldMovingHeight[band] > 0) oldMovingHeight[band] -= 1;
        }
        // the lines controlled by its neighbor
        else {
          // when its neighbor both are 0 or 1, it will go down like its neighbor
          if (band > 0 && band <= 16 && movingArray[(band - 1) / 2] == movingArray[(band - 1) / 2 + 1]
              || band > 16 && movingArray[band / 2] == movingArray[band / 2 + 1]) {
            if (band == 1) {
            }
            if (oldMovingHeight[band] > 0) oldMovingHeight[band]--;
          } else {
            // when they are not the same, it will go down slower than its lower neighbor
            if (oldMovingHeight[band] > 0) {
              if (oldMovingHeight[band + 1] != 0 || oldMovingHeight[band - 1] != 0) {
                oldMovingHeight[band] =
                  (oldMovingHeight[band + 1] + oldMovingHeight[band - 1]) / 2;
              } else {
                oldMovingHeight[band] -= 1;
              }
            }
          }
        }
      }
    }
    movingDownTime = nowTime;
    strip.show();
  }
}


void drawWeather(int weatherId, unsigned long nowT) {

  int thunderColor = 0;
  switch (weatherId) {
    case 0:
      if (nowT - lastThunderTime > 200) {
        strip.clear();
        drawThunders();
        lastThunderTime = nowT;
        strip.show();
      }
      break;
    case 1:
      if (nowT - lastRainTime > 500) {
        strip.clear();
        drawRain(1);
        lastRainTime = nowT;
        strip.show();
      }
      break;
    case 2:
      if (nowT - lastRainTime > 500) {
        strip.clear();
        drawRain(2);
        lastRainTime = nowT;
        strip.show();
      }
      break;
    case 3:
      if (nowT - lastClearTime > 2000) {
        strip.clear();
        Serial.println("CLear");

        drawClear(6, 1, strip.Color(255, 255, 0));  //yellow
        lastClearTime = nowT;
        strip.show();
      }
      break;
    case 4:
    default:
      if (nowT - lastCloudTime > 3000) {
        strip.clear();
        drawCloud(0, 4, strip.Color(125, 125, 125));
        (++lastCloudX) %= 32;
        lastCloudTime = nowT;
        Serial.println("cloud");
        Serial.println(lastCloudTime);
        strip.show();
      }
      if (nowT - lastSmallCloudTime > 1750) {
        strip.clear();

        drawCloud(0, 4, strip.Color(125, 125, 125));
        drawSmallCloud(0, 10, strip.Color(125, 125, 125));
        lastSmallCloudTime = nowT;
        strip.show();
      }
      break;
  }

}

void drawThunder(int offsetX, int offsetY, uint32_t color) {
  for (int x = 0; x < 5; x++) {
    uint8_t column = thunder[x];
    for (int y = 0; y < 7; y++) {
      bool state = column & (1 << y);
      setPixel(offsetX + x, offsetY + y, state, color);
      if(!state && y>4){
        setPixel(offsetX + x, offsetY + y, !state, strip.Color(128, 128, 128));
      }
    }
  }
}


void drawThunders() {
  int thunderColor = 0;

  //0-800ms brighter, 800ms- 1600ms darker, 1600 ms-200*randomThunderDelay ms no change
  if (ThunderIntensity <= 4) {
    63 * ThunderIntensity;
    ThunderIntensity++;
  } else if (ThunderIntensity <= 8) {
    thunderColor = (8 - ThunderIntensity) * 63;
    ThunderIntensity++;
  } else if (ThunderIntensity <= randomThunderDelay) {
    thunderColor = 0;
    ThunderIntensity++;
  } else {
    randomThunderDelay = random(18, 28);
    ThunderIntensity = 0;
  }

  if (thunderColor != 4) {
    drawCloud(4, 5, strip.Color(64, 64, 64));
  } else {
    drawCloud(4, 5, strip.Color(128, 128, 128));
  }

  if (thunderColor != 0) {
    drawThunder(5, 2, strip.Color(thunderColor, thunderColor, 0));
    drawThunder(12, 2, strip.Color(thunderColor, thunderColor, 0));
    drawThunder(19, 2, strip.Color(thunderColor, thunderColor, 0));
  }
  strip.show();
}

void drawRain(int intensity) {
  for (int i = 0; i < 32; i++) {
    rains[i] >>= 1;
    if (random(100) < 7.5 * intensity) {
      rains[i] |= 0x80000;
    }
  }

  for (int x = 0; x < 32; x++) {
    uint32_t column = rains[x];
    for (int y = 0; y < 20; y++) {
      bool state = column & (1 << y);
      setPixel(x, y, state, strip.Color(173, 216, 230));
    }
  }

  for (int i = 2; i < 32; i += 8) {
    uint32_t column = rains[i];
    bool state = column & (1 << 3);
    if (state) {
      setPixel(i - 1, 4, strip.Color(173, 216, 230));
      setPixel(i + 1, 4, strip.Color(173, 216, 230));
    }
    state = column & (1 << 2);
    if (state) {
      setPixel(i - 2, 5, strip.Color(173, 216, 230));
      setPixel(i + 2, 5, strip.Color(173, 216, 230));
    }
  }
  strip.show();
}


void drawClear(int offsetX, int offsetY, uint32_t color) {

  for (int x = 0; x < 19; x++) {
    uint32_t column = sun[lastClear][x];
    for (int y = 0; y < 17; y++) {
      bool state = column & (1 << y);
      setPixel(offsetX + x, offsetY + y, state, color);
    }
  }
  lastClear = (++lastClear) % 2;
}

void drawCloud(int offsetX, int offsetY, uint32_t color) {

  for (int x = 0; x < 22; x++) {
    uint16_t column = cloud[x];
    for (int y = 0; y < 11; y++) {
      bool state = column & (1 << y);
      int finalX = (offsetX + lastCloudX + x) % 32;
      setPixel(finalX, offsetY + y, state, color);
    }
  }
}

void drawSmallCloud(int offsetX, int offsetY, uint32_t color) {

  for (int x = 0; x < 5; x++) {
    uint8_t column = smallCloud[x];
    for (int y = 0; y < 4; y++) {
      bool state = column & (1 << y);
      int finalX = (offsetX + lastSmallCloudX + x) % 32;
      setPixel(finalX, offsetY + y, state, color);
    }
  }
  (++lastSmallCloudX) %= 32;
}


void drawRSun(int status, int offsetY) {
  strip.clear();
  uint32_t skyColor = 0;
  uint32_t sunColor = 0;

  switch (status) {
    case 0:
      skyColor = strip.Color(8, 24, 57);
      sunColor = strip.Color(0, 0, 0);
      break;
    case 1:
      skyColor = strip.Color(36, 49, 86);
      sunColor = strip.Color(255, 102, 0);
      break;
    case 2:
      skyColor = strip.Color(255, 179, 71);
      sunColor = strip.Color(255, 179, 0);
      break;
    case 3:
      skyColor = strip.Color(135, 206, 250);
      sunColor = strip.Color(255, 239, 213);
      break;
  }

  for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
    strip.setPixelColor(i, skyColor);
  }

  for (int x = 0; x < 32; x++) {
    uint16_t column = rSun[x];
    for (int y = 0; y < 15; y++) {
      bool state = column & (1 << y);
      setPixel(x, offsetY + y, state, sunColor);
      if(!state){
        setPixel(x, offsetY + y, !state, skyColor);
      }
    }
  }

  strip.show();
}
void drawHorizontalLine(int y){
  strip.clear();
  for(int i =0 ;i < 32; i++){
    strip.setPixelColor(matrixArray[i][y], strip.Color(125,125,125));
  }
  strip.show();
}
void handleSetup(int i, int type) {
  int j = 0;
  for (; j < type; j++) {
    matrixArray[i][j] = -1;
  }
  if (i % 2 == 0) {
    for (; j < MATRIX_HEIGHT + type; j++) {
      matrixArray[i][j] = (i + 1) * MATRIX_HEIGHT - j + type - 1;
    }
  } else {
    for (; j < MATRIX_HEIGHT + type; j++) {
      matrixArray[i][j] = i * MATRIX_HEIGHT + j - type;
    }
  }
  for (; j < MATRIX_F_HEIGHT; j++) {
    matrixArray[i][j] = -1;
  }
}

void setPixel(int x, int y, bool state, uint32_t color) {
  if (x >= 0 && x < 32 && y >= 0 && y < 19 && matrixArray[x][y] != -1) {
    if (state) {
      strip.setPixelColor(matrixArray[x][y], color);
    } else {
      strip.setPixelColor(matrixArray[x][y], strip.Color(0, 0, 0));  // 关闭像素
    }
  }
}

//override setPixel
void setPixel(int x, int y, uint32_t color) {
  if (x >= 0 && x < 32 && y >= 0 && y < 19 && matrixArray[x][y] != -1) {
    strip.setPixelColor(matrixArray[x][y], color);
  }
}


int getLength(char str[]) {
  int count = 0;
  for (int i = 0; i < 20 && str[i] != '\0'; i++) {
    count++;
  }
  return count;
}