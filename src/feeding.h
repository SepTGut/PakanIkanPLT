/**
 * @file feeding.h
 * @brief Feeding control module interface
 *
 * Manages the servo-driven food dispenser, EEPROM state tracking,
 * and missed-feed detection. The servo opens/closes the hopper gate
 * in cycles to dispense a controlled amount of food.
 *
 * EEPROM layout (address 0):
 *   [0] day    (uint8_t)  — day of last feeding
 *   [1] month  (uint8_t)  — month of last feeding
 *   [2] year   (uint16_t) — year of last feeding
 *   [4] session (uint8_t) — index of last fed session (255 = none)
 */

#ifndef FEEDING_H
#define FEEDING_H

#include <Arduino.h>
#include <Servo.h>
#include "rtc_manager.h"

/**
 * @brief Persistent feeding state stored in EEPROM.
 *        Tracks the last successful feeding to prevent duplicates
 *        and to detect missed feedings after power loss.
 */
struct FeedingState {
    uint8_t  day;     ///< Day of last feeding (1–31)
    uint8_t  month;   ///< Month of last feeding (1–12)
    uint16_t year;    ///< Year of last feeding
    uint8_t  session; ///< Index of last fed session (0–N), 255 = no feeding recorded
};

/**
 * @brief Initialize the feeding subsystem.
 *        Attaches the servo, moves to CLOSED position, then detaches
 *        to prevent jitter and reduce power consumption.
 */
void initFeeding();

/**
 * @brief Start a feeding cycle.
 * @param jumlah  Number of open/close cycles (more = more food)
 *
 * Checks the IR sensor first — if food level is LOW (HIGH on pin),
 * the feeding is aborted and an error is shown on the LCD.
 * Each cycle consists of: attach → OPEN → delay → CLOSED → delay.
 * The servo is detached when all cycles complete.
 */
void startFeeding(int jumlah);

/**
 * @brief Update the feeding state machine — call every loop iteration.
 *        Non-blocking: advances one step every 100ms.
 */
void updateFeeding();

/**
 * @brief Record a completed feeding in EEPROM.
 *        Only writes if the data actually changed (wear leveling).
 * @param time    Current time (date is recorded)
 * @param session Index of the feeding session that was completed
 */
void markFeedingComplete(TimeData time, int session);

/**
 * @brief Check if a specific session has already been fed today.
 * @param time    Current time
 * @param session Session index to check
 * @return true if the session was already fed today
 */
bool hasFedToday(TimeData time, int session);

/**
 * @brief Test the servo by opening and closing it once.
 *        Triggered via serial command 't'.
 */
void testServo();

/**
 * @brief Check for missed feedings after power loss.
 *        Compares the current time against the schedule and the last
 *        recorded feeding in EEPROM.
 * @param time Current time
 * @return Index of the earliest missed session, or -1 if none
 */
int checkMissedFeeds(TimeData time);

/**
 * @brief Load the last feeding state from EEPROM.
 *        Returns {255, 255, 0xFFFF, 255} on first boot (uninitialized EEPROM).
 */
FeedingState loadState();

/**
 * @brief Save runtime settings to EEPROM (called by web portal).
 *        Stores: buzzer toggle, display interval, servo angles, feed amount, schedule.
 * @return true if settings were saved successfully
 */
bool saveSettings();

/**
 * @brief Load runtime settings from EEPROM (called during initFeeding).
 *        Restores: buzzer toggle, display interval, servo angles, feed amount, schedule.
 * @return true if valid settings were found and loaded
 */
bool loadSettings();

#endif // FEEDING_H
