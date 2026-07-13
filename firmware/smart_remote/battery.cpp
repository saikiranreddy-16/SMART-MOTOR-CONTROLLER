#include "battery.h"

void initBattery() {
    pinMode(PIN_BATTERY_ADC, INPUT);
}

int getBatteryPercent() {
    int raw = analogRead(PIN_BATTERY_ADC);
    
    // Scale: divider reduces voltage by half, reference is 5.0V
    float voltage = (float)raw * (5.0f / 1023.0f) * 2.0f;
    
    // Li-Po bounds: 4.2V is 100%, 3.2V is 0%
    float percent = ((voltage - 3.2f) / (4.2f - 3.2f)) * 100.0f;
    
    int pct = (int)percent;
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    
    return pct;
}
