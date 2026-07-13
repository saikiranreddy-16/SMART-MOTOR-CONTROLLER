#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// CC1101 Pin Settings (for Atmega328P / Arduino Pro Mini)
#define PIN_CS    10
#define PIN_GDO0  2
#define PIN_START_BTN  3
#define PIN_STOP_BTN   4
#define PIN_PAIR_BTN   5

// RF Packet definitions
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

// Generate unique ID on first startup or hardcode it
const uint32_t UNIQUE_REMOTE_ID = 0x54A3E912; 
uint32_t pairedControllerId = 0;
bool isPaired = false;

// Display States
String motorStatusText = "DISCONNECTED";
int waterLevelVal = 0;
bool powerAvailableVal = false;
int batteryLevelVal = 100;

// Calculate XOR checksum
uint8_t calculateChecksum(RemotePacket &pkt) {
    uint8_t* ptr = (uint8_t*)&pkt;
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(RemotePacket) - 1; i++) {
        sum ^= ptr[i];
    }
    return sum;
}

// Write to CC1101
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

void cc1101_writeBurst(uint8_t reg, uint8_t* buffer, uint8_t len) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0x40);
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(buffer[i]);
    }
    digitalWrite(PIN_CS, HIGH);
}

void cc1101_readBurst(uint8_t reg, uint8_t* buffer, uint8_t len) {
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(reg | 0xC0);
    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(PIN_CS, HIGH);
}

void initTransceiver() {
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_CS, HIGH);
    SPI.begin();
    
    cc1101_strobe(0x30); // Reset CC1101 SRES
    delay(10);
    
    // Default registers (433MHz, GFSK, same as main board)
    cc1101_writeReg(0x02, 0x06); // IOCFG0
    cc1101_writeReg(0x06, sizeof(RemotePacket)); // PKTLEN
    cc1101_writeReg(0x07, 0x04); // PKTCTRL1
    cc1101_writeReg(0x08, 0x00); // PKTCTRL0
    
    // Frequency Settings
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

void sendRFCommand(uint8_t cmd) {
    cc1101_strobe(0x36); // IDLE
    cc1101_strobe(0x3B); // Flush TX (SFTX)
    
    RemotePacket pkt = {
        0xAA,
        UNIQUE_REMOTE_ID,
        cmd,
        0,
        (uint8_t)batteryLevelVal,
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

void updateOLED() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Header row
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("BATT:");
    display.print(batteryLevelVal);
    display.print("%");
    
    display.setCursor(75, 0);
    display.print(isPaired ? "PAIRED" : "UNPAIRED");
    
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    
    // Motor status (Huge block)
    display.setCursor(0, 16);
    display.print("PUMP STATUS:");
    display.setTextSize(2);
    display.setCursor(0, 26);
    display.print(motorStatusText);
    
    // Sensor readings
    display.setTextSize(1);
    display.setCursor(0, 46);
    display.print("WATER:");
    display.print(waterLevelVal);
    display.print("%");
    
    display.setCursor(70, 46);
    display.print("POWER:");
    display.print(powerAvailableVal ? "YES" : "NO");
    
    display.display();
}

void setup() {
    Serial.begin(9600);
    
    pinMode(PIN_START_BTN, INPUT_PULLUP);
    pinMode(PIN_STOP_BTN, INPUT_PULLUP);
    pinMode(PIN_PAIR_BTN, INPUT_PULLUP);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        for(;;);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 16);
    display.print("Smart Remote v1.0");
    display.setCursor(0, 30);
    display.print("Booting RF Transceiver...");
    display.display();
    
    initTransceiver();
    delay(1000);
}

void loop() {
    // 1. Button scans
    if (digitalRead(PIN_START_BTN) == LOW) {
        display.clearDisplay();
        display.setCursor(0, 20);
        display.print("Transmitting START...");
        display.display();
        sendRFCommand(RF_CMD_START);
        delay(500);
    }
    else if (digitalRead(PIN_STOP_BTN) == LOW) {
        display.clearDisplay();
        display.setCursor(0, 20);
        display.print("Transmitting STOP...");
        display.display();
        sendRFCommand(RF_CMD_STOP);
        delay(500);
    }
    else if (digitalRead(PIN_PAIR_BTN) == LOW) {
        display.clearDisplay();
        display.setCursor(0, 20);
        display.print("Pairing with Base...");
        display.display();
        sendRFCommand(RF_CMD_PAIR);
        delay(1000);
    }

    // 2. Poll status updates every 5 seconds
    static uint32_t lastPoll = 0;
    if (millis() - lastPoll > 5000) {
        sendRFCommand(RF_CMD_STATUS);
        lastPoll = millis();
    }

    // 3. Listen for responses from controller
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(0xFB | 0xC0);
    uint8_t rxBytesCount = SPI.transfer(0x00) & 0x7F;
    digitalWrite(PIN_CS, HIGH);

    if (rxBytesCount >= sizeof(RemotePacket)) {
        RemotePacket packet;
        cc1101_readBurst(0x3F, (uint8_t*)&packet, sizeof(RemotePacket));
        cc1101_strobe(0x3A); // Flush
        cc1101_strobe(0x34); // RX

        if (packet.sync_byte == 0xAA && packet.checksum == calculateChecksum(packet)) {
            // Verify if command is pairing ACK
            if (packet.cmd_state == RF_CMD_PAIR_ACK) {
                isPaired = true;
                pairedControllerId = packet.remote_id;
            }

            if (packet.cmd_state == RF_CMD_STATUS_ACK) {
                waterLevelVal = packet.water_level;
                powerAvailableVal = (packet.power_available == 1);
                
                motorStatusText = whenStateString(packet.motor_state);
            }
        }
    }

    // 4. Update display
    updateOLED();
    delay(100);
}

String whenStateString(uint8_t state) {
    if (state == 1) return "RUNNING";
    if (state == 2) return "FAULT";
    return "OFF";
}
