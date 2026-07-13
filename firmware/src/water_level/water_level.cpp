#include "water_level.h"
#include "sensor_calibration.h"
#include <Arduino.h>

static float filteredADC = 400.0f; // Initialized to default min bounds
static const float EMA_ALPHA_OPEN_WELL = 0.05f;
static const float EMA_ALPHA_POND = 0.10f;
static const float EMA_ALPHA_RESERVOIR = 0.20f;

void initWaterLevelSensor() {
    // Configure input pins
    pinMode(PIN_FLOAT_LOW, INPUT_PULLUP);
    pinMode(PIN_FLOAT_HIGH, INPUT_PULLUP);
    
    initLevelCalibration();
    
    // Seed initial reading
    filteredADC = analogRead(PIN_FLOAT_LOW); // Pin low acts as analog read in level mode
}

float readWaterLevelPercent() {
    LevelCalibration cal = getLevelCalibration();
    
    // 1. Read raw sensor input
    // Supports 4-20mA pressure probes mapped to ESP32 ADC pins
    uint16_t rawAdc = analogRead(PIN_FLOAT_LOW); 
    
    // 2. Resolve filter coefficient (alpha) based on water source characteristics
    float alpha = EMA_ALPHA_OPEN_WELL;
    if (cal.source_type == SOURCE_POND) {
        alpha = EMA_ALPHA_POND; // Filter wave noise
    } else if (cal.source_type == SOURCE_RESERVOIR) {
        alpha = EMA_ALPHA_RESERVOIR; // Responsive to fast filling
    }
    
    // 3. Apply EMA filter
    filteredADC = (alpha * (float)rawAdc) + ((1.0f - alpha) * filteredADC);
    
    // 4. Calculate percentage level based on calibration bounds
    if (cal.max_raw_adc == cal.min_raw_adc) return 0.0f;
    
    float level = ((filteredADC - cal.min_raw_adc) / (float)(cal.max_raw_adc - cal.min_raw_adc)) * 100.0f;
    
    // Bound outputs between 0% and 100%
    if (level < 0.0f) level = 0.0f;
    if (level > 100.0f) level = 100.0f;
    
    // Hysteresis backup: if digital backup float switch indicates empty, force 0%
    if (digitalRead(PIN_FLOAT_LOW) == LOW && cal.source_type == SOURCE_OPEN_WELL) {
        level = 0.0f; 
    }
    
    return level;
}
