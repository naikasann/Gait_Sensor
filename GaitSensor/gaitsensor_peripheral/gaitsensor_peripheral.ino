// for BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
// for timer function
#include <Ticker.h>

// BLE service name
#define SERVER_NAME "Gait_sensor_peripheral"

// UUID defined (for BLE)
#define SERVICE_UUID           "28b0883b-7ec3-4b46-8f64-8559ae036e4e"
#define CHARACTERISTIC_UUID_TX "2049779d-88a9-403a-9c59-c7df79e1dd7c"
#define CHARACTERISTIC_UUID_RX "9348db8a-7c61-4c6e-b12d-643625993b84"
// For communication control
BLECharacteristic *pCharacteristicTX;   // characteristic for sending
BLECharacteristic *pCharacteristicRX;   // characteristic for receiving
bool deviceConnected = false;           // Device connections

// for Terminal
#define SPI_SPEED 115200

// sensor pin defined
#define TOO_PIN 25
#define HEEL_PIN 26

// for LowPath filter.(N)
#define FILTER 1000
//for Low path filter (Moving average filter)
int filter[FILTER] = {0};

/*********************<     BLE callback function     >************************/
// connection and disconnection callback
class funcServerCallbacks: public BLEServerCallbacks{
    void onConnect(BLEServer* pServer){
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer){
        deviceConnected = false;
    }
};
// signal receive callback
class funcReceiveCallback: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristicRX){
        // Store received data in string type
        std::string rxValue = pCharacteristicRX->getValue();
        if(rxValue.length() <= 1){
        
        }
        if(!strcmp(rxValue.c_str(), "20")){
          
        }
    }
};
void setBluetoothService(BLEService *pService){
    // Creating a Characteristic for Notify
    pCharacteristicTX = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
    pCharacteristicTX->addDescriptor(new BLE2902());

    // Create reception character sticks and set up a callback for incoming signals.
    pCharacteristicRX = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
    pCharacteristicRX->setCallbacks(new funcReceiveCallback());
}
/******************************************************************************/
/*********************<        Setup (Prepare)        >************************/
void setup() {
    // for terminal
    Serial.begin(SPI_SPEED);
    // etextile pinMode setting
    pinMode(TOO_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);

    // BLE init
    BLEDevice::init(SERVER_NAME);

    // Create server object & callback setting
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new funcServerCallbacks);

    // Create service object & prepare doing.
    BLEService *pService = pServer->createService(SERVICE_UUID);
    setBluetoothService(pService);

    // service start & advertising service uuid
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("Waiting to connect...");
    // Wait for device to connect
    while(!deviceConnected){Serial.print(""); delay(5);}
    Serial.println("");
    Serial.println("connection!");
    delay(1000);
}
/******************************************************************************/
/*********************<          Main sequence        >************************/
void loop() {
    float ave = 0.0;
}
/******************************************************************************/
