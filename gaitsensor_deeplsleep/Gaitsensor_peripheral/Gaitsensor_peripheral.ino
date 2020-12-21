#include "M5Atom.h"

#include <Ticker.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "BluetoothSerial.h"

// BLE service name
#define SERVER_NAME "Gait_sensor_central"
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
#define TOE_PIN 32
#define HEEL_PIN 26
// for LowPath filter.(N)
#define FILTER 1000
// Moving average filter threshold
#define FILTER_THRESHOLD 0.7
int toe_filter[FILTER] = {0};
int heel_filter[FILTER] = {0};

// Where to temporarily store the measurements when
// memo : main => this micro computer. sub => BLE communication partner microcomputer.
int main_bufftime = 0;
int sub_bufftime = 0;
// One previous walking state (one foot)
// before state (int : heel => 0, stand => 1, toe => 2, swing => 3.)
int main_beforestate = 0;
int sub_beforestate = 0;
// for both foot time.
// on => true, off => false.
bool main_foot_state = false;
bool sub_foot_state = false;
// for bothfoottime buffer valiable
int main_foot_bufftime = 0;
int sub_foot_bufftime = 0;

// Number of gait parameters to be retrieved (no need to change!)
#define WALK_STATE_COUNT 10
// [right heel, stand, toe, swing, left heel, stand, toe, swing, bothtime1, bothtime2]
int walkstime[WALK_STATE_COUNT] = {0};
// Information on Steps for Deep Sleep.
int mainsteps = 0;
int substeps = 0;

// Declaring a timer interrupt handler to check for the working.
Ticker movechecker;
bool isMove = false;
// Setting the interval to check the operation(sec)
#define MOVECHECKTIME 2
// run checker.
#define RUN_THRESHHOLD 4
int run_threshhold = 0;
// Set the time to do deep sleep (sec)
#define DEEPSLEEPTIME 30
// Set the number of gait cycles to perform a deep sleep
#define DEEPSLEEPSTEPS 10

// function : Assign the value to the specified index of walkstime.
void set_walktime(int index, int inputdata){
    walkstime[index] = inputdata;
}

// function : Clean output of the values stored in walkstime
void print_walktime(){
    for(int i = 0; i < WALK_STATE_COUNT; i++){
        Serial.print(walkstime[i]);
        Serial.print(",");
    }
    Serial.println("");
}

void doSleep(bool issend){
    M5.dis.drawpix(0, 0xf0f0f0);
    Serial.println("sleep");
    if (issend){
        // Put out the support so that the other leg starts to sleep deep.
        int sendvalue = 2525;
        pCharacteristicTX->setValue(sendvalue);
        pCharacteristicTX->notify();
    }
    // set deepsleep.
    esp_sleep_enable_timer_wakeup(DEEPSLEEPTIME * 1000 * 1000);
    esp_deep_sleep_start();
}

// function : Operate at regular intervals. If not moving, perform a deep sleep on both feet.
void move_check() {
    Serial.println("movecheck");
    if(isMove){
        isMove = false;
    }else{
        doSleep(true);
    }
}

// connection and disconnection callback (Assign a bool value to the device's connection status)
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
        int measured_val = 0;
        int both_measured = 0;
        // Store received data in string type
        std::string rxValue = pCharacteristicRX->getValue();

        // We'll use a comparison operator to measure things like this.
        // other foot state : heel.
        if(!strcmp(rxValue.c_str(), "0")){
            // toe => off, heel => on.
            measured_val = millis() - sub_bufftime;
            sub_bufftime = millis();
            substeps++;
            // swing time setting.
            set_walktime(7, measured_val);
            sub_foot_bufftime = millis();
            sub_foot_state = true;
        }
        // other foot state : stand.
        if(!strcmp(rxValue.c_str(), "1")){
            // toe => on, heel => on.
            measured_val = millis() - sub_bufftime;
            sub_bufftime = millis();
            // heel time setting.
            set_walktime(4, measured_val);
            sub_foot_state = true;
        }
        // other foot state : toe.
        if(!strcmp(rxValue.c_str(), "2")){
            measured_val = millis() - sub_bufftime;
            sub_bufftime = millis();
            // stand time setting.
            set_walktime(5, measured_val);
            sub_foot_state = true;
        }
        // other foot state : swing.
        if(!strcmp(rxValue.c_str(), "3")){
            // toe => off, heel => off.
            measured_val = millis() - sub_bufftime;
            sub_bufftime = millis();
            // toe time setting.
            set_walktime(6, measured_val);
            if(main_foot_state){
                both_measured = sub_bufftime - main_foot_bufftime;
                set_walktime(9, both_measured);
            }else{
                //Serial.println("RUN!!");
                set_walktime(9, 0);
            }
            sub_foot_state = false;
        }

        // The other leg sent a deep sleep command, and we followed that command to perform a deep sleep.
        if(!strcmp(rxValue.c_str(), "2525")){
            // set deepsleep.
            doSleep(false);
        }
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

void setup() {
    // for M5Atom Lite
    M5.begin(true, false, true);
    // for terminal
    Serial.begin(SPI_SPEED);
    // etextile pinMode setting
    pinMode(TOE_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);
    // Ble setting.
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
    // Wait for device to connect(Blinking LED on AtomLite)
    while(!deviceConnected){M5.dis.drawpix(0, 0x0000f0);delay(500);M5.dis.drawpix(0, 0x000000);delay(500);}
    // Now that the connection is complete, turn on the white LED and start the measurement.
    M5.dis.drawpix(0, 0x000000);
    // Set the timer interrupt handler for checking the operation
    movechecker.attach_ms(MOVECHECKTIME * 1000, move_check);

    Serial.println("connection!");
}

void loop() {
    // Variables for the moving average filter.
    float heel_ave = 0.0;
    float toe_ave = 0.0;
    // Measured value.
    int measured_val = 0;
    int both_measured = 0;
    bool toe_state, heel_state;

    // do Low path filte for toe switch (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) toe_filter[i] = toe_filter[i - 1];
    if((digitalRead(TOE_PIN) == LOW)) toe_filter[0] = 1;
    else  toe_filter[0] = 0;
    // decide walking state for toe state.
    for(int i = 0; i < FILTER; i++) toe_ave += toe_filter[i];
    toe_ave = (float)toe_ave / FILTER;
    if(toe_ave > FILTER_THRESHOLD)  toe_state = true;
    else    toe_state = false;

    // do Low path filte for heel switch (Moving average filter)
    for(int i = FILTER - 1; i > 0; i--) heel_filter[i] = heel_filter[i - 1];
    if((digitalRead(HEEL_PIN) == LOW)) heel_filter[0] = 1;
    else  heel_filter[0] = 0;
    // decide walking state for heel state.
    for(int i = 0; i < FILTER; i++) heel_ave += heel_filter[i];
    heel_ave = (float)heel_ave / FILTER;
    if(heel_ave > FILTER_THRESHOLD)   heel_state = true;
    else    heel_state = false;

    // before state (int : heel => 0, stand => 1, toe => 2, swing => 3.)
    switch (toe_state){
        case true:
            switch (heel_state){
                // foot state := stand.
                case true:
                    // toe => on, heel => on.
                    if(main_beforestate != 1){
                        measured_val = millis() - main_bufftime;
                        main_bufftime = millis();
                        mainsteps++;
                        // heel time setting.
                        set_walktime(0, measured_val);
                        M5.dis.drawpix(0, 0x007070);
                        isMove = true;
                    }
                    main_beforestate = 1;
                    main_foot_state = true;
                    break;
                // foot state := toe.
                case false:
                    // toe => on, heel => off.
                    if(main_beforestate != 2){
                        measured_val = millis() - main_bufftime;
                        main_bufftime = millis();
                        // stand time setting.
                        set_walktime(1, measured_val);
                        M5.dis.drawpix(0, 0x707000);
                        isMove = true;
                    }
                    main_beforestate = 2;
                    main_foot_state = true;
                    break;
            }
            break;
        case false:
            switch (heel_state){
                // foot state := heel
                case true:
                    // toe => off, heel => on.
                    if(main_beforestate != 0){
                        measured_val = millis() - main_bufftime;
                        main_bufftime = millis();
                        // swing state time setting.
                        set_walktime(3, measured_val);
                        M5.dis.drawpix(0, 0x700070);
                        isMove = true;
                        main_foot_bufftime = millis();
                    }
                    main_beforestate = 0;
                    main_foot_state = true;
                    break;
                // foot state := swing.
                case false:
                    // toe => off, heel => off.
                    if(main_beforestate != 3){
                        measured_val = millis() - main_bufftime;
                        main_bufftime = millis();
                        // toe state time setting.
                        set_walktime(2, measured_val);
                        M5.dis.drawpix(0, 0x700000);
                        isMove = true;
                        // both foot time measured. if run, 0 value set.
                        if(sub_foot_state){
                            both_measured = millis() - sub_foot_bufftime;
                            set_walktime(8, both_measured);
                        }else{
                            //Serial.println("RUN!");
                            run_threshhold += 1;
                            set_walktime(8, 0);
                            if(run_threshhold > RUN_THRESHHOLD){
                                doSleep(true);
                            }
                        }

                        // Output the result as one cycle is completed.
                        print_walktime();

                        if(mainsteps == DEEPSLEEPSTEPS){
                            Serial.println("main step deepsleep");
                            doSleep(true);
                        }
                    }
                    main_beforestate = 3;
                    main_foot_state = false;
                    break;
            }
            break;
    }
}
