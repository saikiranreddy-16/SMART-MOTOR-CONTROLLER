#ifndef SENSOR_CALIBRATION_H
#define SENSOR_CALIBRATION_H

#include "../config.h"

enum WaterSourceType {
    SOURCE_OPEN_WELL = 1,
    SOURCE_POND = 2,
    SOURCE_RESERVOIR = 3
};

struct LevelCalibration {
    uint16_t min_raw_adc;      // Raw ADC reading for 0% water level
    uint16_t max_raw_adc;      // Raw ADC reading for 100% water level
    WaterSourceType source_type;
};

// Initialize NVS storage and load calibration
void initLevelCalibration();

// Save new calibration bounds to NVS flash
void saveLevelCalibration(const LevelCalibration &cal);

// Read current active calibration
LevelCalibration getLevelCalibration();

#endif // SENSOR_CALIBRATION_H
