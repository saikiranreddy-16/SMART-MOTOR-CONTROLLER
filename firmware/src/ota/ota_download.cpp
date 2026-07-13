#include "ota_download.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <HardwareSerial.h>

extern HardwareSerial Serial2;

static bool sendModemOTA(const String &cmd, const String &expected, uint32_t timeoutMs) {
    Serial2.println(cmd);
    uint32_t start = millis();
    String resp = "";
    while (millis() - start < timeoutMs) {
        if (Serial2.available()) {
            resp += (char)Serial2.read();
            if (resp.endsWith(expected)) {
                return true;
            }
        }
    }
    return false;
}

bool downloadWiFiStream(const String &url, size_t &outSize) {
    if (WiFi.status() != WL_CONNECTED) return false;

    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    int size = http.getSize();
    if (size <= 0) {
        http.end();
        return false;
    }

    if (!Update.begin(size)) {
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);
    http.end();

    if (written == size) {
        outSize = size;
        return true;
    }
    Update.abort();
    return false;
}

bool downloadCellularChunks(const String &url, size_t &outSize) {
    sendModemOTA("AT+HTTPINIT", "OK", 2000);
    sendModemOTA("AT+HTTPPARA=\"URL\",\"" + url + "\"", "OK", 2000);
    Serial2.println("AT+HTTPACTION=0");
    
    // Parse response
    uint32_t start = millis();
    String actionResp = "";
    int totalSize = 0;
    bool actionOk = false;
    
    while (millis() - start < 15000) {
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
        sendModemOTA("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    if (!Update.begin(totalSize)) {
        sendModemOTA("AT+HTTPTERM", "OK", 2000);
        return false;
    }

    const int CHUNK = 1024;
    int bytesRead = 0;
    uint8_t buffer[CHUNK];

    while (bytesRead < totalSize) {
        int toRead = min(CHUNK, totalSize - bytesRead);
        String readCmd = "AT+HTTPREAD=" + String(bytesRead) + "," + String(toRead);
        Serial2.println(readCmd);
        
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
            Update.abort();
            sendModemOTA("AT+HTTPTERM", "OK", 2000);
            return false;
        }

        int count = 0;
        start = millis();
        while (count < toRead && (millis() - start < 4000)) {
            if (Serial2.available()) {
                buffer[count++] = Serial2.read();
            }
        }

        if (count != toRead) {
            Update.abort();
            sendModemOTA("AT+HTTPTERM", "OK", 2000);
            return false;
        }

        size_t written = Update.write(buffer, toRead);
        if (written != toRead) {
            Update.abort();
            sendModemOTA("AT+HTTPTERM", "OK", 2000);
            return false;
        }

        bytesRead += toRead;
    }

    sendModemOTA("AT+HTTPTERM", "OK", 2000);
    outSize = totalSize;
    return true;
}
