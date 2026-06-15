#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Definitions
#define BUTTON_PIN 5
#define SERVO_PIN 4

// Servo Settings
#define SERVO_OPEN 150
#define SERVO_CLOSED 0

// Display Settings
const long DISPLAY_INTERVAL = 3000;

// Feeding Schedule Structure
struct FeedingSession {
    const char* label;
    int hour;
    int minute;
};

// Feeding Schedules
const FeedingSession SCHEDULE[] = {
    {"Pagi", 6, 0},
    {"Siang", 12, 0},
    {"Sore", 18, 0}
};

const int NUM_SESSIONS = sizeof(SCHEDULE) / sizeof(SCHEDULE[0]);
const int JUMLAH_PAKAN = 15;

// Labels
const char* const DAYS_OF_THE_WEEK[7] = {"Ahad", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

#endif
