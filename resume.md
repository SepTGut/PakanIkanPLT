# Project Resume: PakanIkanPLT (Automatic Fish Feeder)

## Project Overview
An automated fish feeding system built with PlatformIO, targeting Arduino Uno, ESP32 WROOM, and ESP32-C3. Dispenses fish food at scheduled intervals or via manual override. Features a 16x2 I2C LCD, DS1307 RTC, servo-driven dispensing, IR food level sensing, buzzer alerts, and a web-based WiFi configuration portal (ESP32).

## Technical Specifications
- **Microcontroller:** Arduino Uno / ESP32 WROOM / ESP32-C3 (selectable)
- **RTC Module:** DS1307 (I2C 0x68, battery-backed)
- **Display:** 16x2 I2C LCD (address 0x27)
- **Actuator:** Servo Motor (PWM-driven hopper gate)
- **Input:** Push Button (active LOW, internal pull-up)
- **Sensors:** IR sensor (HIGH = food empty)
- **Audio:** 2× buzzers (status + alert)
- **Connectivity:** WiFi captive portal (ESP32 only)

## Key Features
- **Scheduled Feeding:** Up to 5 configurable daily sessions with automatic triggering
- **Missed Feed Recovery:** EEPROM-backed detection of missed feedings after power loss
- **Manual Override:** Physical button or serial command for instant feeding
- **Rotating Status Display:** LCD cycles through date and schedule entries
- **Idle Animations:** 24 random LCD animations during inactivity
- **Non-Blocking Execution:** State machines for feeding, buzzer, and display
- **Web Portal:** Modern dark-themed WiFi configuration (ESP32)
- **Serial Debug:** Full diagnostic interface (115200 baud)

## Hardware Configuration

### Arduino Uno
| Pin | Function |
|-----|----------|
| D2 | Button |
| D3 | IR Sensor |
| D4 | Servo PWM |
| D5 | Buzzer 1 (Status) |
| D6 | Buzzer 2 (Alert) |
| A4 | I2C SDA |
| A5 | I2C SCL |

### ESP32 WROOM / C3
| Pin | Function |
|-----|----------|
| GPIO 0  | Button (BOOT) |
| GPIO 16 | Buzzer 2 (Alert) |
| GPIO 17 | IR Sensor |
| GPIO 18 | Servo PWM |
| GPIO 19 | Buzzer 1 (Status) |
| GPIO 21 | I2C SDA (LCD + RTC) |
| GPIO 22 | I2C SCL (LCD + RTC) |

## Software Architecture

```
include/config.h          → Pin mapping, schedules, toggles
src/main.cpp              → Setup, main loop, serial commands
src/rtc_manager.cpp       → DS1307 RTC (cached, 1s refresh)
src/feeding.cpp           → Servo control, EEPROM, missed feeds
src/display.cpp           → LCD, idle animations, boot splash
src/alerts.cpp            → Non-blocking buzzer FSM
src/state_debug.cpp       → Serial debug output
src/web_portal.cpp        → WiFi captive portal (ESP32)
```

## Build Environments
| Environment | RAM | Flash |
|-------------|-----|-------|
| Arduino Uno | 62.3% (1.3KB) | 76.3% (24.6KB) |
| ESP32 WROOM | 11.8% (38.7KB) | 45.9% (602KB) |
| ESP32-C3 | 9.2% (30.1KB) | 43.4% (568KB) |

## Serial Commands (115200 baud)
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

## Quick Start
1. Open in VS Code with PlatformIO
2. Select environment: `env:uno`, `env:esp32_wroom`, or `env:esp32_c3`
3. Build and upload: `pio run --target upload`
4. Open serial monitor: `pio device monitor -b 115200`
5. Type `h` for available commands
