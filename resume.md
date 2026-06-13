# Project Resume: Pakan Ikan Otomatis (Automatic Fish Feeder)

## Project Overview
An automated fish feeding system built using an Arduino Uno, designed to dispense fish food at scheduled intervals and via a manual override button. The system provides a real-time status display via an I2C LCD.

## Technical Specifications
- **Microcontroller:** Arduino Uno
- **RTC Module:** DS1307 (Precision timekeeping and scheduling)
- **Display:** 16x2 LCD with I2C Adapter (0x27 address)
- **Actuator:** Servo Motor (Dispensing mechanism)
- **Input:** Push Button (Manual feeding trigger)

## Key Features
- **Scheduled Feeding:** Three configurable daily feeding windows (Morning, Afternoon, Evening).
- **Manual Override:** A physical button allows immediate feeding.
- **Rotating Status Display:** The LCD cycles through:
  - Current Date
  - Morning Schedule
  - Afternoon Schedule
  - Evening Schedule
- **Non-Blocking Execution:** Uses a state-machine approach for servo movement and display rotation, ensuring the system remains responsive to button presses.

## Hardware Configuration
- **Servo Pin:** Digital Pin 4
- **Button Pin:** Digital Pin 5 (Input Pull-up)
- **I2C Bus:** SDA/SCL for RTC and LCD

## Software Architecture (Modular)
- `config.h`: Central configuration for pins and schedules.
- `rtc_manager`: Handles time acquisition and date formatting.
- `display`: Manages the I2C LCD rotations and updates.
- `feeding`: Controls the servo motor pulse sequences.
- `main.cpp`: System coordinator.
