# AmpX Open Energy Gateway ESP32 Hardware Project Brief

## Core Purpose
Design and develop the hardware schematic and PCB layout for an open-source energy gateway device based on ESP32 microcontroller with Ethernet connectivity via W5500-Lite module. The device serves as an IoT gateway for monitoring energy meters, supporting modular expansion.

## Key Requirements
- Integrate ESP32-DevKitC for core processing.
- Add W5500-Lite for Ethernet (SPI interface).
- Include basic indicators like LED.
- Ensure power supply handling (3.3V and GND) with proper net assignments.
- Custom symbol and footprint for W5500-Lite to match module specifications.
- Compliance with standard SPI pinouts and electrical rules.

## Goals
- Create a production-ready PCB design in KiCad.
- Resolve netlist and ratsnest issues for accurate routing.
- Support future software integration for Modbus/RS485 and web interfaces.
- Expandable to support up to 32 energy meters.

This document serves as the source of truth for project scope and guides all other memory bank files.
