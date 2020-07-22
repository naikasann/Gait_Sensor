/*  Gaitsensor_Receiver.ino
*   A program that uses Bluetooth communication to exchange the gait sensors of both feet and sends them to Blynk.
*   The master side.
*   Github           : https://github.com/naikasann/Gait_Sensor
*   execution device : M5Atom Lite.
*   authors          : yamazaki kohei, Last Update Date : 2020.7.22
*/

// for Blynk setting.
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
#include <BlynkSimpleEsp32_BLE.h>
// You should get Auth Token in the Blynk App.
char auth[] = "J1t4Y14Q2eNR3r6cN2zfEwZaewck3kuY";
// for BLE
#include <BLEDevice.h>
#include <BLEServer.h>
// for microcomputer
#include "M5Atom.h"

// for bluetooth classic
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
#define DEVICE_NAME "Gaitsensor_Receiver"
// child device mac address.
String MACadd = "50:02:91:8A:57:6A";
uint8_t address[6]  = {0x50, 0x02, 0x91, 0x8A, 0x57, 0x6A};
bool device_connected = false;

// e-textile sensor GPIO pin setting.
#define TOE_PIN 26
#define HEEL_PIN 32

// for LowPath filter.(N)
#define FILTER 500
//for Low path filter (Moving average filter)
int toe_filter[FILTER] = {0};
int heel_filter[FILTER] = {0};

// gait state (bool : do push phase <= true, don't push period =< false)
bool toe_beforestate;
bool heel_beforestate;
bool toe_state;
bool heel_state;

// gait state chenge => True.
// Send gait state send to Blynk app.
bool isChange = false;

// Device setting function.
void setup(){
    // Debug console
    Serial.begin(115200);
    Serial.println("Gaitsensor application start...");
    Serial.println("Devive name : " + String(DEVICE_NAME));
    Serial.println("Bluetooth connection waiting...");
     // for M5Atom Lite
    M5.begin(true, false, true);
    M5.dis.drawpix(0, 0x0000f0);

    // etextile pinMode setting
    pinMode(TOE_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);

    // Blynk set up.
    Blynk.setDeviceName(DEVICE_NAME);
    Blynk.begin(auth);

    // Bluetooth connection to child device.
    SerialBT.begin("Gaitsensor", true);
    device_connected = SerialBT.connect(address);

    // Bluetooth connection report.
    // connect => print(succesfuly),
    // connection timeout => LED red and print(resetpls..)
    if(device_connected) {
        Serial.println("Connected Succesfully!");
    } else {
      while(!SerialBT.connected(10000)) {
        Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app."); 
        M5.dis.drawpix(0, 0x007000);
      }
    }
    // disconnect() may take upto 10 secs max
    if (SerialBT.disconnect()) {
      Serial.println("Disconnected Succesfully!");
    }
    // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
    SerialBT.connect();

    Serial.println("Measure start...!");
    Serial.println("state = 0 : stand. state = 1 : toe.");
    Serial.println("state = 3 : swing. state = 4 : heel.");
    Serial.println("===============================");
}

void loop(){
    // for Moving average filter.
    float ave = 0.0;

    /*=================== toe measure ===================*/
    // do Low path filter (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) toe_filter[i] = toe_filter[i - 1];
    if(digitalRead(TOE_PIN) == LOW) toe_filter[0] = 1;
    else  toe_filter[0] = 0;
    // Calculating the average value
    for(int i = 0; i < FILTER; i++) ave += toe_filter[i];
    ave = (float)ave / FILTER;

    // Check gait state.
    if(ave >= 0.8){
        // toe state : push. if : Same as before state?
        if(!toe_beforestate){
            // toe state : push. if : Same as before state?
            toe_state = true;  isChange = true;
            Blynk.virtualWrite(V0, 1);
        }
        toe_beforestate = true;
    }else{
        // toe state : dont push. if : Same as before state?
        if(toe_beforestate){
            // False(dont push) => toe state. True => transmission variable(isSend).
            toe_state = false;  isChange = true;
            Blynk.virtualWrite(V0, 0);
        }
        toe_beforestate = false;
    }
    /*===================================================*/
    /*=================== heelmeasure ===================*/
    ave = 0;
    // do Low path filter (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) heel_filter[i] = heel_filter[i - 1];
    if(digitalRead(HEEL_PIN) == LOW) heel_filter[0] = 1;
    else  heel_filter[0] = 0;
    // Calculating the average value
    for(int i = 0; i < FILTER; i++) ave += heel_filter[i];
    ave = (float)ave / FILTER;

    // Check gait state.
    if(ave >= 0.8){
        // heel state : push. if : Same as before state?
        if(!heel_beforestate){
            // True(push) => heel state. True => transmission variable(isSend).
            heel_state = true;  isChange = true;
            Blynk.virtualWrite(V1, 1);
        }
        heel_beforestate = true;
    }else{
        // heel state : dont push. if : Same as before state?
        if(heel_beforestate){
            // False(dont push) => heel state. True => transmission variable(isSend).
            heel_state = false;  isChange = true;
            Blynk.virtualWrite(V1, 0);
        }
        heel_beforestate = false;
    }
    /*===================================================*/

    if (SerialBT.available()) {
        char recv = (char)SerialBT.read();
        if(recv == '0'){
            Blynk.virtualWrite(V2, 1);  Blynk.virtualWrite(V3, 1);
        }else if(recv == '1'){
            Blynk.virtualWrite(V2, 1);  Blynk.virtualWrite(V3, 0);
        }else if(recv == '2'){
            Blynk.virtualWrite(V2, 0);  Blynk.virtualWrite(V3, 0);
        }else if(recv == '3'){
            Blynk.virtualWrite(V2, 0);  Blynk.virtualWrite(V3, 1);
        }
    }

    if(isChange){
        /* Transmit the status of the gait. The state is 1~4.
         * state = 0 : stand.
         * state = 1 : toe.
         * state = 3 : swing.
         * state = 4 : heel.
         * This data is analyzed on the master device to analyze the gait on the slave device.*/
        int state;

        // Determination of gait status
        if(toe_state && heel_state) state = 0;
        else if(toe_state && !heel_state) state = 1;
        else if(!toe_state && !heel_state) state = 2;
        else state = 3;

        // stand => Atom lite LED green. other state => blue.
        if(state == 2)  M5.dis.drawpix(0, 0x000070);
        else  M5.dis.drawpix(0, 0x700000);

        // for debug.
        Serial.println("toe state : " + String(toe_state) + "heelstate : " + String(heel_state));
        Serial.println("Gait Sensor state : " + String(state));

        // Resetting gait information updates
        isChange = false;
    }

    Blynk.run();
}
