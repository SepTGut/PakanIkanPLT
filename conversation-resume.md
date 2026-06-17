# Conversation Resume: PakanIkanPLT

## 📌 Project Overview
This document serves as a comprehensive hand-off for the "PakanIkanPLT" (Automatic Fish Feeder) project. The project has been migrated from a monolithic Arduino `.ino` file to a professional, modular PlatformIO structure targeting Arduino Uno, ESP32 WROOM, and ESP32-C3.

**Branch:** `Beta`

## 🛠️ Technical State

### Hardware Configuration
- **MCU:** Arduino Uno / ESP32 WROOM / ESP32-C3 (selectable via PlatformIO environment)
- **RTC:** DS1307 (I2C address 0x68), battery-backed
- **Display:** 16x2 LCD with I2C adapter (Address: `0x27`)
- **Actuator:** Servo Motor (PWM-driven hopper gate)
- **Input:** Manual trigger button (active LOW, internal pull-up)
- **Sensors:** IR sensor for food level detection (HIGH = empty)
- **Audio:** 2× buzzers (status + alert)

### Software Architecture
The code is fully modular with Doxygen-style comments:

| Module | File(s) | Purpose |
|--------|---------|---------|
| Config | `include/config.h` | Pin mapping, schedules, feature toggles, constants |
| RTC | `src/rtc_manager.cpp/h` | DS1307 driver, 1s cached refresh, validity checking |
| Feeding | `src/feeding.cpp/h` | Servo control, EEPROM state, missed-feed detection |
| Display | `src/display.cpp/h` | LCD driver, 24 idle animations, boot splash |
| Alerts | `src/alerts.cpp/h` | Non-blocking buzzer state machine |
| Debug | `src/state_debug.cpp/h` | Serial debug output |
| Web | `src/web_portal.cpp/h` | WiFi captive portal (ESP32 only) |
| Main | `src/main.cpp` | Setup, main loop, serial command handler |

### Serial Commands (115200 baud)
| Command | Description |
|---------|-------------|
| `s` | Detailed system state |
| `p` | Brief device info |
| `t` | Test servo |
| `b` | Test buzzer |
| `f` | Manual feed |
| `r` | Read IR sensor |
| `e` | Show EEPROM state |
| `h` | Help menu |

### Key Improvements Made
1. **Memory Optimization:** Replaced `String` concatenation with `snprintf` and `char` buffers
2. **Non-Blocking Logic:** Feeding uses state machine instead of `delay()`
3. **EEPROM Wear Leveling:** Only writes when data changes
4. **Missed Feed Recovery:** Detects and compensates for missed feedings after power loss
5. **Cross-Platform:** Single codebase targets Arduino Uno, ESP32 WROOM, ESP32-C3
6. **Buzzer Duration:** Configurable duration parameter (was hardcoded 400ms)
7. **Error Display:** Generic `showError()` function instead of misusing `showRTCError()`
8. **LCD Backlight:** Stays on during boot animation
9. **Time Tracking:** Minute+day granularity for feeding triggers (was second-based)
10. **Code Comments:** Comprehensive Doxygen-style comments on all files
11. **Web Portal:** Modern dark aquatic theme with AJAX form submission
12. **Documentation:** Full README.md rewrite, updated CONFIG_GUIDE.md, updated .gitignore

## ⚙️ Calibration & Settings

### Servo Calibration
Adjust angles in `include/config.h`:
- **Open Position:** `150` degrees
- **Closed Position:** `0` degrees

Test via serial command `t` without modifying code.

### Feeding Schedules
Configured in `include/config.h`:
- **Pagi (Morning):** 06:00
- **Siang (Afternoon):** 12:00
- **Sore (Evening):** 18:00
- **Malam (Night):** 21:00
- **Test:** 13:00 (remove in production)
- **Amount:** 100 servo cycles per feeding

### RTC Initialization
To set the RTC to compile-time date/time:
1. Uncomment `rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));` in `src/rtc_manager.cpp`
2. Build and upload
3. Re-comment the line and upload again

## ⚠️ Known Issues & Notes
- **IntelliSense:** VS Code may show "red squiggles." Run `PlatformIO: Rebuild IntelliSense Index` from the Command Palette. The code compiles correctly regardless.
- **ESP32 Web Portal:** On first boot, connect to WiFi AP "PakanIkan-Config" (password: 12345678) and open any browser for the captive portal.

## 🚀 Next Steps for Resuming
1. Open the project folder in VS Code with PlatformIO
2. Select target environment: `env:uno`, `env:esp32_wroom`, or `env:esp32_c3`
3. Build: `pio run --target upload`
4. Open serial monitor: `pio device monitor -b 115200`
5. Type `h` for help, `s` for system state, `t` to test servo
