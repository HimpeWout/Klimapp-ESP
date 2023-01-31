#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

void setup() {
#ifndef __AVR_ATtiny85__
  Serial.begin(115200);
  Serial.println("7 Segment Backpack Test");
#endif
  Wire.begin (0, 4);
  matrix.begin(0x70);
}

void loop() {
  // try to print a number thats too long
  matrix.print(1000, DEC);
  Serial.println("a");
  matrix.writeDisplay();
  delay(500);
}