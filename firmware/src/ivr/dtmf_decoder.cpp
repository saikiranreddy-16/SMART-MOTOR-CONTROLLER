#include "dtmf_decoder.h"

char parseDTMFTone(const String &serialLine) {
    if (serialLine.startsWith("+DTMF:")) {
        int colonIdx = serialLine.indexOf(':');
        if (colonIdx >= 0) {
            String val = serialLine.substring(colonIdx + 1);
            val.trim();
            if (val.length() > 0) {
                return val.charAt(0); // Returns parsed key e.g. '1'
            }
        }
    }
    return '\0';
}
