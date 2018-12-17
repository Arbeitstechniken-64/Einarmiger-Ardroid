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
const int triggerPin = 6;
// Input to add 50 cents to the balance
const int fivetyCentPin = 5;
// Input to add one euro to the balance
const int oneEuroPin = 4;
// Input to add two euros to the balance
const int twoEurosPin = 3;

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
const int waitBeforeIdle = 10000;
// Time to display the spinup animation
const int spinTime = 5000;
// minimum ms per frame
const unsigned long topSpeed = 50;
const unsigned long minSpeed = 450;
const unsigned long startSpeed = 390;
const int minRandomAccell = 30;
const int maxRandomAccell = 40;
const int blinkTime = 250;

/*
Bit order from left to right

   --0--
  |     |
  1     2
  |     |
   --3--
  |     |
  5     4
  |     |
   --6--   7

*/
const int lroffsets[9] = {8,5,0,7,4,1,6,3,2};

const int idleAnimationAmount = 2;
const int idleAnimationSizes[2] = {8, 10};
//          [animation][frames][x][y]
const byte idleAnimations[2][10][3][3] = {
  {
    {
      {0b11111111, 0b00000000, 0b00000000},
      {0b00000000, 0b00010001, 0b00000000},
      {0b00000000, 0b00000000, 0b00010001},
    },
    {
      {0b00000000, 0b00000000, 0b00000000},
      {0b11111111, 0b00010001, 0b00010001},
      {0b00000000, 0b00000000, 0b00000000},
    },
    {
      {0b00000000, 0b00000000, 0b00010001},
      {0b00000000, 0b00010001, 0b00000000},
      {0b11111111, 0b00000000, 0b00000000},
    },
    {
      {0b00000000, 0b00010001, 0b00000000},
      {0b00000000, 0b00010001, 0b00000000},
      {0b00000000, 0b11111111, 0b00000000},
    },
    {
      {0b00010001, 0b00000000, 0b00000000},
      {0b00000000, 0b00010001, 0b00000000},
      {0b00000000, 0b00000000, 0b11111111},
    },
    {
      {0b00000000, 0b00000000, 0b00000000},
      {0b00010001, 0b00010001, 0b11111111},
      {0b00000000, 0b00000000, 0b00000000},
    },
    {
      {0b00000000, 0b00000000, 0b11111111},
      {0b00000000, 0b00010001, 0b00000000},
      {0b00010001, 0b00000000, 0b00000000},
    },
    {
      {0b00000000, 0b11111111, 0b00000000},
      {0b00000000, 0b00010001, 0b00000000},
      {0b00000000, 0b00010001, 0b00000000},
    },
  },
  {
    {
      {0b11000100, 0b10000000, 0b10101000},
      {0b01000100, 0b00000000, 0b00101000},
      {0b01000110, 0b00000010, 0b00101010},
    },
    {
      {0b00110010, 0b01111100, 0b01010010},
      {0b10010010, 0b00000000, 0b10010010},
      {0b10011000, 0b01111100, 0b10010100},
    },
    {
      {0b00001000, 0b00000010, 0b00000100},
      {0b00101000, 0b00000000, 0b01000100},
      {0b00100000, 0b10000000, 0b01000000},
    },
    {
      {0b00000000, 0b00000000, 0b00000000},
      {0b00000000, 0b11101110, 0b00000000},
      {0b00000000, 0b00000000, 0b00000000},
    },
    {
      {0b00000000, 0b00000000, 0b00000000},
      {0b00000000, 0b01111101, 0b00000000},
      {0b00000000, 0b00000000, 0b00000000},
    },
    {
      {0b00000000, 0b00000000, 0b00000000},
      {0b00000000, 0b11101110, 0b00000000},
      {0b00000000, 0b00000000, 0b00000000},
    },
    {
      {0b00001000, 0b00000010, 0b00000100},
      {0b00101000, 0b00000000, 0b01000100},
      {0b00100000, 0b10000000, 0b01000000},
    },
    {
      {0b00110010, 0b01111100, 0b01010010},
      {0b10010010, 0b00000000, 0b10010010},
      {0b10011000, 0b01111100, 0b10010100},
    },
    {
      {0b11000100, 0b10000000, 0b10101000},
      {0b01000100, 0b00000000, 0b00101000},
      {0b01000110, 0b00000010, 0b00101010},
    },
  },
};

// numbers in 7-segment form
const byte numbers[10] = {
  0b11101110, // 0
  0b00101000, // 1
  0b10110110, // 2
  0b10111010, // 3
  0b01111000, // 4
  0b11011010, // 5
  0b11011110, // 6
  0b10101000, // 7
  0b11111110, // 8
  0b11111010, // 9
};

const byte winSymbol = 0b01111100;
const byte loseSymbol = 0b00011110;

const byte scrolling[5] = {
  0b10000000,
  0b01100000,
  0b00010000,
  0b00001100,
  0b00000010,
};


// characters to display the word HELLO in 7-segment form
const byte hello[5] = {
  0b01111100, // H
  0b11010110, // E
  0b01000110, // L
  0b01000110, // L
  0b11101110, // O
};

const int segmentOrder[3][3] = {
  {8, 5, 0},
  {7, 4, 1},
  {6, 3, 2}
};

///////////////////////////////////////
////         State machine         ////
///////////////////////////////////////

enum State { OFF, IDLE, START_SPINNING, SPINUP, SPINNING, SPINDOWN, WAITING };

// the current state of the machine
// this has to be volatile according to the documentation for interrupts
volatile State currentState = OFF;

enum WinType { HTOP, HMID, HBOT, DTL, DTR, NONE };

///////////////////////////////////////
////          Variables            ////
///////////////////////////////////////

// Amount of money belonging to the human
// this has to be volatile according to the documentation for interrupts
volatile int balance = 0;

unsigned long spinEndTime = 0;

unsigned long nextStageTime = 0;
unsigned long nextUpdateTime = 0;
int currentIdleAnimation = 0;
int currentIdleAnimationFrame = 0;

WinType wintype = NONE;
int deltaBalance = 0;
int blinkBalance = 0;

int pos[3];
byte result[3][3];
unsigned long accel[3];
unsigned long speed[3];

unsigned long nextUpdate[3];
unsigned long debounceTime;


///////////////////////////////////////
////       Helper functions        ////
///////////////////////////////////////

void handleInterrupt() {
  if (debounceTime > millis()) {
    debounceTime = millis() + 1000;
    return;
  }
  if (!digitalRead(triggerPin)) {
    if (currentState == OFF) {
      Serial.println("ON");
      currentState = IDLE;
      debounceTime = millis() + 1000;
    } else if (currentState == IDLE || currentState == WAITING) {
      Serial.println("GO");
      currentState = START_SPINNING;
      debounceTime = millis() + 1000;
    }
  }
  if (currentState == IDLE || currentState == WAITING) {
    if (!digitalRead(fivetyCentPin)) {
      Serial.println("BUTTON 0.5");
      balance += 50;
      deltaBalance += 50;
      debounceTime = millis() + 1000;
    }
    if (!digitalRead(oneEuroPin)) {
      Serial.println("BUTTON 1");
      balance += 100;
      deltaBalance += 100;
      debounceTime = millis() + 1000;
    }
    if (!digitalRead(twoEurosPin)) {
      Serial.println("BUTTON 2");
      balance += 200;
      deltaBalance += 200;
      debounceTime = millis() + 1000;
    }
    Serial.println(balance);
  }
}

void printData(byte data[9]) {
  digitalWrite(latchPin, LOW);
  for (int i = 0; i < 9; i++) {
    shiftOut(dataPin, clockPin, LSBFIRST, data[i]);
  }
  digitalWrite(latchPin, HIGH);
}

void fillScreen(byte value) {
  digitalWrite(latchPin, LOW);
  for (int i = 0; i < 9; i++) {
    shiftOut(dataPin, clockPin, LSBFIRST, value);
  }
  digitalWrite(latchPin, HIGH);
}

void printHello(){
  byte output[9];
  for (int i = 0; i < 5; i++) {
    output[lroffsets[i]] = hello[i];
  }
  for (int i = 5; i < 9; i++) {
    output[lroffsets[i]] = 0b00000000;
  }
  printData(output);
}

void fillLoseSymbols(){
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      result[i][j] = loseSymbol;
    }
  }
}

void animateBalanceChange(){
  // todo
}

void animateBalanceBlink() {
  if (nextUpdateTime > millis()) {
    return;
  }
  nextUpdateTime = millis() + 150;
  blinkBalance--;
  // toggle balance visibility

}

void startSpinning() {
  randomSeed(millis());
  fillLoseSymbols();
  long randomOurcome = random(100);

  if (randomOurcome < 25) { // (25%)
    wintype = NONE;
    int emptyRow = random(3);
    for (int j = 0; j < 3; j++) {
      for (int i = 0; i < 3; i++) {
        if (j == emptyRow) {
          break;
        }
        result[0][i] = random(2) ? winSymbol : loseSymbol;
      }
    }
  } else if (randomOurcome < 50) { // (25%)
    if (random(2)) {
      wintype = DTL;
      result[0][0] = winSymbol;
      result[2][2] = winSymbol;
    } else {
      wintype = DTR;
      result[0][2] = winSymbol;
      result[2][0] = winSymbol;
    }
    result[1][1] = winSymbol;
  } else if (randomOurcome < 75) { // (25%)
    int height;
    if(random(2)){
        height = 0;
        wintype = HTOP;
    } else {
      height = 2;
      wintype = HBOT;
    }
    for (int i = 0; i < 3; i++) {
      result[i][height] = winSymbol;
    }
  } else {
    wintype = HMID; // (25%)
    for (int i = 0; i < 3; i++) {
      result[i][1] = winSymbol;
    }
  }

  Serial.print("Win: ");
  Serial.println(wintype);

  for (int i = 0; i < 3; i++) {
    accel[i] = random(minRandomAccell, maxRandomAccell);
    speed[i] = minSpeed;

    Serial.print(' ');
    for (int j = 0; j < 3; j++) {
      Serial.print(result[j][i] == winSymbol ? 'X' : '0');
      Serial.print(' ');
    }
    Serial.print(" | ");
    Serial.println(accel[i]);
    Serial.println("-------");
  }
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
      if (speed[i] < minSpeed) {
        speed[i] += accel[i];
      }
    }
    if (speed[0] >= minSpeed && speed[1] >= minSpeed && speed[2] >= minSpeed) {
      Serial.println("Round over!");
      switch (wintype) {
        case HMID:
          deltaBalance += 200;
          balance += 200;
          break;
        case HTOP:
        case HBOT:
          deltaBalance += 100;
          balance += 100;
          break;
        case DTL:
        case DTR:
          deltaBalance += 50;
          balance += 50;
        break;
        case NONE:
        default:
          break;
      }
      spinEndTime = millis() + waitBeforeIdle;
      currentState = WAITING;
    }
    nextUpdateTime = millis() + frameTime;
  }
}

void renderMatrix(byte matrix[3][3]) {
  byte output[9];

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      output[segmentOrder[i][j]] = matrix[i][j];
    }
  }
  printData(output);
}

void blinkWin() {
  byte data[3][3];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      data[i][j] = result[i][j];
    }
  }
  if (((spinEndTime - millis()) / blinkTime) % 2 == 0) {
    switch (wintype) {
      case HMID:
        data[1][0] = 0b00000000;
        data[1][1] = 0b00000000;
        data[1][2] = 0b00000000;
        break;
      case HTOP:
        data[0][0] = 0b00000000;
        data[0][1] = 0b00000000;
        data[0][2] = 0b00000000;
        break;
      case HBOT:
        data[2][0] = 0b00000000;
        data[2][1] = 0b00000000;
        data[2][2] = 0b00000000;
        break;
      case DTL:
        data[0][0] = 0b00000000;
        data[1][1] = 0b00000000;
        data[2][2] = 0b00000000;
        break;
      case DTR:
        data[0][2] = 0b00000000;
        data[1][1] = 0b00000000;
        data[2][0] = 0b00000000;
        break;
      case NONE:
      default:
        break;
    }
  }
  renderMatrix(data);
}


void nextIdleAnimationFrame() {
  if (currentIdleAnimationFrame >= idleAnimationSizes[currentIdleAnimation]) {
    currentIdleAnimation = (currentIdleAnimation + 1) % idleAnimationAmount;
    currentIdleAnimationFrame = 0;
  }

  byte matrix[3][3];
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      matrix[x][y] = idleAnimations[currentIdleAnimation][currentIdleAnimationFrame][x][y];
    }
  }
  renderMatrix(matrix);

  currentIdleAnimationFrame++;
}

void nextAnimationFrame() {
  for (int i = 0; i < 3; i++) {
    if (nextUpdate[i] < millis()) {
      pos[i] = (pos[i] + 1) % 5;
      nextUpdate[i] = millis() + speed[i];
    }
  }

  byte matrix[3][3];
  for (int i = 0; i < 3; i++) {
    // Serial.print(speed[i]);
    // Serial.print(' ');

    if (speed[i] > startSpeed) {
      for (int j = 0; j < 3; j++) {
        matrix[i][j] = result[j][i];
      }
    } else {
      matrix[0][i] = scrolling[(pos[i] - 1) % 5];
      matrix[1][i] = scrolling[pos[i]];
      matrix[2][i] = scrolling[(pos[i] + 1) % 5];
    }
  }
  // Serial.println();
  renderMatrix(matrix);
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

  Serial.begin(9600);
  Serial.print("Booting...");
  // for (int i = 0; i < 8; i++) {
  //   digitalWrite(latchPin, LOW);
  //   for (int j = 0; j < 9; j++) {
  //     shiftOut(dataPin, clockPin, LSBFIRST, 0b10000000 >> i);
  //   }
  //   digitalWrite(latchPin, HIGH);
  //   delay(1000);
  // }
  // delay(1000000);
  fillScreen(0b11111111);
  delay(500);
  fillScreen(0b00000000);
  delay(500);
  printHello();
  delay(2000);

  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  delay(100);

  Serial.println("Done!");
}

void loop() {
  if (deltaBalance != 0) {
    animateBalanceChange();
  }
  if (blinkBalance > 0) {
    animateBalanceBlink();
  }
  // Serial.println(currentState);
  switch (currentState) {
  case OFF:
    Serial.println("Clear");
    digitalWrite(latchPin, LOW);
    for (int i = 0; i < 9; i++) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
    }
    digitalWrite(latchPin, HIGH);
    delay(1000);
    break;
  case IDLE:
    if (nextUpdateTime < millis()) {
      Serial.println("IDLE");
      nextUpdateTime = millis() + 500;
      nextIdleAnimationFrame();
    }
    break;
  case START_SPINNING:
    Serial.print("Start: ");
    Serial.print(balance);
    if (balance >= 100) {
      balance -= 100;
      deltaBalance -= 100;
      startSpinning();
    } else {
      blinkBalance = 6;
      currentState = IDLE;
    }
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
  case WAITING:
    blinkWin();
    nextAnimationFrame();
    // do not play an animation
    if (millis() > spinEndTime) {
      currentState = IDLE;
    }
    break;
  }
}
