#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

#define PIN_START_BTN  3
#define PIN_STOP_BTN   4
#define PIN_PAIR_BTN   5

void initButtons();
bool isStartButtonPressed();
bool isStopButtonPressed();
bool isPairButtonPressed();

#endif // BUTTONS_H
