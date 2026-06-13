# Conversation Resume: Pakan Ikan Otomatis PlatformIO Migration

## 📌 Project Overview
This document serves as a comprehensive hand-off for the "Pakan Ikan Otomatis" (Automatic Fish Feeder) project. The project has been successfully migrated from a monolithic Arduino `.ino` file to a professional, modular PlatformIO structure.

**Target Folder:** `PlatformIO_Project`

## 🛠️ Technical State
### Hardware Configuration
- **MCU:** Arduino Uno
- **RTC:** DS1307 (Updated from DS3231). *Note: Temperature sensing was removed as DS1307 does not support it.*
- **Display:** 16x2 LCD with I2C adapter (Address: `0x27`).
- **Actuator:** Servo Motor on **Digital Pin 4**.
- **Input:** Manual trigger button on **Digital Pin 5** (`INPUT_PULLUP`).

### Software Architecture
The code was refactored from `main.cpp` into the following modules:
- `include/config.h`: Centralized constants for pins, feeding schedules, and labels.
- `src/rtc_manager.cpp/h`: Time/Date acquisition using `RTClib`.
- `src/display.cpp/h`: LCD logic including the 3-second rotation system.
- `src/feeding.cpp/h`: Non-blocking servo pulse logic (state machine using `millis()`).
- `src/main.cpp`: System coordinator and main loop.

### Key Improvements Made
1. **Memory Optimization:** Replaced heavy `String` concatenation in the main loop with `snprintf` and `char` buffers to prevent heap fragmentation.
2. **Non-Blocking Logic:** The feeding mechanism now uses a state machine instead of `delay()`, ensuring the LCD continues to rotate and the button remains responsive during feeding.
3. **Standardization:** Adopted consistent naming conventions (`camelCase` for variables/functions, `SCREAMING_SNAKE_CASE` for constants).
4. **Dependency Management:** Fixed `platformio.ini` by using the generic `LiquidCrystal_I2C` library to resolve registry package errors.

## ⚙️ Calibration & Settings
### Servo Calibration
If the feeder does not open or close correctly, adjust these values in `src/feeding.cpp`:
- **Open Position:** `150` degrees.
- **Closed Position:** `0` degrees.

### Feeding Schedules
Currently configured in `include/config.h`:
- **Morning:** 06:00
- **Afternoon:** 12:00
- **Evening:** 18:00
- **Amount:** 15 pulses per feeding.

## ⚠️ Known Issues & Notes
- **IntelliSense:** The user reported "red squiggles" in VS Code. This is a known PlatformIO IDE behavior. The solution is to run `PlatformIO: Rebuild IntelliSense Index` from the Command Palette. The code is verified as correct regardless of the squiggles.
- **RTC Setting:** To calibrate the date/time, uncomment `rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));` in `src/rtc_manager.cpp`, upload, then comment it back out and upload again.

## 🚀 Next Steps for Resuming
1. Open the `PlatformIO_Project` folder in VS Code.
2. Build the project ($\checkmark$ icon) to verify all libraries are installed.
3. Upload to the Arduino Uno.
4. If the servo movement needs adjustment, modify the angles in `src/feeding.cpp`.
