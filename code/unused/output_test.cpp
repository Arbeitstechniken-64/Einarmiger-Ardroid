#include <Arduino.h>

// Serial data out to shift registers
const int dataPin = 8;
// Latch to update the current value of the registers
const int latchPin = 11;
// Clock for serial data to shift registers
const int clockPin = 12;

/*
Bit order from left to right

   --0--
  |     |
  1     2
  |     |
   --3--
   --4--
  |     |
  5     6
  |     |
   --7--

*/
// Characters in 7-segment form
const byte characters[10] = {
    0b11100111, // 0
    0b00100010, // 1
    0b10111101, // 2
    0b10111011, // 3
    0b01111010, // 4
    0b11011011, // 5
    0b11011111, // 6
    0b10100010, // 7
    0b11111111, // 8
    0b11111011, // 9
};

// Characters to display the word HELLO in 7-segment form
const byte welcome[6] = {
    0b01111110, // H
    0b11011101, // E
    0b01000101, // L
    0b01000101, // L
    0b11100111, // O
    0b00000000, // space
};

unsigned int i = 0;

byte convertToOutput(byte data) {
  byte firstNibble = data & 0b11110000;
  byte lastNibble = (data & 0b00000111) << 1;

  return firstNibble | lastNibble;
}

byte convertToCircuit(byte data) {
  byte out = 0;
  int offsets[8] = {0, 4, -1, 3, -1, 3, 4};
  for (byte i = 0; i < 8; i++) {
    out |= (data & (0b10000000 >> i)) >> offsets[i];
  }
  return out;
}

void printBits(byte myByte) {
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
  Serial.println();
}

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // Serial.begin(9600);

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  digitalWrite(latchPin, HIGH);

  delay(1000);
}

void loop() {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(welcome[(i + 1) % 6]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(welcome[i % 6]));
  digitalWrite(latchPin, HIGH);

  i++;
  delay(500);
}
