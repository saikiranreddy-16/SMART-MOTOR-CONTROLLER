#ifndef INSTALLER_MODE_H
#define INSTALLER_MODE_H

#include <Arduino.h>

// Parse and execute technician installation terminal commands (received via BLE or Service Serial)
void processInstallerCommand(const String &serialCommand);

// Execute a factory hardware test sequence
void runFactoryTestMode();

#endif // INSTALLER_MODE_H
