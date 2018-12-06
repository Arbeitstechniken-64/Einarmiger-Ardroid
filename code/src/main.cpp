#include <Arduino.h>

///////////////////////////////////////
////             Pins              ////
///////////////////////////////////////

// Serial data out to shift registers
const int dataPin = 9;
// Latch to update the current value of the registers
const int latchPin = 10;
// Clock for serial data to shift registers
const int clockPin = 11;

// Interrupt for buttons
const int interruptPin = 1;
// Input to start game
const int triggerPin = 2;
// Input to add 50 cents to the balance
const int fivetyCentPin = 3;
// Input to add one euro to the balance
const int oneEuroPin = 4;
// Input to add two euros to the balance
const int twoEurosPin = 5;

const int balanceClock = 6;

const int balanceData = 7;

///////////////////////////////////////
////          Constants            ////
///////////////////////////////////////

// Time between frames 1000/50 = 20 fps
const int frameTime = 50;
// Time after a spin to wait before resuming idle animation
const int waitBeforeIdle = 5000;
// Time to display the spinup animation
const int spinupTime = 1500;
// Time to display the spindown animation
const int spindownTime = 1000;
// minimum ms per frame
const unsigned long topSpeed = 100;
const unsigned long minSpeed = 600;

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
int offsets[] = {0, 0, 0, 0, 1, -1, 0, 0};

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

///////////////////////////////////////
////         State machine         ////
///////////////////////////////////////

enum State { OFF, IDLE, START_SPINNING, SPINUP, SPINNING, SPINDOWN, RESULT, WAITING };

// the current state of the machine
// this has to be volatile according to the documentation for interrupts
volatile State currentState = OFF;

///////////////////////////////////////
////          Variables            ////
///////////////////////////////////////

// Amount of money belonging to the human
// this has to be volatile according to the documentation for interrupts
volatile int balance = 0;

unsigned long spinStartTime = 0;
unsigned long spinEndTime = 0;

unsigned long nextStageTime = 0;
unsigned long nextUpdateTime = 0;

int result[3];
unsigned long accel[3];
unsigned long speed[3];
unsigned long nextUpdate[3];
int pos[3];


///////////////////////////////////////
////       Helper functions        ////
///////////////////////////////////////

byte convertToOutput(byte data) {
  return (data & 0b11110000) | (data & 0b00000111) << 1;
}

/**

*/
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

/**
  Plays a 1-bit animation.
*/
byte animation(int frame, int offsets[], int size) {
  int x = frame % size;
    return (0b10000000 >> offsets[x]);
}


unsigned long debounceTime;

void handleInterrupt() {
  if (debounceTime > millis()) {
    debounceTime = millis() + 50;
    return;
  }
  debounceTime = millis() + 50;
  if (!digitalRead(triggerPin)) {
    if (currentState == OFF) {
      currentState = IDLE;
    } else if (currentState == IDLE || currentState == WAITING) {
      currentState = START_SPINNING;
    }
  }
  if (!digitalRead(fivetyCentPin)) {
    balance += 50;
  }
  if (!digitalRead(oneEuroPin)) {
    balance += 100;
  }
  if (!digitalRead(twoEurosPin)) {
    balance += 200;
  }
}

void startSpinning() {
    for (int i = 0; i < 3; i++) {
      result[i] = random(0, 9);
      accel[i] = random(7, 10);
    }
    spinStartTime = millis() + spinupTime;
    spinEndTime = millis() + spinupTime + (unsigned long)(random(3, 6) * 1000) + spindownTime;
    currentState = SPINUP;
}

void spinup() {
    if(millis() > nextUpdateTime) {
      for (int i = 0; i < 3; i++) {
        if (speed[i] < topSpeed) {
          speed[i] += accel[i];
        }
      }
      nextUpdateTime = millis() + frameTime;
    }
    if (millis() > spinStartTime) {
        currentState = SPINNING;
    }
}

void spindown() {
    if(millis() > nextUpdateTime) {
      for (int i = 0; i < 3; i++) {
        if (speed[i] < minSpeed) {
          speed[i] -= accel[i];
        } else if (pos[i] == result[i]) {
          speed[i] = 0;
        }
      }
      if (speed[0] == 0 && speed[1] == 0 && speed[2] == 0) {
        currentState = WAITING;
      }
      nextUpdateTime = millis() + frameTime;
    }
}

void nextIdleAnimationFrame() {

}

void nextAnimationFrame() {
  bool doRedraw = false;
  for (int i = 0; i < 3; i++) {
    if (nextUpdate[i] < millis()) {
      for (int i = 0; i < 3; i++) {
         pos[i] = (pos[i] + 1) % 10;
      }
      doRedraw = true;
    }
  }

  if (doRedraw) {
    int order[3][3] = {
      {0, 1, 2},
      {3, 4, 5},
      {6, 7, 8}
    };
    byte output[9];

    for (int i = 0; i < 3; i++) {
      int before = (pos[i] - 1) % 10;
      int after = (pos[i] + 1) % 10;

      output[order[i][0]] = characters[before];
      output[order[i][1]] = characters[pos[i]];
      output[order[i][2]] = characters[after];
    }

    for (int i = 0; i < 9; i++) {
      shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(output[i]));
    }
  }
}


///////////////////////////////////////
////          Main loop            ////
///////////////////////////////////////

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(fivetyCentPin, INPUT_PULLUP);
  pinMode(oneEuroPin, INPUT_PULLUP);
  pinMode(twoEurosPin, INPUT_PULLUP);



  for (unsigned int i = 0; i < sizeof(welcome); i++) {
    shiftOut(dataPin, clockPin, LSBFIRST, welcome[i]);
  }
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);

  delay(3000);
}

void loop() {
  switch (currentState) {
  case OFF:
    for (int i = 0; i < 9; i++) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
      delay(200);
    }
  case IDLE:
    nextIdleAnimationFrame();
    break;
  case START_SPINNING:
    startSpinning();
    break;
  case SPINUP:
    spinup();
    nextAnimationFrame();
    break;
  case SPINNING:
    if(millis() > nextUpdateTime) {
      nextUpdateTime = millis() + frameTime;
    }
    if (millis() > spinEndTime) {
        currentState = SPINDOWN;
    }
    nextAnimationFrame();
    break;
  case SPINDOWN:
    spindown();
    nextAnimationFrame();
    break;
  case RESULT:
    break;
  case WAITING:
    // do not play an animation
    if (millis() > spinEndTime + waitBeforeIdle) {
      currentState = IDLE;
    }
    break;
  }
}
