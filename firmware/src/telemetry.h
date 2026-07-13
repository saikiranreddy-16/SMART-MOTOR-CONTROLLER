#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "config.h"

// Initialize WiFi and BLE telemetry systems
void initTelemetry();

// Periodically run BLE/WiFi telemetry updates
// This should be called by Core 0 Telemetry Task
void processTelemetryTask();

#endif // TELEMETRY_H
