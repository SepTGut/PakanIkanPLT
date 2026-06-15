/**
 * @file display.h
 * @brief LCD display module interface
 *
 * Manages the 16x2 I2C LCD display. Provides:
 *   - Time/schedule rotation on row 0
 *   - Live clock on row 1
 *   - Idle animations (bottom-right 8 chars of row 1)
 *   - Error/status message display
 *   - Boot splash screen with random animation
 *
 * The display cycles through modes every DISPLAY_INTERVAL ms:
 *   Mode 0: Date (e.g. "Sen,15/06/2026")
 *   Mode 1..N: Feeding schedule entries
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "rtc_manager.h"

// --- Core display functions ---

/**
 * @brief Initialize the LCD. Must be called once in setup().
 *        Initializes I2C communication and turns on the backlight.
 */
void initDisplay();

/**
 * @brief Update the display. Call every loop() iteration.
 *        Row 0: rotates between date and schedule entries.
 *        Row 1: live clock (HH:MM:SS), updated every second.
 * @param time  Current time data from the RTC
 */
void updateDisplay(const TimeData& time);

/**
 * @brief Show an RTC error message on the LCD and Serial.
 */
void showRTCError();

/**
 * @brief Show a custom two-line error message on the LCD.
 * @param line1  First line text (required)
 * @param line2  Second line text (optional, nullptr = single line)
 */
void showError(const char* line1, const char* line2 = nullptr);

// --- Boot splash screen ---

/**
 * @brief Show the static "Made By SetGT" copyright screen.
 */
void showCopyright();

/**
 * @brief Play the boot animation: random LCD animations followed by
 *        the copyright splash. Backlight stays on throughout.
 */
void playCopyrightAnimation();

// --- Idle animation (bottom-right of row 1) ---

/**
 * @brief Start the idle animation. Activates after IDLE_TIMEOUT ms of no input.
 *        Only uses the bottom-right 8 characters of row 1.
 */
void startIdleAnimation();

/**
 * @brief Update the idle animation. Call every loop() when animating.
 */
void updateIdleAnimation();

/**
 * @brief Stop the idle animation and clear its area.
 */
void stopIdleAnimation();

/**
 * @brief Check if the idle animation is currently running.
 * @return true if animating
 */
bool isIdleAnimating();

#endif // DISPLAY_H
