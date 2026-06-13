#include "display.h"
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long previousMillis = 0;
int displayMode = 0;

void initDisplay() {
    lcd.init();
    lcd.backlight();
}

void updateDisplay(const TimeData& time) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= DISPLAY_INTERVAL) {
        previousMillis = currentMillis;
        displayMode++;
        if (displayMode > 3) displayMode = 0;
        lcd.clear();
    }

    // Line 2: Time
    lcd.setCursor(0, 1);
    char timeBuffer[17];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d ", time.hour, time.minute, time.second);
    lcd.print(timeBuffer);

    // Line 1: Rotating Content
    lcd.setCursor(0, 0);
    char tempBuffer[20];
    char line1Buffer[17];

    if (displayMode == 0) {
        snprintf(tempBuffer, sizeof(tempBuffer), "%.3s,%02d/%02d/%d", time.dayName, time.day, time.month, time.year);
    } else if (displayMode == 1) {
        snprintf(tempBuffer, sizeof(tempBuffer), "%s: %02d:%02d", LABEL_PAGI, JAM_PAGI, MENIT_PAGI);
    } else if (displayMode == 2) {
        snprintf(tempBuffer, sizeof(tempBuffer), "%s: %02d:%02d", LABEL_SIANG, JAM_SIANG, MENIT_SIANG);
    } else if (displayMode == 3) {
        snprintf(tempBuffer, sizeof(tempBuffer), "%s: %02d:%02d", LABEL_SORE, JAM_SORE, MENIT_SORE);
    }

    snprintf(line1Buffer, sizeof(line1Buffer), "%-16s", tempBuffer);
    lcd.print(line1Buffer);
}
