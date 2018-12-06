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
const int interruptPin = 2;
// Input to start game
const int triggerPin = 3;
// Input to add 50 cents to the balance
const int fivetyCentPin = 4;
// Input to add one euro to the balance
const int oneEuroPin = 5;
// Input to add two euros to the balance
const int twoEurosPin = 6;

// 4 block 7-segment display clock
const int balanceClock = 12;
// 4 block 7-segment display data
const int balanceData = 13;

///////////////////////////////////////
////          Constants            ////
///////////////////////////////////////

// Time between frames 1000/50 = 20 fps
const int frameTime = 500;
// Time after a spin to wait before resuming idle animation
const int waitBeforeIdle = 5000;
// Time to display the spinup animation
const int spinTime = 5000;
// minimum ms per frame
const unsigned long topSpeed = 200;
const unsigned long almostMinSpeed = 800;
const unsigned long minSpeed = 1000;

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
const byte welcome[5] = {
    0b01111110, // H
    0b11011011, // E
    0b01000011, // L
    0b01000011, // L
    0b11100111, // O
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

unsigned long spinEndTime = 0;

unsigned long nextStageTime = 0;
unsigned long nextUpdateTime = 0;

int result[3];
unsigned long accel[3];
unsigned long speed[3];
boolean running[3];
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
    debounceTime = millis() + 100;
    return;
  }
  if (!digitalRead(triggerPin)) {
    if (currentState == OFF) {
      Serial.println("ON");
      currentState = IDLE;
      debounceTime = millis() + 500;
    } else if (currentState == IDLE || currentState == WAITING) {
      Serial.println("GO");
      currentState = START_SPINNING;
      debounceTime = millis() + 500;
    }
  }
  if (currentState == IDLE || currentState == WAITING) {
    if (!digitalRead(fivetyCentPin)) {
      Serial.println("BUTTON 0.5");
      balance += 50;
      debounceTime = millis() + 500;
    }
    if (!digitalRead(oneEuroPin)) {
      Serial.println("BUTTON 1");
      balance += 100;
      debounceTime = millis() + 500;
    }
    if (!digitalRead(twoEurosPin)) {
      Serial.println("BUTTON 2");
      balance += 200;
      debounceTime = millis() + 500;
    }
  }
}

void startSpinning() {
    for (int i = 0; i < 3; i++) {
      result[i] = random(10);
      accel[i] = random(20, 30);
      speed[i] = minSpeed;
      running[i] = true;

      Serial.print(result[i]);
      Serial.print(' ');
      Serial.print(accel[i]);
      Serial.print('|');
    }
    Serial.println();

    currentState = SPINUP;
}

void spinup() {
    if(millis() > nextUpdateTime) {
      for (int i = 0; i < 3; i++) {
        if (speed[i] > topSpeed) {
          speed[i] -= accel[i];
        }
      }
      nextUpdateTime = millis() + frameTime;
    }
    if (speed[0] <= topSpeed && speed[1] <= topSpeed && speed[2] <= topSpeed) {
        spinEndTime = millis() + spinTime;
        currentState = SPINNING;
    }
}

void spindown() {
    if(millis() > nextUpdateTime) {
      for (int i = 0; i < 3; i++) {
        if (speed[i] > almostMinSpeed && pos[i] == result[i]) {
          running[i] = false;
        }
        if (speed[i] < minSpeed) {
          speed[i] += accel[i];
        }
      }
      if (!running[0] && !running[1] && !running[2]) {
        Serial.println("Round over!");
        spinEndTime = millis() + waitBeforeIdle;
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
    if (running[i] && nextUpdate[i] < millis()) {
      pos[i] = (pos[i] + 1) % 10;
      nextUpdate[i] = millis() + speed[i];
      doRedraw = true;
    }
  }

  if (doRedraw) {
    // Serial.println("Draw");
    int order[3][3] = {
      {0, 1, 2},
      {3, 4, 5},
      {6, 7, 8}
    };
    byte output[9];

    for (int i = 0; i < 3; i++) {
      Serial.print(speed[i]);
      Serial.print(' ');
      int before = (pos[i] + 9) % 10;
      int after = (pos[i] + 1) % 10;

      output[order[i][0]] = characters[before];
      output[order[i][1]] = characters[pos[i]];
      output[order[i][2]] = characters[after];
    }
    Serial.println();

    digitalWrite(latchPin, LOW);
    for (int i = 0; i < 9; i++) {
      shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(output[i]));
    }
    digitalWrite(latchPin, HIGH);
  }
}


///////////////////////////////////////
////          Main loop            ////
///////////////////////////////////////

void setup() {
  randomSeed(analogRead(0));
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(fivetyCentPin, INPUT_PULLUP);
  pinMode(oneEuroPin, INPUT_PULLUP);
  pinMode(twoEurosPin, INPUT_PULLUP);

  Serial.begin(9600);

  delay(500);

  Serial.println("Boot");
  digitalWrite(latchPin, LOW);
  for (int i = 0; i < 9; i++) {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
  }
  digitalWrite(latchPin, HIGH);

  for (unsigned int i = 0; i< sizeof(welcome); i++) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, convertToOutput(welcome[i]));
    digitalWrite(latchPin, HIGH);
    delay(350);
  }
  for (unsigned int i = 0; i < 9; i++) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
    digitalWrite(latchPin, HIGH);
    delay(350);
  }
  Serial.println("Welcome");

  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  delay(2000);
}

void loop() {
  // Serial.println(currentState);
  switch (currentState) {
  case OFF:
    Serial.println("Clear");
    digitalWrite(latchPin, LOW);
    for (int i = 0; i < 9; i++) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
    }
    digitalWrite(latchPin, HIGH);
    delay(5000);
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
    if (millis() > spinEndTime) {
      currentState = IDLE;
    }
    break;
  }
}
