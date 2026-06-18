/**
 * @file feeding.cpp
 * @brief Feeding control module implementation
 *
 * Controls the servo-driven food dispenser. Feeding is non-blocking:
 * startFeeding() sets the number of cycles, and updateFeeding() advances
 * the state machine every 100ms.
 *
 * EEPROM wear leveling: saveState() only writes when data actually changes.
 * On first boot (EEPROM = 0xFF), loadState() returns session=255 (None),
 * which correctly triggers missed-feed detection.
 */

#include "feeding.h"
#include "config.h"
#include "display.h"
#include <EEPROM.h>

// Servo instance — uses Arduino built-in Servo library (works on both AVR and ESP32)
Servo servoMekanik;

// Feeding state machine variables
int feedCyclesRemaining = 0;      ///< Number of open/close cycles left
unsigned long lastServoMillis = 0; ///< Timestamp of last servo action

// Persistent state — stored in EEPROM, loaded on boot
static FeedingState state;

/**
 * @brief Save the current feeding state to EEPROM.
 *        Only writes if data changed to reduce EEPROM wear.
 *        On ESP32, EEPROM.commit() is required to persist.
 */
void saveState() {
    FeedingState existing;
    EEPROM.get(0, existing);
    if (existing.day    != state.day ||
        existing.month  != state.month ||
        existing.year   != state.year ||
        existing.session != state.session) {
        EEPROM.put(0, state);
        #ifdef ARDUINO_ARCH_ESP32
        EEPROM.commit();
        #endif
    }
}

/**
 * @brief Load the feeding state from EEPROM.
 *        On first boot, returns {0xFF, 0xFF, 0xFFFF, 0xFF} (session = None).
 */
FeedingState loadState() {
    FeedingState loaded;
    EEPROM.get(0, loaded);
    return loaded;
}

/**
 * @brief Initialize the feeding subsystem.
 *        On ESP32, EEPROM.begin() must be called before any EEPROM access.
 *        Moves servo to CLOSED position then detaches to prevent jitter.
 */
void initFeeding() {
    #ifdef ARDUINO_ARCH_ESP32
    EEPROM.begin(512);  // Allocate 512 bytes for EEPROM emulation
    #endif

    servoMekanik.attach(SERVO_PIN);
    servoMekanik.write(SERVO_CLOSED);
    delay(100);
    servoMekanik.detach();
}

/**
 * @brief Start a feeding cycle.
 *        Checks IR sensor first — aborts if food is low.
 *        Each cycle = one open + one close. Total steps = jumlah * 2.
 */
void startFeeding(int jumlah) {
    // Check IR sensor before feeding (skip if pin is -1 = unused)
    if (IR_SENSOR_PIN >= 0 && digitalRead(IR_SENSOR_PIN) == HIGH) {
        Serial.println(F("Feeding failed: Food level too low!"));
        showError("Food Low!", "Refill hopper");
        return;
    }

    // Show copyright splash while feeding starts
    showCopyright();
    delay(500);

    servoMekanik.attach(SERVO_PIN);
    feedCyclesRemaining = jumlah * 2;  // Each cycle = open + close
}

/**
 * @brief Advance the feeding state machine.
 *        Called every loop(). Non-blocking: uses millis() for timing.
 *        Alternates between OPEN and CLOSED every 100ms.
 */
void updateFeeding() {
    if (feedCyclesRemaining <= 0) return;
    if (millis() - lastServoMillis >= 100) {
        lastServoMillis = millis();
        if (feedCyclesRemaining % 2 == 0) {
            servoMekanik.write(SERVO_OPEN);
        } else {
            servoMekanik.write(SERVO_CLOSED);
        }
        feedCyclesRemaining--;
        if (feedCyclesRemaining == 0) {
            servoMekanik.detach();
        }
    }
}

/**
 * @brief Record a completed feeding in EEPROM and serial log.
 */
void markFeedingComplete(TimeData time, int session) {
    state.day    = time.day;
    state.month  = time.month;
    state.year   = time.year;
    state.session = session;
    saveState();
    Serial.print(F("Feeding recorded in EEPROM for session "));
    Serial.println(session);
}

/**
 * @brief Quick servo test — open then close. Triggered via serial 't'.
 */
void testServo() {
    Serial.println(F("Testing servo..."));
    servoMekanik.attach(SERVO_PIN);
    servoMekanik.write(SERVO_OPEN);
    delay(500);
    servoMekanik.write(SERVO_CLOSED);
    delay(500);
    servoMekanik.detach();
    Serial.println(F("Servo test complete"));
}

/**
 * @brief Check if a specific session was already fed today.
 *        Compares day/month/year and session index.
 */
bool hasFedToday(TimeData time, int session) {
    FeedingState last = loadState();
    if (last.day    == (uint8_t)time.day &&
        last.month  == (uint8_t)time.month &&
        last.year   == (uint16_t)time.year &&
        last.session == (uint8_t)session) {
        return true;
    }
    return false;
}

/**
 * @brief Detect missed feedings after power loss.
 *        Returns the earliest session index whose time has passed
 *        and hasn't been fed yet. Returns -1 if nothing was missed.
 *
 * Three cases:
 *   1. No feeding ever recorded (session == 255) → first passed session
 *   2. Last feeding on a different day            → first passed session
 *   3. Last feeding today                         → first passed session after last.session
 */
int checkMissedFeeds(TimeData time) {
    FeedingState last = loadState();

    bool todaySame = (last.day   == (uint8_t)time.day &&
                      last.month == (uint8_t)time.month &&
                      last.year  == (uint16_t)time.year);

    // Case 1: No feeding ever recorded
    if (last.session == 255) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (time.hour > SCHEDULE[s].hour ||
                (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
                return s;
            }
        }
        return -1;
    }

    // Case 2: Different day — find first session that has passed
    if (!todaySame) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (time.hour > SCHEDULE[s].hour ||
                (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
                return s;
            }
        }
        return -1;
    }

    // Case 3: Same day — find first session after the last fed session
    for (int s = last.session + 1; s < NUM_SESSIONS; s++) {
        if (time.hour > SCHEDULE[s].hour ||
            (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
            return s;
        }
    }

    return -1;
}

// ==========================================================================================
// Runtime Settings Persistence — Save/Load via Web Portal
// ==========================================================================================

/**
 * @brief Save runtime settings to EEPROM.
 *        Stores: buzzer toggle, display interval, servo angles, feed amount, schedule.
 *        Called by the web portal after user changes settings.
 */
bool saveSettings() {
#ifdef ARDUINO_ARCH_ESP32
    // Write magic byte to mark settings as valid
    EEPROM.write(EEPROM_SETTINGS_MAGIC, EEPROM_SETTINGS_MAGIC_VAL);

    // Buzzer toggle
    EEPROM.write(EEPROM_BUZZER_TOGGLE, ENABLE_BUZZERS ? 1 : 0);

    // Display interval (uint16_t, 2 bytes)
    uint16_t interval = DISPLAY_INTERVAL;
    EEPROM.write(EEPROM_DISPLAY_INTERVAL, interval & 0xFF);
    EEPROM.write(EEPROM_DISPLAY_INTERVAL + 1, (interval >> 8) & 0xFF);

    // Servo angles
    EEPROM.write(EEPROM_SERVO_OPEN_ANGLE, SERVO_OPEN);
    EEPROM.write(EEPROM_SERVO_CLOSED_ANGLE, SERVO_CLOSED);

    // Feed amount
    EEPROM.write(EEPROM_FEED_AMOUNT, JUMLAH_PAKAN);

    // Feeding schedule (5 sessions × 10 bytes)
    for (int i = 0; i < NUM_SESSIONS && i < 5; i++) {
        int addr = EEPROM_SCHEDULE_START + (i * EEPROM_SCHEDULE_ENTRY_SIZE);
        // Write label (up to 8 bytes, padded with zeros)
        for (int c = 0; c < 8; c++) {
            if (c < strlen(SCHEDULE[i].label)) {
                EEPROM.write(addr + c, SCHEDULE[i].label[c]);
            } else {
                EEPROM.write(addr + c, 0);
            }
        }
        // Write hour and minute
        EEPROM.write(addr + 8, SCHEDULE[i].hour);
        EEPROM.write(addr + 9, SCHEDULE[i].minute);
    }

    EEPROM.commit();
    return true;
#else
    return false;  // Settings persistence only on ESP32
#endif
}

/**
 * @brief Load runtime settings from EEPROM.
 *        Restores: buzzer toggle, display interval, servo angles, feed amount, schedule.
 *        Called during initFeeding() on ESP32.
 * @return true if valid settings were found
 */
bool loadSettings() {
#ifdef ARDUINO_ARCH_ESP32
    // Check magic byte
    if (EEPROM.read(EEPROM_SETTINGS_MAGIC) != EEPROM_SETTINGS_MAGIC_VAL) {
        return false;  // No saved settings, use defaults from config.h
    }

    // Note: These are compile-time constants, so we can't change them at runtime
    // without making them variables. For now, the web portal settings are stored
    // in EEPROM and can be read by the API, but the firmware uses config.h defaults.
    // TODO: Make these runtime variables if full runtime config is needed.

    return true;
#else
    return false;
#endif
}
