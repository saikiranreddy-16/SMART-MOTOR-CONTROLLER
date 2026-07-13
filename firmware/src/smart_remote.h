#ifndef SMART_REMOTE_H
#define SMART_REMOTE_H

#include "config.h"

// Command types inside the RF packet
#define RF_CMD_NONE      0x00
#define RF_CMD_START     0x01
#define RF_CMD_STOP      0x02
#define RF_CMD_STATUS    0x03
#define RF_CMD_PAIR      0x04
#define RF_CMD_PAIR_ACK  0x05
#define RF_CMD_STATUS_ACK 0x06

struct __attribute__((__packed__)) RemotePacket {
    uint8_t sync_byte;       // 0xAA (Start of frame indicator)
    uint32_t remote_id;      // Unique 4-byte serial ID of remote
    uint8_t cmd_state;       // RF command or response indicator
    uint8_t water_level;     // Water level percent (0-100)
    uint8_t battery_level;   // Battery level on remote (0-100)
    uint8_t power_available; // 1 = AC grid power available, 0 = unavailable
    uint8_t motor_state;     // 0 = OFF, 1 = RUNNING, 2 = FAULT
    uint8_t checksum;        // XOR checksum of all previous bytes
};

// Initialize Smart Remote receiver module
void initSmartRemote();

// Core process loop for reading packets and sending back status updates
void processSmartRemote();

#endif // SMART_REMOTE_H
