#include "ota_manager.h"
#include "ota_download.h"
#include "ota_verify.h"
#include "ota_rollback.h"
#include <WiFi.h>

bool checkAndRunOTA(const String &serverUrl) {
    size_t size = 0;
    bool downloadSuccess = false;

    // 1. Try WiFi first (fast and free)
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("OTA: WiFi network detected. Starting download stream...");
        downloadSuccess = downloadWiFiStream(serverUrl, size);
    } 
    // 2. Fall back to cellular GPRS chunks
    else {
        Serial.println("OTA: WiFi not available. Attempting GSM/Cellular GPRS chunks...");
        downloadSuccess = downloadCellularChunks(serverUrl, size);
    }

    if (!downloadSuccess || size == 0) {
        Serial.println("OTA: Download sequence failed.");
        return false;
    }

    // 3. Verify signatures and CRC checks
    if (!verifyUpdateSignature()) {
        Serial.println("OTA: Update verification failed. Rollback triggered.");
        return false;
    }

    // 4. Reboot into new partition
    Serial.println("OTA: Validation successful. Rebooting to apply update...");
    executeReboot();
    
    return true; // unreachable
}
