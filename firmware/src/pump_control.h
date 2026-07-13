#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include "config.h"

// Initialize pump control systems and contactors
void initPumpControl();

// Core process loop for the pump FSM (State Machine)
// This should be called periodically by Core 1 State Machine Task
void updatePumpStateMachine();

// Queue a command to start the motor
bool queueStartCommand(int durationMinutes);

// Queue a command to stop the motor
bool queueStopCommand();

// Queue a command to reset any safety faults
bool queueResetCommand();

// Check if manual override switch is set to MANUAL mode
bool isManualModeActive();

#endif // PUMP_CONTROL_H
