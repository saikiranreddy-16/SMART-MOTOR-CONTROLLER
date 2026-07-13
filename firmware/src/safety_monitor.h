#ifndef SAFETY_MONITOR_H
#define SAFETY_MONITOR_H

#include "config.h"

// Initialize Safety Monitor GPIOs and internal states
void initSafetyMonitor();

// Perform fast sensing calculations (RMS voltages, currents, temperatures)
// This should be called periodically by Core 1 Safety Task
void performSafetyCheck();

// Check if there is an active safety fault
bool isSafetyFaultActive();

// Get the active/last fault type
FaultType getActiveFault();

// Reset the safety lockout latch
void resetSafetyFault();

// Read current telemetry in a thread-safe manner
void getSystemTelemetry(SystemTelemetry &dest);

// Update user-configured thresholds dynamically
void updateSafetyThresholds(float nominal_curr, float under_v, float over_v);

// Telemetry state setters
void setSystemLanguage(Language lang);
void setSystemPumpType(PumpType type);
void setSystemPairedRemote(uint32_t remoteId);
void setSystemWaterLevel(float level);

// Retrieve multi-system version struct
VersionInfo getSystemVersion();

#endif // SAFETY_MONITOR_H
