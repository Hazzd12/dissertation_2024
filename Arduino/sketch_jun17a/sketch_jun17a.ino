const int S1 =2;
const int S2 = 3;  // 修改为有效的引脚号
const int S3 = 4;
const int Z = 5;   // 将Z设置为数字引脚，而不是模拟引脚
const int test = 7;

int PIRs[8];

void setup() {
  set_PIRs();
}

void loop() {
  // 读取单独的PIR传感器状态
  Serial.println("________");
  // 读取通过74HC4051的PIR传感器状态
  
  loop_PIRs();

  delay(1000); // 每秒钟读取一次所有通道
}

void set_PIRs(){
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  Serial.begin(9600); // 初始化串口通信
  pinMode(Z, INPUT);  // 设置Z引脚为输入模式，并启用下拉电阻
  pinMode(test, OUTPUT);
  // 如果还有其他PIR传感器需要设置输入模式，可以在此设置
  digitalWrite(test, HIGH);
}

void loop_PIRs(){
for (int i = 0; i < 8; i++) {
    // 设置选择引脚的状态
    digitalWrite(S1, (i & 0x01));
    digitalWrite(S2, (i & 0x02) >> 1);
    digitalWrite(S3, (i & 0x04) >> 2);

    delay(5); // 短暂延迟，确保通道切换完成
    pinMode(Z, OUTPUT);
    digitalWrite(Z, LOW);
    delay(1);
    pinMode(Z, INPUT);

    int value = digitalRead(Z); // 读取选定通道的数字值
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(value);
    PIRs[i] = val;
    delay(200);
  }
}
