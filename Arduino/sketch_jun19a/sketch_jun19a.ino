#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>

#define PIN 13        // LED 数据引脚
#define NUMPIXELS 30 // LED 数量
#define AUDIO_PIN A0 // 音频输入引脚
#define SAMPLES 64   // FFT采样点数量，必须是2的幂
#define SAMPLING_FREQUENCY 10000 // 采样频率

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
ArduinoFFT<double> FFT = ArduinoFFT<double>();

unsigned int samplingPeriod;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];

void setup() {
  Serial.begin(9600);
  pixels.begin(); // 初始化LED灯条
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  // 采集音频信号
  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();
    vReal[i] = analogRead(AUDIO_PIN);
    vImag[i] = 0;
    while (micros() < (microseconds + samplingPeriod)) {
      // 等待下一个采样点
    }
  }

  // 进行FFT变换
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  // 查找频率幅度
  double maxMagnitude = 0;
  int maxIndex = 0;
  for (int i = 1; i < (SAMPLES / 2); i++) {
    if (vReal[i] > maxMagnitude) {
      maxMagnitude = vReal[i];
      maxIndex = i;
    }
  }

  // 计算主频率
  double frequency = (maxIndex * 1.0 * SAMPLING_FREQUENCY) / SAMPLES;

  // 使用主频率和其幅度控制LED灯条
  int brightness = map(maxMagnitude, 0, 1023, 0, 255); // 根据振幅设置亮度
  int color = map(frequency, 20, 2000, 0, 255); // 根据频率设置颜色

  // 设置颜色
  int red = color;
  int green = 255 - color;
  int blue = brightness;

  // 设置所有像素的颜色
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
  }

  pixels.show(); // 更新LED灯条

  delay(10); // 控制刷新率
}
