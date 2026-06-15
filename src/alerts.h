/**
 * @file alerts.h
 * @brief Buzzer alert module interface
 *
 * Provides non-blocking buzzer alerts with configurable duration.
 * Two buzzers are supported: status (short beeps) and alert (longer beeps).
 * The buzzer pattern is ON for the first half of the duration, OFF for
 * the second half, then deactivates automatically.
 */

#ifndef ALERTS_H
#define ALERTS_H

#include <Arduino.h>

/**
 * @brief Trigger a buzzer alert.
 * @param buzzer  Buzzer number: 1 = status buzzer, 2 = alert buzzer
 * @param duration  Alert duration in milliseconds
 *
 * If ENABLE_BUZZERS is false in config.h, this function does nothing.
 * The buzzer pattern is: ON for duration/2, then OFF for duration/2.
 */
void triggerAlert(int buzzer, unsigned long duration);

/**
 * @brief Update the buzzer state — call this every loop iteration.
 *        Non-blocking: uses millis() to track elapsed time.
 */
void handleBuzzer();

#endif // ALERTS_H
