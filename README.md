# 🐟 PakanIkanPLT — Automatic Fish Feeder

An Arduino/ESP32-based automatic fish feeder that ensures your fish are fed on time, every time. Features a 16x2 I2C LCD, DS1307 RTC for reliable scheduling, servo-driven food dispensing, and a web-based WiFi configuration portal (ESP32 only).

## ✨ Features

- **Automatic Scheduling** — Up to 5 programmable feeding sessions per day
- **Missed Feed Recovery** — Detects and compensates for missed feedings after power loss (EEPROM-backed)
- **Manual Feed** — One-touch button or serial command for instant feeding
- **Low Food Detection** — IR sensor alerts when the hopper is empty
- **Smart Display** — 16x2 I2C LCD showing time, date, and upcoming schedules
- **Idle Animations** — 24 random LCD animations during periods of inactivity
- **Buzzer Alerts** — Two buzzers: status (feeding confirmation) and alert (errors)
- **Web Portal** — Modern dark-themed WiFi configuration via captive portal (ESP32 only)
- **Serial Debug** — Full diagnostic interface via USB serial (115200 baud)
- **Cross-Platform** — Runs on Arduino Uno, ESP32 WROOM, and ESP32-C3

## 🛠️ Hardware Requirements

| Component | Purpose | Notes |
|-----------|---------|-------|
| Arduino Uno / ESP32 | Main controller | Select environment in `platformio.ini` |
| DS1307 RTC Module | Time-keeping | I2C (address 0x68), battery-backed |
| 16x2 I2C LCD | Display | Address 0x27 (PCF8574 backpack) |
| Servo Motor (SG90) | Food dispenser | PWM-driven hopper gate |
| IR Sensor Module | Food level detection | HIGH = empty, LOW = food present |
| Push Button | Manual feed trigger | Active LOW (internal pull-up) |
| 2× Buzzers | Audio feedback | Status (short beep) + Alert (long beep) |

## 📌 Pin Mapping

### Arduino Uno
| Pin | Function |
|-----|----------|
| D2  | Button |
| D3  | IR Sensor |
| D4  | Servo PWM |
| D5  | Buzzer 1 (Status) |
| D6  | Buzzer 2 (Alert) |
| A4  | I2C SDA (LCD, RTC) |
| A5  | I2C SCL (LCD, RTC) |

### ESP32 WROOM
| Pin | Function |
|-----|----------|
| GPIO 0  | Button (BOOT) |
| GPIO 16 | Buzzer 1 (Status) |
| GPIO 17 | Buzzer 2 (Alert) |
| GPIO 18 | Servo PWM |
| GPIO 19 | IR Sensor |
| GPIO 21 | I2C SDA (LCD + RTC) |
| GPIO 22 | I2C SCL (LCD + RTC) |

### ESP32-C3
| Pin | Function |
|-----|----------|
| GPIO 0  | Button (BOOT) |
| GPIO 1  | Buzzer 1 (Status) |
| GPIO 2  | Buzzer 2 (Alert) |
| GPIO 3  | IR Sensor |
| GPIO 4  | I2C SDA (LCD + RTC) |
| GPIO 5  | I2C SCL (LCD + RTC) |
| GPIO 18 | Servo PWM |

> **Note:** The ESP32-C3 has fewer GPIOs than the WROOM (only GPIO 0–10 and 18–21). Pins are auto-selected at compile time. You can reassign I2C pins by changing `I2C_SDA_PIN` and `I2C_SCL_PIN` in `include/config.h`. On Arduino Uno, I2C uses fixed hardware pins (A4=SDA, A5=SCL).

## 📂 Project Structure

```
PakanIkanPLT/
├── include/
│   └── config.h          # Pin mapping, schedules, feature toggles
├── src/
│   ├── main.cpp          # Setup, main loop, serial command handler
│   ├── rtc_manager.cpp   # DS1307 RTC driver (cached, 1s refresh)
│   ├── feeding.cpp       # Servo control, EEPROM state, missed-feed detection
│   ├── display.cpp       # LCD driver, idle animations, boot splash
│   ├── alerts.cpp        # Non-blocking buzzer state machine
│   ├── state_debug.cpp   # Serial debug output
│   └── web_portal.cpp    # WiFi captive portal (ESP32 only)
├── docs/
│   └── superpowers/plans # Implementation plans
├── platformio.ini        # PlatformIO project configuration
├── CONFIG_GUIDE.md       # Quick configuration reference
└── README.md             # This file
```

## ⚙️ Installation & Setup

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
2. Clone this repository
3. Open the project folder in PlatformIO
4. Select your target environment in `platformio.ini`:
   - `env:uno` — Arduino Uno
   - `env:esp32_wroom` — ESP32 WROOM
   - `env:esp32_c3` — ESP32-C3
5. Build and upload: `pio run --target upload`
6. Open serial monitor: `pio device monitor -b 115200`

### First Boot (RTC Initialization)

On first boot, the RTC may show an incorrect date/time. To set it to the current compile time:

1. Uncomment this line in `src/rtc_manager.cpp`:
   ```cpp
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   ```
2. Build and upload
3. Re-comment the line and upload again (otherwise it resets on every boot)

## 🔧 Configuration

### Feeding Schedule

Edit the `SCHEDULE` array in `include/config.h`:

```cpp
const FeedingSession SCHEDULE[] = {
    {"Pagi",  6,  0},   // 06:00 — Morning
    {"Siang", 12,  0},   // 12:00 — Afternoon
    {"Sore",  18,  0},   // 18:00 — Evening
    {"Malam", 21,  0},   // 21:00 — Night
    {"Test",  13,  0}    // 13:00 — Test feeding (remove in production)
};
```

> **Note:** Sessions should be in chronological order for correct missed-feed detection.

### Servo Calibration

Adjust the open/close angles in `include/config.h`:

```cpp
#define SERVO_OPEN   150   // Angle for open position
#define SERVO_CLOSED  0    // Angle for closed position
```

### Food Amount

Change the number of servo cycles per feeding:

```cpp
const int JUMLAH_PAKAN = 100;  // More cycles = more food
```

### Buzzer Toggle

Disable buzzers entirely:

```cpp
#define ENABLE_BUZZERS false
```

### Display Rotation Speed

Change how fast the LCD cycles through schedule entries:

```cpp
const long DISPLAY_INTERVAL = 3000;  // milliseconds
```

### Idle Animation Timeout

Change the delay before idle animations start:

```cpp
static const unsigned long IDLE_TIMEOUT = 10000;  // milliseconds
```

## 💻 Serial Commands (115200 baud)

Connect via USB serial to monitor and control the device. The device only outputs data when explicitly requested — no unsolicited messages.

| Command | Description |
|---------|-------------|
| `s` | **Detailed system state** — RTC time, full schedule, EEPROM state |
| `p` | **Brief device info** — Board type, RTC status, feed count, food level |
| `t` | **Test servo** — Open then close once |
| `b` | **Test buzzer** — 100ms beep on status buzzer |
| `f` | **Manual feed** — Trigger a full feeding cycle |
| `r` | **Read IR sensor** — Show current food level state |
| `e` | **Show EEPROM state** — Last recorded feeding |
| `h` | **Help menu** — List all commands |

### Example Serial Output

```
> s
=== System State ===
RTC Time: 14:30:00, Date: 15/6/2026
Feeding Schedule:
  Pagi: 6:00
  Siang: 12:00
  Sore: 18:00
  Malam: 21:00
Last Feeding Record (EEPROM):
  Day: 15, Month: 6, Year: 2026, Session: 1 (Siang)
Watchdog: enabled (2 s timeout)
====================
```

```
> p
=== Device Info ===
Device: PakanIkanPLT (Automatic Fish Feeder)
Board:  ESP32
RTC:    OK
Feeds:  5 sessions/day
Food:   OK
===================
```

```
> t
Testing servo...
Servo test complete
```

## 🌐 Web Portal (ESP32 only)

On first boot (or when WiFi is not configured), the ESP32 creates a WiFi access point:

- **SSID:** `PakanIkan-Config`
- **Password:** `12345678`

Connect to this network and open any web browser. The captive portal will redirect you to the configuration page. Enter your WiFi credentials and the device will restart and connect to your network.

**Design:** Dark aquatic theme with ocean blue / teal accent colors. Responsive layout, centered card, no external dependencies.

## 🏗️ Architecture

### Feeding State Machine

Feeding is non-blocking. `startFeeding(N)` sets the cycle count, and `updateFeeding()` (called every loop) advances one step every 100ms:

```
IDLE → ATTACH → OPEN → (100ms) → CLOSE → (100ms) → OPEN → ... → DETACH → IDLE
```

### Missed Feed Detection

On boot, the system compares the current time against the schedule and the last recorded feeding in EEPROM. If a session was missed (e.g., after power loss), it triggers immediately. The algorithm handles three cases:

1. **No feeding ever recorded** (first boot) → feeds the first passed session
2. **Last feeding on a different day** → feeds the first passed session today
3. **Last feeding today** → feeds the first passed session after the last fed one

### EEPROM Wear Leveling

The feeding state is only written to EEPROM when the data actually changes, reducing wear. On ESP32, `EEPROM.commit()` is called explicitly.

### Idle Animations

After 10 seconds of no user activity, random animations play on the bottom-right 8 characters of the LCD's bottom row. The clock/schedule on the top row is never disturbed. Twenty-four animation effects are available, selected randomly with varied speeds.

### Buzzer Alert System

Non-blocking buzzer control with configurable duration. The buzzer pattern is ON for the first half of the duration, OFF for the second half, then deactivates automatically. Two buzzers are supported:
- **Buzzer 1 (Status):** Short beeps for feeding confirmation
- **Buzzer 2 (Alert):** Longer beeps for errors and low food

### RTC Time Caching

Time is cached in RAM and refreshed from the DS1307 once per second to minimize I2C traffic. Validity is checked on every refresh (year must be 2000–2100).

## 📊 Build Status

| Environment | RAM Usage | Flash Usage |
|-------------|-----------|-------------|
| Arduino Uno | 62.3% (1.3KB) | 76.3% (24.6KB) |
| ESP32 WROOM | 11.8% (38.7KB) | 45.9% (602KB) |
| ESP32-C3 | 9.2% (30.1KB) | 43.4% (568KB) |

## 📝 License

Created by **SetGT**.
