// for SPI setting
#define SPI_SPEED 115200

// e-textile sensor 
#define TOO_PIN 32
#define HEEL_PIN 26

void setup() {
    // for terminal
    Serial.begin(SPI_SPEED);

    // etextile pinMode setting
    pinMode(TOO_PIN,  INPUT_PULLUP);
    pinMode(HEEL_PIN, INPUT_PULLUP);

    Serial.println("Shoes pin test");
}

void loop() {
    Serial.print("Shoes state | TOO : ");
    Serial.print(digitalRead(TOO_PIN));
    Serial.print(", HEEL : ");
    Serial.println(digitalRead(HEEL_PIN));

    delay(500);
}
