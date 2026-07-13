#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>

// Trigger a check and update over the best available channel (WiFi first, cellular failover next)
bool checkAndRunOTA(const String &serverUrl);

#endif // OTA_MANAGER_H
