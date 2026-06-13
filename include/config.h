#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Definitions
#define BUTTON_PIN 5
#define SERVO_PIN 4

// Display Settings
const long DISPLAY_INTERVAL = 3000;

// Feeding Schedules
const int JAM_PAGI = 6;
const int MENIT_PAGI = 0;
const int JAM_SIANG = 12;
const int MENIT_SIANG = 0;
const int JAM_SORE = 18;
const int MENIT_SORE = 0;
const int JUMLAH_PAKAN = 15;

// Labels
const char* const LABEL_PAGI = "Pagi";
const char* const LABEL_SIANG = "Siang";
const char* const LABEL_SORE = "Sore";
const char* const DAYS_OF_THE_WEEK[7] = {"Ahad", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

#endif
