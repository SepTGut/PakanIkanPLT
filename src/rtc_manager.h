#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include "RTClib.h"

struct TimeData {
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
    const char* dayName;
};

void initRTC();
TimeData getCurrentTime();

#endif
