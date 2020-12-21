// for microcomputer
#include "M5Atom.h"
// for BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE server name
#define DEVICE_NAME "Gait_Sensor_Peripheral"
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

// e-textile sensor.
#define TOE_PIN 32
#define HEEL_PIN 26
// for LowPath filter.(N)
#define FILTER 1000
// Moving average filter threshold
#define FILTER_THRESHOLD 0.9
int toe_filter[FILTER] = {0};
int heel_filter[FILTER] = {0};

// before state (int : heel => 0, stand => 1, toe => 2, swing => 3.)
int beforestate;
int toe_state;
int heel_state;

// connection and disconnection callback (Assign a bool value to the device's connection status)
class funcClientCallbacks: public BLEClientCallbacks {
    void onConnect(BLEClient* pClient){
        deviceConnected = true;
    }
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
    // Not done....
}

bool searchTargetDevice(){
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
    // for M5Atom Lite
    M5.begin(true, false, true);
    // for terminal.
    Serial.begin(115200);
    // etextile pinMode setting
    pinMode(TOE_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);

    // Ble setting.
    BLEDevice::init(DEVICE_NAME);
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new advertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(scantime);

    // Wait for device to connect(Blinking LED on AtomLite)
    while(!searchTargetDevice()){M5.dis.drawpix(0, 0x0000f0);delay(500);M5.dis.drawpix(0, 0x000000);delay(500);}

    // Notice as connected
    M5.dis.drawpix(0, 0x707070);
    Serial.println("Connection!");
}

void loop() {
    // Variables for the moving average filter.
    float heel_ave = 0.0;
    float toe_ave = 0.0;

    // do Low path filte for toe switch (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) toe_filter[i] = toe_filter[i - 1];
    if((digitalRead(TOE_PIN) == LOW)) toe_filter[0] = 1;
    else  toe_filter[0] = 0;
    // do Low path filte for toe switch (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) heel_filter[i] = heel_filter[i - 1];
    if((digitalRead(HEEL_PIN) == LOW)) heel_filter[0] = 1;
    else  heel_filter[0] = 0;

    // decide walking state for toe state.
    for(int i = 0; i < FILTER; i++) toe_ave += toe_filter[i];
    toe_ave = (float)toe_ave / FILTER;
    if(toe_ave > FILTER_THRESHOLD)  toe_state = true;
    else    toe_state = false;
    // decide walking state for heel state.
    for(int i = 0; i < FILTER; i++) heel_ave += heel_filter[i];
    heel_ave = (float)heel_ave / FILTER;
    if(heel_ave > FILTER_THRESHOLD)   heel_state = true;
    else    heel_state = false;

    // before state (int : heel => 0, stand => 1, toe => 2, swing => 3.)
    switch (toe_state){
        case true:
            switch (heel_state){
                case true:
                    // toe => on, heel => on.
                    if(beforestate != 1){
                        pRemoteCharacteristicTX->writeValue("1");
                        M5.dis.drawpix(0, 0x007070);
                    }
                    beforestate = 1;
                    break;
                case false:
                    // toe => on, heel => off.
                    if(beforestate != 2){
                        pRemoteCharacteristicTX->writeValue("2");
                        M5.dis.drawpix(0, 0x707000);
                    }
                    beforestate = 2;
                    break;
            }
            break;
        case false:
            switch (heel_state){
                case true:
                    // toe => off, heel => on.
                    if(beforestate != 0){
                        pRemoteCharacteristicTX->writeValue("0");
                        M5.dis.drawpix(0, 0x700070);
                    }
                    beforestate = 0;
                    break;
                case false:
                    // toe => off, heel => off.
                    if(beforestate != 3){
                        pRemoteCharacteristicTX->writeValue("3");
                        M5.dis.drawpix(0, 0x700000);
                    }
                    beforestate = 3;
                    break;
            }
            break;
    }
}
