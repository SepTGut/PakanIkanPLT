/**
 * @file rtc_manager.h
 * @brief RTC (Real-Time Clock) module interface
 *
 * Provides time-keeping via the DS1307 RTC module over I2C.
 * Time is cached and refreshed once per second to minimize I2C traffic.
 * Includes validity checking to detect RTC communication failures.
 */

#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include "RTClib.h"

/**
 * @brief Time data structure returned by getCurrentTime().
 *        All fields are integers; dayName points to a string in config.h.
 */
struct TimeData {
    int hour;           ///< Hour (0–23)
    int minute;         ///< Minute (0–59)
    int second;         ///< Second (0–59)
    int day;            ///< Day of month (1–31)
    int month;          ///< Month (1–12)
    int year;           ///< Full year (e.g. 2026)
    const char* dayName;///< Day of week string (e.g. "Senin")
};

/**
 * @brief Initialize the DS1307 RTC module.
 *        Must be called once in setup(). Primes the time cache.
 *        Sets rtcValidFlag based on whether the RTC responds and
 *        returns a reasonable year (2000–2100).
 */
void initRTC();

/**
 * @brief Check if the RTC is responding and returning valid data.
 *        Returns false if RTC is disconnected, has a dead battery,
 *        or returns an out-of-range year.
 */
bool isRTCValid();

/**
 * @brief Get the current time from the DS1307 RTC.
 *        Time is cached and only refreshed once per second.
 *        Returns a TimeData struct with all fields populated.
 */
TimeData getCurrentTime();

#endif // RTC_MANAGER_H
