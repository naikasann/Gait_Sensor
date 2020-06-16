#include <SoftwareSerial.h>
#include <Wire.h>

#define LoRa_fPort_TEMP  12        // port 12 = 温度湿度気圧等

#define SERIAL_WAIT_TIME     6000
#define LoRa_SEND_INTERVAL   600000    // LoRa送信間隔 (ミリ秒)

#define LoRa_RX_PIN     11  // Arduino D11 to LoRa module TX
#define LoRa_TX_PIN     12  // Arduino D12 to LoRa module RX

SoftwareSerial LoRa(LoRa_RX_PIN, LoRa_TX_PIN);

unsigned long beforetime = 0L;

void setup() {
  //for console serial
  Serial.begin(9600);
  Serial.println("LoRa TEST_SEND...");

  // for LoRa module serial
  LoRa.begin(9600);
  LoRa.setTimeout(SERIAL_WAIT_TIME);

  // send initialize command to LoRa
  initLoRa(SERIAL_WAIT_TIME);

  //for sensor i2c initialize
  Wire.begin();

  // beforetime = -(LoRa_SEND_INTERVAL);
  beforetime = millis();  
}

void loop() {
  if (millis() - beforetime > LoRa_SEND_INTERVAL) {
    Serial.println("\nAuto mode - test_send");

    send_temp_and_light();
    beforetime = millis();
  }
}

void rxFlushLoRa(void) {
  delay(SERIAL_WAIT_TIME);
  // print any response data before send
  while (LoRa.available() > 0) {
    char ch = LoRa.read();
    Serial.print(ch);
  }
}

void initLoRa(int waitTime)
{
  // activate LoRa serial
  LoRa.listen();

  // send dummy enter
  sendCmd("\n", false, "", waitTime);
  rxFlushLoRa();

  //
  // LoRa module status clear
  //
  if (!sendCmd("mod reset", true, "", waitTime)) {
    Serial.println("Request Failed");
  }
  rxFlushLoRa();
  if (!sendCmd("mod set_echo off", false, "Ok", waitTime)) {
    Serial.println("Request Failed");
  }
  //
  // LoRa module various value get
  //
  if (!sendCmd("mod get_hw_model", true, "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("mod get_ver", true, "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("lorawan get_deveui", true, "", waitTime)) {
    Serial.println("Request Failed");
  }

  //
  // LoRa module join to Network Server by OTAA
  //
  while (!sendCmd("lorawan join otaa", true, "accepted", waitTime * 2)) {
    Serial.println("Request Failed");
    // forever loop until join success
  }
}

bool sendCmd(String cmd, bool echo, String waitStr, int waitTime)
{
  unsigned long tim;
  String str;
  
  if (echo) {
    Serial.print(cmd);
  }

  LoRa.listen();

  LoRa.print(cmd);
  LoRa.print('\r');
  LoRa.flush();

  tim = millis() + waitTime;

  while (millis() < tim) {
    if (LoRa.available() > 0) {
      char ch = LoRa.read();
      Serial.print(ch);
      str += String(ch);
      if (str.indexOf("\r> ") >= 0) break;
    }
  }
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0) return true;
  
  return false;
}

bool sendCmd2(String cmd, bool echo, String waitStr, String waitStr2, int waitTime)
{
  unsigned long tim;
  String str;
  int in_byte;
  
  if (echo) {
    Serial.print(cmd);
  }

  LoRa.listen();

  LoRa.print(cmd);
  LoRa.print('\r');
  LoRa.flush();

  tim = millis() + waitTime;

  while (millis() < tim) {
    if (LoRa.available() > 0) {
      char ch = LoRa.read();
      Serial.print(ch);
      str += String(ch);
      if (str.indexOf("\r> ") >= 0) break;
    }
  }
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0 || str.indexOf(waitStr2) >= 0) return true;
  
  return false;
}

bool send_temp_and_light(){
  char cmdline[300];
  short port = LoRa_fPort_TEMP;    // port 12 = Temperature

  if (!sendCmd("lorawan set_dr 5", true, "Ok", SERIAL_WAIT_TIME)) {
    Serial.println("Request Failed");
  }

  // (小数点以下2桁まで有効にするため)100倍して整数と見なし、16進数変換して送信する。
  sprintf(cmdline, "lorawan tx ucnf %d ", port);
  if (!sendCmd2(cmdline, true, "tx_ok", "rx", SERIAL_WAIT_TIME)) {
    Serial.println("Request Failed");
    return false;
  }
  return true;
}
