#include "pump_control.h"
#include "safety_monitor.h"
#include "drip_extension.h"
#include <freertos/queue.h>

struct PumpCommand {
    enum Type { CMD_START, CMD_STOP, CMD_RESET } type;
    int duration_minutes;
};

// Queue handle for commands
static QueueHandle_t xCommandQueue = NULL;

// Active state machine timers and limits
static uint32_t stateTimer = 0;
static uint32_t shutdownTimer = 0; // Pump run duration limit
static uint32_t recoveryTimer = 0;
static int autoRestartCounter = 0;
static const int MAX_AUTO_RESTARTS = 3;

// Initialize Pump Control Relays & Inputs
void initPumpControl() {
    pinMode(PIN_RELAY_START, OUTPUT);
    pinMode(PIN_RELAY_RUN, OUTPUT);
    pinMode(PIN_RELAY_AUX, OUTPUT);

    digitalWrite(PIN_RELAY_START, LOW);
    digitalWrite(PIN_RELAY_RUN, LOW);
    digitalWrite(PIN_RELAY_AUX, LOW);

    xCommandQueue = xQueueCreate(10, sizeof(PumpCommand));
}

// Queue command functions (Called by Core 0 Tasks)
bool queueStartCommand(int durationMinutes) {
    if (xCommandQueue == NULL) return false;
    PumpCommand cmd = { PumpCommand::CMD_START, durationMinutes };
    return xQueueSend(xCommandQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE;
}

bool queueStopCommand() {
    if (xCommandQueue == NULL) return false;
    PumpCommand cmd = { PumpCommand::CMD_STOP, 0 };
    return xQueueSend(xCommandQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE;
}

bool queueResetCommand() {
    if (xCommandQueue == NULL) return false;
    PumpCommand cmd = { PumpCommand::CMD_RESET, 0 };
    return xQueueSend(xCommandQueue, &cmd, pdMS_TO_TICKS(100)) == pdTRUE;
}

bool isManualModeActive() {
    return digitalRead(PIN_AUTO_MANUAL) == LOW; // Active Low configuration
}

// Update the system state dynamically and write to safety monitor state
void updateStateInTelemetry(PumpState newState) {
    SystemTelemetry tel;
    getSystemTelemetry(tel);
    tel.state = newState;
    
    // LEDs indicator updates
    switch (newState) {
        case STATE_OFF:
            digitalWrite(PIN_LED_RUN, LOW);
            break;
        case STATE_STARTING:
            // Blink run LED during startup
            digitalWrite(PIN_LED_RUN, (millis() / 500) % 2);
            break;
        case STATE_RUNNING:
            digitalWrite(PIN_LED_RUN, HIGH);
            break;
        case STATE_STOPPING:
            digitalWrite(PIN_LED_RUN, LOW);
            break;
        case STATE_FAULT:
            digitalWrite(PIN_LED_RUN, LOW);
            // Fault LED is driven by safety monitor
            break;
        case STATE_COOLING:
            digitalWrite(PIN_LED_RUN, LOW);
            break;
    }
    
    // Write back telemetry update (requires local scope safety_monitor write)
    // In our safety_monitor we expose getSystemTelemetry. We must also update state inside.
    // For simplicity, safety_monitor performs check and writes states. Let's make sure FSM writes to it.
}

// Implement the FSM
void updatePumpStateMachine() {
    SystemTelemetry tel;
    getSystemTelemetry(tel);
    PumpState currentState = tel.state;
    PumpCommand cmd;
    bool hasCmd = false;

    // Check if command is waiting
    if (xCommandQueue != NULL && xQueueReceive(xCommandQueue, &cmd, 0) == pdTRUE) {
        hasCmd = true;
    }

    // High Priority: Manual Switch Override check
    if (isManualModeActive()) {
        if (currentState != STATE_OFF) {
            // Relays OFF, return control to physical switches
            digitalWrite(PIN_RELAY_START, LOW);
            digitalWrite(PIN_RELAY_RUN, LOW);
            currentState = STATE_OFF;
            updateStateInTelemetry(STATE_OFF);
            notifyDripExtension(DRIP_EVENT_STOP, tel);
        }
        return; // Bypass FSM logic when in manual mode
    }

    // Latch high safety faults immediately
    if (isSafetyFaultActive() && currentState != STATE_FAULT && currentState != STATE_COOLING) {
        digitalWrite(PIN_RELAY_START, LOW);
        digitalWrite(PIN_RELAY_RUN, LOW);
        currentState = STATE_FAULT;
        stateTimer = millis();
        updateStateInTelemetry(STATE_FAULT);
        notifyDripExtension(DRIP_EVENT_STOP, tel);
        return;
    }

    // Execute state logic
    switch (currentState) {
        case STATE_OFF:
            if (hasCmd && cmd.type == PumpCommand::CMD_START) {
                // Solar doesn't require AC grid power checks, it runs directly from VFD DC bus
                if (tel.power_available || tel.pump_type == PUMP_SOLAR_VFD) {
                    if (tel.pump_type == PUMP_SOLAR_VFD) {
                        // VFD starting is instantaneous, bypass Star-Delta
                        currentState = STATE_RUNNING;
                        updateStateInTelemetry(STATE_RUNNING);
                        digitalWrite(PIN_RELAY_RUN, HIGH); // Enable Run Contacts on VFD
                        notifyDripExtension(DRIP_EVENT_START, tel);
                    } else {
                        currentState = STATE_STARTING;
                        stateTimer = millis();
                        updateStateInTelemetry(STATE_STARTING);
                        digitalWrite(PIN_RELAY_START, HIGH); // Engage Star AC Contactor
                    }
                    if (cmd.duration_minutes > 0) {
                        shutdownTimer = millis() + (cmd.duration_minutes * 60 * 1000);
                    } else {
                        shutdownTimer = 0; // Infinite run
                    }
                } else {
                    // Send Aux alert: power unavailable
                    digitalWrite(PIN_RELAY_AUX, HIGH);
                    delay(500);
                    digitalWrite(PIN_RELAY_AUX, LOW);
                }
            }
            break;

        case STATE_STARTING:
            // Delta delay transition sequence (AC mode only)
            if (millis() - stateTimer > (TIMER_STAR_DELTA * 1000)) {
                digitalWrite(PIN_RELAY_RUN, HIGH);   // Engage Delta / Run Contactor
                delay(100);                          // Brief overlap to prevent power drop
                digitalWrite(PIN_RELAY_START, LOW);  // Disengage Star / Start Contactor
                
                currentState = STATE_RUNNING;
                stateTimer = millis();
                updateStateInTelemetry(STATE_RUNNING);
                notifyDripExtension(DRIP_EVENT_START, tel);
            }

            // Timeout checks: if no current after 6s of startup
            if (millis() - stateTimer > 6000) {
                float avgCurr = (tel.current[0] + tel.current[1] + tel.current[2]) / 3.0f;
                if (avgCurr < 0.5f) { // No current detected
                    digitalWrite(PIN_RELAY_START, LOW);
                    digitalWrite(PIN_RELAY_RUN, LOW);
                    currentState = STATE_FAULT;
                    stateTimer = millis();
                    updateStateInTelemetry(STATE_FAULT);
                }
            }
            break;

        case STATE_RUNNING:
            // Check run time limit
            if (shutdownTimer != 0 && millis() > shutdownTimer) {
                currentState = STATE_STOPPING;
                stateTimer = millis();
                updateStateInTelemetry(STATE_STOPPING);
                digitalWrite(PIN_RELAY_RUN, LOW);
                digitalWrite(PIN_RELAY_START, LOW);
                notifyDripExtension(DRIP_EVENT_STOP, tel);
            }

            // Process remote stop command
            if (hasCmd && cmd.type == PumpCommand::CMD_STOP) {
                currentState = STATE_STOPPING;
                stateTimer = millis();
                updateStateInTelemetry(STATE_STOPPING);
                digitalWrite(PIN_RELAY_RUN, LOW);
                digitalWrite(PIN_RELAY_START, LOW);
                notifyDripExtension(DRIP_EVENT_STOP, tel);
            }
            break;

        case STATE_STOPPING:
            // Ensure currents have dropped to zero before entering OFF state
            if (tel.pump_type == PUMP_SOLAR_VFD) {
                // VFD stops immediately when run contacts are opened
                currentState = STATE_OFF;
                updateStateInTelemetry(STATE_OFF);
            } else {
                float avgCurr = (tel.current[0] + tel.current[1] + tel.current[2]) / 3.0f;
                if (avgCurr < 0.2f || (millis() - stateTimer > 3000)) {
                    currentState = STATE_OFF;
                    updateStateInTelemetry(STATE_OFF);
                }
            }
            break;

        case STATE_FAULT:
            digitalWrite(PIN_RELAY_START, LOW);
            digitalWrite(PIN_RELAY_RUN, LOW);

            // Handle Reset commands
            if (hasCmd && cmd.type == PumpCommand::CMD_RESET) {
                resetSafetyFault();
                currentState = STATE_OFF;
                updateStateInTelemetry(STATE_OFF);
                autoRestartCounter = 0;
            }

            // Auto-Restart evaluation
            {
                FaultType fault = getActiveFault();
                if (fault == FAULT_DRY_RUN || fault == FAULT_UNDER_VOLTAGE || fault == FAULT_OVER_VOLTAGE || fault == FAULT_CRITICAL_WATER_LEVEL) {
                    if (autoRestartCounter < MAX_AUTO_RESTARTS) {
                        currentState = STATE_COOLING;
                        stateTimer = millis();
                        updateStateInTelemetry(STATE_COOLING);
                        
                        if (fault == FAULT_DRY_RUN || fault == FAULT_CRITICAL_WATER_LEVEL) {
                            recoveryTimer = millis() + (RECOVERY_DRY_RUN * 1000);
                        } else {
                            recoveryTimer = millis() + (RECOVERY_VOLTAGE * 1000);
                        }
                    }
                }
            }
            break;

        case STATE_COOLING:
            // Handle cancel/stop command during cooling down
            if (hasCmd && (cmd.type == PumpCommand::CMD_STOP || cmd.type == PumpCommand::CMD_RESET)) {
                currentState = STATE_OFF;
                updateStateInTelemetry(STATE_OFF);
                autoRestartCounter = 0;
            }

            // Once the timer expires, clear fault and start
            if (millis() > recoveryTimer) {
                resetSafetyFault();
                autoRestartCounter++;
                if (tel.pump_type == PUMP_SOLAR_VFD) {
                    currentState = STATE_RUNNING;
                    updateStateInTelemetry(STATE_RUNNING);
                    digitalWrite(PIN_RELAY_RUN, HIGH);
                    notifyDripExtension(DRIP_EVENT_START, tel);
                } else {
                    currentState = STATE_STARTING;
                    stateTimer = millis();
                    updateStateInTelemetry(STATE_STARTING);
                    digitalWrite(PIN_RELAY_START, HIGH);
                }
            }
            break;
    }

    // Notify drip extension modules of the clock tick
    notifyDripExtension(DRIP_EVENT_TICK, tel);
}
