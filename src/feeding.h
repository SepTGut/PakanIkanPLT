#ifndef FEEDING_H
#define FEEDING_H

#include <Arduino.h>
#include <Servo.h>
#include "rtc_manager.h"

void initFeeding();
void startFeeding(int jumlah);
void updateFeeding();
void markFeedingComplete(TimeData time, int session);
bool hasFedToday(TimeData time, int session);
int checkMissedFeeds(TimeData time);

#endif
