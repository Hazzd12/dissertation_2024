
#include "DFRobot_DF2301Q.h"
#define Led 9
#include "BME680.h"

DFRobot_DF2301Q_UART* asr;

void setup() {
  Serial.begin(9600);
  BMEsetup();
  
  // 在setup中初始化
  asr = new DFRobot_DF2301Q_UART(&Serial1);
  pinMode(Led, OUTPUT);    //初始化LED引脚为输出模式
  digitalWrite(Led, LOW);  //LED引脚低电平

  // Init the sensor
  while (!(asr->begin())) {
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
  }
  Serial.println("Begin ok!");

  asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_MUTE, 1);
  asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_VOLUME, 7);
  asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_WAKE_TIME, 10);
  //asr->settingCMD(DF2301Q_UART_MSG_CMD_SET_ENTERWAKEUP, 0);

  asr->playByCMDID(23);
  Serial.println("IN");
  
}

void loop() {
  /**
     @brief 获取命令词对应的ID
     @return 返回获取的命令词ID, 返回0表示未获取到有效id
  */
  
  uint8_t CMDID = asr->getCMDID();
  switch (CMDID) {
    case 117:                                                  //若是指令“打开灯光”
      digitalWrite(Led, HIGH);                                 //点亮LED
      Serial.println("received'打开灯光',command flag'117'");  //串口发送received"打开灯光",command flag"117"
      break;

    case 118:                                                  //若是指令“关闭灯光”
      digitalWrite(Led, LOW);                                  //熄灭LED
      Serial.println("received'关闭灯光',command flag'118'");  //串口发送received"关闭灯光",command flag"118"
      break;

    default:
      if (CMDID != 0) {
        Serial.print("CMDID = ");  //打印命令ID
        Serial.println(CMDID);
      }
  }
  BMEloop();
  delay(300);
}

