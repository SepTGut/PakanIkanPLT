#include <Arduino.h>
#include <avr/wdt.h>
#include "config.h"
#include "rtc_manager.h"
#include "display.h"
#include "feeding.h"
#include "state_debug.h"

bool buttonState = false;
bool lastButtonState = false;
bool buttonPressedFlag = false;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

static const unsigned long IDLE_TIMEOUT = 10000;
static unsigned long lastActivityTime = 0;

static void markActivity() {
    lastActivityTime = millis();
    if (isIdleAnimating()) stopIdleAnimation();
}

void setup() {
    Serial.begin(9600);

    initRTC();
    getCurrentTime();
    initDisplay();

    playCopyrightAnimation();
    initFeeding();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    wdt_enable(WDTO_2S);

    lastActivityTime = millis();

    if (isRTCValid()) {
        TimeData now = getCurrentTime();
        int missedSession = checkMissedFeeds(now);
        if (missedSession != -1) {
            Serial.print(F("Missed feed detected for session "));
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
                Serial.println(F("Button pressed - Kasih pakan manual"));
                markActivity();
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
            Serial.println(F("RTC Error: Invalid or missing time data"));
            showRTCError();
            rtcErrorDisplayed = true;
        }
        handleManualButton();
        updateFeeding();
        return;
    } else {
        rtcErrorDisplayed = false;
    }

    TimeData currentTime = getCurrentTime();
    handleManualButton();

    if (Serial.available() > 0) {
        char cmd = Serial.read();
        markActivity();
        if (cmd == 's' || cmd == 'S' || cmd == 'p' || cmd == 'P') {
            printSystemState();
        } else if (cmd == 't' || cmd == 'T') {
            testServo();
        }
    }

    static int lastTriggerSecond = -1;
    if (currentTime.second != lastTriggerSecond) {
        for (size_t s = 0; s < (size_t)NUM_SESSIONS; s++) {
            if (currentTime.hour == SCHEDULE[s].hour &&
                currentTime.minute == SCHEDULE[s].minute &&
                !hasFedToday(currentTime, s)) {

                Serial.print(F("Automatic feeding triggered for session "));
                Serial.println(s);
                markActivity();
                startFeeding(JUMLAH_PAKAN);
                markFeedingComplete(currentTime, s);
                break;
            }
        }
        lastTriggerSecond = currentTime.second;
    }

    updateDisplay(currentTime);

    unsigned long now = millis();
    if (!isIdleAnimating() && (now - lastActivityTime >= IDLE_TIMEOUT)) {
        startIdleAnimation();
    }
    if (isIdleAnimating()) {
        updateIdleAnimation();
    }

    updateFeeding();
}
