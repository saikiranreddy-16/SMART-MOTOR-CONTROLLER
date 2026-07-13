#ifndef DTMF_DECODER_H
#define DTMF_DECODER_H

#include "../config.h"

// Parse DTMF indicator from serial line. Returns key char (e.g. '1') or '\0'
char parseDTMFTone(const String &serialLine);

#endif // DTMF_DECODER_H
