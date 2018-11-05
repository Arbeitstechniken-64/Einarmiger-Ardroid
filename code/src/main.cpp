#include <Arduino.h>

const int frameTime = 50;
const byte animations[1] = {0b11111111};
const byte characters[1] = {0b10101010};

float balance = 0.0;
int currentAnimationFrame = 0;
int currentAnimation = 0;
int framesSinceLastPlay = 0;

void setup() {
  // setup
}

void showOutcome(int a, int b, int c) {
  // stop the 'rolls' and show outcome
}

bool isLeverPulled() {
  return false;
  // add proper check later (or handle with interrupt)
}

bool hasAdded50Cent() { return false; }
bool hasAdded1Euro() { return false; }
bool hasAdded2Euros() { return false; }

void nextAnimationFrame() {
  // render next animation frame
}

void renderBalance() {
  // show balance on display
}

void addWinningsToBalance() { balance += 0.5; }

void incrementFunds(float addedBalance) {
  balance += addedBalance;
  renderBalance();
}

void spin() {
  int resultA = random(0, 9);
  int resultB = random(0, 9);
  int resultC = random(0, 9);
  int spinningTime = random(5, 10);
  int currentTime = 0;

  while (currentTime < spinningTime) {
    nextAnimationFrame();
    delay(frameTime);
    spinningTime += frameTime;
  }
  showOutcome(resultA, resultB, resultC);
  int someTime = 7;
  delay(someTime);
  if (winner) {
    addWinningsToBalance();
  }
}

void loop() {
  if (isLeverPulled()) {
    spin();
  } else if (hasAdded50Cent()) {
    incrementFunds(0.5);
  } else if (hasAdded1Euro()) {
    incrementFunds(1.0);
  } else if (hasAdded2Euros()) {
    incrementFunds(2.0);
  } else if (framesSinceLastPlay > 2048 &&
             balance <= 0.0) { // wait a bit after the last game
    nextAnimationFrame();
  } else {
    delay(frameTime);
  }
}
