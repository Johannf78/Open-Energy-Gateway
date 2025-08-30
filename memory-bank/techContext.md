# Technical Context - AmpX Open Energy Gateway

## Development Environment

### Platform & IDE
- **Primary IDE**: Arduino IDE (recommended)
- **Alternative**: PlatformIO
- **Target Board**: ESP32 Node32S (ESP32 Wroom 32U)
- **Board Package**: ESP32 by Espressif Systems
- **Board Manager URL**: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

### Build Configuration
- **Partition Scheme**: "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)" or "Huge APP (3MB No OTA/1MB SPIFFS)"
- **Serial Baud Rate**: 9600
- **Debug Level**: Configurable via `#define DEBUG 1`

## Hardware Specifications

### ESP32 Configuration
- **Microcontroller**: ESP32 Wroom 32U
- **Flash Memory**: Program + SPIFFS storage
- **RAM**: Dynamic JSON documents up to 2048 bytes
- **GPIO**: Multiple pins for status LEDs and communication interfaces

### Communication Variants

#### RS485 Variant
```cpp
#define MAX485_DE 4     // Driver Enable (White)
#define MAX485_RE_NEG 4 // Receiver Enable (Active Low)
#define RX_PIN 16       // Receive Pin (Orange)
#define TX_PIN 17       // Transmit Pin (Yellow)
// Modbus A (Positive, Green), Modbus B (Negative, Blue)
```

#### TCP/IP Variant
```cpp
#define ETH_SPI_SCS_PIN 5  // Chip Select for W5500
// Hardware SPI: SCK=18, MISO=19, MOSI=23
byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0x94, 0xB5};
IPAddress ip(192, 168, 1, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress meter_ip(192, 168, 1, 55);
```

### Status LEDs
```cpp
#define LED_1_POWER     12  // Power indicator
#define LED_2_METER     14  // Meter communication
#define LED_3_WIFI      27  // WiFi connection
#define LED_4_INTERNET  26  // Internet connectivity
#define LED_5_SERVER    25  // Server communication
```

## Dependencies

### Required Arduino Libraries
```cpp
#include <WiFi.h>              // ESP32 built-in WiFi
#include <WebServer.h>         // HTTP server functionality
#include <WebSocketsServer.h>  // Real-time updates (Markus Sattler)
#include <ArduinoJson.h>       // JSON processing (Benoit Blanchon)
#include <HTTPClient.h>        // HTTP client for API calls
#include <WiFiManager.h>       // WiFi configuration (tzapu)
#include <Preferences.h>       // Non-volatile storage (Vlodomyr Shymanskyy)
#include <HardwareSerial.h>    // RS485 communication
#include <time.h>              // NTP time synchronization
```

### Custom Libraries
- **RS485 Variant**: `ampx_modbus_rs485.h` (custom AmpX library)
- **TCP/IP Variant**: `ampx_modbus_tcpip.h` (custom AmpX library)
- Location: `Documents/Arduino/libraries/` folder

## Data Structures

### JSON Documents
```cpp
DynamicJsonDocument JsonDoc(2048);        // Main data storage
JsonDocument MeterRegisterDefs;           // Register definitions
```

### Meter Register Format
```cpp
// Format: [register_address, num_registers, data_type, friendly_name, json_key]
// Data Types: 1=int32, 2=int64, 3=float
[70, 2, 1, "Serial Number", "serial"]
[1010, 2, 3, "Voltage L1", "voltage_L1"]
[2500, 4, 2, "Energy Imported L1", "active_energy_imported_L1"]
```

## Network Configuration

### WiFi Setup
- **AP Mode**: `AmpX-Energy-Gateway-AP` (no password)
- **Hostname**: `AmpX-Energy-Gateway`
- **Web Server**: Port 80
- **WebSocket**: Port 81

### API Endpoints
```cpp
const char* ampxportal_server_local = "http://192.168.2.32:8080/api/v2/";
const char* ampxportal_server_live = "https://portal.ampx.app/api/v2/";
#define USE_LOCAL_SERVER true  // Development flag
```

## Memory Management

### File System
- **Current**: HTML templates in header files (`webpage.h`, `web_settings.h`, `web_admin.h`)
- **Future SPIFFS Option**: Infrastructure ready with tools and data folder
- **Tools Available**: `create_spiffs.bat`, `upload_spiffs.bat`, `spiffsgen.py`
- **Web Assets Ready**: `index.html`, `settings.html`, `admin.html`, `meter_registers_meatrol.json`

### Storage Optimization
- OTA functionality disabled to save space
- Conditional debug output compilation
- Static string templates with runtime replacement
- Optimized JSON document sizing

## Timing & Performance

### Update Intervals
```cpp
const unsigned long METER_CONNECTION_INTERVAL = 3000;    // 3 seconds
const unsigned long REMOTE_SERVER_INTERVAL = 30000;     // 30 seconds  
const unsigned long REBOOT_INTERVAL = 86400000;         // 24 hours
```

### Processing Pipeline
1. **Meter Reading**: 3-second intervals for up to 32 meters
2. **WebSocket Update**: Immediate after meter reading
3. **API Upload**: 30-second intervals
4. **Status Monitoring**: Continuous LED status updates

## Security Considerations

### Network Security
- HTTPS API communication
- No hardcoded WiFi credentials
- WiFiManager for secure credential storage

### Data Integrity
- NTP time synchronization for accurate timestamps
- Serial number validation for meter identification
- JSON schema validation for API uploads

## Development Tools

### Future SPIFFS Tools (Not Currently Used)
- `spiffsgen.py`: Generate SPIFFS image from data folder
- `create_spiffs.bat`: Windows batch script for SPIFFS creation
- `upload_spiffs.bat`: Upload SPIFFS to ESP32

### Debugging
- Serial output at 9600 baud
- Configurable debug levels
- WiFi signal strength reporting
- Modbus communication status
