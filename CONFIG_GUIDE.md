# Configuration Guide: PakanIkanPLT

This guide explains how to adjust the feeding schedules, labels, amounts, and other settings without needing to understand the core system logic.

## ⚙️ Where to Configure

All user-configurable settings are located in the file: `include/config.h`

## 📅 Changing Feeding Schedules

The system uses a structured table called `SCHEDULE`. Each entry represents one feeding session.

### Example Configuration:

```cpp
const FeedingSession SCHEDULE[] = {
    {"Pagi",  6, 0},    // Feed at 06:00 AM
    {"Siang", 12, 0},   // Feed at 12:00 PM
    {"Sore",  18, 0},   // Feed at 06:00 PM
    {"Malam", 21, 0}    // Feed at 09:00 PM
};
```

### How to Modify:

1. **Change a Time:** Change the number for `hour` (0–23) and `minute` (0–59).
   - *Example:* To change "Pagi" to 5:30 AM, change `{"Pagi", 6, 0}` to `{"Pagi", 5, 30}`.

2. **Change a Label:** Change the text in quotes. This is what appears on the LCD.
   - *Example:* Change `"Pagi"` to `"Breakfast"`.

3. **Add a New Session:** Simply add another line to the array.
   - *Example:* To add a 9 PM feed:
     ```cpp
     const FeedingSession SCHEDULE[] = {
         {"Pagi",  6, 0},
         {"Siang", 12, 0},
         {"Sore",  18, 0},
         {"Malam", 21, 0}  // Added this line
     };
     ```

4. **Remove a Session:** Delete the corresponding line from the array.

> **Important:** Sessions should be in chronological order (earliest to latest) for correct missed-feed detection.

The system automatically calculates how many sessions exist, so you don't need to change any other numbers!

## 🍲 Adjusting Feeding Amount

To change how much food is dispensed, find the `JUMLAH_PAKAN` constant:

```cpp
const int JUMLAH_PAKAN = 100;
```

- **Increasing** this number will make the servo pulse more times (more food).
- **Decreasing** this number will dispense less food.

## 🔧 Servo Calibration

If the servo doesn't open or close the food container correctly, adjust the angles:

```cpp
#define SERVO_OPEN   150   // Angle for open position
#define SERVO_CLOSED  0    // Angle for closed position
```

You can test the servo via serial command `t` without changing any code.

## 🔊 Buzzer Settings

To disable all buzzers (silent operation):

```cpp
#define ENABLE_BUZZERS false
```

## 🖥️ Display Settings

Change how fast the LCD cycles through schedule entries:

```cpp
const long DISPLAY_INTERVAL = 3000;  // milliseconds (3 seconds)
```

## ⏱️ Idle Animation Timeout

Change the delay before idle animations start:

```cpp
static const unsigned long IDLE_TIMEOUT = 10000;  // milliseconds (10 seconds)
```

## 📌 Pin Assignments

Pin mappings are defined in `include/config.h` and are selected automatically based on the target board:

- **Arduino Uno:** Pins D2–D6, A4–A5
- **ESP32 WROOM / C3:** GPIO 0, 18, 19, 21, 22

Edit the `#define` values to change pin assignments for your hardware.

## 🛠️ Troubleshooting

| Problem | Solution |
|---------|----------|
| Schedules not triggering | Check RTC time via serial command `s`. Initialize RTC if needed (see README). |
| LCD displaying wrong info | After modifying `config.h`, rebuild and upload the code. |
| Servo not opening enough | Adjust `SERVO_OPEN` angle in `config.h`. Test with serial command `t`. |
| RTC Error on boot | Check DS1307 wiring (SDA, SCL, VCC, GND). Ensure battery is installed. |
| No buzzer sound | Check `ENABLE_BUZZERS` is `true`. Verify buzzer wiring. |
| Web portal not appearing (ESP32) | Ensure WiFi is not already configured. Hold GPIO 0 (BOOT) during reset to force AP mode. |
