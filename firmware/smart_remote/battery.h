#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

#define PIN_BATTERY_ADC  A0

void initBattery();
int getBatteryPercent();

#endif // BATTERY_H
