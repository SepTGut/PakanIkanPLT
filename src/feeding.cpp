#include "feeding.h"
#include "config.h"
#include <EEPROM.h>

Servo servoMekanik;
int feedCyclesRemaining = 0;
unsigned long lastServoMillis = 0;

struct FeedingState {
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t session; // 0: Morning, 1: Afternoon, 2: Evening, 255: None
} state;

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
    servoMekanik.write(0);
    delay(100); // Give it time to move
    servoMekanik.detach();
}

void startFeeding(int jumlah) {
    servoMekanik.attach(SERVO_PIN);
    feedCyclesRemaining = jumlah * 2;
}

void updateFeeding() {
    if (feedCyclesRemaining <= 0) return;
    if (millis() - lastServoMillis >= 100) {
        lastServoMillis = millis();
        if (feedCyclesRemaining % 2 == 0) {
            servoMekanik.write(150); // OPEN
        } else {
            servoMekanik.write(0);   // CLOSED
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
    Serial.print("Feeding recorded in EEPROM for session ");
    Serial.println(session);
}

bool hasFedToday(TimeData time, int session) {
    FeedingState last = loadState();
    // Cast time fields to unsigned types to match FeedingState members and avoid signed/unsigned warnings
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

    // Only check if the last feed was not today or was an earlier session today
    bool todaySame = (last.day == time.day && last.month == time.month && last.year == time.year);

    int missedSession = -1;
    for (size_t s = 0; s < (size_t)NUM_SESSIONS; s++) {
        int schedHour = SCHEDULE[s].hour;
        int schedMin = SCHEDULE[s].minute;

        if (time.hour > schedHour || (time.hour == schedHour && time.minute >= schedMin)) {
            // This session should have happened
            if (last.session == 255) {
                missedSession = s;
            } else if (!todaySame || last.session < s) {
                missedSession = s;
            }
        }
    }
    return missedSession;
}
