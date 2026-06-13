#include "feeding.h"
#include "config.h"

Servo servoMekanik;
int feedCyclesRemaining = 0;
unsigned long lastServoMillis = 0;

void initFeeding() {
    servoMekanik.attach(SERVO_PIN);
    servoMekanik.write(0);
}

void startFeeding(int jumlah) {
    feedCyclesRemaining = jumlah * 2;
}

void updateFeeding() {
    if (feedCyclesRemaining <= 0) return;
    if (millis() - lastServoMillis >= 100) {
        lastServoMillis = millis();
        if (feedCyclesRemaining % 2 == 0) {
            servoMekanik.write(150); // OPEN
        } else {
            servoMekanik.write(0);   // CLOSED
        }
        feedCyclesRemaining--;
    }
}
