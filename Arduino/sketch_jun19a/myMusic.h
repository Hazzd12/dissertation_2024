
#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>
#define NUMPIXELS 60 // LED 数量

extern int rhythmBandsModel;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

unsigned int sampling_period_us; //采样周期
byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


unsigned int samplingPeriod;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];

unsigned long starTime;
unsigned long peekDecayTime;
unsigned long changeColorTime;

void drawRHYTHM() {

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }

  // 重置频率块的值
  for (int i = 0; i < NUM_BANDS; i++) {
    bandValues[i] = 0;
  }

  // 采样SAMPLES次
  for (int i = 0; i < SAMPLES; i++) {
    starTime = micros();
    vReal[i] = analogRead(AUDIO_PIN);
    vImag[i] = 0;
    // Serial.println(micros() - starTime);
    while ((micros() - starTime) < sampling_period_us) { /* chill */
    }
    // Serial.println(vReal[i]);
  }
  // 进行FFT计算
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();
  // 解析计算结果
  for (int i = 2; i < (SAMPLES / 2); i++) {
    if (vReal[i] > NOISE) {
      // Serial.println(vReal[i]);
      // 去除前面6段低频杂音和一些高频尖叫
      if (i > 6 && i <= 9) bandValues[15] += (int)vReal[i];
      if (i > 9 && i <= 11) bandValues[16] += (int)vReal[i];
      if (i > 11 && i <= 13) bandValues[14] += (int)vReal[i];
      if (i > 13 && i <= 15) bandValues[17] += (int)vReal[i];
      if (i > 15 && i <= 17) bandValues[13] += (int)vReal[i];
      if (i > 17 && i <= 19) bandValues[18] += (int)vReal[i];
      if (i > 19 && i <= 21) bandValues[12] += (int)vReal[i];
      if (i > 21 && i <= 23) bandValues[19] += (int)vReal[i];
      if (i > 23 && i <= 25) bandValues[11] += (int)vReal[i];
      if (i > 25 && i <= 27) bandValues[20] += (int)vReal[i];
      if (i > 27 && i <= 29) bandValues[10] += (int)vReal[i];
      if (i > 29 && i <= 31) bandValues[21] += (int)vReal[i];
      if (i > 31 && i <= 33) bandValues[9] += (int)vReal[i];
      if (i > 33 && i <= 35) bandValues[22] += (int)vReal[i];
      if (i > 35 && i <= 38) bandValues[8] += (int)vReal[i];
      if (i > 38 && i <= 41) bandValues[23] += (int)vReal[i];
      if (i > 41 && i <= 44) bandValues[7] += (int)vReal[i];
      if (i > 44 && i <= 47) bandValues[24] += (int)vReal[i];
      if (i > 47 && i <= 50) bandValues[6] += (int)vReal[i];
      if (i > 50 && i <= 53) bandValues[25] += (int)vReal[i];
      if (i > 53 && i <= 56) bandValues[5] += (int)vReal[i];
      if (i > 56 && i <= 59) bandValues[26] += (int)vReal[i];
      if (i > 59 && i <= 62) bandValues[4] += (int)vReal[i];
      if (i > 62 && i <= 65) bandValues[27] += (int)vReal[i];
      if (i > 65 && i <= 68) bandValues[3] += (int)vReal[i];
      if (i > 68 && i <= 71) bandValues[28] += (int)vReal[i];
      if (i > 71 && i <= 74) bandValues[2] += (int)vReal[i];
      if (i > 74 && i <= 77) bandValues[29] += (int)vReal[i];
      if (i > 77 && i <= 80) bandValues[1] += (int)vReal[i];
      if (i > 80 && i <= 83) bandValues[30] += (int)vReal[i];
      if (i > 83 && i <= 87) bandValues[0] += (int)vReal[i];
      if (i > 87 && i <= 91) bandValues[31] += (int)vReal[i];
    }
  }

  // 将FFT数据处理为条形高度
  int color = 0;
  int r, g, b;
  for (byte band = 0; band < NUM_BANDS; band++) {
    // 根据倍率缩放条形图高度
    int barHeight = bandValues[band] / AMPLITUDE;
    if (barHeight > NUMPIXELS) barHeight = NUMPIXELS;  //可修改
    // 旧的高度值和新的高度值平均一下
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;
    // 如果条形的高度大于顶点高度，则调整顶点高度
    if (barHeight > peak[band]) {
      peak[band] = min(1, barHeight);
    }
    // 绘制操作
    pixels.setPixelColor(band, pixels.Color(255,0,255)); 

    //matrix.drawFastVLine((MATRIX_WIDTH - 1 - band), (MATRIX_SIDE - barHeight), barHeight, hsv2rgb(color, 100, 100));


    // switch (rhythmPage){
    //   case RHYTHM_MODEL1:
    //     // 绘制条形
    //     matrix.drawFastVLine((MATRIX_WIDTH - 1 - band), (MATRIX_SIDE - barHeight), barHeight, hsv2rgb(color,100,100));
    //     color += 360 / (NUM_BANDS + 4);
    //     // 绘制顶点
    //     matrix.drawPixel((MATRIX_WIDTH - 1 - band), MATRIX_SIDE - peak[band] - 1, matrix.Color(150,150,150));
    //     break;
    //   case RHYTHM_MODEL2:
    //     // 绘制条形
    //     r = model2ColorArray[0][0];
    //     g = model2ColorArray[0][1];
    //     b = model2ColorArray[0][2];
    //     for (int y = MATRIX_SIDE; y >= MATRIX_SIDE - barHeight; y--) {
    //       matrix.drawPixel((MATRIX_WIDTH - 1 - band), y, matrix.Color(r, g, b));
    //       r+=(model2ColorArray[1][0] - model2ColorArray[0][0]) / barHeight;
    //       g+=(model2ColorArray[1][1] - model2ColorArray[0][1]) / barHeight;
    //       b+=(model2ColorArray[1][2] - model2ColorArray[0][2]) / barHeight;
    //     }
    //     // 绘制顶点
    //     matrix.drawPixel((MATRIX_WIDTH - 1 - band), MATRIX_SIDE - peak[band] - 1, matrix.Color(150,150,150));
    //     break;
    //   case RHYTHM_MODEL3:
    //     // 绘制条形,此模式不绘制顶点
    //     for (int y = MATRIX_SIDE; y >= MATRIX_SIDE - barHeight; y--) {
    //       matrix.drawPixel((MATRIX_WIDTH - 1 - band), y, hsv2rgb(y * (255 / MATRIX_WIDTH / 5) + colorTime, 100, 100));
    //     }
    //     break;
    //   case RHYTHM_MODEL4:
    //     // 此模式下，只绘制顶点
    //     matrix.drawPixel((MATRIX_WIDTH - 1 - band), (MATRIX_SIDE - peak[band] - 1),
    //     matrix.Color(model4ColorArrar[peak[band]][0], model4ColorArrar[peak[band]][1], model4ColorArrar[peak[band]][2]));
    //     break;
    //   default:
    //     break;
    // }
    // 将值记录到oldBarHeights
    oldBarHeights[band] = barHeight;
  }
  // 70毫秒降低一次顶点
  if ((millis() - peekDecayTime) >= 70) {
    for (byte band = 0; band < NUM_BANDS; band++) {
      if (peak[band] > 0) peak[band] -= 1;
    }
    colorTime++;
    peekDecayTime = millis();
  }
  // 10毫秒变换一次颜色
  if ((millis() - changeColorTime) >= 10) {
    colorTime++;
    changeColorTime = millis();
  }
  pixel.show();
}

// HSV转RGB格式
uint16_t hsv2rgb(uint16_t hue, uint8_t saturation, uint8_t value) {
  uint8_t r, g, b;
  uint16_t h = (hue / 60) % 6;
  uint16_t F = 100 * hue / 60 - 100 * h;
  uint16_t P = value * (100 - saturation) / 100;
  uint16_t Q = value * (10000 - F * saturation) / 10000;
  uint16_t T = value * (10000 - saturation * (100 - F)) / 10000;
  switch (h) {
    case 0:
      r = value;
      g = T;
      b = P;
      break;
    case 1:
      r = Q;
      g = value;
      b = P;
      break;
    case 2:
      r = P;
      g = value;
      b = T;
      break;
    case 3:
      r = P;
      g = Q;
      b = value;
      break;
    case 4:
      r = T;
      g = P;
      b = value;
      break;
    case 5:
      r = value;
      g = P;
      b = Q;
      break;
    default:
      return pixel.Color(255, 0, 0);
  }
  r = r * 255 / 100;
  g = g * 255 / 100;
  b = b * 255 / 100;
  return pixel.Color(r, g, b);
}
