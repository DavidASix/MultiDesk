#include <Arduino.h>
#include "SoftwareSerial.h"

SoftwareSerial btSerial(3, 2); // TX RX
int hdmi_1 = 4;
int hdmi_2 = 5;
int hdmi_3 = 6;
int hdmis[3] = {hdmi_1, hdmi_2, hdmi_3};
int mp_i_io = 13;
int mp_i_a = 12;
int mp_i_b = 11;
int mp_i_c = 10;
int mp_i_input = A0;

char txt;
int del = 250;
byte prevState = 0;

int selected_hdmi = 0;
int selected_usb = 0;
byte selected_hdmi_state;

int display_1_state = 0;
int display_2_state = 0;
int usb_state = 0;

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

void printBits(byte j) {
  for (int i = 0; i < 8; i++)
  {
    Serial.print(j >> i & 1);
  }
  Serial.println();
  
}

/**
 * This function simulates a click on any given pin.
 * Pin ints used as arguments should corrospond to the center pin on a MOSFET
**/
void click(int pin)
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(pin, i % 2);
    // delay for 50ms after setting to high, and half that after setting to low
    delay(50 / (((i+1) % 2) + 1));
  }
}

byte getCurrentState()
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

/**
 * This function checks each HDMI connection and lists which ports currently have a connection 
 * Running this function WILL cycle the screens, so they all turn black momentarily.
**/
byte listConnectedDevices() {
  byte states[3];
  for (int i = 0; i<3; i++) {
    states[i]  = getCurrentState();
    click(hdmis[0]);
    click(hdmis[1]);
    delay(100);
  }
  byte availble_hdmi = states[0] | states[1] | states[2];
  return availble_hdmi;
}

/**
 * This function is called in a while loop after any bluetooth interaction.
 * While the desired state != actual state, recurse.
**/
void moveDisplayInputs(byte connected, byte state) {
  for (int i = 0; i < 2; i++)
  {
      if (((connected >> (i*3)) & selected_hdmi_state) == 0) {
      // Selected input is not plugged in, nothing to switch to.
      // In this case, set all selected_hdmi_State bits to high. This makes any bitwise & logic default !=0 to true, which breaks the while loop.
      selected_hdmi_state = 0b111;
    } else if (((state >> (i*3)) & selected_hdmi_state) == 0) {
      // Selected input IS plugged in, but is not currently active. Click once.
      Serial.print("Clicking hdmi "); Serial.print(hdmis[i]);
      click(hdmis[i]);
    }
  }
  // else, selected input is active.
}

void btTextReceived(char bt_input)
{
  if (strchr("147", bt_input)) {
    selected_hdmi_state = 0b001;
  } else if (strchr("258", bt_input)) {
    selected_hdmi_state = 0b010;
  } else if (strchr("369", bt_input)) {
    selected_hdmi_state = 0b100;
  } else if (bt_input == 'A') {
    click(hdmis[0]);
    click(hdmis[1]);
  }
}

void loop()
{
  // Read the incoming bluetooth signals
  // strchr condition is in there to avoid accidental calls to btTextReceived when receiving line ends or BT disconnects
  if (btSerial.available()) {
    txt = btSerial.read();
    if (strchr("123456789*0#ABCD", txt)) btTextReceived(txt);

    byte state = getCurrentState();
    // EDIT: if state = desired state, don't enter loop.
    
    byte connected = listConnectedDevices();
    // Do not enter while loop if no devices are connected, as it  will loop infinitely
    if (connected == 0) {
      Serial.println("No connected devices");
      return;
    }

    while (((selected_hdmi_state & state) == 0) && ((selected_hdmi_state & (state >> 3)) == 0))
    {
      moveDisplayInputs(connected, state);
      state = getCurrentState();
    }
  }
}