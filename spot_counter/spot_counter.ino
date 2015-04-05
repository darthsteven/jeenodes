#include <JeeLib.h>

// boilerplate for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#define SERIAL  1   // set to 1 to also report readings on the serial port

// This program does the following:
// 1. Turn on a digital out to illuminate an IR LED.
// 2. Read a value from a digital phototransistor
// 3. Put that state onto an LED on a different port.

#define PHOTOPIN   17   // AIO4
#define IRLEDPIN   5  // DIO2
#define DEBUGOUTPIN  6 // DIO3

int globalCount;

MilliTimer scanTimer, sendTimer;

typedef struct {
 int count;
 byte lobat :1; 
} Payload;
Payload measurement;

static void serialFlush() {
  #if ARDUINO >= 100
      Serial.flush();
  #endif
  delay(2); // make sure tx buf is empty before going back to sleep
}

void setup() {
  // put your setup code here, to run once:
  
  // Setup our pins.
  pinMode(DEBUGOUTPIN, OUTPUT);
  pinMode(IRLEDPIN, OUTPUT);
  pinMode(PHOTOPIN, INPUT);
 
  // The count starts at 0.
  globalCount = 0;
  
  // turn the radio off in the most power-efficient manner
  Sleepy::loseSomeTime(32);
  rf12_initialize(17, RF12_868MHZ, 5);
  rf12_sleep(RF12_SLEEP);
  
  #if SERIAL
    Serial.begin(57600);
    Serial.println("\n[gasMeter]");
    serialFlush();
  #endif 
}

int outputState;
bool sampleMeterState(int irledpin, int photopin) {
  // Spin up the IR led.
  digitalWrite(irledpin, true);    
      
  // Read the value.
  outputState = digitalRead(photopin);    

  // Turn off the IR LED.
  digitalWrite(irledpin, false);

  return outputState;  
}

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
    Serial.print("METER ");
    Serial.print((int) measurement.count);
    Serial.print(' ');
    Serial.print((int) measurement.lobat);
    Serial.println();
    serialFlush();
  #endif
}

bool currentMeterState;
byte state;
void loop() {
  
  // put your main code here, to run repeatedly:
  
  // Send the count every 60 seconds.
  if (sendTimer.poll(60000)) {
    // Add the battery measurement.
    measurement.lobat = rf12_lowbat();
    sendReport();
  }
  
  // Scan for a pulse every 100ms
  if (scanTimer.poll(100)) {
    // track the last 8 pin states, scanned every 100 milliseconds
    // if state is 0x01, then we saw 7 times 0 followed by a 1
    state <<= 1;
    state |= sampleMeterState(IRLEDPIN, PHOTOPIN);
    if (state == 0x01) {
      globalCount++;
      measurement.count = globalCount;
      sendReport();
      // Sleep for about 250ms, the dials can't turn that quickly.
      Sleepy::loseSomeTime(250);
    }
    else {
      // Sleep for around 100ms.
      Sleepy::loseSomeTime(100);
    }
    
  }
  else {
    Sleepy::loseSomeTime(25);
  } 
}
