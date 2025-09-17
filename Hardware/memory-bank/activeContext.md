# Active Context: AmpX Open Energy Gateway ESP32 Hardware

## Current Work Focus
Refining the KiCad schematic and PCB for ESP32 with W5500-Lite integration. Recent efforts on custom symbol/footprint edits, resolving No Connect flags, and fixing ratsnest/net mismatches (e.g., MOSI to 3V3 errors).

## Recent Changes
- Created/edited custom W5500-Lite symbol: Updated pin types (e.g., MOSI as Input, GND as Power Input) and removed embedded No Connect markers.
- Verified and linked custom footprint (W5500-Lite-Module.kicad_mod) with correct pad numbering.
- Corrected schematic wiring: Ensured SPI pins (MOSI, MISO, SCLK, CS) connect to appropriate ESP32 GPIOs; fixed accidental net links.
- Updated PCB from schematic multiple times to sync nets.

## Next Steps
- Complete routing in PCBNew, add GND/3V3 zones.
- Add pull-up resistors or capacitors as per W5500 datasheet (e.g., for RST/INT).
- Run final ERC/DRC and generate Gerbers.
- Test physical prototype if fabricated.

## Active Decisions/Considerations
- Use GPIO23 for MOSI, GPIO19 for MISO, etc., per standard ESP32 SPI.
- Leave NC pin unspecified unless grounding is needed.
- Monitor for any recurring net issues during routing; re-update from schematic if needed.
