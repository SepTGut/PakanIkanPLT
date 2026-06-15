#include "alerts.h"
#include "config.h"

static unsigned long alertMillis = 0;
static bool alertActive = false;
static int alertBuzzer = 1;

void triggerAlert(int buzzer, unsigned long duration) {
    if (!ENABLE_BUZZERS) return; // Exit immediately if buzzers are disabled
    
    alertBuzzer = buzzer;
    alertMillis = millis();
    alertActive = true;
}

void handleBuzzer() {
    if (!ENABLE_BUZZERS || !alertActive) return; // Check toggle and state
    
    if (millis() - alertMillis < 200) {
        digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, HIGH);
    } else if (millis() - alertMillis < 400) {
        digitalWrite(alertBuzzer == 1 ? BUZZER_1_PIN : BUZZER_2_PIN, LOW);
    } else {
        alertActive = false;
    }
}
