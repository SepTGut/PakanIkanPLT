#include "rtc_manager.h"
#include "config.h"

RTC_DS1307 rtc;
bool rtcInitialized = false;
bool rtcValidFlag = false;
static TimeData cachedTime;
static unsigned long lastUpdate = 0;

void initRTC() {
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        rtcInitialized = false;
        rtcValidFlag = false;
        return;
    }
    rtcInitialized = true;
    // After successful init, read the current time once to prime the cache and set validity.
    DateTime now = rtc.now();
    // Basic sanity check – year reasonable range
    if (now.year() >= 2000 && now.year() <= 2100) {
        rtcValidFlag = true;
    } else {
        rtcValidFlag = false;
    }
    // Populate cachedTime with the initial read
    cachedTime.hour = now.hour();
    cachedTime.minute = now.minute();
    cachedTime.second = now.second();
    cachedTime.day = now.day();
    cachedTime.month = now.month();
    cachedTime.year = now.year();
    cachedTime.dayName = DAYS_OF_THE_WEEK[now.dayOfTheWeek()];
    lastUpdate = millis();
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

bool isRTCValid() {
    return rtcValidFlag;
}

TimeData getCurrentTime() {
    if (millis() - lastUpdate >= 1000) {
        DateTime now = rtc.now();

        // Update validity flag based on data
        if (rtcInitialized && now.year() >= 2000 && now.year() <= 2100) {
            rtcValidFlag = true;
        } else {
            rtcValidFlag = false;
        }

        cachedTime.hour = now.hour();
        cachedTime.minute = now.minute();
        cachedTime.second = now.second();
        cachedTime.day = now.day();
        cachedTime.month = now.month();
        cachedTime.year = now.year();
        cachedTime.dayName = DAYS_OF_THE_WEEK[now.dayOfTheWeek()];
        lastUpdate = millis();
    }
    return cachedTime;
}
