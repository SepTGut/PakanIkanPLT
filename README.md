# Pakan Ikan Otomatis (Automatic Fish Feeder)

An Arduino-based automatic fish feeder that ensures your fish are fed on time, every time.

## 🚀 Features
- **Automatic Scheduling:** Three programmable feeding times per day.
- **Manual Feed:** One-touch manual feeding button.
- **Smart Display:** 16x2 I2C LCD showing current time, date, and upcoming schedules.
- **Reliable Timing:** DS1307 RTC ensures the schedule is maintained even after power loss.
- **Precise Dispensing:** Servo-driven mechanism with adjustable pulse counts.

## 🛠️ Hardware Requirements
- Arduino Uno
- DS1307 RTC Module
- 16x2 I2C LCD Display
- Servo Motor (e.g., SG90)
- Push Button
- Jumper wires and Breadboard/PCB

## 📂 Project Structure (PlatformIO)
The project is modularized for easy maintenance:
- `include/config.h`: Change feeding times and pin assignments here.
- `src/main.cpp`: The main system loop.
- `src/rtc_manager.cpp`: Time and date logic.
- `src/display.cpp`: LCD rotation and formatting.
- `src/feeding.cpp`: Servo control logic.

## ⚙️ Installation & Setup
1. Install [PlatformIO](https://platformio.org/) (VS Code extension).
2. Clone this repository.
3. Open the `PlatformIO_Project` folder.
4. Click **Build** (Checkmark icon) and **Upload** (Arrow icon).

## 🔧 Calibration
If the servo doesn't open or close the food container correctly, adjust the angles in `src/feeding.cpp`:
- `servoMekanik.write(150);` $\rightarrow$ Adjust for "Open" position.
- `servoMekanik.write(0);` $\rightarrow$ Adjust for "Closed" position.
