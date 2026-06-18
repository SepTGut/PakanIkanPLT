/**
 * @file ntp_sync.h
 * @brief NTP time synchronization interface (ESP32 only)
 *
 * Synchronizes the DS1307 RTC with internet NTP servers.
 * Called automatically when WiFi is connected.
 * Uses Indonesian NTP pool servers for best regional accuracy.
 */

#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32

/**
 * @brief Sync the DS1307 RTC with NTP servers.
 *        Call this after WiFi is connected.
 *        Updates the RTC via rtc.adjust() with the NTP time.
 * @return true if sync succeeded, false otherwise
 */
bool syncTimeNTP();

/**
 * @brief Check if NTP sync has been performed successfully.
 * @return true if time was synced at least once
 */
bool isNTPSynced();

/**
 * @brief Get the millis() of the last successful NTP sync.
 * @return millis() timestamp of last sync, or 0 if never synced
 */
unsigned long getLastNTPSyncTime();

#endif // ARDUINO_ARCH_ESP32

#endif // NTP_SYNC_H
