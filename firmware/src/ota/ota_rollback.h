#ifndef OTA_ROLLBACK_H
#define OTA_ROLLBACK_H

#include <Arduino.h>

// Mark the current active firmware partition as working (prevents automatic rollback on next boot)
void markActivePartitionValid();

// Trigger soft hardware reboot to apply updates
void executeReboot();

#endif // OTA_ROLLBACK_H
