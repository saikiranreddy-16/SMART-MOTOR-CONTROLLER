#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void initDisplay();
void drawDashboard(const String &statusText, int waterLevel, bool powerAvailable, int batteryLevel, bool paired);
void drawTextScreen(const String &line1, const String &line2);

#endif // DISPLAY_H
