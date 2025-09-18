# Hardware Notes and Connections

## ESP32 Pin Assignments

### SPI Interface (W5500 Ethernet)
- **MOSI**: GPIO 23
- **MISO**: GPIO 19  
- **SCLK**: GPIO 18
- **CS**: GPIO 5
- **RST**: GPIO (TBD)

### RS485 Interface (Modbus)
- **DE/RE**: GPIO 4
- **RX**: GPIO 16
- **TX**: GPIO 17

### Status LEDs
- **POWER**: GPIO 12
- **METER**: GPIO 14
- **WIFI**: GPIO 27
- **INTERNET**: GPIO 26
- **SERVER**: GPIO 25

## Module Specifications

### W5500 Ethernet Module
- **Part Number**: MO-W5500
- **Interface**: SPI
- **Voltage**: 3.3V
- **Features**: Hardware TCP/IP stack, RJ45 connector

### MAX485 RS485 Transceiver
- **Interface**: UART with direction control
- **Voltage**: 3.3V/5V compatible
- **Termination**: 120Ω resistor required

## Connection Notes

- All GPIO assignments match the Open Energy Gateway firmware
- W5500 module requires 3.3V power supply
- RS485 network needs proper termination at both ends
- Status LEDs are active HIGH

## Schematic References

- Main schematic: `AmpX Open Energy Gateway.kicad_sch`
- Custom symbols: `New_Library.kicad_sym`
- Custom footprints: `AmpX-Footprint-Library.pretty`