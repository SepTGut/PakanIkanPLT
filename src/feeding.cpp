#include "feeding.h"
#include "config.h"
#include "display.h"
#include <EEPROM.h>

Servo servoMekanik;
int feedCyclesRemaining = 0;
unsigned long lastServoMillis = 0;

FeedingState state;

void saveState() {
    // Only write to EEPROM if the state actually changed to reduce wear.
    FeedingState existing;
    EEPROM.get(0, existing);
    if (existing.day != state.day ||
        existing.month != state.month ||
        existing.year != state.year ||
        existing.session != state.session) {
        EEPROM.put(0, state);
    }
}

FeedingState loadState() {
    FeedingState loaded;
    EEPROM.get(0, loaded);
    return loaded;
}

void initFeeding() {
    servoMekanik.attach(SERVO_PIN);
    servoMekanik.write(SERVO_CLOSED);
    delay(100); // Give it time to move
    servoMekanik.detach();
}

void startFeeding(int jumlah) {
    // Show copyright splash at the start of every feeding
    showCopyright();
    delay(500);

    servoMekanik.attach(SERVO_PIN);
    feedCyclesRemaining = jumlah * 2;
}

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

void markFeedingComplete(TimeData time, int session) {
    state.day = time.day;
    state.month = time.month;
    state.year = time.year;
    state.session = session;
    saveState();
    Serial.print(F("Feeding recorded in EEPROM for session "));
    Serial.println(session);
}

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

bool hasFedToday(TimeData time, int session) {
    FeedingState last = loadState();
    if (last.day == (uint8_t)time.day &&
        last.month == (uint8_t)time.month &&
        last.year == (uint16_t)time.year &&
        last.session == (uint8_t)session) {
        return true;
    }
    return false;
}

int checkMissedFeeds(TimeData time) {
    FeedingState last = loadState();

    bool todaySame = (last.day == (uint8_t)time.day &&
                        last.month == (uint8_t)time.month &&
                        last.year == (uint16_t)time.year);

    int missedSession = -1;
    for (size_t s = 0; s < (size_t)NUM_SESSIONS; s++) {
        int schedHour = SCHEDULE[s].hour;
        int schedMin = SCHEDULE[s].minute;

        if (time.hour > schedHour || (time.hour == schedHour && time.minute >= schedMin)) {
            if (last.session == 255) {
                missedSession = s;
            } else if (!todaySame || last.session < s) {
                missedSession = s;
            }
        }
    }
    return missedSession;
}
