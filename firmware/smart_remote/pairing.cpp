#include "pairing.h"
#include <SPI.h>

// Helper to calculate XOR Checksum
static uint8_t calculateChecksum(RemotePacket &pkt) {
    uint8_t* ptr = (uint8_t*)&pkt;
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(RemotePacket) - 1; i++) {
        sum ^= ptr[i];
    }
    return sum;
}

static void cc1101_writeReg(uint8_t reg, uint8_t val) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg);
    SPI.transfer(val);
    digitalWrite(PIN_CS, HIGH);
}

static void cc1101_strobe(uint8_t strobe) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(strobe);
    digitalWrite(PIN_CS, HIGH);
}

static void cc1101_writeBurst(uint8_t reg, uint8_t* buffer, uint8_t len) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0x40);
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(buffer[i]);
    }
    digitalWrite(PIN_CS, HIGH);
}

static void cc1101_readBurst(uint8_t reg, uint8_t* buffer, uint8_t len) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0xC0);
    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(PIN_CS, HIGH);
}

void initRFTransceiver() {
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_CS, HIGH);
    SPI.begin();
    
    cc1101_strobe(0x30); // Reset CC1101 SRES
    delay(10);
    
    // Config registers for GFSK packet operation
    cc1101_writeReg(0x02, 0x06); // IOCFG0
    cc1101_writeReg(0x06, sizeof(RemotePacket)); // PKTLEN
    cc1101_writeReg(0x07, 0x04); // PKTCTRL1
    cc1101_writeReg(0x08, 0x00); // PKTCTRL0
    
    // Frequency Settings (433MHz)
    cc1101_writeReg(0x0D, 0x10); // FREQ2
    cc1101_writeReg(0x0E, 0xB1); // FREQ1
    cc1101_writeReg(0x0F, 0x3B); // FREQ0
    
    // Modem configurations
    cc1101_writeReg(0x10, 0xCA); // MDMCFG4
    cc1101_writeReg(0x11, 0x83); // MDMCFG3
    cc1101_writeReg(0x12, 0x13); // MDMCFG2
    cc1101_writeReg(0x18, 0x18); // MCSM0
    
    // Calibration settings
    cc1101_writeReg(0x23, 0xE9); // FSCAL3
    cc1101_writeReg(0x24, 0x2A); // FSCAL2
    cc1101_writeReg(0x25, 0x00); // FSCAL1
    cc1101_writeReg(0x26, 0x1F); // FSCAL0
    
    cc1101_strobe(0x3A); // Flush RX (SFRX)
    cc1101_strobe(0x34); // Enter RX mode (SRX)
}

void sendRFCommand(uint8_t cmd, uint32_t remoteId, int batteryPct) {
    cc1101_strobe(0x36); // IDLE
    cc1101_strobe(0x3B); // Flush TX (SFTX)
    
    RemotePacket pkt = {
        0xAA,
        remoteId,
        cmd,
        0,
        (uint8_t)batteryPct,
        0,
        0,
        0
    };
    pkt.checksum = calculateChecksum(pkt);
    
    cc1101_writeBurst(0x3F, (uint8_t*)&pkt, sizeof(RemotePacket));
    cc1101_strobe(0x35); // STX
    delay(20);
    cc1101_strobe(0x34); // Enter RX Mode to wait for status ACK
}

bool listenRFResponse(RemotePacket &dest, uint32_t timeoutMs) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        digitalWrite(PIN_CS, LOW);
        SPI.transfer(0xFB | 0xC0);
        uint8_t rxBytesCount = SPI.transfer(0x00) & 0x7F;
        digitalWrite(PIN_CS, HIGH);

        if (rxBytesCount >= sizeof(RemotePacket)) {
            cc1101_readBurst(0x3F, (uint8_t*)&dest, sizeof(RemotePacket));
            cc1101_strobe(0x3A); // Flush RX
            cc1101_strobe(0x34); // RX mode
            
            if (dest.sync_byte == 0xAA && dest.checksum == calculateChecksum(dest)) {
                return true;
            }
        }
        delay(10);
    }
    return false;
}
