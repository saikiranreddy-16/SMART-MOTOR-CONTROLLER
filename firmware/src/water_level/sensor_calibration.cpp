#include "sensor_calibration.h"
#include <Preferences.h>

static Preferences prefs;
static LevelCalibration activeCal = { 400, 2000, SOURCE_OPEN_WELL }; // Default settings

void initLevelCalibration() {
    prefs.begin("water", false);
    
    activeCal.min_raw_adc = prefs.getUShort("min_adc", 400);
    activeCal.max_raw_adc = prefs.getUShort("max_adc", 2000);
    
    uint16_t srcVal = prefs.getUShort("source", 1);
    if (srcVal == 2) activeCal.source_type = SOURCE_POND;
    else if (srcVal == 3) activeCal.source_type = SOURCE_RESERVOIR;
    else activeCal.source_type = SOURCE_OPEN_WELL;
    
    prefs.end();
    Serial.println("Water level calibration loaded from NVS.");
}

void saveLevelCalibration(const LevelCalibration &cal) {
    prefs.begin("water", false);
    
    prefs.putUShort("min_adc", cal.min_raw_adc);
    prefs.putUShort("max_adc", cal.max_raw_adc);
    prefs.putUShort("source", (uint16_t)cal.source_type);
    
    prefs.end();
    activeCal = cal;
    Serial.println("Level calibration bounds saved to NVS.");
}

LevelCalibration getLevelCalibration() {
    return activeCal;
}
