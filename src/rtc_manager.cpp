/**
 * @file rtc_manager.cpp
 * @brief RTC (Real-Time Clock) module implementation
 *
 * Manages the DS1307 RTC over I2C. Time is cached in RAM and refreshed
 * once per second to reduce I2C bus traffic. Validity is checked on
 * every refresh by verifying the year is in the range 2000–2100.
 */

#include "rtc_manager.h"
#include "config.h"

// DS1307 RTC instance (I2C address 0x68, fixed in hardware)
RTC_DS1307 rtc;

// Internal state tracking
bool rtcInitialized = false;   ///< true if rtc.begin() succeeded
bool rtcValidFlag   = false;   ///< true if RTC returns sane data

// Cached time data — updated once per second
static TimeData cachedTime;
static unsigned long lastUpdate = 0;  ///< millis() of last RTC read

void initRTC() {
#ifdef ARDUINO_ARCH_ESP32
    // Use configurable I2C pins (default: GPIO 21=SDA, GPIO 22=SCL)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
#endif
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        rtcInitialized = false;
        rtcValidFlag = false;
        return;
    }
    rtcInitialized = true;

    // Prime the cache with the first reading and validate
    DateTime now = rtc.now();
    if (now.year() >= 2000 && now.year() <= 2100) {
        rtcValidFlag = true;
    } else {
        rtcValidFlag = false;
    }

    cachedTime.hour   = now.hour();
    cachedTime.minute = now.minute();
    cachedTime.second = now.second();
    cachedTime.day    = now.day();
    cachedTime.month  = now.month();
    cachedTime.year   = now.year();
    cachedTime.dayName = DAYS_OF_THE_WEEK[now.dayOfTheWeek()];
    lastUpdate = millis();

    // Uncomment the following line to set the RTC to the compile-time date/time.
    // Only needed once — then re-upload with this line commented out.
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

bool isRTCValid() {
    return rtcValidFlag;
}

TimeData getCurrentTime() {
    // Refresh from RTC at most once per second
    if (millis() - lastUpdate >= 1000) {
        DateTime now = rtc.now();

        // Validate RTC data — year should be reasonable
        if (rtcInitialized && now.year() >= 2000 && now.year() <= 2100) {
            rtcValidFlag = true;
        } else {
            rtcValidFlag = false;
        }

        cachedTime.hour   = now.hour();
        cachedTime.minute = now.minute();
        cachedTime.second = now.second();
        cachedTime.day    = now.day();
        cachedTime.month  = now.month();
        cachedTime.year   = now.year();
        cachedTime.dayName = DAYS_OF_THE_WEEK[now.dayOfTheWeek()];
        lastUpdate = millis();
    }
    return cachedTime;
}
