#include "safety_monitor.h"
#include <freertos/semphr.h>
#include <math.h>
#include <Preferences.h>
#include "water_level/water_level.h"

static Preferences preferences;

// Semaphore for thread-safe access to telemetry
static SemaphoreHandle_t xTelemetryMutex = NULL;
static SystemTelemetry currentTelemetry;

// Internal safety limits (dynamically configurable)
static float nominalCurrent = DEFAULT_NOMINAL_CURR;
static float underVoltageLimit = DEFAULT_UNDER_VOLT;
static float overVoltageLimit = DEFAULT_OVER_VOLT;
static float dryRunMultiplier = DEFAULT_DRY_RUN_MUL;

// Safety accumulators and trip timers
static float thermalAccumulator = 0.0f;
static const float THERMAL_TRIP_LIMIT = 300.0f; // Represents maximum thermal capacity
static uint32_t underVoltageTripTimer = 0;
static uint32_t overVoltageTripTimer = 0;
static uint32_t dryRunTripTimer = 0;
static uint32_t phaseImbalanceTimer = 0;

static FaultType activeFault = FAULT_NONE;
static bool safetyFaultLatched = false;

// Offset constants (for 12-bit ADC centered at ~1.65V bias)
static const int ADC_OFFSET = 2048; 
static const int RMS_SAMPLES = 200; // Sample for ~200ms (10 full cycles at 50Hz)

// Initialize safety system
void initSafetyMonitor() {
    xTelemetryMutex = xSemaphoreCreateMutex();
    
    pinMode(PIN_ADC_VOLT_R, INPUT);
    pinMode(PIN_ADC_VOLT_Y, INPUT);
    pinMode(PIN_ADC_VOLT_B, INPUT);
    pinMode(PIN_ADC_CURR_R, INPUT);
    pinMode(PIN_ADC_CURR_Y, INPUT);
    pinMode(PIN_ADC_CURR_B, INPUT);
    pinMode(PIN_LED_POWER, OUTPUT);
    pinMode(PIN_LED_RUN, OUTPUT);
    pinMode(PIN_LED_FAULT, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_E_STOP, INPUT_PULLUP);
    pinMode(PIN_AUTO_MANUAL, INPUT_PULLUP);
    
    // Float switch inputs
    pinMode(PIN_FLOAT_LOW, INPUT_PULLUP);
    pinMode(PIN_FLOAT_HIGH, INPUT_PULLUP);

    digitalWrite(PIN_LED_POWER, HIGH);
    digitalWrite(PIN_LED_RUN, LOW);
    digitalWrite(PIN_LED_FAULT, LOW);

    // Initial state
    currentTelemetry.state = STATE_OFF;
    currentTelemetry.last_fault = FAULT_NONE;
    currentTelemetry.power_available = true;
    currentTelemetry.water_level = 100.0f;
    currentTelemetry.soil_moisture = 50.0f;
    currentTelemetry.casing_temp = 32.0f;

    // Load persisted settings from NVS
    preferences.begin("system", true);
    currentTelemetry.active_language = (Language)preferences.getInt("lang", 1); // 1 = English
    currentTelemetry.pump_type = (PumpType)preferences.getInt("type", 2); // 2 = Three Phase
    currentTelemetry.paired_remote_id = preferences.getUInt("remote", 0);
    preferences.end();
}

// Function to calculate True RMS for a given pin
float calculateRMS(uint8_t pin, float calibrationFactor) {
    long long squaredSum = 0;
    for (int i = 0; i < RMS_SAMPLES; i++) {
        int sample = analogRead(pin);
        int val = sample - ADC_OFFSET;
        squaredSum += (val * val);
        delayMicroseconds(1000); // 1ms delay = 1kHz sampling rate
    }
    float meanSquare = (float)squaredSum / RMS_SAMPLES;
    return sqrt(meanSquare) * calibrationFactor;
}

// Check Phase Sequence (R-Y-B order check based on positive slopes)
bool verifyPhaseSequence() {
    // High-performance sequence checking by sampling zero crossings
    uint32_t tR = 0, tY = 0, tB = 0;
    uint32_t startTime = millis();

    // Look for positive zero crossings (transitioning from below bias to above bias)
    while (millis() - startTime < 100) { // Limit window to 100ms
        int valR = analogRead(PIN_ADC_VOLT_R) - ADC_OFFSET;
        int valY = analogRead(PIN_ADC_VOLT_Y) - ADC_OFFSET;
        int valB = analogRead(PIN_ADC_VOLT_B) - ADC_OFFSET;

        if (tR == 0 && valR > 50) tR = micros();
        if (tY == 0 && tR != 0 && valY > 50) tY = micros();
        if (tB == 0 && tY != 0 && valB > 50) tB = micros();

        if (tR != 0 && tY != 0 && tB != 0) break;
        delayMicroseconds(200);
    }

    if (tR == 0 || tY == 0 || tB == 0) return false; // Missing phases
    return (tR < tY) && (tY < tB); // Correct sequence: R leads Y, which leads B
}

// Perform safety monitoring logic
void performSafetyCheck() {
    // 1. Read hardware sensors (True RMS values)
    float vR = calculateRMS(PIN_ADC_VOLT_R, CAL_VOLT_FACTOR);
    float vY = calculateRMS(PIN_ADC_VOLT_Y, CAL_VOLT_FACTOR);
    float vB = calculateRMS(PIN_ADC_VOLT_B, CAL_VOLT_FACTOR);

    float iR = calculateRMS(PIN_ADC_CURR_R, CAL_CURR_FACTOR);
    float iY = calculateRMS(PIN_ADC_CURR_Y, CAL_CURR_FACTOR);
    float iB = calculateRMS(PIN_ADC_CURR_B, CAL_CURR_FACTOR);

    float avgVolt = (vR + vY + vB) / 3.0f;
    float avgCurr = (iR + iY + iB) / 3.0f;

    // 2. Hardware Emergency Stop (E-Stop) Check
    if (digitalRead(PIN_E_STOP) == LOW) { // E-Stop is active low
        activeFault = FAULT_EMERGENCY_STOP;
        safetyFaultLatched = true;
    }

    // 3. Single Phasing Detection (Bypassed for SOLAR VFD)
    if (currentTelemetry.pump_type != PUMP_SOLAR_VFD) {
        if ((vR < 80.0f || vY < 80.0f || vB < 80.0f) && (vR > 100.0f || vY > 100.0f || vB > 100.0f)) {
            activeFault = FAULT_SINGLE_PHASING;
            safetyFaultLatched = true;
        }
    }

    // 4. Over/Under Voltage Checks (Bypassed for SOLAR VFD)
    if (currentTelemetry.pump_type != PUMP_SOLAR_VFD) {
        if (avgVolt < underVoltageLimit) {
            if (underVoltageTripTimer == 0) underVoltageTripTimer = millis();
            else if (millis() - underVoltageTripTimer > (TRIP_TIME_VOLTAGE * 1000)) {
                activeFault = FAULT_UNDER_VOLTAGE;
                safetyFaultLatched = true;
            }
        } else if (avgVolt > overVoltageLimit) {
            if (overVoltageTripTimer == 0) overVoltageTripTimer = millis();
            else if (millis() - overVoltageTripTimer > (TRIP_TIME_VOLTAGE * 1000)) {
                activeFault = FAULT_OVER_VOLTAGE;
                safetyFaultLatched = true;
            }
        } else {
            underVoltageTripTimer = 0;
            overVoltageTripTimer = 0;
        }
    } else {
        underVoltageTripTimer = 0;
        overVoltageTripTimer = 0;
    }

    // 5. Phase Sequence Check on initial start/power up (Bypassed for SOLAR VFD)
    static bool phaseChecked = false;
    if (currentTelemetry.pump_type != PUMP_SOLAR_VFD) {
        if (!phaseChecked && avgVolt > 150.0f) {
            if (!verifyPhaseSequence()) {
                activeFault = FAULT_PHASE_REVERSAL;
                safetyFaultLatched = true;
            }
            phaseChecked = true;
        }
    } else {
        phaseChecked = true;
    }

    // 6. Current overload check (thermal model: I^2t accumulator)
    if (currentTelemetry.state == STATE_RUNNING) {
        float maxCurrent = fmax(iR, fmax(iY, iB));
        if (maxCurrent > nominalCurrent) {
            float ratio = maxCurrent / nominalCurrent;
            thermalAccumulator += ((ratio * ratio) - 1.0f) * 0.2f; // dt is approx 200ms
            if (thermalAccumulator > THERMAL_TRIP_LIMIT) {
                activeFault = FAULT_OVER_CURRENT;
                safetyFaultLatched = true;
            }
        } else {
            thermalAccumulator -= 0.1f; // Slow cooling down
            if (thermalAccumulator < 0.0f) thermalAccumulator = 0.0f;
        }

        // Dry Run Protection (Undercurrent check)
        float dryRunLimit = nominalCurrent * dryRunMultiplier;
        if (avgCurr < dryRunLimit && avgCurr > 0.5A) { // Ignore if pump is deliberately OFF
            if (dryRunTripTimer == 0) dryRunTripTimer = millis();
            else if (millis() - dryRunTripTimer > (TRIP_TIME_DRY_RUN * 1000)) {
                activeFault = FAULT_DRY_RUN;
                safetyFaultLatched = true;
            }
        } else {
            dryRunTripTimer = 0;
        }
    } else {
        dryRunTripTimer = 0;
        thermalAccumulator = 0.0f; // Reset thermal model if motor is off
    }

    // 7. Update Telemetry and Read Float Switches
    float calculatedWaterLevel = readWaterLevelPercent();

    if (xSemaphoreTake(xTelemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        currentTelemetry.voltage[0] = vR;
        currentTelemetry.voltage[1] = vY;
        currentTelemetry.voltage[2] = vB;
        currentTelemetry.current[0] = iR;
        currentTelemetry.current[1] = iY;
        currentTelemetry.current[2] = iB;
        currentTelemetry.power_available = (avgVolt > 100.0f);
        currentTelemetry.water_level = calculatedWaterLevel;

        // Safety trip for critical low water level
        if (currentTelemetry.water_level < 10.0f) {
            activeFault = FAULT_CRITICAL_WATER_LEVEL;
            safetyFaultLatched = true;
        }

        if (safetyFaultLatched) {
            currentTelemetry.last_fault = activeFault;
            digitalWrite(PIN_LED_FAULT, HIGH);
        } else {
            digitalWrite(PIN_LED_FAULT, LOW);
        }
        xSemaphoreGive(xTelemetryMutex);
    }
}

bool isSafetyFaultActive() {
    return safetyFaultLatched;
}

FaultType getActiveFault() {
    return activeFault;
}

void resetSafetyFault() {
    activeFault = FAULT_NONE;
    safetyFaultLatched = false;
    thermalAccumulator = 0.0f;
    underVoltageTripTimer = 0;
    overVoltageTripTimer = 0;
    dryRunTripTimer = 0;
}

void getSystemTelemetry(SystemTelemetry &dest) {
    if (xSemaphoreTake(xTelemetryMutex, portMAX_DELAY) == pdTRUE) {
        dest = currentTelemetry;
        xSemaphoreGive(xTelemetryMutex);
    }
}

void updateSafetyThresholds(float nominal_curr, float under_v, float over_v) {
    if (xSemaphoreTake(xTelemetryMutex, portMAX_DELAY) == pdTRUE) {
        nominalCurrent = nominal_curr;
        underVoltageLimit = under_v;
        overVoltageLimit = over_v;
        xSemaphoreGive(xTelemetryMutex);
    }
}

void setSystemLanguage(Language lang) {
    if (xSemaphoreTake(xTelemetryMutex, portMAX_DELAY) == pdTRUE) {
        currentTelemetry.active_language = lang;
        xSemaphoreGive(xTelemetryMutex);
    }
    preferences.begin("system", false);
    preferences.putInt("lang", (int)lang);
    preferences.end();
}

void setSystemPumpType(PumpType type) {
    if (xSemaphoreTake(xTelemetryMutex, portMAX_DELAY) == pdTRUE) {
        currentTelemetry.pump_type = type;
        xSemaphoreGive(xTelemetryMutex);
    }
    preferences.begin("system", false);
    preferences.putInt("type", (int)type);
    preferences.end();
}

void setSystemPairedRemote(uint32_t remoteId) {
    if (xSemaphoreTake(xTelemetryMutex, portMAX_DELAY) == pdTRUE) {
        currentTelemetry.paired_remote_id = remoteId;
        xSemaphoreGive(xTelemetryMutex);
    }
    preferences.begin("system", false);
    preferences.putUInt("remote", remoteId);
    preferences.end();
}

void setSystemWaterLevel(float level) {
    if (xSemaphoreTake(xTelemetryMutex, portMAX_DELAY) == pdTRUE) {
        currentTelemetry.water_level = level;
        xSemaphoreGive(xTelemetryMutex);
    }
}

VersionInfo getSystemVersion() {
    VersionInfo v = { "1.0.0", "1.0", "Rev_B", "1.0", "TEL_1.0" };
    return v;
}
