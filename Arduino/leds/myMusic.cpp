
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



unsigned int sampling_period_us;  //采样周期
byte peak[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int oldBarHeights[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int bandValues[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

unsigned int samplingPeriod;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];

unsigned long starTime;
unsigned long peekDecayTime;
unsigned long changeColorTime;
int colorTime;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_SIDE, 1, 1, LED_PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800);

arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);
uint16_t hsv2rgb(uint16_t hue, uint8_t saturation, uint8_t value);

void ledSetup() {
  matrix.begin();  // 初始化LED灯条
  matrix.clear();
  matrix.setBrightness(64);
}

// 清空屏幕
void clearMatrix() {
  matrix.fillScreen(0);
  //matrix.show();
}



void modifyVReal(int bandValues[], int len, int i, int val) {
  int width = 85 / len;
  for (int j = 0; j < len; j++) {
    if (i > 6 + width * j && i <= 6 + width * (j + 1)) {
      bandValues[j] += val;
      break;
    }
  }
  if (i > 6 + width * len && i < 91) bandValues[len - 1] += val;
}


void drawRHYTHM() {

  matrix.clear();

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

      // if (i > 6 && i <= 9) bandValues[0] += (int)vReal[i];
      // if (i > 9 && i <= 11) bandValues[1] += (int)vReal[i];
      // if (i > 11 && i <= 13) bandValues[2] += (int)vReal[i];
      // if (i > 13 && i <= 15) bandValues[3] += (int)vReal[i];
      // if (i > 15 && i <= 17) bandValues[4] += (int)vReal[i];
      // if (i > 17 && i <= 19) bandValues[5] += (int)vReal[i];
      // if (i > 19 && i <= 21) bandValues[6] += (int)vReal[i];
      // if (i > 21 && i <= 23) bandValues[7] += (int)vReal[i];
      // if (i > 23 && i <= 25) bandValues[8] += (int)vReal[i];
      // if (i > 25 && i <= 27) bandValues[9] += (int)vReal[i];
      // if (i > 27 && i <= 29) bandValues[10] += (int)vReal[i];
      // if (i > 29 && i <= 31) bandValues[11] += (int)vReal[i];
      // if (i > 31 && i <= 33) bandValues[12] += (int)vReal[i];
      // if (i > 33 && i <= 35) bandValues[13] += (int)vReal[i];
      // if (i > 35 && i <= 38) bandValues[14] += (int)vReal[i];
      // if (i > 38 && i <= 41) bandValues[15] += (int)vReal[i];
      // if (i > 41 && i <= 44) bandValues[16] += (int)vReal[i];
      // if (i > 44 && i <= 47) bandValues[17] += (int)vReal[i];
      // if (i > 47 && i <= 50) bandValues[18] += (int)vReal[i];
      // if (i > 50 && i <= 53) bandValues[19] += (int)vReal[i];
      // if (i > 53 && i <= 56) bandValues[20] += (int)vReal[i];
      // if (i > 56 && i <= 59) bandValues[21] += (int)vReal[i];
      // if (i > 59 && i <= 62) bandValues[22] += (int)vReal[i];
      // if (i > 62 && i <= 65) bandValues[23] += (int)vReal[i];
      // if (i > 65 && i <= 68) bandValues[24] += (int)vReal[i];
      // if (i > 68 && i <= 71) bandValues[25] += (int)vReal[i];
      // if (i > 71 && i <= 74) bandValues[26] += (int)vReal[i];
      // if (i > 74 && i <= 77) bandValues[27] += (int)vReal[i];
      // if (i > 77 && i <= 80) bandValues[28] += (int)vReal[i];
      // if (i > 80 && i <= 83) bandValues[29] += (int)vReal[i];
      // if (i > 83 && i <= 87) bandValues[30] += (int)vReal[i];
      // if (i > 87 && i <= 91) bandValues[31] += (int)vReal[i];

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
    int barHeight = bandValues[band] *1.25 / AMPLITUDE;
    if (barHeight > MATRIX_SIDE) barHeight = MATRIX_SIDE;
    // 旧的高度值和新的高度值平均一下
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;
    // 如果条形的高度大于顶点高度，则调整顶点高度
    if (barHeight > peak[band]) {
      peak[band] = min(MATRIX_SIDE - 0, barHeight);
    }
    matrix.drawFastVLine((MATRIX_WIDTH - 1 - band), (MATRIX_SIDE - barHeight), barHeight, hsv2rgb(color, 100, 100));
    color += 360 / (NUM_BANDS+2);
    // 绘制顶点
    matrix.drawPixel((MATRIX_WIDTH - 1 - band), MATRIX_SIDE - peak[band] - 1, matrix.Color(150, 150, 150));
    // 将值记录到oldBarHeights
     oldBarHeights[band] = barHeight;
  }  
// 50毫秒降低一次顶点
  if((millis() - peekDecayTime) >= 50){
    for (byte band = 0; band < NUM_BANDS; band++){
      if (peak[band] > 0) peak[band] -= 1;
    }
    colorTime++;
    peekDecayTime = millis();
  }

  matrix.setBrightness(50);
  matrix.show();
}

// HSV转RGB格式
uint16_t hsv2rgb(uint16_t h, uint8_t s, uint8_t v){
  uint8_t r, g, b;
    int i;
  float f, p, q, t;

  h = max(0.0, min(360.0, h));
  s = max(0.0, min(100.0, s));
  v = max(0.0, min(100.0, v));

  s /= 100.0;
  v /= 100.0;

  if (s == 0) {
    // Achromatic (grey)
    r = g = b = round(v * 255.0);
    return 0;
  }

  h /= 60.0; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
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
  return matrix.Color(r, g, b);
}
