#include "myMusic.h"
   // LED 数据引脚

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
ledSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  drawRHYTHM();
//matrix.drawPixel(0, 0, matrix.Color(150, 150, 150));
//matrix.drawPixel(1, 1, matrix.Color(150, 150, 150));
//matrix.show();
}
