#ifndef SELF_TEST_H
#define SELF_TEST_H

#include <Arduino.h>

// Diagnostic status values
struct DiagnosticStatus {
    bool memory_ok;
    bool nvs_ok;
    bool gsm_ok;
    bool rf_ok;
    bool sensors_ok;
    String active_error_code; // Returns "E005" etc on fail
};

// Run POST sequence on startup. Returns true if everything is fully operational.
bool runPowerOnSelfTest(DiagnosticStatus &status);

#endif // SELF_TEST_H
