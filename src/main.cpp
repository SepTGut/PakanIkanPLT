#include <Arduino.h>
#include "config.h"
#include "rtc_manager.h"
#include "display.h"
#include "feeding.h"

bool buttonState = false;
bool lastButtonState = false;

void setup() {
    Serial.begin(9600);

    initRTC();
    initDisplay();
    initFeeding();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
    // 1. Update Time Data
    TimeData currentTime = getCurrentTime();

    // 2. Log to Serial
    char serialBuffer[64];
    snprintf(serialBuffer, sizeof(serialBuffer), "%s, %02d-%02d-%d", currentTime.dayName, currentTime.day, currentTime.month, currentTime.year);
    Serial.println(serialBuffer);
    snprintf(serialBuffer, sizeof(serialBuffer), "%02d:%02d:%02d", currentTime.hour, currentTime.minute, currentTime.second);
    Serial.println(serialBuffer);
    Serial.println();

    // 3. Update Display
    updateDisplay(currentTime);

    // 4. Manual Button Check
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH) {
        delay(50); // basic debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("Button pressed - Kasih pakan manual");
            startFeeding(JUMLAH_PAKAN);
        }
    }
    lastButtonState = buttonState;

    // 5. Automatic Feeding Triggers
    static int lastTriggerSecond = -1;
    if (currentTime.second != lastTriggerSecond) {
        if ((currentTime.hour == JAM_PAGI && currentTime.minute == MENIT_PAGI && currentTime.second == 1) ||
            (currentTime.hour == JAM_SIANG && currentTime.minute == MENIT_SIANG && currentTime.second == 1) ||
            (currentTime.hour == JAM_SORE && currentTime.minute == MENIT_SORE && currentTime.second == 1)) {
            startFeeding(JUMLAH_PAKAN);
        }
        lastTriggerSecond = currentTime.second;
    }

    // 6. Update Servo State Machine
    updateFeeding();
}
