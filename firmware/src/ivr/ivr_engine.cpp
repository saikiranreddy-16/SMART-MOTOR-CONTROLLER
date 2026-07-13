#include "ivr_engine.h"
#include "voice_manager.h"
#include "language_manager.h"
#include "../safety_monitor.h"
#include "../pump_control.h"
#include <HardwareSerial.h>

extern HardwareSerial Serial2;

static uint32_t ivrSessionTimer = 0;
static bool ivrCallActive = false;
static String activeCaller = "";

void startIvrSession(const String &callerNumber) {
    Serial.println("IVR Session started with: " + callerNumber);
    ivrCallActive = true;
    activeCaller = callerNumber;
    ivrSessionTimer = millis();

    // Small delay to allow the voice path to establish
    delay(1000);
    
    // Bypasses language selection entirely. Plays the menu immediately.
    playVoicePrompt(PROMPT_MENU);
}

void handleIvrDTMF(char key) {
    if (!ivrCallActive) return;

    Serial.println("IVR Processing Key: " + String(key));
    ivrSessionTimer = millis(); // Reset inactivity timer

    SystemTelemetry tel;
    getSystemTelemetry(tel);

    switch (key) {
        case '1': // Start Motor
            stopVoicePrompt();
            if (queueStartCommand(0)) {
                playVoicePrompt(PROMPT_STARTING);
            } else {
                playVoicePrompt(PROMPT_ERROR);
            }
            break;

        case '2': // Stop Motor
            stopVoicePrompt();
            if (queueStopCommand()) {
                playVoicePrompt(PROMPT_STOPPING);
            } else {
                playVoicePrompt(PROMPT_ERROR);
            }
            break;

        case '3': // Query Motor Status
            stopVoicePrompt();
            
            // Check if water level is low first (<20%) to announce alert
            if (tel.water_level < 20.0f) {
                playVoicePrompt(PROMPT_LOW_WATER);
                delay(3000); // Allow warning voice prompt to finish
            }

            if (tel.state == STATE_RUNNING) {
                playVoicePrompt(PROMPT_STATUS_RUN);
            } else if (tel.state == STATE_OFF) {
                playVoicePrompt(PROMPT_STATUS_OFF);
            } else if (tel.state == STATE_FAULT) {
                playVoicePrompt(PROMPT_STATUS_FAULT);
            } else {
                playVoicePrompt(PROMPT_STATUS_OFF);
            }
            break;

        case '0': // Repeat Menu
            stopVoicePrompt();
            playVoicePrompt(PROMPT_MENU);
            break;

        default:
            playVoicePrompt(PROMPT_INVALID_KEY);
            break;
    }
}

void processIvrTimeout() {
    if (ivrCallActive && (millis() - ivrSessionTimer > 30000)) { // 30s timeout
        Serial.println("IVR Session inactive. Terminating call...");
        Serial2.println("ATH"); // Strobe hangup
        terminateIvrSession();
    }
}

void terminateIvrSession() {
    ivrCallActive = false;
    activeCaller = "";
    ivrSessionTimer = 0;
    Serial.println("IVR Session Ended.");
}
