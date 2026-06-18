/**
 * @file config.h
 * @brief Central configuration file for PakanIkanPLT (Automatic Fish Feeder)
 *
 * This file contains all hardware pin mappings, feature toggles, timing
 * constants, and feeding schedule definitions. Modify this file to adapt
 * the project to different boards or change feeding behavior.
 *
 * Board selection is automatic via Arduino preprocessor defines:
 *   - ARDUINO_ARCH_ESP32: ESP32 WROOM / C3 pin mapping
 *   - Otherwise:          Arduino Uno pin mapping
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================================================================
// HARDWARE CONFIGURATION — Pin Mapping
// ==========================================================================================
// Pins are selected based on board architecture. ESP32 uses GPIO numbers,
// Arduino Uno uses digital pin numbers.
//
// Use -1 for any pin that is not connected / not used.
// Code checks for (PIN != -1) before using the pin.

#ifdef ARDUINO_ARCH_ESP32
    // --- ESP32 WROOM Pins ---
    // GPIO 0  : Built-in BOOT button (active LOW, internal pull-up)
    // GPIO 16 : Status buzzer
    // GPIO 17 : Alert buzzer
    // GPIO 18 : Servo PWM
    // GPIO 19 : IR sensor
    // GPIO 21 : I2C SDA (LCD + RTC)
    // GPIO 22 : I2C SCL (LCD + RTC)
    //
    // ESP32-C3 is also ARDUINO_ARCH_ESP32 but has fewer GPIOs.
    // Use ARDUINO_ESP32C3_DEV to detect C3 specifically.
    //
    // ESP32-C3 available GPIOs: 0-10, 18-21
    // Safe for all ESP32 variants: 0, 1, 2, 3, 4, 5, 18, 19
    // GPIO 16,17,21,22 exist on WROOM but NOT on C3

    #ifdef ARDUINO_ESP32C3_DEV
        // --- ESP32-C3 Safe Pin Mapping ---
        #define BUTTON_PIN    0
        #define SERVO_PIN     1
        #define BUZZER_1_PIN  -1      // Status Buzzer
        #define BUZZER_2_PIN  3     // Alert Buzzer
        #define IR_SENSOR_PIN 2      // Food Level Sensor
        #define I2C_SDA_PIN   8      // I2C Data (LCD + RTC) — change to 18 for default
        #define I2C_SCL_PIN   9      // I2C Clock (LCD + RTC) — change to 19 for default
    #else
        // --- ESP32 WROOM Pin Mapping ---
        #define BUTTON_PIN    0
        #define SERVO_PIN     18
        #define BUZZER_1_PIN  16     // Status Buzzer
        #define BUZZER_2_PIN  17     // Alert Buzzer
        #define IR_SENSOR_PIN 19     // Food Level Sensor
        #define I2C_SDA_PIN   21     // I2C Data (default — LCD + RTC)
        #define I2C_SCL_PIN   22     // I2C Clock (default — LCD + RTC)
    #endif
#else
    // --- Arduino Uno Pins ---
    // Pin 2  : Push button (external pull-up or INPUT_PULLUP)
    // Pin 4  : Servo PWM output (Timer1, compatible with Arduino Servo lib)
    // Pin 5  : Status buzzer
    // Pin 6  : Alert buzzer
    // Pin 3  : IR sensor input
    // I2C uses fixed hardware pins: A4=SDA, A5=SCL (not configurable on AVR)
    #define BUTTON_PIN    2
    #define SERVO_PIN     4
    #define BUZZER_1_PIN  5
    #define BUZZER_2_PIN  6
    #define IR_SENSOR_PIN 3
#endif

// ==========================================================================================
// FEATURE TOGGLES
// ==========================================================================================
// Set ENABLE_BUZZERS to false to completely disable all buzzer output.
// Useful for silent operation or when buzzers are not connected.
#define ENABLE_BUZZERS true

// Set ENABLE_IR_SENSOR to false to completely disable the IR food level sensor.
// Useful when the IR sensor is not connected or causing false triggers.
#define ENABLE_IR_SENSOR true

// ==========================================================================================
// SERVO SETTINGS
// ==========================================================================================
// Angle values for the servo-driven food dispenser.
// SERVO_OPEN  : Angle that opens the hopper gate (food falls through)
// SERVO_CLOSED: Angle that closes the hopper gate (food retained)
// Adjust these values based on your mechanical linkage.
#define SERVO_OPEN   150
#define SERVO_CLOSED 0

// ==========================================================================================
// DISPLAY SETTINGS
// ==========================================================================================
// Time (in milliseconds) between display mode rotations.
// The LCD cycles through: date → schedule 1 → schedule 2 → ... → schedule N
const long DISPLAY_INTERVAL = 3000;

// ==========================================================================================
// FEEDING SCHEDULE
// ==========================================================================================
// Each session defines a label (shown on LCD) and a trigger time (hour:minute).
// The system checks every minute whether a session has passed and hasn't been
// fed yet today. Up to NUM_SESSIONS sessions are supported.

struct FeedingSession {
    const char* label;  // Human-readable name (e.g. "Pagi", "Siang")
    int hour;           // Trigger hour   (0–23)
    int minute;         // Trigger minute (0–59)
};

// Feeding schedule table — edit times here to match your fish's needs.
// NOTE: Sessions should be in chronological order for correct missed-feed detection.
const FeedingSession SCHEDULE[] = {
    {"Pagi",  6,  0},   // 06:00 — Morning feeding
    {"Siang", 12,  0},   // 12:00 — Afternoon feeding
    {"Sore",  18,  0},   // 18:00 — Evening feeding
    {"Malam", 21,  0},   // 21:00 — Night feeding
};

// Total number of feeding sessions (auto-calculated from the array above).
const int NUM_SESSIONS = sizeof(SCHEDULE) / sizeof(SCHEDULE[0]);

// Number of servo open/close cycles per feeding.
// Each cycle dispenses a small amount of food. Total food ≈ JUMLAH_PAKAN × cycle_volume.
const int JUMLAH_PAKAN = 10;

// ==========================================================================================
// EEPROM Settings Storage — Persistent configuration saved by web portal
// ==========================================================================================
// Address 0–3:   FeedingState (day, month, year, session) — used by feeding.cpp
// Address 10:    Settings magic byte (0xA5 = settings valid)
// Address 11:    Buzzer toggle (0 = disabled, 1 = enabled)
// Address 12–13: Display interval (uint16_t, milliseconds)
// Address 14:    Servo open angle (uint8_t)
// Address 15:    Servo closed angle (uint8_t)
// Address 16:    Feed amount JUMLAH_PAKAN (uint8_t)
// Address 20–69: Feeding schedule (5 sessions × 10 bytes each: 8-byte label + hour + minute)
#define EEPROM_SETTINGS_MAGIC   10
#define EEPROM_BUZZER_TOGGLE    11
#define EEPROM_DISPLAY_INTERVAL 12
#define EEPROM_SERVO_OPEN_ANGLE 14
#define EEPROM_SERVO_CLOSED_ANGLE 15
#define EEPROM_FEED_AMOUNT      16
#define EEPROM_SCHEDULE_START   20
#define EEPROM_SCHEDULE_ENTRY_SIZE 10  // 8 bytes label + 1 byte hour + 1 byte minute
#define EEPROM_SETTINGS_MAGIC_VAL 0xA5
// WiFi credentials storage (after schedule area)
#define EEPROM_WIFI_SSID_START   70
#define EEPROM_WIFI_PASS_START   100
#define EEPROM_WIFI_SSID_LEN     32
#define EEPROM_WIFI_PASS_LEN     32

// ==========================================================================================
// DAY-OF-WEEK LABELS
// ==========================================================================================
// Indonesian day names used by the RTC module (0 = Sunday).
const char* const DAYS_OF_THE_WEEK[7] = {
    "Ahad", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"
};

#endif // CONFIG_H
