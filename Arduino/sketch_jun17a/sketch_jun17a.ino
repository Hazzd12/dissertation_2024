#define S10 25
#define S11 33  // 修改为有效的引脚号
#define S12 32
#define Z1  26   // 将Z设置为数字引脚，而不是模拟引脚
#define PIR8 27
#define S20 19
#define S21 18
#define S22 5
#define Z2 21


int PIRs[17];

void setup() {
  set_PIRs();
}

void loop() {
  // 读取单独的PIR传感器状态
  Serial.println("________");
  // 读取通过74HC4051的PIR传感器状态
  
  loop_PIRs();

}

void set_PIRs(){
  pinMode(S10, OUTPUT);
  pinMode(S11, OUTPUT);
  pinMode(S12, OUTPUT);
  Serial.begin(9600);
  pinMode(Z1, INPUT_PULLDOWN);
  pinMode(S20, OUTPUT);
  pinMode(S21, OUTPUT);
  pinMode(S22, OUTPUT);
  pinMode(Z2, INPUT_PULLDOWN);
  pinMode(PIR8, INPUT);
}

void loop_PIRs(){
for (int i = 0; i < 8; i++) {
    // 设置选择引脚的状态

    digitalWrite(S10, (i & 0x01));
    digitalWrite(S11, (i & 0x02) >> 1);
    digitalWrite(S12, (i & 0x04) >> 2);

    digitalWrite(S20, (i & 0x01));
    digitalWrite(S21, (i & 0x02) >> 1);
    digitalWrite(S22, (i & 0x04) >> 2);

    delay(10); // 增加延迟，确保通道切换完成

    int value1 = digitalRead(Z1); // 读取选定通道的数字值
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(value1);
    PIRs[i] = value1;

    int value2 = digitalRead(Z2); // 读取选定通道的数字值
    Serial.print("Channel ");
    Serial.print(i+9);
    Serial.print(": ");
    Serial.println(value2);
    PIRs[i+9] = value1;
  }

  int value = digitalRead(PIR8);
  PIRs[8] = value;
  Serial.print("PIR8: ");
  Serial.println(value);

  delay(1000); // 每秒钟读取一次所有通道
}
