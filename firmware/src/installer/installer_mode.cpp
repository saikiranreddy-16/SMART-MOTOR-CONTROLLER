#include "installer_mode.h"
#include "../config.h"
#include "../safety_monitor.h"
#include "../water_level/sensor_calibration.h"
#include <Preferences.h>
#include <HardwareSerial.h>

extern HardwareSerial Serial2;

void processInstallerCommand(const String &cmd) {
    String command = cmd;
    command.trim();
    
    Serial.println("[Installer] Processing Terminal Command: " + command);
    
    if (command.startsWith("SET LANG ")) {
        int val = command.substring(9).toInt();
        if (val >= 1 && val <= 3) {
            setSystemLanguage((Language)val);
            Serial.println("[Installer SUCCESS]: Language configured to NVS.");
        }
    }
    else if (command.startsWith("SET TYPE ")) {
        int val = command.substring(9).toInt();
        if (val >= 1 && val <= 3) {
            setSystemPumpType((PumpType)val);
            Serial.println("[Installer SUCCESS]: Pump Type configured to NVS.");
        }
    }
    else if (command.startsWith("SET SOURCE ")) {
        int val = command.substring(11).toInt();
        if (val >= 1 && val <= 3) {
            LevelCalibration cal = getLevelCalibration();
            cal.source_type = (WaterSourceType)val;
            saveLevelCalibration(cal);
            Serial.println("[Installer SUCCESS]: Water Source Type configured to NVS.");
        }
    }
    else if (command.startsWith("TEST RELAY ")) {
        int pin = command.substring(11).toInt();
        if (pin == 1) {
            Serial.println("[Installer]: Activating RELAY_START for 1s...");
            digitalWrite(PIN_RELAY_START, HIGH);
            delay(1000);
            digitalWrite(PIN_RELAY_START, LOW);
        } else if (pin == 2) {
            Serial.println("[Installer]: Activating RELAY_RUN for 1s...");
            digitalWrite(PIN_RELAY_RUN, HIGH);
            delay(1000);
            digitalWrite(PIN_RELAY_RUN, LOW);
        }
    }
    else if (command.equals("TEST LED")) {
        Serial.println("[Installer]: Cycling LEDs...");
        digitalWrite(PIN_LED_POWER, HIGH);
        digitalWrite(PIN_LED_RUN, HIGH);
        digitalWrite(PIN_LED_FAULT, HIGH);
        delay(2000);
        digitalWrite(PIN_LED_RUN, LOW);
        digitalWrite(PIN_LED_FAULT, LOW);
    }
    else if (command.equals("TEST BUZZER")) {
        Serial.println("[Installer]: Cycling Buzzer...");
        digitalWrite(PIN_BUZZER, HIGH);
        delay(500);
        digitalWrite(PIN_BUZZER, LOW);
    }
    else if (command.equals("SAVE")) {
        Serial.println("[Installer]: Settings saved. Rebooting controller...");
        delay(1000);
        ESP.restart();
    }
}

void runFactoryTestMode() {
    Serial.println("==========================================");
    Serial.println("FACTORY PRODUCTION TESTING ACTIVATED...");
    Serial.println("==========================================");
    
    // 1. Relay Test
    Serial.println("Factory: Testing Start Relay...");
    digitalWrite(PIN_RELAY_START, HIGH);
    delay(1000);
    digitalWrite(PIN_RELAY_START, LOW);
    
    Serial.println("Factory: Testing Run Relay...");
    digitalWrite(PIN_RELAY_RUN, HIGH);
    delay(1000);
    digitalWrite(PIN_RELAY_RUN, LOW);
    
    // 2. LED Cycle Test
    Serial.println("Factory: Cycling LEDs...");
    digitalWrite(PIN_LED_POWER, HIGH);
    digitalWrite(PIN_LED_RUN, HIGH);
    digitalWrite(PIN_LED_FAULT, HIGH);
    delay(1000);
    digitalWrite(PIN_LED_RUN, LOW);
    digitalWrite(PIN_LED_FAULT, LOW);
    
    // 3. Buzzer Test
    Serial.println("Factory: Sounding Buzzer...");
    digitalWrite(PIN_BUZZER, HIGH);
    delay(500);
    digitalWrite(PIN_BUZZER, LOW);
    
    // 4. GSM Check
    Serial.println("Factory: Querying SIM7600...");
    Serial2.println("AT+CPIN?");
    delay(500);
    while(Serial2.available()) {
        Serial.write(Serial2.read());
    }
    
    Serial.println("Factory Test Sequence Completed.");
    Serial.println("==========================================");
}
