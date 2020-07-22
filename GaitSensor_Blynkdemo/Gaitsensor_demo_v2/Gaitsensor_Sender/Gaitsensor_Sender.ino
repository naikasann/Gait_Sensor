/*  Gaitsensor_Sender.ino
*   A program that uses Bluetooth communication to exchange the gait sensors of both feet and sends them to Blynk.
*   The child side.
*   Github           : https://github.com/naikasann/Gait_Sensor
*   execution device : M5Atom Lite.
*   authors          : yamazaki kohei, Last Update Date : 2020.7.22
*/

// for micro computer lib
#include "M5Atom.h"
// for Bluetooth lib
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// for bluetooth setting.
#define DEVICE_NAME "Gaitsensor_Sender"
BluetoothSerial SerialBT;
bool device_connect = false;

// for Serial terminal.(debug)
#define SPI_SPEED 115200

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
bool toe_state;
bool heel_beforestate;
bool heel_state;

// gait state chenge => True.
// Send gait state send to master device.
bool isSend = false;

// Device setting function.
void setup() {
    // for terminal
    Serial.begin(SPI_SPEED);
    // etextile pinMode setting
    pinMode(TOE_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);
     // for M5Atom Lite
    M5.begin(true, false, true);
    // for bluetooth classic
    SerialBT.begin(DEVICE_NAME);

    // intro print. (debug)
    Serial.println("Gaitsensor application start...");
    Serial.println("Devive name : " + String(DEVICE_NAME));
    Serial.println("Measure start...!");
    Serial.println("state = 0 : stand. state = 1 : toe.");
    Serial.println("state = 3 : swing. state = 4 : heel.");
    Serial.println("===============================");
}

void loop() {
    // for average filter.
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
            // True(push) => toe state. True => transmission variable(isSend).
            toe_state = true;  isSend = true;
        }
        toe_beforestate = true;
    }else{
        // toe state : dont push. if : Same as before state?
        if(toe_beforestate){
            // False(dont push) => toe state. True => transmission variable(isSend).
            toe_state = false;  isSend = true;
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
            heel_state = true;  isSend = true;
        }
        heel_beforestate = true;
    }else{
        // heel state : dont push. if : Same as before state?
        if(heel_beforestate){
            // False(dont push) => heel state. True => transmission variable(isSend).
            heel_state = false;  isSend = true;
        }
        heel_beforestate = false;
    }
    /*===================================================*/

    // Is the gait status updated?
    if(isSend){
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
        // Bluetooth send execute.
        SerialBT.println(state);
        // Resetting gait information updates
        isSend = false;
    }
}
