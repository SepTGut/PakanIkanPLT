#include "rtc_manager.h"
#include "config.h"

RTC_DS1307 rtc;

void initRTC() {
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1) delay(10);
    }
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

TimeData getCurrentTime() {
    DateTime now = rtc.now();
    TimeData data;
    data.hour = now.hour();
    data.minute = now.minute();
    data.second = now.second();
    data.day = now.day();
    data.month = now.month();
    data.year = now.year();
    data.dayName = DAYS_OF_THE_WEEK[now.dayOfTheWeek()];
    return data;
}
