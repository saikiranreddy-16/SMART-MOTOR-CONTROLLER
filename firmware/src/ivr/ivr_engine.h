#ifndef IVR_ENGINE_H
#define IVR_ENGINE_H

#include "../config.h"

// Prompt IDs mapped in voice/config.json
#define PROMPT_MENU         1
#define PROMPT_STARTING     2
#define PROMPT_STOPPING     3
#define PROMPT_STATUS_RUN   4
#define PROMPT_STATUS_OFF   5
#define PROMPT_STATUS_FAULT 6
#define PROMPT_LOW_WATER    7
#define PROMPT_ERROR        8
#define PROMPT_INVALID_KEY  9

// Trigger answering call and playing prompt
void startIvrSession(const String &callerNumber);

// Process DTMF input inside active call
void handleIvrDTMF(char key);

// Check if call timed out due to inactivity
void processIvrTimeout();

// Clean up IVR state when call drops
void terminateIvrSession();

#endif // IVR_ENGINE_H
