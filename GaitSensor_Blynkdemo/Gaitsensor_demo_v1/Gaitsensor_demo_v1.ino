#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
#include <BlynkSimpleEsp32_BLE.h>
// You should get Auth Token in the Blynk App.
char auth[] = "J1t4Y14Q2eNR3r6cN2zfEwZaewck3kuY";
// for microcomputer
#include "M5Atom.h"
// for BLE
#include <BLEDevice.h>
#include <BLEServer.h>

// e-textile sensor 
#define TOO_PIN 26
#define HEEL_PIN 32

#define FILTER 100
int too_filter[FILTER] = {0};
int heel_filter[FILTER] = {0};

bool before_too;
bool before_heel;

void setup(){
    // Debug console
    Serial.begin(115200);
    Serial.println("Waiting for connections...");
     // for M5Atom Lite
    M5.begin(true, false, true);
    M5.dis.drawpix(0, 0x0000f0);
        
    // etextile pinMode setting
    pinMode(TOO_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);
    
    Blynk.setDeviceName("GaitSensor");
    Blynk.begin(auth);
}

void loop(){
    if(digitalRead(TOO_PIN) == LOW){
        if(!before_too){
            // send execute.
        }
        before_too = true;
        Blynk.virtualWrite(V0, 1);  
    }else{
        if(before_too){
        }
        Blynk.virtualWrite(V0, 0);
        before_too = false;
    }
    if(digitalRead(HEEL_PIN) == LOW){
        if(!before_heel){
        }
        before_heel = true;
        Blynk.virtualWrite(V1, 1);  
    }else{
        if(before_heel){
        }
        Blynk.virtualWrite(V1, 0);
        before_heel = false;
    }
    M5.dis.drawpix(0, 0x707070);
    Blynk.run();
}
