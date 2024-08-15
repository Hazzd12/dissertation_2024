#ifndef __MYMUSIC_H
#define __MYMUSIC_H

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <arduinoFFT.h>
#define NUMPIXELS 64         // LED 数量
#define LED_PIN 13           // LED 数据引脚
#define AUDIO_PIN A5         // 拾音器
#define SAMPLES 256          //采样个数
#define SAMPLING_FREQ 10000  //采样频率
#define AMPLITUDE 1000       //声音强度调整倍率（柱状高度倍率）
#define NUM_BANDS 32          //频段个数
#define NOISE 100              //噪音
#define BAR_WIDTH 1          //每个频段的宽度
#define MATRIX_SIDE 15
#define MATRIX_WIDTH 32

extern Adafruit_NeoMatrix matrix;

void ledSetup();
uint16_t hsv2rgb(uint16_t hue, uint8_t saturation, uint8_t value);
void drawRHYTHM();



#endif