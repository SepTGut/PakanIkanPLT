#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "rtc_manager.h"

void initDisplay();
void updateDisplay(const TimeData& time);

#endif
