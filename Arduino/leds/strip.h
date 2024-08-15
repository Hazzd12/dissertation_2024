#ifndef __STRIP_H
#define __STRIP_H

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 15
#define MATRIX_F_HEIGHT 19
#define LED_PIN 13

#include <Adafruit_NeoPixel.h>
extern Adafruit_NeoPixel strip;
extern int brightness;
extern int matrixArray[32][19];



void setupStrip();
void drawLine(int line, uint32_t color1, int height = MATRIX_HEIGHT);
void drawCharacter(char str, uint32_t color, int offsetX, int offsetY, int index);
void drawString(char str[],  uint32_t color, int offsetX, int offsetY);
void handleMoving(int movingArray[], int oldMovingHeight[]);
void drawMoving(int movingArray[], int oldMovingHeight[]);
void drawWeather(int weatherId, unsigned long nowT);
void drawRSun(int status, int offsetY);
void drawHorizontalLine(int y);

#endif