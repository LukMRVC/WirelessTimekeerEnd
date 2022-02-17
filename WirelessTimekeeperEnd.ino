#include "cc1101.h"

#define RADIO_CHANNEL             16
#define THIS_DEVICE_ADDRESS       22
#define DESTINATION_DEVICE_ADDRESS BROADCAST_ADDRESS // Broadcast


#ifdef ESP32
  #define GDO0_INTERRUPT_PIN 13
#elif ESP8266
  #define GDO0_INTERRUPT_PIN D2
#elif __AVR__
  #define GDO0_INTERRUPT_PIN 5 // Digital D2 or D3 on the Arduino Nano allow external interrupts only
#endif


#define LED_PIN 13
#define DEBOUNCE_DELAY 50

// TODO: Change these input button 
const short inputBtn1Pin = 4; // change!
const short inputBtn2Pin = 5;
const size_t inputBtn1Idx = 0;
const size_t inputBtn2Idx = 1;

// for buttons 1 and 2
short buttonState[2] = { HIGH, HIGH };
short lastButtonState[2] = { LOW, LOW };
unsigned long lastButtonChangeTime[2] = { 0, 0 };
unsigned long resultTime[2] = { 0, 0 };

CC1101 radio;
String rec_payload;

// returns boolean if state changed
void readButton(const short btnPin, const size_t btnIdx) {
   const auto reading = digitalRead(btnPin);
   if (reading != lastButtonState[btnIdx]) {
    lastButtonChangeTime[btnIdx] = millis();
   }

   // check if the same reading time is bigger than the debounce delay
   if ((millis() - lastButtonChangeTime[btnIdx]) > DEBOUNCE_DELAY) {
    // if button state has changed
    if (reading != buttonState[btnIdx]) {
      buttonState[btnIdx] = reading;
    }
   }
}

void writeBtnStateAndResult(const short btnPin, const short btnState, unsigned long result) {
  Serial.print("Button on PIN ");
  Serial.print(btnPin);
  Serial.print(" has clicked to state ");
  Serial.print(btnState);
  Serial.print(" with end RESULT: ");
  Serial.println(result);
}

void setupRadio() {
  Serial.println("Starting radio...");
                      // freq     bitrate   channel
  while (!radio.begin(
    CFREQ_868, KBPS_250, RADIO_CHANNEL, THIS_DEVICE_ADDRESS, GDO0_INTERRUPT_PIN )) {
      Serial.println(F("Radio initialization failed, trying again in 5 seconds..."));
      delay(5000);
    }
    radio.printCConfigCheck();
    Serial.println(F("CC1101 radio initialized"));
    rec_payload.reserve(100);
    radio.setRxState();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(inputBtn1Pin, INPUT);
  pinMode(inputBtn2Pin, INPUT);
  delay(100);
  setupRadio();
}

void loop() {
  // put your main code here, to run repeatedly:
  readButton(inputBtn1Pin, inputBtn1Idx);
  readButton(inputBtn2Pin, inputBtn2Idx);

 
  if (buttonState[inputBtn1Idx] == LOW && resultTime[inputBtn1Idx] == 0) {
    resultTime[inputBtn1Idx] = millis();
    writeBtnStateAndResult(inputBtn1Pin, buttonState[inputBtn1Idx], resultTime[inputBtn1Idx]);
  }

  if (buttonState[inputBtn2Idx] == LOW && resultTime[inputBtn2Idx] == 0) {
    resultTime[inputBtn2Idx] = millis();
    writeBtnStateAndResult(inputBtn2Pin, buttonState[inputBtn2Idx], resultTime[inputBtn2Idx]);
  }

  if (buttonState[inputBtn1Idx] == LOW && buttonState[inputBtn2Idx] == LOW) {
    digitalWrite(LED_PIN, HIGH);
  }

}
