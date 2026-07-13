#include "telemetry.h"
#include "safety_monitor.h"
#include "pump_control.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID_TELEMETRY    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_UUID_CONTROL      "cba1a95e-18e1-4c2f-b2b9-e137351b26a9"

static BLEServer* pServer = NULL;
static BLECharacteristic* pTelemetryChar = NULL;
static BLECharacteristic* pControlChar = NULL;
static bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        // Restart advertising so it can connect again
        pServer->startAdvertising();
    }
};

class ControlCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            String command = String(value.c_str());
            command.trim();

            if (command.startsWith("START")) {
                int duration = 0;
                int spaceIndex = command.indexOf(' ');
                if (spaceIndex > 0) {
                    duration = command.substring(spaceIndex + 1).toInt();
                }
                queueStartCommand(duration);
            } 
            else if (command.equals("STOP")) {
                queueStopCommand();
            } 
            else if (command.equals("RESET")) {
                queueResetCommand();
            } 
            else if (command.startsWith("CONFIG")) {
                // Format: CONFIG <nominal_curr> <under_v> <over_v>
                int firstSpace = command.indexOf(' ');
                int secondSpace = command.indexOf(' ', firstSpace + 1);
                int thirdSpace = command.indexOf(' ', secondSpace + 1);

                if (firstSpace > 0 && secondSpace > 0 && thirdSpace > 0) {
                    float nomCurr = command.substring(firstSpace + 1, secondSpace).toFloat();
                    float underV = command.substring(secondSpace + 1, thirdSpace).toFloat();
                    float overV = command.substring(thirdSpace + 1).toFloat();
                    updateSafetyThresholds(nomCurr, underV, overV);
                }
            }
        }
    }
};

void initTelemetry() {
    // Unique device name with ESP32 mac address suffix
    uint64_t chipId = ESP.getEfuseMac();
    String deviceName = "SmartPump_" + String((uint16_t)(chipId >> 32), HEX);

    BLEDevice::init(deviceName.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create Telemetry Characteristic (Notify/Read)
    pTelemetryChar = pService->createCharacteristic(
        CHAR_UUID_TELEMETRY,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTelemetryChar->addDescriptor(new BLE2902());

    // Create Control Characteristic (Write)
    pControlChar = pService->createCharacteristic(
        CHAR_UUID_CONTROL,
        BLECharacteristic::PROPERTY_WRITE
    );
    pControlChar->setCallbacks(new ControlCallbacks());

    // Start Service
    pService->start();

    // Start Advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // helper for iPhone connections
    pAdvertising->setMinPreferred(0x12);
    pServer->startAdvertising();
}

void processTelemetryTask() {
    if (deviceConnected) {
        SystemTelemetry tel;
        getSystemTelemetry(tel);

        // Format telemetry packet as a CSV string to save BLE bandwidth
        // Format: State,LastFault,VR,VY,VB,IR,IY,IB,WaterLevel,SoilMoisture,CasingTemp
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
                 (int)tel.state,
                 (int)tel.last_fault,
                 tel.voltage[0], tel.voltage[1], tel.voltage[2],
                 tel.current[0], tel.current[1], tel.current[2],
                 tel.water_level,
                 tel.soil_moisture,
                 tel.casing_temp);

        pTelemetryChar->setValue(buffer);
        pTelemetryChar->notify();
    }
    delay(1000); // Wait 1 second before notifying again
}
