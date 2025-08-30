# AmpX Open Energy Gateway - Project Brief

## Project Overview
**Name**: AmpX Open Energy Gateway  
**Platform**: ESP32 (Node32S)  
**License**: MIT  
**Purpose**: Industrial energy monitoring gateway for Meatrol brand energy meters

## Core Mission
Create a comprehensive ESP32-based energy monitoring gateway that communicates with up to 32 Meatrol energy meters via Modbus protocols (RS485 and TCP/IP), provides real-time web interface monitoring, and uploads data to remote API for centralized energy management.

## Key Requirements

### Hardware Support
- ESP32 Wroom 32U microcontroller
- Dual communication variants:
  - **RS485 Version**: MAX485 interface for serial communication
  - **TCP/IP Version**: W5500 Lite Ethernet module for network communication
- 4 status LEDs for system monitoring
- WiFi connectivity with WiFiManager

### Core Functionality
- **Multi-meter support**: Monitor up to 32 energy meters simultaneously
- **Real-time monitoring**: 3-second update intervals for web interface
- **Data collection**: Voltage, current, power, and energy readings per phase
- **Web interface**: Live dashboard with WebSocket updates
- **API integration**: Automatic 5-minute uploads to AmpX Portal
- **Auto-discovery**: Automatic detection of connected meters

### Data Management
- Modbus register reading and processing
- JSON data serialization
- Non-volatile storage (NVS) for settings
- Real-time timestamp synchronization via NTP

## Technical Constraints
- Arduino IDE development environment
- Memory optimization for ESP32 platform
- Modular code structure with separate .ino files
- Web interface via header files (SPIFFS option available for future)
- Debug output control for production builds

## Success Metrics
- Reliable communication with multiple meters
- Real-time web interface performance
- Successful API data transmission
- System stability over 24-hour operation cycles
- Industrial-grade reliability for commercial deployments
