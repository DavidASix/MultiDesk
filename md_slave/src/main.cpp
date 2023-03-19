#include <Arduino.h>
#include "SoftwareSerial.h"

char txt;

void setup(){
  Serial.begin(9600);
}

void loop(){
   if(Serial.available()){
    txt = Serial.read();
  }
}