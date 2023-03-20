#include <Arduino.h>
#include "SoftwareSerial.h"

SoftwareSerial btSerial(3, 2); // TX RX

char txt;
int del = 250;
void setup()
{
  Serial.begin(9600);
  btSerial.begin(9600);
}

void loop()
{
  if (btSerial.available())
  {
    txt = btSerial.read();
    del = 250;
    if (txt == 'A')
    {
      del = 1000;
    }
    Serial.print('reading');
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(del);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(del);
  }
  Serial.print(txt);
}