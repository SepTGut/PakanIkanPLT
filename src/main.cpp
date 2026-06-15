#include <Arduino.h>
#include "config.h"
#include "rtc_manager.h"
#include "display.h"
#include "feeding.h"
#include "state_debug.h"
#include "alerts.h"
#include "web_portal.h"

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
    Serial.begin(115200);

    // Hardware Init
    pinMode(BUZZER_1_PIN, OUTPUT);
    pinMode(BUZZER_2_PIN, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    digitalWrite(BUZZER_1_PIN, LOW);
    digitalWrite(BUZZER_2_PIN, LOW);

    initRTC();
    getCurrentTime();
    initDisplay();
    playCopyrightAnimation();
    initFeeding();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
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

    #ifdef ARDUINO_ARCH_ESP32
    initWebPortal();
    #endif
}

void handleManualButton() {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (buttonState == LOW) {
            if (!buttonPressedFlag) {
                Serial.println(F("Button pressed - Manual feed"));
                markActivity();
                triggerAlert(1, 100);
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
    handleBuzzer();

    #ifdef ARDUINO_ARCH_ESP32
    handleWebRequests();
    #endif

    static bool rtcErrorDisplayed = false;
    if (!isRTCValid()) {
        if (!rtcErrorDisplayed) {
            Serial.println(F("RTC Error!"));
            showRTCError();
            rtcErrorDisplayed = true;
        }
        triggerAlert(2, 500);
        handleManualButton();
        updateFeeding();
        return;
    } else {
        rtcErrorDisplayed = false;
    }

    TimeData currentTime = getCurrentTime();
    handleManualButton();

    if (digitalRead(IR_SENSOR_PIN) == HIGH) {
        static unsigned long lastLowFoodAlert = 0;
        if (millis() - lastLowFoodAlert > 30000) {
            triggerAlert(2, 200);
            lastLowFoodAlert = millis();
        }
    }

    if (Serial.available() > 0) {
        char cmd = Serial.read();
        markActivity();
        if (cmd == 's' || cmd == 'S' || cmd == 'p' || cmd == 'P') {
            printSystemState();
        } else if (cmd == 't' || cmd == 'T') {
            testServo();
        }
    }

    static int lastTriggerMinute = -1;
    static int lastTriggerDay = -1;
    if (currentTime.minute != lastTriggerMinute || currentTime.day != lastTriggerDay) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (currentTime.hour == SCHEDULE[s].hour &&
                currentTime.minute == SCHEDULE[s].minute &&
                !hasFedToday(currentTime, s)) {

                Serial.print(F("Auto feeding triggered: "));
                Serial.println(s);
                markActivity();
                triggerAlert(1, 100);
                startFeeding(JUMLAH_PAKAN);
                markFeedingComplete(currentTime, s);
                break;
            }
        }
        lastTriggerMinute = currentTime.minute;
        lastTriggerDay = currentTime.day;
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
