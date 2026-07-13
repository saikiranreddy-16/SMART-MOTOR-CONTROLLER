#ifndef GSM_HANDLER_H
#define GSM_HANDLER_H

#include "config.h"

// Initialize UART connection to SIM7600/800 and boot the modem
void initGSM();

// Primary task routine for processing incoming cellular calls, DTMF, and SMS
// This should be called periodically by Core 0 GSM task
void processGSM();

// UTILITY: Send SMS alert to a specific recipient
bool sendSMS(const String &phoneNumber, const String &message);

#endif // GSM_HANDLER_H
