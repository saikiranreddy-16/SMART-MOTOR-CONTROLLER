#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// HARDWARE PIN ASSIGNMENTS (ESP32)
// =============================================================================

// Relays & Contactors
#define PIN_RELAY_START      23  // Trigger for Start Contactor (coil)
#define PIN_RELAY_RUN        19  // Trigger for Run Contactor (coil)
#define PIN_RELAY_AUX        18  // Trigger for auxiliary alarm/external alert

// Isolated Analog Sensor Inputs (ADC1 only to avoid WiFi conflict)
#define PIN_ADC_VOLT_R       36  // Phase R Voltage (ADC1_CH0)
#define PIN_ADC_VOLT_Y       39  // Phase Y Voltage (ADC1_CH3)
#define PIN_ADC_VOLT_B       34  // Phase B Voltage (ADC1_CH6)
#define PIN_ADC_CURR_R       35  // Phase R Current (ADC1_CH7)
#define PIN_ADC_CURR_Y       32  // Phase Y Current (ADC1_CH4)
#define PIN_ADC_CURR_B       33  // Phase B Current (ADC1_CH5)

// Environmental / Soil Sensors (RS485 Modbus - Commercial Standard)
#define PIN_RS485_RX         13  // Modbus RX for Soil NPK/Moisture sensor
#define PIN_RS485_TX         14  // Modbus TX for Soil NPK/Moisture sensor

// Local Interfaces & Controls
#define PIN_E_STOP           12  // Hardware Emergency Stop (Active Low, Interrupt)
#define PIN_AUTO_MANUAL      5   // Auto/Manual Select Switch (Active Low)
#define PIN_BUZZER           27  // PWM Buzzer output
#define PIN_LED_POWER        2   // Status LED - Power (Green)
#define PIN_LED_RUN          4   // Status LED - Run (Red)
#define PIN_LED_FAULT        15  // Status LED - Fault (Amber)

// OLED Display (I2C)
#define PIN_OLED_SDA         21
#define PIN_OLED_SCL         22

// GSM Modem (SIM7600 / UART2)
#define PIN_GSM_TX           17
#define PIN_GSM_RX           16
#define PIN_GSM_PWRKEY       26  // Power key pin to boot SIM7600
#define PIN_GSM_RESET        25  // Reset pin for hard reset

// Digital Inputs for Water Level (Float switches)
#define PIN_FLOAT_HIGH       26  // High water level limit switch
#define PIN_FLOAT_LOW        25  // Low water level limit switch

// =============================================================================
// SYSTEM PARAMETERS & CALIBRATION DEFAULTS
// =============================================================================

// Calibration Factors (To map ADC raw values to AC Volt/Amps)
#define CAL_VOLT_FACTOR      1.21f  // Resistor divider + PT scaling
#define CAL_CURR_FACTOR      0.033f // Current Transformer scaling (e.g., 30A:5A CT)

// Default Limits
#define DEFAULT_UNDER_VOLT   180.0f // Volts
#define DEFAULT_OVER_VOLT    260.0f // Volts
#define DEFAULT_MAX_IMBALANCE 15.0f // Percent
#define DEFAULT_DRY_RUN_MUL   0.60f  // Undercurrent threshold (60% of nominal)
#define DEFAULT_NOMINAL_CURR 12.0f  // Default Nominal Amps for a 5HP pump
#define DEFAULT_TEMP_LIMIT   75.0f  // Casing high temperature limit (C)

// Timers (Seconds)
#define TIMER_STAR_DELTA     4.0f   // Transition time from Star to Delta contactor
#define TRIP_TIME_VOLTAGE    3.0f   // Under/Over voltage trip confirmation delay
#define TRIP_TIME_DRY_RUN    5.0f   // Dry run confirmation delay
#define RECOVERY_DRY_RUN     2700   // Auto-restart delay after dry-run trip (45 mins)
#define RECOVERY_VOLTAGE     30     // Auto-restart delay after voltage normalization (30s)

// =============================================================================
// STATE MACHINE & COMMAND TYPEDEFS
// =============================================================================

enum Language {
    LANG_ENG = 1,
    LANG_TEL = 2,
    LANG_HIN = 3
};

enum PumpType {
    PUMP_AC_SINGLE = 1,
    PUMP_AC_THREE = 2,
    PUMP_SOLAR_VFD = 3
};

enum PumpState {
    STATE_OFF,
    STATE_STARTING,
    STATE_RUNNING,
    STATE_STOPPING,
    STATE_FAULT,
    STATE_COOLING
};

enum FaultType {
    FAULT_NONE = 0,
    FAULT_UNDER_VOLTAGE,
    FAULT_OVER_VOLTAGE,
    FAULT_PHASE_IMBALANCE,
    FAULT_SINGLE_PHASING,
    FAULT_PHASE_REVERSAL,
    FAULT_DRY_RUN,
    FAULT_OVER_CURRENT,
    FAULT_OVER_HEATING,
    FAULT_EMERGENCY_STOP,
    FAULT_STARTUP_FAILED,
    FAULT_CRITICAL_WATER_LEVEL
};

struct SystemTelemetry {
    PumpState state;
    FaultType last_fault;
    float voltage[3];   // R, Y, B
    float current[3];   // R, Y, B
    float water_level;  // 0 - 100%
    float soil_moisture;// 0 - 100%
    float casing_temp;  // Celsius
    bool power_available;
    int rssi;
    Language active_language;
    PumpType pump_type;
    uint32_t paired_remote_id;
};

// =============================================================================
// HARDWARE EXPANSION PORT MAP (10-Pin Layout)
// =============================================================================
#define PIN_EXP_5V           -1  // Physical Pin Only
#define PIN_EXP_33V          -1  // Physical Pin Only
#define PIN_EXP_GND          -1  // Physical Pin Only
#define PIN_EXP_UART_TX      25  // UART2 TX Expansion Node
#define PIN_EXP_UART_RX      26  // UART2 RX Expansion Node
#define PIN_EXP_I2C_SDA      21  // Shared I2C SDA
#define PIN_EXP_I2C_SCL      22  // Shared I2C SCL
#define PIN_EXP_SPI_SCK      18  // VSPI SCK
#define PIN_EXP_SPI_MISO     19  // VSPI MISO
#define PIN_EXP_SPI_MOSI     23  // VSPI MOSI

// =============================================================================
// MULTI-SYSTEM VERSION INFORMATION STRUCT
// =============================================================================
struct VersionInfo {
    char firmware[8];   // "1.0.0"
    char hardware[6];   // "1.0"
    char pcb_rev[6];    // "Rev_B"
    char bootloader[6]; // "1.0"
    char voice_pack[8]; // "TEL_1.0"
};

#endif // CONFIG_H
