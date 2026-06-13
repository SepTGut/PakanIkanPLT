#include "display.h"
#include "config.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long previousMillis = 0;
int displayMode = 0;

void showRTCError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC Error!");
    lcd.setCursor(0, 1);
    lcd.print("Check Hardware");
}

void initDisplay() {
    lcd.init();
    lcd.backlight();
}

void updateDisplay(const TimeData& time) {
    unsigned long currentMillis = millis();
    static int lastSecond = -1;
    static int lastMode = -1;

    if (currentMillis - previousMillis >= DISPLAY_INTERVAL) {
        previousMillis = currentMillis;
        displayMode++;
        if (displayMode > NUM_SESSIONS) displayMode = 0;
    }

    // Update Line 2: Time (only if second changes)
    if (time.second != lastSecond) {
        lcd.setCursor(0, 1);
        char timeStr[10];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", time.hour, time.minute, time.second);
        char timeBuffer[17];
        snprintf(timeBuffer, sizeof(timeBuffer), "%-16s", timeStr);
        lcd.print(timeBuffer);
        lastSecond = time.second;
    }

    // Update Line 1: Rotating Content (only if mode changes)
    if (displayMode != lastMode) {
        lcd.setCursor(0, 0);
        char tempBuffer[20];
        char line1Buffer[17];

        if (displayMode == 0) {
            snprintf(tempBuffer, sizeof(tempBuffer), "%.3s,%02d/%02d/%d", time.dayName, time.day, time.month, time.year);
        } else {
            int sessionIdx = displayMode - 1;
            snprintf(tempBuffer, sizeof(tempBuffer), "%s: %02d:%02d",
                     SCHEDULE[sessionIdx].label,
                     SCHEDULE[sessionIdx].hour,
                     SCHEDULE[sessionIdx].minute);
        }

        snprintf(line1Buffer, sizeof(line1Buffer), "%-16s", tempBuffer);
        lcd.print(line1Buffer);
        lastMode = displayMode;
    }
}
