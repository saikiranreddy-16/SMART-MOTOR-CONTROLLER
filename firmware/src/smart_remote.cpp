#include "smart_remote.h"
#include "safety_monitor.h"
#include "pump_control.h"
#include <SPI.h>

// CC1101 SPI Registers
#define CC1101_IOCFG0        0x02
#define CC1101_FIFOTHR       0x03
#define CC1101_SYNC1         0x04
#define CC1101_SYNC0         0x05
#define CC1101_PKTLEN        0x06
#define CC1101_PKTCTRL1      0x07
#define CC1101_PKTCTRL0      0x08
#define CC1101_FSCTRL1       0x0B
#define CC1101_FREQ2         0x0D
#define CC1101_FREQ1         0x0E
#define CC1101_FREQ0         0x0F
#define CC1101_MDMCFG4       0x10
#define CC1101_MDMCFG3       0x11
#define CC1101_MDMCFG2       0x12
#define CC1101_MDMCFG1       0x13
#define CC1101_MDMCFG0       0x14
#define CC1101_DEVIATN       0x15
#define CC1101_MCSM0         0x18
#define CC1101_FOCCFG        0x19
#define CC1101_BSCFG         0x1A
#define CC1101_AGCCTRL2      0x1B
#define CC1101_AGCCTRL1      0x1C
#define CC1101_AGCCTRL0      0x1D
#define CC1101_FSCAL3        0x23
#define CC1101_FSCAL2        0x24
#define CC1101_FSCAL1        0x25
#define CC1101_FSCAL0        0x26

// Command Strobes
#define CC1101_SRES          0x30
#define CC1101_SFSTXON       0x31
#define CC1101_SXOFF         0x32
#define CC1101_SCAL          0x33
#define CC1101_SRX           0x34
#define CC1101_STX           0x35
#define CC1101_SIDLE         0x36
#define CC1101_SFRX          0x3A
#define CC1101_SFTX          0x3B

#define CC1101_FIFO          0x3F

static const int PIN_CS = 5;

// SPI Helper Functions
void cc1101_writeReg(uint8_t reg, uint8_t val) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg);
    SPI.transfer(val);
    digitalWrite(PIN_CS, HIGH);
}

void cc1101_strobe(uint8_t strobe) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(strobe);
    digitalWrite(PIN_CS, HIGH);
}

void cc1101_readBurst(uint8_t reg, uint8_t* buffer, uint8_t len) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0xC0); // Read + Burst bits
    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(PIN_CS, HIGH);
}

void cc1101_writeBurst(uint8_t reg, uint8_t* buffer, uint8_t len) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0x40); // Write + Burst bits
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(buffer[i]);
    }
    digitalWrite(PIN_CS, HIGH);
}

// Calculate simple XOR checksum
uint8_t calculateChecksum(RemotePacket &pkt) {
    uint8_t* ptr = (uint8_t*)&pkt;
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(RemotePacket) - 1; i++) {
        sum ^= ptr[i];
    }
    return sum;
}

// Initialize registers for 433MHz GFSK packet operation
void initSmartRemote() {
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_CS, HIGH);
    SPI.begin();

    // Reset transceiver
    cc1101_strobe(CC1101_SRES);
    delay(10);

    // Register configuration for 433.92 MHz GFSK, 38.4 kBaud
    cc1101_writeReg(CC1101_IOCFG0, 0x06);   // GDO0 asserts when sync word sent/received
    cc1101_writeReg(CC1101_FIFOTHR, 0x47);  // TX FIFO Threshold
    cc1101_writeReg(CC1101_SYNC1, 0xD3);    // Sync Word MSB
    cc1101_writeReg(CC1101_SYNC0, 0x91);    // Sync Word LSB
    cc1101_writeReg(CC1101_PKTLEN, sizeof(RemotePacket)); // Fixed Packet Length
    cc1101_writeReg(CC1101_PKTCTRL1, 0x04); // Append status, no addr check
    cc1101_writeReg(CC1101_PKTCTRL0, 0x00); // Fixed packet length mode, CRC disabled for custom check
    
    // Frequency Settings (433.92 MHz)
    cc1101_writeReg(CC1101_FREQ2, 0x10);
    cc1101_writeReg(CC1101_FREQ1, 0xB1);
    cc1101_writeReg(CC1101_FREQ0, 0x3B);
    
    // Modem Configuration
    cc1101_writeReg(CC1101_MDMCFG4, 0xCA);
    cc1101_writeReg(CC1101_MDMCFG3, 0x83);
    cc1101_writeReg(CC1101_MDMCFG2, 0x13); // GFSK, 30/32 sync word bits
    cc1101_writeReg(CC1101_MDMCFG1, 0x22);
    cc1101_writeReg(CC1101_MDMCFG0, 0xF8);
    cc1101_writeReg(CC1101_DEVIATN, 0x35);
    
    cc1101_writeReg(CC1101_MCSM0, 0x18);    // Auto-calibrate when transitioning from IDLE to RX/TX
    cc1101_writeReg(CC1101_FOCCFG, 0x16);
    cc1101_writeReg(CC1101_BSCFG, 0x6C);
    cc1101_writeReg(CC1101_AGCCTRL2, 0x43);
    cc1101_writeReg(CC1101_AGCCTRL1, 0x40);
    cc1101_writeReg(CC1101_AGCCTRL0, 0x91);
    
    // Calibration Settings
    cc1101_writeReg(CC1101_FSCAL3, 0xE9);
    cc1101_writeReg(CC1101_FSCAL2, 0x2A);
    cc1101_writeReg(CC1101_FSCAL1, 0x00);
    cc1101_writeReg(CC1101_FSCAL0, 0x1F);
    
    cc1101_strobe(CC1101_SFRX);             // Flush RX FIFO
    cc1101_strobe(CC1101_SRX);              // Enter RX Mode

    Serial.println("Smart Remote RF CC1101 Transceiver Initialized.");
}

void sendRFResponse(RemotePacket &resp) {
    cc1101_strobe(CC1101_SIDLE);
    cc1101_strobe(CC1101_SFTX); // Flush TX FIFO
    
    resp.checksum = calculateChecksum(resp);
    cc1101_writeBurst(CC1101_FIFO, (uint8_t*)&resp, sizeof(RemotePacket));
    
    cc1101_strobe(CC1101_STX); // Enter TX mode to transmit
    delay(20);                 // Wait for packet to clear over the air
    
    cc1101_strobe(CC1101_SRX); // Return back to RX mode
}

void processSmartRemote() {
    SystemTelemetry tel;
    getSystemTelemetry(tel);

    // GDO0 asserts high when sync word is matched. We check GDO0 on GPIO 19
    // In this module we check if there are bytes in the CC1101 RX FIFO
    // Since we are reading fixed packet sizes, check FIFO status
    
    // Read status byte for RX FIFO occupancy:
    digitalWrite(PIN_CS, LOW);
    uint8_t status = SPI.transfer(0xFB | 0xC0); // Read RXBYTES status register
    uint8_t rxBytesCount = SPI.transfer(0x00) & 0x7F;
    digitalWrite(PIN_CS, HIGH);

    if (rxBytesCount >= sizeof(RemotePacket)) {
        RemotePacket packet;
        cc1101_readBurst(CC1101_FIFO, (uint8_t*)&packet, sizeof(RemotePacket));
        cc1101_strobe(CC1101_SFRX); // Flush remaining bytes to prevent overflow
        cc1101_strobe(CC1101_SRX);  // Back to RX

        // Validate sync byte and checksum
        if (packet.sync_byte == 0xAA && packet.checksum == calculateChecksum(packet)) {
            Serial.print("RF Packet Received from Remote: ");
            Serial.println(packet.remote_id, HEX);

            // Handle pairing request
            if (packet.cmd_state == RF_CMD_PAIR) {
                // If paired_remote_id is not set, pair with the first device that requests it
                if (tel.paired_remote_id == 0) {
                    setSystemPairedRemote(packet.remote_id);
                    Serial.println("Paired successfully with Remote ID: " + String(packet.remote_id, HEX));
                    
                    RemotePacket ack = {0xAA, packet.remote_id, RF_CMD_PAIR_ACK, (uint8_t)tel.water_level, 100, (uint8_t)(tel.power_available ? 1 : 0), (uint8_t)tel.state, 0};
                    sendRFResponse(ack);
                }
                return;
            }

            // Reject commands if the ID does not match whitelisted paired ID
            if (packet.remote_id != tel.paired_remote_id) {
                Serial.println("RF Packet ignored: Remote ID mismatch.");
                return;
            }

            // Command Processing
            if (packet.cmd_state == RF_CMD_START) {
                queueStartCommand(0);
            } 
            else if (packet.cmd_state == RF_CMD_STOP) {
                queueStopCommand();
            } 

            // Send telemetry status update back to remote display screen
            RemotePacket response = {
                0xAA,
                tel.paired_remote_id,
                RF_CMD_STATUS_ACK,
                (uint8_t)tel.water_level,
                100, // remote battery (placeholder)
                (uint8_t)(tel.power_available ? 1 : 0),
                (uint8_t)tel.state,
                0
            };
            sendRFResponse(response);
        }
    }
}
