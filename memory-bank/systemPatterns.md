# System Patterns - AmpX Open Energy Gateway

## Architecture Overview

### Modular Design Pattern
The system uses a clean separation of concerns with multiple `.ino` files:

```
open_energy_gateway.ino     # Main application & coordination
├── functions.ino           # Core utilities & NVS
├── functions_modbus.ino    # Modbus communication & data processing
├── functions_web.ino       # Web server & WebSocket handling
├── functions_wifi.ino      # WiFi management with WiFiManager
├── functions_ethernet.ino  # Ethernet initialization (TCP/IP variant)
├── functions_meter.ino     # Meter management & register definitions
├── functions_api.ino       # Remote API communication
├── functions_ntp.ino       # Time synchronization
└── functions_ota.ino       # Over-the-air updates (disabled)
```

### Hardware Abstraction Pattern
Conditional compilation for different hardware variants:
```cpp
#define MODBUS_TYPE_RS485 1
#define MODBUS_TYPE_TCPIP 2
#define MODBUS_TYPE MODBUS_TYPE_TCPIP  // Set variant here

#if MODBUS_TYPE == MODBUS_TYPE_RS485
    // RS485-specific code
#else
    // TCP/IP-specific code
#endif
```

## Data Processing Patterns

### Register Processing Pipeline
1. **Read Modbus Registers**: Raw 16-bit values from meters
2. **Type Conversion**: Convert based on data type (int32, int64, float)
3. **JSON Serialization**: Store in global JsonDoc for web interface
4. **WebSocket Broadcasting**: Real-time updates to connected clients

### Data Type Handling
```cpp
const int dataTypeInt32 = 1;  // 2 registers → 32-bit integer
const int dataTypeInt64 = 2;  // 4 registers → 64-bit integer (energy)
const int dataTypeFloat = 3;  // 2 registers → 32-bit float
```

### JSON Document Pattern
Global `DynamicJsonDocument` serves as central data store:
- Updated by Modbus reading functions
- Consumed by WebSocket for real-time updates
- Serialized for API transmission

## Communication Patterns

### WebSocket Real-Time Updates
- 3-second interval meter reading
- Immediate JSON broadcast to connected clients
- No polling required from web interface

### API Upload Pattern
- 30-second interval for remote server uploads
- Separate JSON formatting for API requirements
- Configurable local vs live server endpoints

### Status LED Feedback Pattern
```cpp
LED_1_POWER     // Power on indicator
LED_2_METER     // Modbus communication status
LED_3_WIFI      // WiFi connection status
LED_4_INTERNET  // Internet connectivity
LED_5_SERVER    // Successful API communication
```

## Memory Management Patterns

### Debug Output Control
```cpp
#define DEBUG 1
#if DEBUG == 1
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif
```

### Preferences Storage Pattern
- Non-volatile storage for meter names and settings
- Key-value pairs: `m1_name`, `m2_name`, etc.
- Persistent across reboots

### SPIFFS File System
- Web assets stored in flash memory
- Reduces program memory usage
- Batch tools for SPIFFS creation and upload

## Error Handling Patterns

### Connection Management
- Automatic reconnection on Modbus failures
- WiFi reconnection with WiFiManager fallback
- Graceful degradation when meters are offline

### Timeout Handling
- NTP synchronization timeout (20 seconds)
- Meter communication timeouts
- Configurable retry mechanisms

## Timing Patterns

### Multi-Timer System
```cpp
static unsigned long counter1 = 0;  // Meter reading (3s)
static unsigned long counter2 = 0;  // API upload (30s)
static unsigned long counter3 = 0;  // Reboot timer (24h)
```

### Non-Blocking Operations
- `millis()` based timing instead of `delay()`
- Continuous `server.handleClient()` and `webSocket.loop()`
- Preserves real-time responsiveness

## Security Patterns

### Network Security
- WiFiManager with AP mode for initial setup
- HTTPS API endpoints for data transmission
- No hardcoded credentials in code

### Gateway Identification
- Unique `AMPX_GATEWAY_ID` for device identification
- Serial number tracking for individual meters
- Structured data format for audit trails
