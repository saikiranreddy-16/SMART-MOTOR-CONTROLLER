#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#include "../config.h"

// Initialize SPIFFS and parse voice/config.json
bool initLanguageManager();

// Set the active language (saves to NVS flash)
void setLanguage(Language lang);

// Get the active language
Language getLanguage();

// Resolve a prompt ID to a full file-system path
// Example: getAudioPath(1) -> "/voice/english/menu.wav"
String getAudioPath(int promptId);

#endif // LANGUAGE_MANAGER_H
