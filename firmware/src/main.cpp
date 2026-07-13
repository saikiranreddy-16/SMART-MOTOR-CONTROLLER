#include <Arduino.h>
#include "config.h"
#include "safety_monitor.h"
#include "pump_control.h"
#include "telemetry.h"
#include "gsm_handler.h"
#include "installer/self_test.h"

// FreeRTOS Task Handles
TaskHandle_t xSafetyTaskHandle = NULL;
TaskHandle_t xStateMachineTaskHandle = NULL;
TaskHandle_t xTelemetryTaskHandle = NULL;
TaskHandle_t xGsmTaskHandle = NULL;

// Task Declarations
void vSafetyTask(void *pvParameters);
void vStateMachineTask(void *pvParameters);
void vTelemetryTask(void *pvParameters);
void vGsmTask(void *pvParameters);

void setup() {
    // Debug console output
    Serial.begin(115200);
    Serial.println("=========================================");
    Serial.println("Smart Agricultural Motor Controller Booting...");
    Serial.println("=========================================");

    // Initialize systems
    initSafetyMonitor();
    initPumpControl();
    initTelemetry();
    initGSM();

    // Run Power-On Self-Test (POST)
    DiagnosticStatus diag;
    if (!runPowerOnSelfTest(diag)) {
        Serial.print("POST CRITICAL FAILURE: ");
        Serial.println(diag.active_error_code);
        
        // Blink fault LED to indicate hardware check failure
        pinMode(PIN_LED_FAULT, OUTPUT);
        while (true) {
            digitalWrite(PIN_LED_FAULT, HIGH);
            delay(300);
            digitalWrite(PIN_LED_FAULT, LOW);
            delay(300);
        }
    }

    // Create FreeRTOS Tasks
    
    // Safety check task must run at highest priority on Core 1 to protect the physical motor
    xTaskCreatePinnedToCore(
        vSafetyTask,
        "SafetyTask",
        4096,
        NULL,
        5, // Highest priority
        &xSafetyTaskHandle,
        1 // Core 1
    );

    // Pump Control State Machine runs on Core 1, slightly lower priority than safety
    xTaskCreatePinnedToCore(
        vStateMachineTask,
        "StateMachineTask",
        4096,
        NULL,
        4,
        &xStateMachineTaskHandle,
        1 // Core 1
    );

    // Telemetry and Communication Tasks run on Core 0
    xTaskCreatePinnedToCore(
        vTelemetryTask,
        "TelemetryTask",
        4096,
        NULL,
        2,
        &xTelemetryTaskHandle,
        0 // Core 0
    );

    // GSM task runs on Core 0, priority 3
    xTaskCreatePinnedToCore(
        vGsmTask,
        "GsmTask",
        8192,
        NULL,
        3,
        &xGsmTaskHandle,
        0 // Core 0
    );

    Serial.println("FreeRTOS Scheduler Started.");
}

void loop() {
    // FreeRTOS handles the scheduling, loop remains empty.
    vTaskDelete(NULL);
}

// -----------------------------------------------------------------------------
// FreeRTOS Task Implementations
// -----------------------------------------------------------------------------

void vSafetyTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // Run safety loop every 20ms

    Serial.println("Safety Task Started on Core 1.");

    for (;;) {
        performSafetyCheck();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vStateMachineTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // Run state machine every 100ms

    Serial.println("Pump State Machine Task Started on Core 1.");

    for (;;) {
        updatePumpStateMachine();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTelemetryTask(void *pvParameters) {
    Serial.println("Telemetry Task Started on Core 0.");

    for (;;) {
        processTelemetryTask();
        // Delay is handled inside processTelemetryTask (1000ms)
    }
}

void vGsmTask(void *pvParameters) {
    Serial.println("GSM Task Started on Core 0.");

    for (;;) {
        processGSM();
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
    }
}
