#ifndef OTA_VERIFY_H
#define OTA_VERIFY_H

#include <Arduino.h>

// Validate the written passive partition (performs internal size and CRC verification)
bool verifyUpdateSignature();

#endif // OTA_VERIFY_H
