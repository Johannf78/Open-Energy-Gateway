# Technical Context: AmpX Open Energy Gateway ESP32 Hardware

## Technologies Used
- KiCad for schematic/PCB design (version 9.0+ based on file headers).
- Custom libraries: New_Library.kicad_sym for symbols, AmpX-Footprint-Library.pretty for footprints.
- Components: ESP32-DevKitC, W5500-Lite Ethernet module, basic passives (LED, resistor).

## Development Setup
- Workspace: D:\OneDrive\Dev\Ardruino\AmpX\ESP32\Open-Energy-Gateway\Hardware
- Tools: Eeschema for schematics, Pcbnew for layout, Symbol/Footprint Editors for customs.
- Datasheets: W550Lite USR-ES1-EN-V1.0_Datasheet.pdf (open), ESP32 datasheets.

## Technical Constraints
- Power: 3.3V logic level; ensure no 5V mix-ups.
- SPI bus: Standard pins to avoid conflicts with ESP32 internals.
- PCB: 2-4 layers, with potential GND plane for noise reduction.
- Expandability: Pins for up to 32 meters via Modbus.

## Dependencies
- KiCad project files (.kicad_pro, .kicad_sch, .kicad_pcb).
- Custom libs in KiCad\Symbols and KiCad\Footprints.
- External datasheets for pinouts and electrical specs.
