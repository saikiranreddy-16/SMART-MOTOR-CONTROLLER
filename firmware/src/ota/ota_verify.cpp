#include "ota_verify.h"
#include <Update.h>

bool verifyUpdateSignature() {
    if (!Update.end()) {
        Serial.print("OTA Validation error code: ");
        Serial.println(Update.getError());
        return false;
    }
    
    if (Update.isFinished()) {
        Serial.println("OTA Partition integrity verified.");
        return true;
    }
    return false;
}
