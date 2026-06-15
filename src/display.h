#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "rtc_manager.h"

void initDisplay();
void updateDisplay(const TimeData& time);
void showRTCError();
void showError(const char* line1, const char* line2 = nullptr);

// Copyright / splash screen (full-screen, blocking)
void showCopyright();
void playCopyrightAnimation();

// Idle animation — non-call, runs on bottom row only
// Clock/schedule always stays on top row undisturbed
void startIdleAnimation();
void updateIdleAnimation();
void stopIdleAnimation();
bool isIdleAnimating();

#endif
