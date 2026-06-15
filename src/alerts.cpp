#include "alerts.h"
#include "config.h"

static unsigned long alertStartTime = 0;
static unsigned long alertDuration = 0;
static bool alertActive = false;
static int alertBuzzer = 1;

void triggerAlert(int buzzer, unsigned long duration) {
    if (!ENABLE_BUZZERS) return;

    alertBuzzer = buzzer;
    alertDuration = duration;
    alertStartTime = millis();
    alertActive = true;
}

void handleBuzzer() {
    if (!ENABLE_BUZZERS || !alertActive) return;

    unsigned long elapsed = millis() - alertStartTime;
    if (elapsed < alertDuration) {
        // Beep pattern: ON for first half, OFF for second half
        if (elapsed < alertDuration / 2) {
            digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, HIGH);
        } else {
            digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, LOW);
        }
    } else {
        digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, LOW);
        alertActive = false;
    }
}
