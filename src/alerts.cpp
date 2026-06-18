/**
 * @file alerts.cpp
 * @brief Buzzer alert module implementation
 *
 * Non-blocking buzzer control. When triggerAlert() is called, the buzzer
 * is activated and automatically deactivates after the specified duration.
 * handleBuzzer() must be called every loop() iteration to update the state.
 *
 * Pattern: ON for first half of duration → OFF for second half → deactivate
 */

#include "alerts.h"
#include "config.h"

// Internal state for the buzzer FSM
static unsigned long alertStartTime = 0;   ///< millis() when alert was triggered
static unsigned long alertDuration  = 0;   ///< total requested duration (ms)
static bool          alertActive    = false; ///< true while alert is in progress
static int           alertBuzzer    = 1;     ///< buzzer number (1 or 2)

void triggerAlert(int buzzer, unsigned long duration) {
    if (!ENABLE_BUZZERS) return;
    // Skip if the requested buzzer pin is not connected (-1)
    int pin = (buzzer == 1) ? BUZZER_1_PIN : BUZZER_2_PIN;
    if (pin < 0) return;

    alertBuzzer   = buzzer;
    alertDuration = duration;
    alertStartTime = millis();
    alertActive   = true;
}

void handleBuzzer() {
    if (!ENABLE_BUZZERS || !alertActive) return;

    int pin = (alertBuzzer == 1) ? BUZZER_1_PIN : BUZZER_2_PIN;
    if (pin < 0) { alertActive = false; return; }  // Safety: disable if pin is -1

    unsigned long elapsed = millis() - alertStartTime;

    if (elapsed < alertDuration) {
        // First half: buzzer ON; second half: buzzer OFF
        if (elapsed < alertDuration / 2) {
            digitalWrite(pin, HIGH);
        } else {
            digitalWrite(pin, LOW);
        }
    } else {
        // Duration expired — ensure buzzer is OFF and deactivate
        digitalWrite(pin, LOW);
        alertActive = false;
    }
}
