# Configuration Guide: Pakan Ikan Otomatis

This guide explains how to adjust the feeding schedules, labels, and amounts without needing to understand the core system logic.

## ⚙️ Where to Configure
All settings are located in the file: `include/config.h`

## 📅 Changing Feeding Schedules
The system uses a structured table called `SCHEDULE`. Each entry represents one feeding session.

### Example Configuration:
```cpp
const FeedingSession SCHEDULE[] = {
    {"Pagi", 6, 0},    // Feed at 06:00 AM
    {"Siang", 12, 0},  // Feed at 12:00 PM
    {"Sore", 18, 0}    // Feed at 06:00 PM
};
```

### How to Modify:
1. **Change a Time:** Change the number for `hour` (0-23) and `minute` (0-59).
   - *Example:* To change "Pagi" to 5:30 AM, change `{"Pagi", 6, 0}` to `{"Pagi", 5, 30}`.
2. **Change a Label:** Change the text in quotes. This is what appears on the LCD.
   - *Example:* Change `"Pagi"` to `"Breakfast"`.
3. **Add a New Session:** Simply add another line to the array.
   - *Example:* To add a 9 PM feed:
     ```cpp
     const FeedingSession SCHEDULE[] = {
         {"Pagi", 6, 0},
         {"Siang", 12, 0},
         {"Sore", 18, 0},
         {"Malam", 21, 0} // Added this line
     };
     ```
4. **Remove a Session:** Delete the corresponding line from the array.

The system automatically calculates how many sessions exist, so you don't need to change any other numbers!

## 🍲 Adjusting Feeding Amount
To change how much food is dispensed, find the `JUMLAH_PAKAN` constant:

```cpp
const int JUMLAH_PAKAN = 15;
```
- **Increasing** this number will make the servo pulse more times (more food).
- **Decreasing** this number will dispense less food.

## 🛠️ Troubleshooting Configuration
- **Schedules not triggering?** Ensure the time is set correctly using the RTC calibration process described in `conversation-resume.md`.
- **LCD displaying wrong info?** After modifying `config.h`, you must **Rebuild** and **Upload** the code to the Arduino.
- **Servo not opening enough?** If the amount of food is correct but it's not dispensing, adjust the servo angles in `src/feeding.cpp` (see Calibration section in `README.md`).
