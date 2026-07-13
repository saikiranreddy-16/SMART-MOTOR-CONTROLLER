#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include "config.h"

// Trigger firmware check and update over WiFi
bool startWiFiOTA(const String &serverUrl);

// Trigger firmware check and update over 4G Cellular (SIM7600)
bool startGsmOTA(const String &serverUrl);

// Check current active partition
String getActivePartitionName();

#endif // OTA_HANDLER_H
