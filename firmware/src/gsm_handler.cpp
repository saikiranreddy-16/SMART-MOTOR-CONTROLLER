#include "gsm_handler.h"
#include "safety_monitor.h"
#include "pump_control.h"
#include <HardwareSerial.h>

// Use UART2 (Serial2) for SIM7600
#define gsmSerial Serial2

// Whitelist array (in production, loaded from NVS/EEPROM)
struct WhitelistedUser {
    String phone;
    String role;
};

static WhitelistedUser whitelist[5] = {
    { "+919876543210", "Owner" },
    { "+919111222333", "Family" },
    { "+919444555666", "Worker" }
};
static int whitelistCount = 3;

#include "ivr/ivr_engine.h"
#include "ivr/dtmf_decoder.h"

static bool callActive = false;
static String activeCallerNumber = "";

// Function declarations
void handleIncomingCall(const String &clipLine);
void handleIncomingSMS(const String &sender, const String &body);
void sendATCommand(const String &cmd, uint32_t timeoutMs = 1000);
String readSerialLine(uint32_t timeoutMs);
bool isNumberWhitelisted(const String &number, String &role);

// Boot SIM7600 module
void initGSM() {
    pinMode(PIN_GSM_PWRKEY, OUTPUT);
    pinMode(PIN_GSM_RESET, OUTPUT);
    
    // Hard reset line setup
    digitalWrite(PIN_GSM_RESET, HIGH);
    
    Serial.println("Power cycling SIM7600 modem...");
    digitalWrite(PIN_GSM_PWRKEY, HIGH);
    delay(500);
    digitalWrite(PIN_GSM_PWRKEY, LOW);  // Pull low to start power-on sequence
    delay(2000);                         // Hold for 2 seconds
    digitalWrite(PIN_GSM_PWRKEY, HIGH); // Release
    delay(5000);                         // Wait for boot up

    gsmSerial.begin(115200, SERIAL_8N1, PIN_GSM_RX, PIN_GSM_TX);
    
    // Flush serial buffers
    while (gsmSerial.available()) gsmSerial.read();

    // Initial setup AT commands
    sendATCommand("ATE0", 1000);       // Disable echo
    sendATCommand("AT+CMGF=1", 1000);   // Set SMS to Text Mode
    sendATCommand("AT+CNMI=2,2,0,0,0", 1000); // Direct SMS routing to serial
    sendATCommand("AT+CLIP=1", 1000);   // Enable Caller ID Presentation
    sendATCommand("AT+DDET=1,100,0,0", 1000); // Enable DTMF, 100ms duration, detect start/stop
    sendATCommand("AT+CSCLK=0", 1000); // Disable sleep mode to keep serial responsive

    Serial.println("GSM/SIM7600 Initialized Successfully.");
}

// Write AT Command and check OK response
void sendATCommand(const String &cmd, uint32_t timeoutMs) {
    gsmSerial.println(cmd);
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (gsmSerial.available()) {
            String resp = gsmSerial.readStringUntil('\n');
            resp.trim();
            if (resp.length() > 0) {
                Serial.println("[Modem Resp]: " + resp);
            }
        }
    }
}

// Send an SMS
bool sendSMS(const String &phoneNumber, const String &message) {
    Serial.println("Sending SMS to " + phoneNumber + "...");
    gsmSerial.print("AT+CMGS=\"");
    gsmSerial.print(phoneNumber);
    gsmSerial.println("\"");
    delay(200);
    
    gsmSerial.print(message);
    delay(100);
    gsmSerial.write(0x1A); // Send Ctrl+Z to submit SMS
    
    uint32_t start = millis();
    while (millis() - start < 5000) {
        if (gsmSerial.available()) {
            String line = gsmSerial.readStringUntil('\n');
            line.trim();
            if (line.indexOf("+CMGS:") >= 0) {
                Serial.println("SMS Sent successfully.");
                return true;
            }
        }
    }
    Serial.println("SMS sending failed.");
    return false;
}

// Check Whitelist
bool isNumberWhitelisted(const String &number, String &role) {
    for (int i = 0; i < whitelistCount; i++) {
        if (number.endsWith(whitelist[i].phone) || whitelist[i].phone.endsWith(number)) {
            role = whitelist[i].role;
            return true;
        }
    }
    return false;
}

// Non-blocking GSM reader loop
void processGSM() {
    if (gsmSerial.available()) {
        String line = gsmSerial.readStringUntil('\n');
        line.trim();

        if (line.length() > 0) {
            Serial.println("[Modem Event]: " + line);

            // Handle Ringing / Caller ID (+CLIP: "+919876543210",145,,,,0)
            if (line.startsWith("+CLIP:")) {
                handleIncomingCall(line);
            }

            // Handle DTMF Key Press (+DTMF: 1)
            else if (line.startsWith("+DTMF:")) {
                char key = parseDTMFTone(line);
                if (key != '\0') {
                    handleIvrDTMF(key);
                }
            }

            // Handle incoming SMS (+CMT: "+919876543210",,"2026/06/29...")
            else if (line.startsWith("+CMT:")) {
                int firstQuote = line.indexOf('"');
                int secondQuote = line.indexOf('"', firstQuote + 1);
                String sender = line.substring(firstQuote + 1, secondQuote);
                
                // Read next line which is the body of SMS
                uint32_t waitStart = millis();
                while (!gsmSerial.available() && (millis() - waitStart < 1000));
                
                if (gsmSerial.available()) {
                    String body = gsmSerial.readStringUntil('\n');
                    body.trim();
                    handleIncomingSMS(sender, body);
                }
            }

            // Handle call hang-ups
            else if (line.equals("NO CARRIER") || line.equals("BUSY") || line.equals("NO ANSWER")) {
                Serial.println("Call Disconnected.");
                callActive = false;
                terminateIvrSession();
            }
        }
    }

    // Call active timeouts
    if (callActive) {
        processIvrTimeout();
    }
}

// Caller ID Handling
void handleIncomingCall(const String &clipLine) {
    int startIdx = clipLine.indexOf('"') + 1;
    int endIdx = clipLine.indexOf('"', startIdx);
    String number = clipLine.substring(startIdx, endIdx);
    number.replace(" ", "");

    Serial.println("Incoming call from: " + number);
    
    String role = "";
    if (isNumberWhitelisted(number, role)) {
        Serial.println("Caller authorized (" + role + "). Answering call...");
        sendATCommand("ATA", 2000); // Answer command
        callActive = true;
        activeCallerNumber = number;
        
        // Handover session to ivr engine
        startIvrSession(number);
    } else {
        Serial.println("Unauthorized number. Rejecting call.");
        sendATCommand("ATH", 1000); // Hang up command
    }
}

// SMS Processing Engine
void handleIncomingSMS(const String &sender, const String &body) {
    Serial.println("SMS Command received from: " + sender + " | Body: " + body);
    
    String role = "";
    if (!isNumberWhitelisted(sender, role)) {
        Serial.println("SMS ignored: Unauthorized number.");
        return;
    }

    String uppercaseCmd = body;
    uppercaseCmd.toUpperCase();
    uppercaseCmd.trim();

    SystemTelemetry tel;
    getSystemTelemetry(tel);

    if (uppercaseCmd.equals("ON")) {
        // Farm Worker role is restricted from turning pump ON
        if (role.equals("Worker")) {
            sendSMS(sender, "SF DENIED: WORKER CANNOT START PUMP.");
            return;
        }
        
        if (queueStartCommand(0)) {
            sendSMS(sender, "SF PUMP START COMMAND RECEIVED. CHECKING SAFETY...");
        } else {
            sendSMS(sender, "SF ERROR: START QUEUE FULL.");
        }
    } 
    else if (uppercaseCmd.equals("OFF")) {
        if (queueStopCommand()) {
            sendSMS(sender, "SF PUMP STOP COMMAND RECEIVED.");
        } else {
            sendSMS(sender, "SF ERROR: STOP QUEUE FULL.");
        }
    } 
    else if (uppercaseCmd.equals("STATUS")) {
        char resp[160];
        snprintf(resp, sizeof(resp), 
                 "SF STATUS: %s\nVolt: R:%.0f Y:%.0f B:%.0f\nCurr: R:%.1f Y:%.1f B:%.1f\nWater: %.0f%%\nSoil: %.0f%%\nTemp: %.0fC",
                 (tel.state == STATE_RUNNING) ? "RUNNING" : ((tel.state == STATE_OFF) ? "OFF" : "FAULT"),
                 tel.voltage[0], tel.voltage[1], tel.voltage[2],
                 tel.current[0], tel.current[1], tel.current[2],
                 tel.water_level, tel.soil_moisture, tel.casing_temp);
        sendSMS(sender, String(resp));
    } 
    else if (uppercaseCmd.equals("VOLT")) {
        char resp[100];
        snprintf(resp, sizeof(resp), "SF VOLTAGE: R:%.0f Y:%.0f B:%.0f. Balance: OK. Status: %s",
                 tel.voltage[0], tel.voltage[1], tel.voltage[2],
                 tel.power_available ? "POWER OK" : "POWER FAIL");
        sendSMS(sender, String(resp));
    } 
    else if (uppercaseCmd.equals("RESET")) {
        if (role.equals("Owner") || role.equals("Family")) {
            if (queueResetCommand()) {
                sendSMS(sender, "SF LOCKOUT RESET SUCCESSFUL.");
            }
        } else {
            sendSMS(sender, "SF ERROR: UNAUTHORIZED ROLE FOR RESET.");
        }
    } 
    else if (uppercaseCmd.startsWith("LANG ")) {
        String langCode = uppercaseCmd.substring(5);
        langCode.trim();
        if (langCode.equals("ENG")) {
            setSystemLanguage(LANG_ENG);
            sendSMS(sender, "SF LANGUAGE UPDATED TO ENGLISH.");
        } else if (langCode.equals("TEL")) {
            setSystemLanguage(LANG_TEL);
            sendSMS(sender, "SF LANGUAGE UPDATED TO TELUGU.");
        } else if (langCode.equals("HIN")) {
            setSystemLanguage(LANG_HIN);
            sendSMS(sender, "SF LANGUAGE UPDATED TO HINDI.");
        } else {
            sendSMS(sender, "SF ERROR: SUPPORTED LANGUAGES ARE ENG, TEL, HIN.");
        }
    }
    else {
        sendSMS(sender, "SF ERROR: INVALID COMMAND. OPTIONS: ON, OFF, STATUS, VOLT, RESET, LANG <CODE>.");
    }
}
