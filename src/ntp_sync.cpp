/**
 * @file ntp_sync.cpp
 * @brief NTP time synchronization implementation (ESP32 only)
 *
 * Syncs the DS1307 RTC with Indonesian NTP pool servers.
 * Uses multiple servers for redundancy.
 * Timezone: UTC+7 (WIB) — adjust as needed.
 */

#include "ntp_sync.h"

#ifdef ARDUINO_ARCH_ESP32

#include "config.h"
#include "rtc_manager.h"
#include <WiFi.h>
#include <time.h>

// Indonesian NTP pool servers
static const char* NTP_SERVERS[] = {
    "0.id.pool.ntp.org",
    "1.id.pool.ntp.org",
    "2.id.pool.ntp.org",
    "3.id.pool.ntp.org"
};

// Timezone offset in seconds (UTC+7 for WIB)
static const long GMT_OFFSET_SEC = 7 * 3600;
static const int DAYLIGHT_OFFSET_SEC = 0;

static bool ntpSynced = false;
static unsigned long lastSyncTime = 0;

bool syncTimeNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("NTP: WiFi not connected"));
        return false;
    }

    Serial.println(F("NTP: Syncing..."));

    // Configure NTP with multiple servers and timezone
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC,
               NTP_SERVERS[0], NTP_SERVERS[1], NTP_SERVERS[2]);

    // Wait for time to be set (up to 10 seconds)
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 1000000000L && retries < 20) {
        delay(500);
        now = time(nullptr);
        retries++;
        Serial.print(F("."));
    }
    Serial.println();

    if (now < 1000000000L) {
        Serial.println(F("NTP: Failed"));
        return false;
    }

    // Update RTC and cache via the public API
    setRTCTimeFromEpoch(now);

    ntpSynced = true;
    lastSyncTime = millis();

    Serial.println(F("NTP: Sync complete"));
    return true;
}

bool isNTPSynced() {
    return ntpSynced;
}

unsigned long getLastNTPSyncTime() {
    return lastSyncTime;
}

#endif // ARDUINO_ARCH_ESP32
