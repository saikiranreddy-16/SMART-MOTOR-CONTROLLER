#include "ota_rollback.h"
#include <esp_ota_ops.h>

void markActivePartitionValid() {
    esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
    if (err == ESP_OK) {
        Serial.println("App validated successfully. Rollback canceled.");
    } else {
        Serial.println("Failed to validate app partition.");
    }
}

void executeReboot() {
    Serial.println("Rebooting ESP32...");
    delay(1000);
    ESP.restart();
}
