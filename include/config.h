#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================================================================
// HARDWARE CONFIGURATION
// ==========================================================================================

// --- Board Specific Pin Mapping ---
#ifdef ARDUINO_ARCH_ESP32
    // ESP32 WROOM / C3 Pins
    #define BUTTON_PIN 0        
    #define SERVO_PIN 18       
    #define BUZZER_1_PIN 19     // Status Buzzer
    #define BUZZER_2_PIN 21     // Alert Buzzer
    #define IR_SENSOR_PIN 22    // Food Level Sensor
#else
    // Arduino Uno Pins
    #define BUTTON_PIN 5
    #define SERVO_PIN 4
    #define BUZZER_1_PIN 6
    #define BUZZER_2_PIN 7
    #define IR_SENSOR_PIN 8
#endif

// --- Feature Toggles ---
#define ENABLE_BUZZERS true   // Set to false to completely disable all buzzers

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
    {"Sore", 18, 0},
    {"Malam", 21, 0},
    {"Test", 13, 0}
};

const int NUM_SESSIONS = sizeof(SCHEDULE) / sizeof(SCHEDULE[0]);
const int JUMLAH_PAKAN = 300;

// Labels
const char* const DAYS_OF_THE_WEEK[7] = {"Ahad", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

#endif
