#include "display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initDisplay() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        for(;;);
    }
    display.clearDisplay();
    display.display();
}

void drawDashboard(const String &statusText, int waterLevel, bool powerAvailable, int batteryLevel, bool paired) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Header row
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("BATT:");
    display.print(batteryLevel);
    display.print("%");
    
    display.setCursor(75, 0);
    display.print(paired ? "PAIRED" : "UNPAIRED");
    
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    
    // Motor status
    display.setCursor(0, 16);
    display.print("PUMP STATUS:");
    display.setTextSize(2);
    display.setCursor(0, 26);
    display.print(statusText);
    
    // Sensor readings
    display.setTextSize(1);
    display.setCursor(0, 46);
    display.print("WATER:");
    display.print(waterLevel);
    display.print("%");
    
    display.setCursor(70, 46);
    display.print("POWER:");
    display.print(powerAvailable ? "YES" : "NO");
    
    display.display();
}

void drawTextScreen(const String &line1, const String &line2) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print(line1);
    display.setCursor(0, 36);
    display.print(line2);
    display.display();
}
