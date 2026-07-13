#ifndef OTA_DOWNLOAD_H
#define OTA_DOWNLOAD_H

#include <Arduino.h>

// Downloads from WiFi client directly into update partition
bool downloadWiFiStream(const String &url, size_t &outSize);

// Downloads from SIM7600 HTTP in 1024-byte chunks and writes to update partition
bool downloadCellularChunks(const String &url, size_t &outSize);

#endif // OTA_DOWNLOAD_H
