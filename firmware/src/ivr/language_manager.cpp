#include "language_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../safety_monitor.h"

static DynamicJsonDocument doc(1024);
static bool configLoaded = false;

bool initLanguageManager() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed inside Language Manager.");
        return false;
    }

    File configFile = SPIFFS.open("/voice/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open voice config.json from flash SPIFFS.");
        return false;
    }

    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Serial.println("Failed to parse voice config.json JSON.");
        return false;
    }

    configLoaded = true;
    Serial.println("Language Manager Voice Config Loaded.");
    return true;
}

void setLanguage(Language lang) {
    setSystemLanguage(lang);
}

Language getLanguage() {
    SystemTelemetry tel;
    getSystemTelemetry(tel);
    return tel.active_language;
}

String getAudioPath(int promptId) {
    if (!configLoaded) {
        // Fallback if config is missing
        String prefix = "/voice/english/";
        if (getLanguage() == LANG_TEL) prefix = "/voice/telugu/";
        else if (getLanguage() == LANG_HIN) prefix = "/voice/hindi/";

        if (promptId == 1) return prefix + "menu.wav";
        if (promptId == 2) return prefix + "starting.wav";
        if (promptId == 3) return prefix + "stopping.wav";
        if (promptId == 4) return prefix + "status_running.wav";
        if (promptId == 5) return prefix + "status_off.wav";
        if (promptId == 6) return prefix + "status_fault.wav";
        if (promptId == 7) return prefix + "low_water.wav";
        return prefix + "error.wav";
    }

    // Load directory prefix
    String langFolder = "english";
    if (getLanguage() == LANG_TEL) langFolder = "telugu";
    else if (getLanguage() == LANG_HIN) langFolder = "hindi";

    // Read filename from parsed JSON config
    const char* filename = doc["prompts"][String(promptId).c_str()];
    if (!filename) {
        return "/voice/" + langFolder + "/error.wav";
    }

    return "/voice/" + langFolder + "/" + String(filename);
}
