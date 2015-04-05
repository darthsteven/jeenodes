#include <JeeLib.h>

#define SUPPLYPIN 4
#define VOLTREADPIN 15
 
 
void setup () {
  pinMode(SUPPLYPIN, OUTPUT);
  pinMode(VOLTREADPIN, INPUT);
  
  Serial.begin(57600);
  Serial.println("\n[voltMeter]");
  
  digitalWrite(SUPPLYPIN , true);
}
 
int reading;
void loop () {
  
  reading = analogRead(VOLTREADPIN);
  Serial.print("Read: ");
  Serial.print((int) reading);
  Serial.println();
  

  delay(1000);
}
