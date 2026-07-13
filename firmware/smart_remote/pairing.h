#ifndef PAIRING_H
#define PAIRING_H

#include <Arduino.h>

#define PIN_CS    10
#define PIN_GDO0  2

// RF Command Constants
#define RF_CMD_NONE      0x00
#define RF_CMD_START     0x01
#define RF_CMD_STOP      0x02
#define RF_CMD_STATUS    0x03
#define RF_CMD_PAIR      0x04
#define RF_CMD_PAIR_ACK  0x05
#define RF_CMD_STATUS_ACK 0x06

struct __attribute__((__packed__)) RemotePacket {
    uint8_t sync_byte;
    uint32_t remote_id;
    uint8_t cmd_state;
    uint8_t water_level;
    uint8_t battery_level;
    uint8_t power_available;
    uint8_t motor_state;
    uint8_t checksum;
};

void initRFTransceiver();
void sendRFCommand(uint8_t cmd, uint32_t remoteId, int batteryPct);
bool listenRFResponse(RemotePacket &dest, uint32_t timeoutMs);

#endif // PAIRING_H
