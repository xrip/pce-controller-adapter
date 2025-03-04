# PCE Controller Adapter

An RP2040-based adapter that allows you to use your Famicom/NES clone gamepad with DB9 or Sega Genesis controllers as PC Engine/TurboGrafx-16 controller.

## Overview

This project provides a bridge between Famiclone/Genesis controllers and the PC Engine console. It uses an RP2040 microcontroller (like the Raspberry Pi Pico) to read input from standard NES and Sega Genesis controllers and translate those inputs to the PC Engine controller protocol.
The system automatically detects which controller is connected at startup, with Genesis detection taking priority.

## Features

- Auto-detection of controller type (NES or Genesis)
- Support for all standard buttons
- Visual status indication via WS2812B RGB LED
- Low-latency performance with 8ms polling rate

## Hardware Connections

### PC Engine Mini DIN8 Connections (Output) 

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

### LED Connection

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


