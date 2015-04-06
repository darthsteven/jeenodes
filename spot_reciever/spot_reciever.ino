#include <JeeLib.h>

// boilerplate for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#define SERIAL  1   // set to 1 to also report readings on the serial port

/**
 * This is the struct that we recieve over the wire.
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

/**
 * Setup out little program.
 */
void setup() {
  rf12_initialize(18, RF12_868MHZ, 5);
  
  #if SERIAL
    Serial.begin(57600);
    Serial.println("\n[gasMeter]");
    serialFlush();
  #endif
}

/**
 * Loop over and wait until something needs doing.
 *
 * We just keep looking for data packets, and report them if they match..
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

