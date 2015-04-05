#include <JeeLib.h>

// boilerplate for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#define SERIAL  1   // set to 1 to also report readings on the serial port

#define PHOTOPIN    14 // AIO1 - Phototransistor input
#define IRLEDPIN     4 // DIO1 - IR LED supply
#define DEBUGOUTPIN  6 // DIO3 - DEBUG LED output
#define VOLTREADPIN 15 // AIO2 - Voltage reading input

#define SAMPLINGINTERVAL 100 // Sample the dial every <SAMPLINGINTERVAL>ms. 
                             // We can't detect dials turning faster than
                             // 10 * <SAMPLINGINTERVAL>.

int globalCount;
int batteryReading;

MilliTimer scanTimer, sendTimer;

/**
 * This is the struct that we will transmit over the wire.
 */
typedef struct {
 int count;
 int battery; 
} Payload;
Payload measurement;

/**
 * Little helper function to make sure the serial port is flushed before continuing on.
 */
static void serialFlush() {
  #if ARDUINO >= 100
      Serial.flush();
  #endif
  delay(2); // make sure tx buf is empty before going back to sleep
}

int outputState;
/**
 * Sample the state of the phototransitor.
 */
bool sampleMeterState(int irledpin, int photopin) {
  // Spin up the IR led.
  digitalWrite(irledpin, true);    

  // Read the value.
  outputState = digitalRead(photopin);    

//  Serial.println((int) analogRead(photopin));
//  serialFlush();

  // Turn off the IR LED.
  digitalWrite(irledpin, false);

  return outputState;  
}

/**
 * Send our report of what's going on.
 */
static void sendReport() {
  // Wake up the radio.
  rf12_sleep(RF12_WAKEUP);
  rf12_sendNow(0, &measurement, sizeof measurement);
  // Power down and wait for transmission to complete.
  rf12_sendWait(2);
  // Fully power down.
  rf12_sleep(RF12_SLEEP);
  
  // Opttionally log to serial.
  #if SERIAL
    Serial.print("GASMETER COUNTS:");
    Serial.print((int) measurement.count);
    Serial.print(" BATTERY:");
    Serial.print((int) measurement.battery);
    Serial.print(" VOLTAGE:");
    Serial.print((float) measurement.battery / 1024 * 3.3 * 2);
    Serial.print('v');
    Serial.println();
    serialFlush();
  #endif
}

/**
 * Setup out little program.
 */
void setup() {
  // Setup our pins.
  //pinMode(DEBUGOUTPIN, OUTPUT);
  //pinMode(IRLEDPIN, OUTPUT);
  //pinMode(PHOTOPIN, INPUT);
  //pinMode(VOLTREADPIN, INPUT);
 
  // The count starts at 0.
  //globalCount = 0;
  //batteryReading = 0;
  
  // turn the radio off in the most power-efficient manner
  //Sleepy::loseSomeTime(32);
  rf12_initialize(18, RF12_868MHZ, 5);
  //rf12_sleep(RF12_SLEEP);
  
  #if SERIAL
    Serial.begin(57600);
    Serial.println("\n[gasMeter]");
    serialFlush();
  #endif 
  
  //batteryReading = analogRead(VOLTREADPIN);
  //measurement.battery = batteryReading;
}

byte state;
/**
 * Loop over and wait until something needs doing.
 *
 * We report the state of our counter every 60 seconds, or when a count is detected.
 */
void loop() {
  if (rf12_recvDone() && rf12_crc == 0 && rf12_len == sizeof (Payload)) {
    measurement = *(Payload*) rf12_data;
    #if SERIAL
      Serial.print("GASMETER COUNTS:");
      Serial.print((int) measurement.count);
      Serial.print(" BATTERY:");
      Serial.print((int) measurement.battery);
      Serial.print(" VOLTAGE:");
      Serial.print((float) measurement.battery / 1024 * 3.3 * 2);
      Serial.print('v');
      Serial.println();
      serialFlush();
    #endif
  }
  
}

