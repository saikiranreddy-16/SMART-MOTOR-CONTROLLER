#include "self_test.h"
#include <SPI.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include "../config.h"

extern HardwareSerial Serial2;

static bool checkModemResponse() {
    Serial2.println("AT");
    uint32_t start = millis();
    String resp = "";
    while (millis() - start < 1500) {
        if (Serial2.available()) {
            resp += (char)Serial2.read();
            if (resp.indexOf("OK") >= 0) return true;
        }
    }
    return false;
}

static bool checkRFTransceiver() {
    // Read CC1101 FREQ0 register (address 0x0F)
    digitalWrite(5, LOW); // CS pin
    SPI.transfer(0x0F | 0x80); // Read bit + addr
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(5, HIGH);
    
    // Register default or set value should match 0x3B
    return (val == 0x3B);
}

bool runPowerOnSelfTest(DiagnosticStatus &status) {
    status.memory_ok = true;
    status.nvs_ok = true;
    status.gsm_ok = true;
    status.rf_ok = true;
    status.sensors_ok = true;
    status.active_error_code = "";

    Serial.println("==========================================");
    Serial.println("RUNNING POWER-ON SELF-TEST (POST)...");
    Serial.println("==========================================");

    // 1. Check Heap Memory
    if (ESP.getFreeHeap() < 25000) {
        Serial.println("POST: Heap memory failure.");
        status.memory_ok = false;
        status.active_error_code = "E011";
        return false;
    }

    // 2. Check NVS Flash
    Preferences testPrefs;
    if (testPrefs.begin("post_test", false)) {
        testPrefs.putInt("test_val", 99);
        if (testPrefs.getInt("test_val", 0) != 99) {
            status.nvs_ok = false;
        }
        testPrefs.clear();
        testPrefs.end();
    } else {
        status.nvs_ok = false;
    }
    
    if (!status.nvs_ok) {
        Serial.println("POST: NVS memory read/write failure.");
        status.active_error_code = "E012";
        return false;
    }

    // 3. Check GSM Modem Connection
    if (!checkModemResponse()) {
        Serial.println("POST: GSM/SIM7600 response failure.");
        status.gsm_ok = false;
        status.active_error_code = "E005"; // SIM Missing or Modem Connection error
        return false;
    }

    // 4. Check CC1101 RF Transceiver Connection
    if (!checkRFTransceiver()) {
        Serial.println("POST: RF CC1101 SPI response failure.");
        status.rf_ok = false;
        status.active_error_code = "E013"; // RF failure code
        return false;
    }

    // 5. Check Sensors ADC bias (Should rest near bias value ~2048 when motor is off)
    int rVolt = analogRead(PIN_ADC_VOLT_R);
    int rCurr = analogRead(PIN_ADC_CURR_R);
    
    // If ADC values are completely dead (0) or pegged to max (4095), it is open/short circuit
    if (rVolt == 0 || rVolt == 4095 || rCurr == 0 || rCurr == 4095) {
        Serial.println("POST: Voltage or Current Sensor open/short circuit failure.");
        status.sensors_ok = false;
        status.active_error_code = "E014"; // Sensor Connection Failure
        return false;
    }

    Serial.println("POST STATUS: ALL MODULES FUNCTIONAL. BOOTING...");
    Serial.println("==========================================");
    return true;
}
