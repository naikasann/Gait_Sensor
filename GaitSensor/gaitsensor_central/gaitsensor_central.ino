// for microcomputer
#include "M5Atom.h"
// for BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE server name
#define DEVICE_NAME "Gait_Sensor_Central"
// UUID defined (for BLE device)
#define SERVICE_UUID           "28b0883b-7ec3-4b46-8f64-8559ae036e4e"
#define CHARACTERISTIC_UUID_RX "2049779d-88a9-403a-9c59-c7df79e1dd7c"
#define CHARACTERISTIC_UUID_TX "9348db8a-7c61-4c6e-b12d-643625993b84"
static BLEUUID serviceUUID(SERVICE_UUID);   // Service UUID
static BLEUUID CHARA_UUID_RX(CHARACTERISTIC_UUID_RX); // recieve for UUID
static BLEUUID CHARA_UUID_TX(CHARACTERISTIC_UUID_TX); // send for UUID
BLERemoteCharacteristic* pRemoteCharacteristicTX;    // characteristic for sending
BLERemoteCharacteristic* pRemoteCharacteristicRX;    // characteristic for receiving
BLEAdvertisedDevice* targetDevice;      // target BLE device
bool deviceConnected = false;           // Device connection
unsigned int scantime = 1;   // Time to find a BLE device(s)

// for SPI setting
#define SPI_SPEED 115200

// e-textile sensor 
#define TOO_PIN 25
#define HEEL_PIN 26

// for LowPath filter.(N)
#define FILTER 1000
//for Low path filter (Moving average filter)
int filter[FILTER] = {0};

// for BLE lag measured
bool enableMeasurement = false;
int lagMeasurecount = 0;
// Number of transmissions and receptions to check for lag
#define SEND_AND_RECV_COUNT 10

// before state (bool : Standing phase <= true, Swing period =< false)
bool beforestate;

/*********************<     BLE callback function     >************************/
// connect callback func
class funcClientCallbacks: public BLEClientCallbacks {
    // on connect
    void onConnect(BLEClient* pClient){
        deviceConnected = true;
    }
    // on disconnect
    void onDisconnect(BLEClient* pClient){
        deviceConnected = false;
    }
};

// advertised device callbacks
class advertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks{
    void onResult(BLEAdvertisedDevice advertisedDevice){
        Serial.println("Advertised Device found :");
        Serial.println(advertisedDevice.toString().c_str());

        // if target devive, end scan.and prepare.
        if(advertisedDevice.haveServiceUUID()){
            BLEUUID service = advertisedDevice.getServiceUUID();
            Serial.print("Have ServiceUUI: "); Serial.println(service.toString().c_str());
            // do connect.
            if (service.equals(serviceUUID)) {
                BLEDevice::getScan()->stop();
                targetDevice = new BLEAdvertisedDevice(advertisedDevice);
            }
        }
    }
};

// Notify Callback func
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                uint8_t* pData, size_t length, bool isNotify) {
    int number;

    // recv data print
    memcpy(&number, pData, length);
    Serial.print("Client Received data: ");  Serial.println(number);
    
    enableMeasurement = true;
}

// Find and connect to a BLE device.
bool searchTargetDevice(){
    // If the BLE is not found, 
    // a coredump is caused and an error occurs, 
    // and the LED turns blue. Because it reboots(bug?)
    // It will be a flashing light in a certain period of time.
    M5.dis.drawpix(0, 0x0000f0);
    
    // Create Client
    BLEClient* pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new funcClientCallbacks());
    Serial.println(" - Created client.");

    // Connection to Peripheral
    pClient->connect(targetDevice);
    Serial.println(" - Connected to peripheral.");

    // service token take
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.print("Failed to find serviceUUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found target service.");

    // Get a reference to the characteristic
    pRemoteCharacteristicRX = pRemoteService->getCharacteristic(CHARA_UUID_RX);
    if (pRemoteCharacteristicRX == nullptr) {
      Serial.print("Failed to find characteristicUUID: ");
      Serial.println(CHARA_UUID_RX.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found characteristic CHARA_UUID_RX.");

    // Assign a Notify callback function
    if(pRemoteCharacteristicRX->canNotify()) {
        pRemoteCharacteristicRX->registerForNotify(notifyCallback);
        Serial.println(" - Registered notify callback function.");
    }

    // Get a reference to the characteristic for Send
    pRemoteCharacteristicTX = pRemoteService->getCharacteristic(CHARA_UUID_TX);
    if (pRemoteCharacteristicTX == nullptr) {
      Serial.print("Failed to find CHARA_UUID_TX: ");
      Serial.println(CHARA_UUID_TX.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found characteristic CHARA_UUID_TX.");

    // Return true and escape from the infinite loop.
    deviceConnected = true;
    return true;
}
void setup() {
    // for terminal
    Serial.begin(SPI_SPEED);
    // etextile pinMode setting
    pinMode(TOO_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);
     // for M5Atom Lite
    M5.begin(true, false, true);
    
    // BLE init
    BLEDevice::init(DEVICE_NAME);
    
    Serial.println("Client application start...");
    // Create Scan object & setting callback
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new advertisedDeviceCallbacks());
    // do active scan
    pBLEScan->setActiveScan(true);
    pBLEScan->start(scantime);
    // Continue scanning until you can connect
    while(!searchTargetDevice()){}
    // Now that the BLE device is connected, 
    // we can change to white LEDs and start the measurement.
    M5.dis.drawpix(0, 0x707070);
    Serial.println("connetion!.");

    //The lag measurement is complete.
    Serial.println("The lag measurement is complete.");
    Serial.println("Analyzes gait information");

    // Send and receive data SEND_AND_RECV_COUNT times
    while(lagMeasurecount != SEND_AND_RECV_COUNT){
        // When receiving lag check data
        delay(1);
        if(enableMeasurement){
            // send execute.
            pRemoteCharacteristicTX->writeValue("100");

            // lag Measure count plus
            lagMeasurecount++;
            // Switch measurement permission
            enableMeasurement = false;
        }
    }

    delay(500);
    Serial.println(3);
    delay(1000);
    Serial.println(2);
    delay(1000);
    Serial.println(1);
    delay(1000);
}

void loop() {
    // send string value buffer.
    char senddata[100];
    // average filter;
    float ave = 0.0;

    // do Low path filter (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) filter[i] = filter[i - 1];
    if((digitalRead(TOO_PIN) == LOW)||(digitalRead(HEEL_PIN) == LOW)) filter[0] = 1;
    else  filter[0] = 0;

    // decide walking state
    for(int i = 0; i < FILTER; i++) ave += filter[i];
    ave = (float)ave / FILTER;
    
    if(ave >= 0.7){
        if(!beforestate){
            // send execute.
            pRemoteCharacteristicTX->writeValue("1");
            M5.dis.drawpix(0, 0x000070);
        }
        beforestate = true;        
    }else{
        if(beforestate){
            pRemoteCharacteristicTX->writeValue("0");
            M5.dis.drawpix(0, 0x700000);
        }
        beforestate = false;
    }
}
