#include "buttons.h"

void initButtons() {
    pinMode(PIN_START_BTN, INPUT_PULLUP);
    pinMode(PIN_STOP_BTN, INPUT_PULLUP);
    pinMode(PIN_PAIR_BTN, INPUT_PULLUP);
}

static bool checkDebouncedButton(uint8_t pin) {
    if (digitalRead(pin) == LOW) {
        delay(20); // Debounce delay
        if (digitalRead(pin) == LOW) {
            return true;
        }
    }
    return false;
}

bool isStartButtonPressed() {
    return checkDebouncedButton(PIN_START_BTN);
}

bool isStopButtonPressed() {
    return checkDebouncedButton(PIN_STOP_BTN);
}

bool isPairButtonPressed() {
    return checkDebouncedButton(PIN_PAIR_BTN);
}
