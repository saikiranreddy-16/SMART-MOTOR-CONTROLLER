#include "voice_manager.h"
#include "language_manager.h"
#include <HardwareSerial.h>

extern HardwareSerial Serial2;

void playVoicePrompt(int promptId) {
    // Resolve path (e.g. "/voice/english/menu.wav")
    String relativePath = getAudioPath(promptId);
    
    // Convert to SIM7600 drive format (e.g., "D:/voice/english/menu.wav")
    String sim7600Path = "D:" + relativePath;
    
    // Command to play local file over voice call
    String cmd = "AT+CREC=4,\"" + sim7600Path + "\",0,100";
    Serial2.println(cmd);
    Serial.println("[VoiceManager Play]: " + cmd);
}

void stopVoicePrompt() {
    Serial2.println("AT+CREC=5"); // Stop command strobe
    Serial.println("[VoiceManager Stop]");
}
