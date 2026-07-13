#ifndef DRIP_EXTENSION_H
#define DRIP_EXTENSION_H

#include "config.h"

enum DripEvent {
    DRIP_EVENT_START,
    DRIP_EVENT_STOP,
    DRIP_EVENT_TICK
};

typedef void (*DripCallback)(DripEvent event, const SystemTelemetry &telemetry);

// Register a callback for drip irrigation events
void registerDripCallback(DripCallback cb);

// Trigger/Notify drip callbacks of events from FSM
void notifyDripExtension(DripEvent event, const SystemTelemetry &telemetry);

#endif // DRIP_EXTENSION_H
