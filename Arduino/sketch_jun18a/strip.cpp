
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 15
#define MATRIX_F_HEIGHT 19
#define LED_PIN 13

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "characters.h"
int brightness = 50;
int matrixArray[32][19];
unsigned long movingPeekTime = 0;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(MATRIX_WIDTH * MATRIX_HEIGHT, LED_PIN, NEO_GRB + NEO_KHZ800);
void handleSetup(int i, int type);
void setPixel(int x, int y, bool state, uint32_t color = strip.Color(255, 0, 0));
void setPixel(int x, int y, uint32_t color);
int getLength(char str[]);

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


void drawCharacter(char str, uint32_t color, int offsetX, int offsetY) {
  int index;
  if (str >= 'A' && str <= 'Z') {
    index = str - 'A';
  } else if (str >= 'a' && str <= 'z') {
    index = str - 'a' + 26;
  } else {
    return;  // 非法字符
  }
  for (int x = 0; x < 5; x++) {
    uint8_t column = font5x7[index][x];
    for (int y = 0; y < 7; y++) {
      bool state = column & (1 << y);
      setPixel(offsetX + x, offsetY + y, state);
    }
  }
}

void drawString(char str[], uint32_t color, int offsetX, int offsetY) {
  for (int i = 0; i < getLength(str); i++) {
    Serial.println(str[i]);
    Serial.print("X: ");
    Serial.println(offsetX + i * 6);
    Serial.print("Y: ");
    Serial.println(offsetY);
    drawCharacter(str[i], color, offsetX + i * 6, offsetY);
  }
}

void drawMoving(int movingArray[], int oldMovingHeight[]) {
  byte i = 0;
  uint32_t white = strip.Color(255, 255, 255);
  for (; i < 9; i++) {
    if (movingArray[i]) {
      oldMovingHeight[i * 2] = MATRIX_HEIGHT;
      if (i > 0 && movingArray[i - 1]) {
        oldMovingHeight[i * 2 - 1] = MATRIX_HEIGHT + 2;
      }
    }
  }

  for (; i < 17; i++) {
    if (movingArray[i]) {
      oldMovingHeight[i * 2 - 1] = MATRIX_HEIGHT;
      if (i > 9 && movingArray[i - 1]) {
        oldMovingHeight[(i - 1) * 2] = MATRIX_HEIGHT + 2;
      }
    }
  }

  unsigned long nowTime = millis();

  if (nowTime - movingPeekTime >= 50) {
    for (int band = 0; band < 32; band++) {
      drawLine(band, white, oldMovingHeight[band]);
      //handle the lines directly controlled by PIR
      if (band <= 16 && band % 2 == 0 || band > 16 && band % 2 == 1) {
        if (oldMovingHeight[band] > 0) oldMovingHeight[band] -= 1;
      }
      // the lines controlled by its neighbor
      else {
        // when its neighbor both are 0 or 1, it will go down like its neighbor
        if (band > 0 && band <= 16 && movingArray[(i - 1) / 2] == movingArray[(i - 1) / 2 + 1]
            || band > 16 && movingArray[i / 2] == movingArray[i / 2 + 1]) {
          if (oldMovingHeight[band] > 0) oldMovingHeight[band] = oldMovingHeight[band - 1];
        } else {
          // when they are not the same, it will go down slower than its lower neighbor
          if (oldMovingHeight[band] > 0) {
            if (oldMovingHeight[band + 1] != 0 || oldMovingHeight[band - 1] != 0) {
              oldMovingHeight[band] =
                oldMovingHeight[band + 1] < oldMovingHeight[band - 1] ? oldMovingHeight[band + 1] + 2 : oldMovingHeight[band - 1] + 2;
            } else {
              oldMovingHeight[band] -= 1;
            }
          }
        }
      }
    }
  }
  strip.show();
  movingPeekTime = nowTime;
}


void handleSetup(int i, int type) {
  int j = 0;
  for (; j < type; j++) {
    matrixArray[i][j] = -1;
  }
  if (i % 2 == 0) {
    for (; j < MATRIX_HEIGHT + type; j++) {
      matrixArray[i][j] = i * MATRIX_HEIGHT + j - type;
    }
  } else {
    for (; j < MATRIX_HEIGHT + type; j++) {
      matrixArray[i][j] = (i + 1) * MATRIX_HEIGHT - j + type - 1;
    }
  }
  for (; j < MATRIX_F_HEIGHT; j++) {
    matrixArray[i][j] = -1;
  }
}

void setPixel(int x, int y, bool state, uint32_t color) {
  if (x >= 0 && x < 32 && y >= 0 && y < 19 && matrixArray[x][y] != -1) {
    if (state) {
      Serial.println(matrixArray[x][y]);
      strip.setPixelColor(matrixArray[x][y], color);  // 设为红色
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