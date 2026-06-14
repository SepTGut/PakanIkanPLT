#ifndef FEEDING_H
#define FEEDING_H

#include <Arduino.h>
#include <Servo.h>
#include "rtc_manager.h"

// FeedingState stores the last successful feeding record in EEPROM
struct FeedingState {
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t session; // 0: Morning, 1: Afternoon, 2: Evening, 255: None
};

void initFeeding();
void startFeeding(int jumlah);
void updateFeeding();
void markFeedingComplete(TimeData time, int session);
bool hasFedToday(TimeData time, int session);

// Test the feeding servo (trigger via serial command)
void testServo();
int checkMissedFeeds(TimeData time);

// Load the last feeding state from EEPROM
FeedingState loadState();

#endif
