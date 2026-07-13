#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include "../config.h"

// Set up sensor interfaces (analog pins or RS485 transceiver pins)
void initWaterLevelSensor();

// Read, filter, and calculate depth percentage (0.0 to 100.0) based on source characteristics
float readWaterLevelPercent();

#endif // WATER_LEVEL_H
