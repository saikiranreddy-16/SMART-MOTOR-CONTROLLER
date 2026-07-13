#include "display.h"
#include "buttons.h"
#include "battery.h"
#include "pairing.h"

// Hardcode or generate unique serial ID
const uint32_t UNIQUE_REMOTE_ID = 0x54A3E912;
uint32_t pairedControllerId = 0;
bool isPaired = false;

// Display status strings
String motorStatusText = "DISCONNECTED";
int waterLevelVal = 0;
bool powerAvailableVal = false;
int batteryLevelVal = 100;

void setup() {
    Serial.begin(9600);
    
    initDisplay();
    initButtons();
    initBattery();
    
    drawTextScreen("Smart Remote v1.0", "Booting Transceiver...");
    initRFTransceiver();
    delay(1000);
}

void loop() {
    batteryLevelVal = getBatteryPercent();

    // 1. Scan and process push buttons
    if (isStartButtonPressed()) {
        drawTextScreen("START COMMAND", "Transmitting...");
        sendRFCommand(RF_CMD_START, UNIQUE_REMOTE_ID, batteryLevelVal);
        delay(500);
    }
    else if (isStopButtonPressed()) {
        drawTextScreen("STOP COMMAND", "Transmitting...");
        sendRFCommand(RF_CMD_STOP, UNIQUE_REMOTE_ID, batteryLevelVal);
        delay(500);
    }
    else if (isPairButtonPressed()) {
        drawTextScreen("PAIR COMMAND", "Pairing with Base...");
        sendRFCommand(RF_CMD_PAIR, UNIQUE_REMOTE_ID, batteryLevelVal);
        
        RemotePacket response;
        if (listenRFResponse(response, 3000)) {
            if (response.cmd_state == RF_CMD_PAIR_ACK) {
                isPaired = true;
                pairedControllerId = response.remote_id;
                drawTextScreen("PAIRING OK", "Connected to base!");
                delay(1500);
            }
        } else {
            drawTextScreen("PAIRING FAIL", "No response from base.");
            delay(1500);
        }
    }

    // 2. Periodically poll motor status (every 4 seconds)
    static uint32_t lastPoll = 0;
    if (millis() - lastPoll > 4000) {
        sendRFCommand(RF_CMD_STATUS, UNIQUE_REMOTE_ID, batteryLevelVal);
        lastPoll = millis();
    }

    // 3. Listen for asynchronous telemetry updates from base station
    RemotePacket response;
    if (listenRFResponse(response, 100)) {
        if (response.cmd_state == RF_CMD_STATUS_ACK) {
            waterLevelVal = response.water_level;
            powerAvailableVal = (response.power_available == 1);
            
            // Map state to text
            if (response.motor_state == STATE_RUNNING) motorStatusText = "RUNNING";
            else if (response.motor_state == STATE_FAULT) motorStatusText = "FAULT";
            else motorStatusText = "OFF";
        }
    }

    // 4. Update the OLED display dashboard
    drawDashboard(motorStatusText, waterLevelVal, powerAvailableVal, batteryLevelVal, isPaired);
    delay(50);
}
