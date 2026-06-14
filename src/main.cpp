#include <Arduino.h>
#include <avr/wdt.h>
#include "config.h"
#include "rtc_manager.h"
#include "display.h"
#include "feeding.h"

bool buttonState = false;
bool lastButtonState = false;
bool buttonPressedFlag = false;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

void setup() {
    Serial.begin(9600);

    initRTC();
    initDisplay();
    initFeeding();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    wdt_enable(WDTO_2S);

    // Check for missed feeds on boot
    if (isRTCValid()) {
        TimeData now = getCurrentTime();
        int missedSession = checkMissedFeeds(now);
        if (missedSession != -1) {
            Serial.print("Missed feed detected for session ");
            Serial.println(missedSession);
            startFeeding(JUMLAH_PAKAN);
            markFeedingComplete(now, missedSession);
        }
    }
}

void handleManualButton() {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (buttonState == LOW) {
            if (!buttonPressedFlag) {
                Serial.println("Button pressed - Kasih pakan manual");
                startFeeding(JUMLAH_PAKAN);
                buttonPressedFlag = true;
            }
        } else {
            buttonPressedFlag = false;
        }
    }
    lastButtonState = buttonState;
}

void loop() {
    wdt_reset();
    static bool rtcErrorDisplayed = false;

    if (!isRTCValid()) {
        if (!rtcErrorDisplayed) {
            Serial.println("RTC Error: Invalid or missing time data");
            showRTCError();
            rtcErrorDisplayed = true;
        }
        handleManualButton();
        updateFeeding();
        return;
    } else {
        // RTC is valid again; reset error flag so future errors are reported
        rtcErrorDisplayed = false;
    }

    // 1. Update Time Data (Cached inside rtc_manager)
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
    handleManualButton();

    // 5. Automatic Feeding Triggers
    static int lastTriggerSecond = -1;
    if (currentTime.second != lastTriggerSecond) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (currentTime.hour == SCHEDULE[s].hour &&
                currentTime.minute == SCHEDULE[s].minute &&
                !hasFedToday(currentTime, s)) {

                Serial.print("Automatic feeding triggered for session ");
                Serial.println(s);
                startFeeding(JUMLAH_PAKAN);
                markFeedingComplete(currentTime, s);
                break; // Trigger only one session per second
            }
        }
        lastTriggerSecond = currentTime.second;
    }

    // 6. Update Servo State Machine
    updateFeeding();
}
