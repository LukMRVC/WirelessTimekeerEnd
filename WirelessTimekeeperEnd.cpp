#include <Arduino.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

#define RADIO_CHANNEL 16
#define DEVICE_ADDRESS 0x1F
#define DEST_DEVICE_ADDRESS 0x2F
#define FREQ 868.0

#define LED_PIN 13
#define DEBOUNCE_DELAY 50

// TODO: Change these input button
const short inputBtn1Pin = 3; // change!
const short inputBtn2Pin = 5;
const size_t inputBtn1Idx = 0;
const size_t inputBtn2Idx = 1;

// for buttons 1 and 2
short buttonState[2] = {HIGH, HIGH};
short lastButtonState[2] = {LOW, LOW};
unsigned long lastButtonChangeTime[2] = {0, 0};
unsigned long resultTime[2] = {0, 0};

/*
 * The communication protocol is very simple, it will be text based with 2 basic command types
 * A - meaning answer
 * S - meaning set
 *
 * The message payload will follow right after a command type, delimited with a colon
 * Some examples:
 * A:discover - answer for discovery
 * S:reset - resets timers
 */

// returns boolean if state changed
bool readButton(const short btnPin, const size_t btnIdx)
{
  bool changed = false;
  const auto reading = digitalRead(btnPin);
  if (reading != lastButtonState[btnIdx])
  {
    lastButtonChangeTime[btnIdx] = millis();
  }

  // check if the same reading time is bigger than the debounce delay
  if ((millis() - lastButtonChangeTime[btnIdx]) > DEBOUNCE_DELAY)
  {
    // if button state has changed
    if (reading != buttonState[btnIdx])
    {
      buttonState[btnIdx] = reading;
      changed = true;
    }
  }
  lastButtonState[btnIdx] = reading;
  return changed;
}

void writeBtnStateAndResult(const short btnPin, const short btnState, unsigned long result)
{
  Serial.print("Button on PIN ");
  Serial.print(btnPin);
  Serial.print(" has clicked to state ");
  Serial.print(btnState);
  Serial.print(" with end RESULT: ");
  Serial.println(result);
}

void resetTimers()
{
  resultTime[0] = 0;
  resultTime[1] = 0;
}

bool clientDiscovered = false;

void discoverClient()
{
  /*
  auto received = String(radio.getChars());
  if (received.equals("A:discover"))
  {
    radio.setTxState();
    // TODO: send Answer in loop and wait for ACK, something LIKE
    /*
     * while (!answered) {
     *   radio.sendChars();
     * }
     */
}

void sendResults()
{
}

bool resetSignalReceived()
{
  return false;
}

void shutdownSignalReceived()
{
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  // pinMode(inputBtn2Pin, INPUT);
  delay(100);

  Serial.println("GETTING CC1101");
  if (ELECHOUSE_cc1101.getCC1101())
  { // Check the CC1101 Spi connection.
    Serial.println("CC1101 Connection OK");
  }
  else
  {
    Serial.println("Connection Error");
  }

  ELECHOUSE_cc1101.Init();           // must be set to initialize the cc1101!
  ELECHOUSE_cc1101.setCCMode(1);     // set config for internal transmission mode.
  ELECHOUSE_cc1101.setModulation(0); // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
  ELECHOUSE_cc1101.setMHZ(FREQ);     // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
  ELECHOUSE_cc1101.setSyncMode(2);   // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.

  ELECHOUSE_cc1101.setCrc(1); // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.
  ELECHOUSE_cc1101.setChannel(RADIO_CHANNEL);
  ELECHOUSE_cc1101.setAddr(DEVICE_ADDRESS);
  ELECHOUSE_cc1101.setAdrChk(1);
  Serial.println("Rx Mode");
}

bool timers_running = 0;

bool is_timer_running(short timer_idx)
{
  return (1 << timer_idx) & timers_running == 1;
}

void set_timer(short timer_idx, bool running)
{
  timers_running |= (running ? 1 : 0) << timer_idx;
}

const short buffer_size = 32;
char transmission_buffer[buffer_size];
byte receiving_buffer[buffer_size];

void loop()
{
  memset(transmission_buffer, 0, buffer_size);
  memset(receiving_buffer, 0, buffer_size);
  *transmission_buffer = DEST_DEVICE_ADDRESS;
  // Checks whether something has been received.
  // When something is received we give some time to receive the message in full.(time in millis)
  if (ELECHOUSE_cc1101.CheckRxFifo(100))
  {

    if (ELECHOUSE_cc1101.CheckCRC())
    { // CRC Check. If "setCrc(false)" crc returns always OK!
      Serial.print("Rssi: ");
      Serial.println(ELECHOUSE_cc1101.getRssi());
      Serial.print("LQI: ");
      Serial.println(ELECHOUSE_cc1101.getLqi());

      int len = ELECHOUSE_cc1101.ReceiveData(receiving_buffer);
      receiving_buffer[len] = '\0';
      Serial.println((char *)receiving_buffer + 1);

      if (strncmp(((char *)receiving_buffer + 1), "START", 5) == 0)
      {
        Serial.println(F("Timers are running"));
        set_timer(0, true);
        set_timer(1, true);
      }
    }
    else
    {
      Serial.println("Data received but CRC failed");
    }
  }

  if (is_timer_running(0) || is_timer_running(1))
  {
    if (readButton(inputBtn1Pin, inputBtn1Idx) && buttonState[inputBtn1Idx] == LOW)
    {
      sprintf((transmission_buffer + 1), "END1");
      ELECHOUSE_cc1101.SendData(transmission_buffer, buffer_size);
      Serial.println(F("Timer 1 stopped."));
      set_timer(0, false);
    }
    else if (readButton(inputBtn2Pin, inputBtn2Idx) && buttonState[inputBtn2Idx] == LOW)
    {
      sprintf((transmission_buffer + 1), "END2");
      ELECHOUSE_cc1101.SendData(transmission_buffer, buffer_size);
      Serial.println(F("Timer 2 stopped."));
      set_timer(1, false);
    }
  }

  // TODO: Add radio communication here,
  /*
  if (!clientDiscovered)
  {
    discoverClient();
    if (clientDiscovered)
    {
      resetTimers();
    }
  }

  if (resultTime[0] > 0 && resultTime[1] > 0)
  {
    sendResults();
  }*/

  if (resetSignalReceived())
  {
    set_timer(0, false);
    set_timer(1, false);
  }

  // if (shutdownSignalReceived())
  // {
  //   shutdown();
  // }

  /*
    The communication should be like this:
     1. Discover Start Client acknoledge each other and negotiate time
     2. Reset button times
     3. On button times change, send result to Start Client.
     4. Receive reset signal, then go to step 2, if received shutdown signal, shutdown.

  */
}
