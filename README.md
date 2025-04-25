# PCE Controller Adapter

An RP2040-based adapter that allows you to use your Famicom/NES clone gamepad with DB9 or Sega Genesis controllers as PC Engine/TurboGrafx-16 controller.

## Overview

This project provides a bridge between Famiclone/Genesis controllers and the PC Engine console. It uses an RP2040 microcontroller (like the Raspberry Pi Pico) to read input from standard NES and Sega Genesis controllers and translate those inputs to the PC Engine controller protocol.
The system automatically detects which controller is connected at startup, with Genesis detection taking priority.

<img src="https://github.com/user-attachments/assets/9da0fe81-0740-4952-b68c-200be59f9244" width=400>
<img src="https://github.com/user-attachments/assets/2c51bea0-37c1-49ba-bdb0-a4e6d421fa1b" width=400>
<img src="https://github.com/user-attachments/assets/93c4d1c6-8894-41a7-abff-7e9aaeef95c0" width=400>
<img src="https://github.com/user-attachments/assets/df543db1-3c9e-47ef-a518-976dd1d149ac" width=400>


## Features

- Auto-detection of controller type (NES or Genesis)
- Support for all standard buttons
- Visual status indication via WS2812B RGB LED
- Low-latency performance with 8ms polling rate (twice per frame)

## Hardware required
- RP2040 Zero
- DB9 Male
- Mini DIN8 male
- 8-wire cable

## PC Engine Mini DIN8 Connections (Output) 

| PC Engine Pin | RP2040 GPIO |
|---------------|-------------|
| 1 (5V)        | 5V          |
| 2 (Up/A)      | GPIO26      |
| 3 (Right/B)   | GPIO27      |
| 4 (Down/Sel)  | GPIO28      |
| 5 (Left/Start)| GPIO29      |
| 6 (SELECT)    | GPIO14      |
| 7 (ENABLE)    | GPIO15      |
| 8 (GND)       | GND         |

### Genesis or Famiclone DB9 (Input)

| DB9 Pin | RP2040 GPIO |
|---------|-------------|
| 1       | GPIO7       |
| 2       | GPIO2       |
| 3       | GPIO1       |
| 4       | GPIO0       |
| 5       | GPIO8       |
| 6       | GPIO3       |
| 7       | GPIO4       |
| 8       | GND         |
| 9       | GPIO6       |

## LED Connection

The project uses a WS2812B RGB LED for status indication:
- Yellow pulsing: NES controller detected and working
- Cyan pulsing: Genesis controller detected and working

## Building and Flashing

1. Set up the Raspberry Pi Pico SDK
2. Clone this repository
3. Build the project:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```
4. Connect the Pico in bootloader mode and flash the `.uf2` file

## Button Mapping

### NES to PC Engine Mapping

| NES Button | PC Engine Button |
|------------|------------------|
| D-Pad      | D-Pad            |
| A          | I                |
| B          | II               |
| Start      | Run              |
| Select     | Select           |

### Genesis to PC Engine Mapping

| Genesis Button | PC Engine Button |
|----------------|------------------|
| D-Pad          | D-Pad            |
| A              | I                |
| B              | II               |
| Start          | Run              |
| C              | Select           |

## Implementation Details

The adapter uses a dual-core approach:
- Core 0: Handles the PC Engine interface and outputs controller signals
- Core 1: Reads and processes input from NES/Genesis controllers and updates LED status


