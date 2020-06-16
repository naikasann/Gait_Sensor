void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  Serial2.begin(9600); // EXT_IO
  Serial.println("Connect to the Sigfox Breakout board...");
  Serial.println("AT$I=10 : get Device ID");
  Serial.println("AT$I=11 : get PAC");
  Serial.println("AT$T? : get Temperature");
  Serial.println("AT$V? : get Voltages");
  Serial.println("AT$P=unit : set Power mode (unit = 0:software reset 1:sleep 2:deep_sleep)");
  Serial.println("AT$TR=unit : set Transmit repeat (unit = 0..2)");
  Serial.println("AT$WR : save config");
  Serial.println("AT$SF=[payload] : SEND SIGFOX MESSAGE");
  Serial.print("Device ID : ");
  Serial2.print("AT$I=10\r");
}

void loop() { // run over and over
  /*if (Serial2.available()) {
    Serial.write(Serial2.read());
  }*/
  if (Serial.available()) {
    Serial2.write(Serial.read());
  }
  datasend_sigfox();
  delay(1000);
}

void datasend_sigfox(){
    String sendcommand = "AT$SF=";
    int a = 2345;
    char data[10];

    sprintf(data, "%4d", a);
    //Serial.println(data);
    sendcommand += String(data) + "\r";
    if(!sendCmd(sendcommand, "OK", 2000))  Serial.println("Request Failed");
    delay(60000);
}

bool sendCmd(String cmd, String waitStr, int waitTime){
  unsigned long tim;
  String str;
  
  Serial2.print(cmd);
  Serial2.flush();

  tim = millis() + waitTime;
  while (millis() < tim) {
      if (Serial2.available() > 0) {
          char ch = Serial2.read();
          Serial.print(ch);
          str += String(ch);
          if (str.indexOf("\r> ") >= 0) break;
      }
  }
  if (waitStr == NULL) return true;
  if (str.indexOf(waitStr) >= 0) return true;
  
  return false;
}
