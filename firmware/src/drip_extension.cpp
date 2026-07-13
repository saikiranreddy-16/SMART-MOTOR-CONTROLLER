#include "drip_extension.h"

#define MAX_CALLBACKS 4
static DripCallback dripCallbacks[MAX_CALLBACKS] = { nullptr };
static int callbackCount = 0;

void registerDripCallback(DripCallback cb) {
    if (callbackCount < MAX_CALLBACKS && cb != nullptr) {
        dripCallbacks[callbackCount++] = cb;
    }
}

void notifyDripExtension(DripEvent event, const SystemTelemetry &telemetry) {
    for (int i = 0; i < callbackCount; i++) {
        if (dripCallbacks[i] != nullptr) {
            dripCallbacks[i](event, telemetry);
        }
    }
}
