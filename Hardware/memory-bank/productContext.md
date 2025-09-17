# Product Context: AmpX Open Energy Gateway ESP32 Hardware

## Why This Project Exists
The AmpX Open Energy Gateway addresses the need for an affordable, open-source hardware platform to monitor and aggregate data from energy meters in industrial or residential settings. It bridges legacy systems (e.g., RS485 Modbus) with modern networks (Ethernet/TCP-IP) for real-time data access and IoT integration.

## Problems It Solves
- Lack of modular, expandable hardware for multi-meter monitoring (up to 32 meters).
- Proprietary gateways are expensive and closed-source; this provides a customizable alternative.
- Enables real-time web dashboards (WebSocket updates) and API uploads without relying on cloud services.
- Handles power management and connectivity issues in ESP32-based designs.

## How It Should Work
- ESP32 processes data from connected meters.
- W5500-Lite provides stable Ethernet connectivity via SPI.
- Users access data via web interface or API.
- Hardware is robust, with proper power/ground routing to avoid shorts.

## User Experience Goals
- Easy to assemble and expand (modular pins).
- Reliable operation with minimal setup (e.g., auto-config for 4 meters, expandable).
- Visual feedback via LED for status.
- Seamless integration with software for data visualization.
