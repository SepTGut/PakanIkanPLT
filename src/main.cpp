/**
 * @file main.cpp
 * @brief Main application entry point for PakanIkanPLT (Automatic Fish Feeder)
 *
 * Boot sequence:
 *   1. Initialize serial, GPIO pins, RTC, LCD, feeding servo
 *   2. Play boot animation + copyright splash
 *   3. Check for missed feedings (after power loss)
 *   4. Start web portal (ESP32 only)
 *   5. Enter main loop
 *
 * Main loop handles:
 *   - Buzzer alert state machine
 *   - Web portal requests (ESP32)
 *   - RTC validity monitoring
 *   - Manual button (debounced)
 *   - IR sensor (low food detection)
 *   - Serial debug commands
 *   - Automatic feeding schedule
 *   - LCD display rotation
 *   - Idle animation
 *   - Feeding state machine
 *
 * Serial commands (115200 baud):
 *   's' — Detailed system state (RTC, schedule, EEPROM)
 *   'p' — Brief device info (same as 's' for now)
 *   't' — Test servo (open/close once)
 *   'b' — Test buzzer (100ms beep on buzzer 1)
 *   'f' — Manual feed (same as pressing the button)
 *   'r' — Read IR sensor state
 *   'e' — Show EEPROM feeding state
 *   'h' — Show help menu
 */

#include <Arduino.h>
#include "config.h"
#include "rtc_manager.h"
#include "display.h"
#include "feeding.h"
#include "state_debug.h"
#include "alerts.h"
#include "web_portal.h"

// ==========================================================================================
// BUTTON DEBOUNCE STATE
// ==========================================================================================
bool buttonState       = false;   ///< Current raw button reading
bool lastButtonState   = false;   ///< Previous raw button reading
bool buttonPressedFlag = false;   ///< true while button is held (prevents repeat)
unsigned long lastDebounceTime = 0;  ///< millis() of last state change
const unsigned long DEBOUNCE_DELAY = 50;  ///< Debounce interval (ms)

// ==========================================================================================
// IDLE ANIMATION TIMEOUT
// ==========================================================================================
static const unsigned long IDLE_TIMEOUT = 10000;  ///< Time before idle animation starts (ms)
static unsigned long lastActivityTime = 0;        ///< millis() of last user activity

/**
 * @brief Mark user activity. Resets the idle timeout and stops any running animation.
 */
static void markActivity() {
    lastActivityTime = millis();
    if (isIdleAnimating()) stopIdleAnimation();
}

// ==========================================================================================
// SERIAL COMMAND HANDLER
// ==========================================================================================

/**
 * @brief Print a brief device info summary to Serial.
 *        Triggered via serial command 'p'.
 */
static void printDeviceInfo() {
    Serial.println(F("=== Device Info ==="));
    Serial.print(F("Device: PakanIkanPLT (Automatic Fish Feeder)\n"));
    Serial.print(F("Board:  "));
#ifdef ARDUINO_ARCH_ESP32
    Serial.println(F("ESP32"));
#else
    Serial.println(F("Arduino Uno"));
#endif
    Serial.print(F("RTC:    "));
    Serial.println(isRTCValid() ? F("OK") : F("ERROR"));
    Serial.print(F("Feeds:  "));
    Serial.print(NUM_SESSIONS);
    Serial.println(F(" sessions/day"));
    Serial.print(F("Food:   "));
    Serial.println(digitalRead(IR_SENSOR_PIN) == HIGH ? F("LOW") : F("OK"));
    Serial.println(F("==================="));
}

/**
 * @brief Print the current IR sensor state to Serial.
 *        Triggered via serial command 'r'.
 */
static void readIRSensor() {
    int state = digitalRead(IR_SENSOR_PIN);
    Serial.print(F("IR Sensor: "));
    Serial.println(state == HIGH ? F("HIGH (empty)") : F("LOW (food present)"));
}

/**
 * @brief Print the EEPROM feeding state to Serial.
 *        Triggered via serial command 'e'.
 */
static void showEEPROMState() {
    FeedingState last = loadState();
    Serial.println(F("=== EEPROM State ==="));
    Serial.print(F("  Day:     "));
    Serial.println(last.day);
    Serial.print(F("  Month:   "));
    Serial.println(last.month);
    Serial.print(F("  Year:    "));
    Serial.println(last.year);
    Serial.print(F("  Session: "));
    if (last.session == 255) {
        Serial.println(F("None (first boot)"));
    } else {
        Serial.print(last.session);
        Serial.print(F(" ("));
        Serial.print(SCHEDULE[last.session].label);
        Serial.println(F(")"));
    }
    Serial.println(F("===================="));
}

/**
 * @brief Print the serial command help menu.
 *        Triggered via serial command 'h'.
 */
static void printHelpMenu() {
    Serial.println(F("=== Serial Commands ==="));
    Serial.println(F("  s — Detailed system state (RTC, schedule, EEPROM)"));
    Serial.println(F("  p — Brief device info"));
    Serial.println(F("  t — Test servo (open/close once)"));
    Serial.println(F("  b — Test buzzer (100ms beep)"));
    Serial.println(F("  f — Manual feed"));
    Serial.println(F("  r — Read IR sensor state"));
    Serial.println(F("  e — Show EEPROM feeding state"));
    Serial.println(F("  h — Show this help menu"));
    Serial.println(F("======================"));
}

/**
 * @brief Process a single serial command character.
 * @param cmd  The command character (case-insensitive for letters)
 */
static void handleSerialCommand(char cmd) {
    switch (cmd) {
        case 's':
        case 'S':
            printSystemState();
            break;
        case 'p':
        case 'P':
            printDeviceInfo();
            break;
        case 't':
        case 'T':
            testServo();
            break;
        case 'b':
        case 'B':
            Serial.println(F("Testing buzzer 1 (100ms)..."));
            triggerAlert(1, 100);
            break;
        case 'f':
        case 'F':
            Serial.println(F("Manual feed triggered via serial"));
            triggerAlert(1, 100);
            startFeeding(JUMLAH_PAKAN);
            break;
        case 'r':
        case 'R':
            readIRSensor();
            break;
        case 'e':
        case 'E':
            showEEPROMState();
            break;
        case 'h':
        case 'H':
        case '?':
            printHelpMenu();
            break;
        default:
            Serial.print(F("Unknown command: "));
            Serial.println(cmd);
            Serial.println(F("Type 'h' for help"));
            break;
    }
}

// ==========================================================================================
// SETUP
// ==========================================================================================

void setup() {
    Serial.begin(115200);

    // --- GPIO initialization ---
    pinMode(BUZZER_1_PIN, OUTPUT);
    pinMode(BUZZER_2_PIN, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    digitalWrite(BUZZER_1_PIN, LOW);
    digitalWrite(BUZZER_2_PIN, LOW);

    // --- Subsystem initialization ---
    initRTC();
    getCurrentTime();          // Prime the RTC cache
    initDisplay();             // LCD on, backlight on
    playCopyrightAnimation();  // Boot splash (backlight stays on)
    initFeeding();             // Servo to CLOSED, then detach

    // --- Button ---
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    lastActivityTime = millis();

    // --- Missed feed recovery ---
    // After power loss, check if any feeding sessions were missed
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

    // --- Web portal (ESP32 only) ---
    #ifdef ARDUINO_ARCH_ESP32
    initWebPortal();
    #endif
}

// ==========================================================================================
// MAIN LOOP
// ==========================================================================================

/**
 * @brief Handle the manual feed button with debouncing.
 *        Active LOW (INPUT_PULLUP). Triggers a single feed per press.
 */
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
    // --- Buzzer state machine (non-blocking) ---
    handleBuzzer();

    // --- Web portal (ESP32 only) ---
    #ifdef ARDUINO_ARCH_ESP32
    handleWebRequests();
    #endif

    // --- RTC error handling ---
    static bool rtcErrorDisplayed = false;
    if (!isRTCValid()) {
        if (!rtcErrorDisplayed) {
            Serial.println(F("RTC Error!"));
            showRTCError();
            rtcErrorDisplayed = true;
        }
        triggerAlert(2, 500);  // Alert buzzer every loop while RTC is bad
        handleManualButton();
        updateFeeding();
        return;  // Skip normal operation if RTC is not working
    } else {
        rtcErrorDisplayed = false;
    }

    // --- Get current time (cached, refreshed once per second) ---
    TimeData currentTime = getCurrentTime();

    // --- Manual button ---
    handleManualButton();

    // --- IR sensor: low food alert (rate-limited to once per 30 seconds) ---
    if (ENABLE_IR_SENSOR && digitalRead(IR_SENSOR_PIN) == HIGH) {
        static unsigned long lastLowFoodAlert = 0;
        if (millis() - lastLowFoodAlert > 30000) {
            triggerAlert(2, 200);
            lastLowFoodAlert = millis();
        }
    }

    // --- Serial command handler ---
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        markActivity();
        handleSerialCommand(cmd);
    }

    // --- Automatic feeding schedule ---
    // Check once per minute (when minute or day changes)
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

    // --- LCD display rotation ---
    updateDisplay(currentTime);

    // --- Idle animation (starts after IDLE_TIMEOUT ms of no activity) ---
    unsigned long now = millis();
    if (!isIdleAnimating() && (now - lastActivityTime >= IDLE_TIMEOUT)) {
        startIdleAnimation();
    }
    if (isIdleAnimating()) {
        updateIdleAnimation();
    }

    // --- Feeding state machine (non-blocking) ---
    updateFeeding();
}
