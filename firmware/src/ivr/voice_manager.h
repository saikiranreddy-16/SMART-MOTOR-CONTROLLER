#ifndef VOICE_MANAGER_H
#define VOICE_MANAGER_H

#include "../config.h"

// Trigger playing a WAV prompt file on the SIM7600 voice channel
void playVoicePrompt(int promptId);

// Stop any active audio play on the modem
void stopVoicePrompt();

#endif // VOICE_MANAGER_H
