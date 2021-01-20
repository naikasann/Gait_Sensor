// for microcomputer
#include "M5Atom.h"
// for BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
// for Bluetooth Serial function.(not BLE)
#include "BluetoothSerial.h"

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

// Define Bluetooth Serial variables.
#define SERIALBT_NAME "Gaitsensor_BT_Serial(2)"
BluetoothSerial SerialBT;

// for Terminal
#define SPI_SPEED 115200

// sensor pin defined
#define TOO_PIN 32
#define HEEL_PIN 26

// for LowPath filter.(N)
#define FILTER 1000

//移動平均用フィルター
#define GAIT_FILTER 10

//for Low path filter (Moving average filter)
int filter[FILTER] = {0};

// for BLE lag
// Number of transmissions and receptions to check for lag
#define SEND_AND_RECV_COUNT 10
//{左立脚期,左遊脚期,右立脚期,右遊脚期,左両脚支持期,右両脚支持期}
#define DATANUM 6

/*********************<     walking time value     >************************/
// send data.(send before time.The lag is calculated by comparing with the received time.)
int beforetime = 0;
// received time.
int aftertime = 0;
// Total measured data communication time
int sumMeasuretime = 0;
// Average measured data communication time
int aveMeasuretime = 0;

// before state (bool : Standing phase <= true, Swing period =< false)
bool beforestate;

// Walking data (stand time, swing time.)
int standtime = 0;
int swingtime = 0;
// other Walking data (stand time, swing time.)
int otherstandtime = 0;
int otherswingtime = 0;

//for Both feet support period
int maintime = 0;
bool mainon;
int subtime = 0;
bool subon;
int bothfoottime = 0;
int subbothfoottime = 0;

int walkingdata[DATANUM] = {0,0,0,0,0,0};

void input_data(int x, int n){
    if( x == 0 && n == 0 ){for(int i=0;i<=5;i++){walkingdata[i] = 0;}}
    else{walkingdata[x] = n;}
}

/*******************************************************************/

/*********************<     BLE callback function     >************************/
// connection and disconnection callback
class funcServerCallbacks: public BLEServerCallbacks{
    // on connect
    void onConnect(BLEServer* pServer){
        deviceConnected = true;
    }
    // on disconnect
    void onDisconnect(BLEServer* pServer){
        deviceConnected = false;
    }
};
// signal receive callback
class funcReceiveCallback: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristicRX){
        // Store received data in string type
        std::string rxValue = pCharacteristicRX->getValue();
        // Measured lag time (1 time measured)
        int measurelagtime;

        // We'll use a comparison operator to measure things like this.
        if(!strcmp(rxValue.c_str(), "100")){
            // Stores the execution time when received
            aftertime = millis();
            // Calculate the time required for communication.
            measurelagtime = aftertime - beforetime;
            sumMeasuretime += measurelagtime;
            // for debug
            Serial.print("Send and recv time : ");
            Serial.println(measurelagtime);
       //****************************************************************************************sub****************************************
        }else if(!strcmp(rxValue.c_str(), "1")){
            subon = true;
            subtime = millis();
            
            if(!mainon){
                    Serial.println("run!");
                    input_data(0,0);
                    input_data(1,swingtime);
            }
            
            otherswingtime = subtime - aftertime;
            Serial.println("            other swing time : " + String(otherswingtime));
            input_data(3,otherswingtime);
                 
        }else if(!strcmp(rxValue.c_str(), "0")){
            subon = false;
            aftertime = millis();
            otherstandtime = aftertime - subtime;
            if(otherstandtime > 50){
                Serial.println("            other stand time : " + String(otherstandtime));
                input_data(2,otherstandtime); 
                
                if(mainon){
                    subbothfoottime = aftertime - maintime;
                    Serial.println("            sub both foot on! time : " + String(subbothfoottime));
                    input_data(5,subbothfoottime);
                }
            }
        }
     //****************************************************************************************sub**************************************
    }
};

// Service Type Definition
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
    // for M5Atom Lite
    M5.begin(true, false, true);

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

    // Start a Bluetooth Serial.
    SerialBT.begin(SERIALBT_NAME);
    
    Serial.println("Waiting to connect...");
    // Wait for device to connect(Blinking LED on AtomLite)
    while(!deviceConnected){M5.dis.drawpix(0, 0x0000f0);delay(1000);M5.dis.drawpix(0, 0x000000);delay(200);}
    // Now that the connection is complete, turn on the white LED and start the measurement.
    M5.dis.drawpix(0, 0x707070);
    Serial.println("connection!");
    delay(1000);

    // Start measuring lag
    Serial.println("Start measuring lag!");
    for(int i = 0;i < SEND_AND_RECV_COUNT; i++){
        // send count data.Give the first time of lag measurement. for debug
        Serial.println("send data :" + String(i));
        // set senddata. this is test send count.
        pCharacteristicTX->setValue(i);
        pCharacteristicTX->notify();
        // data send (checking send before time.)
        beforetime = millis();
        delay(500);
    }
    Serial.println("The lag measurement is complete.");
    // Calculation
    aveMeasuretime = sumMeasuretime / SEND_AND_RECV_COUNT;
    // for debug
    Serial.print("Total time when communication was performed : ");
    Serial.println(sumMeasuretime);
    Serial.print("Total average time :");
    Serial.println(aveMeasuretime);
    Serial.println("Analyzes gait information");

    delay(aveMeasuretime);
    Serial.println(3);
    delay(1000);
    Serial.println(2);
    delay(1000);
    Serial.println(1);
    delay(1000);
    beforetime = millis(); aftertime = millis();
    maintime = millis(); subtime = millis();
}
/******************************************************************************/
/*********************<          Main sequence        >************************/
void loop() {
    float ave = 0.0;
    // do Low path filter (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) filter[i] = filter[i - 1];
    if((digitalRead(TOO_PIN) == LOW)||(digitalRead(HEEL_PIN) == LOW)) filter[0] = 1;
    else  filter[0] = 0;
    // decide walking state
    for(int i = 0; i < FILTER; i++) ave += filter[i];
    ave = (float)ave / FILTER;

    if(ave >= 0.7){       //左足が地に着いた
        if(!beforestate){
            M5.dis.drawpix(0, 0x000070);
            mainon = true;
            maintime = millis();
            if(!subon){
                    Serial.println("run!");
                    input_data(0,0);
                    input_data(3,otherswingtime);
            }
            swingtime = maintime - beforetime;
            Serial.println("swingtime : " + String(swingtime));
            input_data(1,swingtime);
            
            SerialBT.printf("%d,%d,%d,%d,%d,%d\n",walkingdata[0],walkingdata[1],walkingdata[2],walkingdata[3],walkingdata[4],walkingdata[5]);
            Serial.printf("   %d,%d,%d,%d,%d,%d\n",walkingdata[0],walkingdata[1],walkingdata[2],walkingdata[3],walkingdata[4],walkingdata[5]);
            input_data(0,0); //初期化
        }
        beforestate = true;
    }else{
        if(beforestate){
            M5.dis.drawpix(0, 0x700000);
            mainon = false;
            beforetime = millis();
            standtime = beforetime - maintime;
            if(standtime > 50){
                Serial.println("standtime : " + String(standtime));
                input_data(0,standtime);
                
                if(subon){
                    bothfoottime = beforetime - subtime;
                    Serial.println("main both foot on! time : " + String(bothfoottime));
                    input_data(4,bothfoottime);
                }
            }
        }
        beforestate = false;
    }
}
