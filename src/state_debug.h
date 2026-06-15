/**
 * @file state_debug.h
 * @brief System state debug output interface
 *
 * Provides serial debug output for system diagnostics.
 * Triggered via serial command 's' (detailed) or 'p' (brief).
 */

#ifndef STATE_DEBUG_H
#define STATE_DEBUG_H

#include "rtc_manager.h"
#include "feeding.h"
#include "config.h"

/**
 * @brief Print a complete system state snapshot to Serial.
 *        Includes: RTC time, feeding schedule, EEPROM state.
 *        Triggered via serial command 's'.
 */
void printSystemState();

#endif // STATE_DEBUG_H
