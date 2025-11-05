# Blackenshovel ESP32 – 2432S028R CYD 

## Overview
This repository contains firmware for the BlackenShovel ESP32 platform using PlatformIO (VS Code). Use the guide as a setup reference:
https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/

## Why Platformio?
PlatformIO is more than just an IDE plugin — it’s a complete ecosystem for embedded development. Unlike the Arduino IDE, which is mainly for writing and uploading sketches, PlatformIO handles everything from building and managing projects to library dependency resolution, debugging, and testing. It supports a wide variety of boards and frameworks, allowing developers to reuse code across platforms seamlessly. Its built-in project management, intelligent code completion, and continuous integration support make it ideal for larger, professional-grade projects, where maintaining code quality and scalability is essential.

## Prerequisits
- Python installed
- VS Code with the PlatformIO IDE extension (recommended)
- PlatformIO (installed by the extension)
- ESP32 board and USB cable

## Setup
- Build the project. This will install all labs automatically
- Inside .pio/libdeps replace User_Setup.h file of TFT_eSPI library by the one provided in references folder
- Platformio finds your device port automatically when uploading. In case of any issues, you can check PIO Home -> Devices.
