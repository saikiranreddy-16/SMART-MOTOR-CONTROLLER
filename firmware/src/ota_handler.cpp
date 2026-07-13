#include "ota_handler.h"
#include <Update.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_ota_ops.h>
#include <HardwareSerial.h>

// Use UART2 (Serial2) for SIM7600
extern HardwareSerial Serial2;

String getActivePartitionName() {
    const esp_partition_t *running = esp_ota_get_running_partition();
    return String(running->label);
}

// 1. WiFi OTA Update (Standard HTTP stream)
bool startWiFiOTA(const String &serverUrl) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Aborting OTA.");
        return false;
    }

    Serial.println("Starting OTA Update over WiFi from: " + serverUrl);
    HTTPClient http;
    http.begin(serverUrl);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.println("HTTP connection failed, status: " + String(httpCode));
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("Invalid content size. Aborting.");
        http.end();
        return false;
    }

    bool canBegin = Update.begin(contentLength);
    if (!canBegin) {
        Serial.println("Not enough space to begin OTA update.");
        http.end();
        return false;
    }

    WiFiClient *client = http.getStreamPtr();
    size_t written = Update.writeStream(*client);

    if (written != contentLength) {
        Serial.println("Written only " + String(written) + "/" + String(contentLength) + ". FAILED.");
        Update.abort();
        http.end();
        return false;
    }

    if (!Update.end()) {
        Serial.println("Update end failed: " + String(Update.getError()));
        http.end();
        return false;
    }

    if (Update.isFinished()) {
        Serial.println("OTA Update Successful! Rebooting system...");
        delay(1000);
        ESP.restart();
        return true;
    }

    http.end();
    return false;
}

// Helper: send AT commands during cellular OTA and wait for expected response
bool sendGsmOTACommand(const String &cmd, const String &expected, uint32_t timeoutMs) {
    Serial2.println(cmd);
    uint32_t start = millis();
    String resp = "";
    while (millis() - start < timeoutMs) {
        if (Serial2.available()) {
            char c = Serial2.read();
            resp += c;
            if (resp.endsWith(expected)) {
                return true;
            }
        }
    }
    Serial.println("GSM OTA timeout: " + cmd + " | Resp: " + resp);
    return false;
}

// 2. 4G/Cellular Chunked OTA (SIM7600)
// Reads the binary file in 1024-byte chunks to prevent UART buffer overflow.
bool startGsmOTA(const String &serverUrl) {
    Serial.println("Starting OTA Update over 4G GPRS from: " + serverUrl);

    // Stop normal GSM task operations temporarily to claim UART channel exclusively
    // (In production, a mutex or task suspension handles this)

    // Init HTTP Service on modem
    sendGsmOTACommand("AT+HTTPINIT", "OK", 2000);
    sendGsmOTACommand("AT+HTTPPARA=\"URL\",\"" + serverUrl + "\"", "OK", 2000);

    // Trigger GET request action
    Serial2.println("AT+HTTPACTION=0");
    
    // Parse Action response: +HTTPACTION: 0,200,file_size
    uint32_t start = millis();
    String actionResp = "";
    int totalSize = 0;
    bool actionOk = false;
    
    while (millis() - start < 15000) { // Allow up to 15s for DNS/HTTP response
        if (Serial2.available()) {
            char c = Serial2.read();
            actionResp += c;
            int idx = actionResp.indexOf("+HTTPACTION: 0,200,");
            if (idx >= 0 && actionResp.endsWith("\n")) {
                String sizeStr = actionResp.substring(idx + 19);
                sizeStr.trim();
                totalSize = sizeStr.toInt();
                actionOk = true;
                break;
            }
        }
    }

    if (!actionOk || totalSize <= 0) {
        Serial.println("Failed to fetch update binary over GPRS. Response: " + actionResp);
        sendGsmOTACommand("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    Serial.println("File size confirmed: " + String(totalSize) + " bytes. Writing to passive partition...");
    
    if (!Update.begin(totalSize)) {
        Serial.println("Not enough space to write partition.");
        sendGsmOTACommand("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    const int CHUNK_SIZE = 1024;
    int bytesRead = 0;
    uint8_t buffer[CHUNK_SIZE];

    while (bytesRead < totalSize) {
        int lengthToRead = min(CHUNK_SIZE, totalSize - bytesRead);
        
        // Request chunk from offset
        String readCmd = "AT+HTTPREAD=" + String(bytesRead) + "," + String(lengthToRead);
        Serial2.println(readCmd);
        
        // Wait for modem output format: "+HTTPREAD: DATA,length" followed by raw bytes
        // We parse this header, then capture the raw binary stream
        start = millis();
        String header = "";
        bool headerFound = false;
        
        while (millis() - start < 3000) {
            if (Serial2.available()) {
                char c = Serial2.read();
                header += c;
                if (header.indexOf("+HTTPREAD: DATA,") >= 0 && header.endsWith("\n")) {
                    headerFound = true;
                    break;
                }
            }
        }

        if (!headerFound) {
            Serial.println("Modem read header fail at offset: " + String(bytesRead));
            Update.abort();
            sendGsmOTACommand("AT+HTTPTERM", "OK", 2000);
            return false;
        }

        // Read raw binary bytes into buffer
        int readCount = 0;
        start = millis();
        while (readCount < lengthToRead && (millis() - start < 4000)) {
            if (Serial2.available()) {
                buffer[readCount++] = Serial2.read();
            }
        }

        if (readCount != lengthToRead) {
            Serial.println("GPRS data chunk read timeout at offset: " + String(bytesRead));
            Update.abort();
            sendGsmOTACommand("AT+HTTPTERM", "OK", 2000);
            return false;
        }

        // Write chunk to flash partition
        size_t written = Update.write(buffer, lengthToRead);
        if (written != lengthToRead) {
            Serial.println("Flash write mismatch at offset: " + String(bytesRead));
            Update.abort();
            sendGsmOTACommand("AT+HTTPTERM", "OK", 2000);
            return false;
        }

        bytesRead += lengthToRead;
        Serial.print("OTA progress: ");
        Serial.print((float)bytesRead / totalSize * 100, 1);
        Serial.println("%");
    }

    sendGsmOTACommand("AT+HTTPTERM", "OK", 2000);

    if (!Update.end()) {
        Serial.println("Update validation failed.");
        return false;
    }

    if (Update.isFinished()) {
        Serial.println("OTA Firmware Update via GPRS Successful! Rebooting...");
        delay(1000);
        ESP.restart();
        return true;
    }

    return false;
}
