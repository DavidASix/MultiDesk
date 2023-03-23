#include <Arduino.h>
#include "SoftwareSerial.h"

SoftwareSerial btSerial(3, 2); // TX RX
int hdmi_1 = 4;
int hdmi_2 = 5;
int hdmi_3 = 6;
int mp_i_io = 13;
int mp_i_a = 12;
int mp_i_b = 11;
int mp_i_c = 10;
int mp_i_input = A0;

char txt;
int del = 250;
void setup()
{
  Serial.begin(9600);
  btSerial.begin(9600);
  pinMode(hdmi_1, OUTPUT);
  pinMode(hdmi_2, OUTPUT);
  // Inputs Multiplexer
  pinMode(mp_i_a, OUTPUT);
  pinMode(mp_i_b, OUTPUT);
  pinMode(mp_i_c, OUTPUT);
  pinMode(mp_i_io, OUTPUT);
  digitalWrite(mp_i_a, LOW);
  digitalWrite(mp_i_b, LOW);
  digitalWrite(mp_i_c, LOW);
  digitalWrite(mp_i_io, HIGH); // Disable the multiplexer initially
}

byte readInputMultiplexer()
{
  /***
   * MAP
   * X0 = Switch 1, HDMI 1
   * X1 = Switch 1, HDMI 2
   * X2 = Switch 1, HDMI 3
   * X3 = Switch 2, HDMI 1 
   * X4 = Switch 2, HDMI 2 
   * X5 = Switch 2, HDMI 3 
  ***/
  digitalWrite(mp_i_a, LOW);
  digitalWrite(mp_i_b, LOW);
  digitalWrite(mp_i_c, LOW);
  byte result = 0;
  for (int i = 0; i < 8; i++)
  {
    // Select the input using the control pins
    digitalWrite(mp_i_a, i & 0x1);
    digitalWrite(mp_i_b, i & 0x2);
    digitalWrite(mp_i_c, i & 0x4);

    // Enable the multiplexer
    digitalWrite(mp_i_io, LOW);

    // Read the input from the multiplexer and store it in a bit variable
    int bitValue = digitalRead(mp_i_input);

    // Use bitwise OR and left shift to store the bit in the result byte
    result |= (bitValue << i);
    // Disable the multiplexer
    digitalWrite(mp_i_io, HIGH);

    // Wait a short time before reading the next input
    delay(10);
  }
  return result;
}

void click(int hdmi)
{
  // Length of time "button" will be pressed for
  int del = 50;
  digitalWrite(hdmi, LOW);
  delay(del / 2);
  digitalWrite(hdmi, HIGH);
  delay(del);
  digitalWrite(hdmi, LOW);
  delay(del / 2);
}

void buttonPressed(char bt_input)
{
  String text = "";
  switch (bt_input)
  {
  case '1':
    text = "Change All, Input 1";
    break;
  case '2':
    text = "Change All, Input 2";
    break;
  case '3':
    text = "Change All, Input 3";
    break;
  case '4':
    text = "Change HDMI, Input 1";
    break;
  case '5':
    text = "Change HDMI, Input 2";
    break;
  case '6':
    text = "Change HDMI, Input 3";
    break;
  case '7':
    text = "Change USB, Input 1";
    break;
  case '8':
    text = "Change USB, Input 2";
    break;
  case '9':
    text = "Change USB, Input 3";
    break;
  case 'A':
    text = "Moving all";
    click(hdmi_1);
    click(hdmi_2);
    break;
  default:
    text = "This was pressed: ";
    break;
  }
  Serial.print(text);Serial.print("|");
  Serial.println(bt_input);
}

void loop()
{
  Serial.print("Input read: ");
  byte state = readInputMultiplexer();
  for (int i = 0; i < 8; i++) {
    // Use bitwise AND with a bit mask that has a 1 in the i-th bit and 0s elsewhere
    // This will extract the i-th bit and discard all other bits
    bool isBitSet = (state & (1 << i)) != 0;
    Serial.print(isBitSet);
}
  Serial.println();
  delay(500);
  if (btSerial.available()) {
    txt = btSerial.read();
    if (strchr("123456789*0#ABCD", txt)) buttonPressed(txt);
  }
}