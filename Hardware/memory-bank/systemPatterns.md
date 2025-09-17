# System Patterns: AmpX Open Energy Gateway ESP32 Hardware

## System Architecture
- Microcontroller-centric design with ESP32 as core, handling processing and wireless (if enabled).
- Ethernet via W5500-Lite module over SPI bus.
- Power distribution: 3.3V rail for logic, GND plane for stability.
- Modular interfaces for RS485/Modbus expansion (not yet in schematic).

## Key Technical Decisions
- Custom symbol/footprint for W5500-Lite to match module pinout and avoid library mismatches.
- SPI pin assignments: MOSI (GPIO23), MISO (GPIO19), SCLK (GPIO18), CS (GPIO5), INT (GPIO21), RST (GPIO17).
- Use PWR_FLAG for power nets to satisfy ERC.
- Multi-layer PCB potential for GND plane and signal routing.

## Design Patterns in Use
- Star topology for power distribution to minimize noise.
- Decoupled power: Separate 3.3V for ESP32 and W5500.
- Error suppression via proper pin types (e.g., no unnecessary No Connect flags).

## Component Relationships
- ESP32 (U1) controls W5500 (U2) via SPI.
- LED (D1) + Resistor (R1) for status indication, connected to 3V3/GND.
- All components share common GND; 3V3 isolated to power pins.
