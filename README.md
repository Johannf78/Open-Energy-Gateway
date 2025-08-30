# AmpX Open Energy Gateway

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://espressif.com/en/products/socs/esp32)
[![Protocol: Modbus](https://img.shields.io/badge/Protocol-Modbus-green.svg)](https://modbus.org/)

A comprehensive ESP32-based energy monitoring gateway that communicates with Meatrol Brand energy meters via Modbus protocols (RS485 and TCP/IP). The system collects data from up to 32 meters simultaneously, displays it through a web interface, and uploads measurements to a remote API for centralized monitoring.

## 🌟 Features

### Core Capabilities
- **Multi-Protocol Support**: Compatible with both Modbus RS485 and Modbus TCP/IP communication
- **Multi-Meter Support**: Monitor up to 32 energy meters simultaneously
- **Real-Time Web Interface**: Live data display with WebSocket updates
- **API Integration**: Automatic data upload to AmpX Portal every 30 seconds
- **Auto-Discovery**: Automatic detection of connected meters
- **Dual Hardware Variants**: Separate builds for RS485 and TCP/IP configurations

### Hardware Integration
- **ESP32 Wroom 32U**: Main microcontroller platform
- **RS485 Interface**: MAX485 module for serial communication
- **Ethernet Interface**: W5500 Lite for TCP/IP communication
- **Status Indicators**: Four LEDs for system status monitoring
- **WiFi Support**: Wireless connectivity with WiFiManager

### Data Management
- **Real-Time Monitoring**: 3-second update intervals for web interface
- **Scheduled Uploads**: 30-second intervals for API data transmission
- **JSON Data Format**: Structured data exchange and storage
- **Meter Identification**: Unique serial number tracking for each meter

## 🏗️ Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Energy Meters │    │  AmpX Gateway    │    │   AmpX Portal   │
│                 │    │                  │    │                 │
│ Meatrol Meters  │◄──►│ ESP32 + Modbus   │◄──►│ Remote API      │
│ (RS485/TCP-IP)  │    │ Web Interface    │    │ Data Storage    │
│ Up to 32 units  │    │ Data Processing  │    │ Analytics       │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## 📋 Hardware Requirements

### Base Components
- **ESP32 Wroom 32U** microcontroller
- **Power Supply**: 3.3V regulated power
- **Status LEDs**: 5x LEDs for system monitoring (Power, Meter, WiFi, Internet, Server)

### RS485 Configuration
- **MAX485 Interface Module**
- **RS485 Cable**: For meter communication
- **Pull-up/Pull-down Resistors**: 120Ω termination resistors

### TCP/IP Configuration
- **W5500 Lite Ethernet Module**
- **Ethernet Cable**: CAT5/CAT6 network connection
- **Network Switch/Router**: For meter and internet connectivity

## 🚀 Installation

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) (recommended)
- ESP32 Board Package for Arduino IDE
- Required Arduino libraries (see Dependencies section)
- Optional: [PlatformIO](https://platformio.org/) (alternative IDE)

### Quick Start

1. **Clone the Repository**
   ```bash
   git clone https://github.com/Johannf78/Open-Energy-Gateway.git
   cd Open-Energy-Gateway
   ```

2. **Open the Project**
   - **Arduino IDE** (recommended): Open `src/open_energy_gateway/open_energy_gateway.ino`
   - Alternative - PlatformIO: Open the project folder

3. **Configure Hardware Type**
   
   In `open_energy_gateway.ino`, set the communication type:
   ```cpp
   #define MODBUS_TYPE MODBUS_TYPE_RS485  // For RS485 version
   // OR
   #define MODBUS_TYPE MODBUS_TYPE_TCPIP  // For TCP/IP version
   ```

4. **Upload to ESP32**
   - Connect your ESP32 via USB
   - Select the correct board and port
   - Compile and upload the code

### Dependencies

The following libraries are required:
```cpp
#include <WiFi.h>              // ESP32 built-in WiFi
#include <WebServer.h>         // ESP32 built-in web server
#include <WebSocketsServer.h>  // Real-time updates (by Markus Sattler)
#include <ArduinoJson.h>       // JSON processing (by Benoit Blanchon)
#include <HTTPClient.h>        // ESP32 built-in HTTP client
#include <WiFiManager.h>       // WiFi configuration (by tzapu)
#include <Preferences.h>       // ESP32 built-in NVS storage
#include <HardwareSerial.h>    // ESP32 built-in serial communication
#include <time.h>              // ESP32 built-in time functions
```

### Custom Libraries (Required)
The project requires custom AmpX Modbus libraries:
- **For RS485**: `ampx_modbus_rs485.h` 
- **For TCP/IP**: `ampx_modbus_tcpip.h`

These should be installed in your Arduino libraries folder:
`Documents/Arduino/libraries/`

## 🔧 Configuration

### WiFi Setup
On first boot, the device creates a WiFi access point:
- **SSID**: `AmpX-Energy-Gateway-AP`
- **Password**: None (open network)
- Connect and configure your WiFi credentials through the captive portal

### Meter Configuration
Meters are automatically discovered during startup. The system supports:
- **Meter Addresses**: 1-32 (configurable)
- **Baud Rate**: 9600 (RS485)
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

### API Configuration
The system supports dual API endpoints (configurable in code):
```cpp
const char* ampxportal_server_local = "http://192.168.2.32:8080/api/v2/";
const char* ampxportal_server_live = "https://portal.ampx.app/api/v2/";
#define USE_LOCAL_SERVER true  // Set to false for production
```

## 💻 Usage

### Web Interface
1. Connect to your local network
2. Find the ESP32's IP address (displayed on serial monitor)
3. Open `http://[ESP32_IP_ADDRESS]` in your browser
4. View real-time meter data in tabular format

### Data Format
The system collects and transmits:
- **Meter Serial Numbers**: Unique identification
- **Energy Measurements**: kWh, kW, Voltage, Current
- **Timestamps**: Measurement time
- **Status Information**: Connection and error states

### API Integration
Data is automatically uploaded to the AmpX Portal every 30 seconds in JSON format:
```json
{
  "gateway_id": "100001",
  "meter_id": "1",
  "serial_number": "12345678",
  "timestamp": "2024-01-15T10:30:00Z",
  "values": {
    "serial": "12345678",
    "voltage_L1": "230.1",
    "current_L1": "24.6",
    "active_power_tot": "5.67",
    "active_energy_imported_tot": "1234.56"
  }
}
```

## 🛠️ Development

### Project Structure
```
Open-Energy-Gateway/
├── src/
│   └── open_energy_gateway/
│       ├── open_energy_gateway.ino       # Main application
│       ├── functions.ino                 # NVS utilities
│       ├── functions_modbus.ino          # Modbus communication
│       ├── functions_web.ino             # Web interface & WebSocket
│       ├── functions_wifi.ino            # WiFi management
│       ├── functions_ethernet.ino        # Ethernet initialization
│       ├── functions_meter.ino           # Meter register definitions
│       ├── functions_api.ino             # API communication
│       ├── functions_ntp.ino             # Time synchronization
│       ├── functions_ota.ino             # OTA updates (disabled)
│       ├── meter_registers.h             # Register definitions
│       ├── webpage.h                     # HTML templates
│       ├── web_admin.h                   # Admin interface
│       ├── web_settings.h                # Settings page
│       ├── data/                         # Web assets (future SPIFFS option)
│       │   ├── index.html
│       │   ├── settings.html
│       │   ├── admin.html
│       │   └── meter_registers_meatrol.json
│       └── tools/                        # SPIFFS tools (future use)
│           ├── create_spiffs.bat
│           ├── upload_spiffs.bat
│           └── spiffsgen.py
├── memory-bank/                          # Project documentation
├── Documentation/                        # User documentation
└── README.md
```

### Building Different Variants
- **RS485 Version**: Set `MODBUS_TYPE_RS485`
- **TCP/IP Version**: Set `MODBUS_TYPE_TCPIP`

### Debugging
Enable debug output by defining:
```cpp
#define DEBUG 1
```

### Web Interface Implementation
Currently, the web interface is implemented using header files containing HTML templates:

- **webpage.h**: Main dashboard HTML template
- **web_settings.h**: Settings page HTML template  
- **web_admin.h**: Admin page HTML template

The templates use string replacement for dynamic content (meter serial numbers, values, etc.).

### Future SPIFFS Option
The project includes infrastructure for future SPIFFS implementation:

1. **SPIFFS Tools Available** (not currently used):
   ```bash
   cd src/open_energy_gateway/tools
   create_spiffs.bat  # Future: Create SPIFFS image
   upload_spiffs.bat  # Future: Upload to ESP32
   ```

2. **Web Assets Ready** in `data/` folder for future SPIFFS deployment

### Current Limitations
- **Active Meters**: Currently configured for 4 meters (expandable to 32)
- **Hardcoded Settings**: Gateway ID and server URLs require code modification
- **OTA Updates**: Disabled to conserve memory
- **Admin Interface**: Structure exists but functionality incomplete

## 🔍 Troubleshooting

### Common Issues

**No Meter Communication**
- Check wiring connections
- Verify meter addresses (1-32)
- Ensure proper termination resistors (RS485)
- Check network connectivity (TCP/IP)

**Web Interface Not Loading**
- Verify ESP32 is connected to WiFi
- Check IP address in serial monitor
- Ensure port 80 is not blocked

**API Upload Failures**
- Verify internet connectivity
- Check API endpoint configuration
- Monitor serial output for error messages

### Status LED Indicators
- **LED 1 (Power - Pin 12)**: System power status
- **LED 2 (Meter - Pin 14)**: Meter communication status  
- **LED 3 (WiFi - Pin 27)**: WiFi connection status
- **LED 4 (Internet - Pin 26)**: Internet connectivity status
- **LED 5 (Server - Pin 25)**: API server communication status

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow Arduino coding standards
- Test with actual hardware when possible
- Update documentation for new features
- Maintain backwards compatibility

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🏢 Commercial Use

This project is designed for industrial and commercial energy monitoring applications:
- **Manufacturing Facilities**: Multi-zone energy monitoring
- **Commercial Buildings**: Tenant energy tracking
- **Energy Auditing**: Standardized data collection
- **Remote Monitoring**: Centralized meter management

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Johannf78/Open-Energy-Gateway/issues)
- **Documentation**: See `memory-bank/` folder for detailed technical documentation
- **Hardware Support**: Compatible with Meatrol Brand energy meters

## 🙏 Acknowledgments

- ESP32 Community for excellent documentation and libraries
- Modbus community for protocol specifications
- Contributors to the WebSocket and WiFiManager libraries

---

**Built with ❤️ for the energy monitoring community**
