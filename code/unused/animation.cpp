#include <Arduino.h>

// Serial data out to shift registers
const int dataPin = 9;
// Latch to update the current value of the registers
const int latchPin = 10;
// Clock for serial data to shift registers
const int clockPin = 11;

/*
Bit order from left to right

   --0--
  |     |
  1     2
  |     |
   --3--
   --4--
  |     |
  6     5
  |     |
   --7--

*/
// Characters in 7-segment form
const byte characters[10] = {
    0b11100111, // 0
    0b00100100, // 1
    0b10111011, // 2
    0b10111101, // 3
    0b01111100, // 4
    0b11011101, // 5
    0b11011111, // 6
    0b10100100, // 7
    0b11111111, // 8
    0b11111101, // 9
};

// Propper offsets from top to bottom:
// int offsets[] = {0, 0, 0, 0, 1, -1, 0, 0};

// Characters to display the word HELLO in 7-segment form
const byte welcome[6] = {
    0b01111110, // H
    0b11011101, // E
    0b01000011, // L
    0b01000011, // L
    0b11100111, // O
    0b00000000, // space
};

const int loadingSpinnerAnimation[6] = {0, 0, 3, 3, 0, -3};

byte convertToOutput(byte data) {
  return (data & 0b11110000) | (data & 0b00000111) << 1;
}

byte shiftBits(byte data, int *offsets) {
  byte out = 0;
  for (byte i = 0; i < sizeof(offsets); i++) {
    if (offsets[i] > 0) {
      out |= (data & (0b10000000 >> i)) >> offsets[i];
    } else {
      out |= (data & (0b10000000 >> i)) << -offsets[i];
    }
  }
  return out;
}

byte animation(int frame, int offsets[], int size) {
  int x = frame % size;

  if (offsets[x] > 0) {
    return (0b10000000 >> x) >> offsets[x];
  } else {
    return (0b10000000 >> x) << -offsets[x];
  }
}

void test(int x[], int xSize) {}
void printBits(byte myByte) {
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // Serial.begin(9600);

  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b11111111));
  digitalWrite(latchPin, HIGH);
  delay(1000);

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(0b00000000));
  digitalWrite(latchPin, HIGH);
  delay(1000);

  // Serial.println("I'm alive!");

  // printBits(shiftBits(0b01010101, new int[8]{0, 0, 0, 0, 0, 0, 0, 0}));
}

unsigned int i = 0;

void loop() {
  //   Serial.print(i);
  // Serial.print(" : ");
  //

  digitalWrite(latchPin, LOW);
  // shiftOut(dataPin, clockPin, LSBFIRST, animation(i, loadingSpinnerAnimation,
  // 6));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(characters[i % 10]));
  digitalWrite(latchPin, HIGH);
  // delay(1000);
  // digitalWrite(latchPin, LOW);
  // // shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
  //
  // shiftOut(dataPin, clockPin, LSBFIRST, animation(i));
  //
  // digitalWrite(latchPin, HIGH);
  // shiftOut(dataPin, clockPin, LSBFIRST, 0b10000000 >> (i % 8));
  // animationOne(0b10000000 >> (i % 8)));
  // shiftOut(dataPin, clockPin, LSBFIRST, animationOne(0b10000000 >> (i % 8)));
  // printBits(0b10000000 >> (i % 7));
  // Serial.print(' ');
  // printBits(animation(i));
  // Serial.println();

  i++;
  delay(200);
}
