#include "feeding.h"
#include "config.h"
#include "display.h"
#include <EEPROM.h>

Servo servoMekanik;
int feedCyclesRemaining = 0;
unsigned long lastServoMillis = 0;

static FeedingState state;


void saveState() {
    FeedingState existing;
    EEPROM.get(0, existing);
    if (existing.day != state.day ||
        existing.month != state.month ||
        existing.year != state.year ||
        existing.session != state.session) {
        EEPROM.put(0, state);
        #ifdef ARDUINO_ARCH_ESP32
        EEPROM.commit();
        #endif
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
    delay(100); 
    servoMekanik.detach();
}

void startFeeding(int jumlah) {
    // Check IR Sensor before feeding
    if (digitalRead(IR_SENSOR_PIN) == HIGH) { // Assuming HIGH = Empty
        Serial.println(F("Feeding failed: Food level too low!"));
        showError("Food Low!", "Refill hopper");
        return;
    }

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

    // If no feeding has ever been recorded (session == 255 means None),
    // return the first session that has passed.
    if (last.session == 255) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (time.hour > SCHEDULE[s].hour ||
                (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
                return s;
            }
        }
        return -1;
    }

    // If last feeding was on a different day, find first session that has passed
    if (!todaySame) {
        for (int s = 0; s < NUM_SESSIONS; s++) {
            if (time.hour > SCHEDULE[s].hour ||
                (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
                return s;
            }
        }
        return -1;
    }

    // Same day: find first session after the last fed session that has passed
    for (int s = last.session + 1; s < NUM_SESSIONS; s++) {
        if (time.hour > SCHEDULE[s].hour ||
            (time.hour == SCHEDULE[s].hour && time.minute >= SCHEDULE[s].minute)) {
            return s;
        }
    }

    return -1;
}
