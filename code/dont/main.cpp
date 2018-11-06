#include <Arduino.h>

///////////////////////////////////////
////             Pins              ////
///////////////////////////////////////

// Serial data out to shift registers
const int dataPin = 8;
// Latch to update the current value of the registers
const int latchPin = 11;
// Clock for serial data to shift registers
const int clockPin = 12;

// Interrupt for buttons
const int interruptPin = 2;
// Input to start game
const int triggerPin = 5;
// Input to add 50 cents to the balance
const int fivetyCentPin = 6;
// Input to add one euro to the balance
const int oneEuroPin = 7;
// Input to add two euros to the balance
const int twoEurosPin = 8;

///////////////////////////////////////
////          Constants            ////
///////////////////////////////////////

// Time between frames 1000/50 = 20 fps
const int frameTime = 50;
// Time after a spin to wait before resuming idle animation
const int waitBeforeIdle = 10000;
// Time to display the spinup animation
const int spinupTime = 1500;
// Time to display the spindown animation
const int spindownTime = 1000;

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
const byte welcome[5] = {
    0b01111110, // H
    0b10111011, // E
    0b01000101, // L
    0b01000101, // L
    0b11100111, // O
};

///////////////////////////////////////
////         State machine         ////
///////////////////////////////////////

enum State { IDLE, SPINUP, SPINNING, SPINDOWN, WAITING };

// the current state of the machine
// this has to be volatile according to the documentation for interrupts
volatile State currentState = IDLE;

///////////////////////////////////////
////          Variables            ////
///////////////////////////////////////

// Amount of money belonging to the human
// this has to be volatile according to the documentation for interrupts
volatile float balance = 0.0;
//
unsigned long spinStartTime = 0;
unsigned long spinEndTime = 0;

int resultA = 0;
int resultB = 0;
int resultC = 0;

int speedA = 0;
int speedB = 0;
int speedC = 0;

///////////////////////////////////////
////       Helper functions        ////
///////////////////////////////////////

void handleInterrupt() {}

void showOutcome(int a, int b, int c) {
  // stop the 'rolls' and show outcome
}

void nextAnimationFrame() {

  // render next animation frame
}

void renderBalance() {
  // show balance on display
}

void spin() {
  currentState = SPINNING;
  resultA = random(0, 9);
  resultB = random(0, 9);
  resultC = random(0, 9);
  spinStartTime = millis();
  spinEndTime = millis() + spinupTime + (unsigned long)(random(5, 10) * 1000) +
                spindownTime;
}

///////////////////////////////////////
////          Main loop            ////
///////////////////////////////////////

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(interruptPin, INPUT);
  pinMode(triggerPin, INPUT);
  pinMode(fivetyCentPin, INPUT);
  pinMode(oneEuroPin, INPUT);
  pinMode(twoEurosPin, INPUT);

  for (unsigned int i = 0; i < sizeof(welcome); i++) {
    shiftOut(dataPin, clockPin, MSBFIRST, welcome[i]);
  }
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);

  delay(3000);
}

void loop() {
  switch (currentState) {
  case IDLE:
    // play idle animation
    break;
  case SPINUP:
    // play spinup animation
    break;
  case SPINNING:
    // play spinning animation
    break;
  case SPINDOWN:
    // play spindown animation
    break;
  case WAITING:
    // do not play an animation
    if (millis() > spinEndTime + waitBeforeIdle) {
      currentState = IDLE;
    }
    break;
  }
}
