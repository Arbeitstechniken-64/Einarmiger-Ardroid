#include "Arduino"

const int frameTime = 50;
const byte[]
animations = {};
const byte[]
characters = {};

float balance = 0.0;
int currentAnimationFrame = 0;
int currentAnimation = 0;
int framesSinceLastPlay = 0;

void setup() {
    // setup
}

void loop() {
    if (isLeverUplled()) {
        spin();
    } else if (hasAdded50Cent()) {
        incrementFunds(0.5);
    } else if (hasAdded1Euro()) {
        incrementFunds(1.0);
    } else if (hasAdded2Euros()) {
        incrementFunds(2.0);
    } else if (framesSinceLastPlay > 2048 && funds <= 0.0) { // wait a bit after the last game
        nextAnimationFrame();
    } else {
        delay(frameTime);
    }
}

void spin() {
    int resultA = rand(0, 9);
    int resultB = rand(0, 9);
    int resultC = rand(0, 9);
    int spinningTime = rand(5, 10);
    int currentTime = 0;
    setAnimation('spinning');
    while (currentTime < spinningTime) {
        nextAnimationFrame();
        delay(frameTime);
        spinningTime += frameTime;
    }
    showOutcome(resultA, resultB, resultC);
    delay(someTime);
    if (winner) {
        addWinningsToBalance();
    }
}

bool isLeverPulled() {
    return false;
    // add proper check later (or handle with interrupt)
}

void nextAnimationFrame() {
    // render next animation frame
}

void showOutcome(int a, b, c) {
    // stop the 'rolls' and show outcome
}

void incrementFunds(float addedBalance) {
    balance += addedBalance;
    renderBalance();
}

void renderBalance() {
    // show balance on display
}
