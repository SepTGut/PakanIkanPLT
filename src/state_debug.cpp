/**
 * @file state_debug.cpp
 * @brief System state debug output implementation
 *
 * Prints a formatted snapshot of the system state to the serial port.
 * Useful for debugging without a connected LCD.
 */

#include "state_debug.h"
#include <Arduino.h>

void printSystemState() {
    Serial.println(F("=== System State ==="));

    // 1. Current RTC time (cached, refreshed once per second)
    TimeData now = getCurrentTime();
    Serial.print(F("RTC Time: "));
    Serial.print(now.hour);
    Serial.print(':');
    Serial.print(now.minute);
    Serial.print(':');
    Serial.print(now.second);
    Serial.print(F(", Date: "));
    Serial.print(now.day);
    Serial.print('/');
    Serial.print(now.month);
    Serial.print('/');
    Serial.println(now.year);

    // 2. Feeding schedule array
    Serial.println(F("Feeding Schedule:"));
    for (size_t i = 0; i < (size_t)NUM_SESSIONS; ++i) {
        Serial.print(F("  "));
        Serial.print(SCHEDULE[i].label);
        Serial.print(F(": "));
        Serial.print(SCHEDULE[i].hour);
        Serial.print(F(":"));
        Serial.println(SCHEDULE[i].minute);
    }

    // 3. EEPROM saved feeding state
    FeedingState last = loadState();
    Serial.println(F("Last Feeding Record (EEPROM):"));
    Serial.print(F("  Day: "));
    Serial.print(last.day);
    Serial.print(F(", Month: "));
    Serial.print(last.month);
    Serial.print(F(", Year: "));
    Serial.print(last.year);
    Serial.print(F(", Session: "));
    if (last.session == 255) {
        Serial.println(F("None"));
    } else {
        Serial.println(last.session);
    }

    // 4. System info
    Serial.println(F("Watchdog: enabled (2 s timeout)"));
    Serial.println(F("===================="));
}
