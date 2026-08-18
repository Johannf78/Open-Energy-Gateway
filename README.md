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
- In Arduino IDE, add this under `File -> Preferences -> Additional Board Manager URLs`:  
  `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
  Then install `esp32` from `Tools -> Board -> Boards Manager`.
- ESP32 Board Package for Arduino IDE
- USB-to-UART driver for your ESP32 board (CP210x VCP driver): [Silicon Labs CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Required Arduino libraries (see Dependencies section)


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

4. **Install USB Driver (Required)**
   - Most ESP32 development boards use a CP210x USB-to-UART bridge chip
   - Install the CP210x VCP driver from Silicon Labs:  
     [https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - On the Silicon Labs download list (Windows), select **CP210x Windows Drivers**
   - Reconnect the board after driver installation so the serial COM port is detected

5. **Upload to ESP32**
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
The project requires custom AmpX Modbus libraries available at:
**Repository**: [https://github.com/Johannf78/AmpXModbus](https://github.com/Johannf78/AmpXModbus)

- **For RS485**: `ampx_modbus_rs485.h` 
- **For TCP/IP**: `ampx_modbus_tcpip.h`

**Installation**:
1. Clone or download the AmpXModbus repository
2. Copy the library folders to your Arduino libraries directory:
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
The system supports dual API endpoints (configurable in `open_energy_gateway.ino`):
```cpp
// Local: LAN IP of the PC running XAMPP (port 80). Update when the PC IP changes.
const char* ampxportal_server_local = "http://192.168.2.120/api/v2/";
const char* ampxportal_server_live = "https://ampx.app/api/v2/";
const char* ampxportal_api_key = "...";  // must match AMPX_API_KEY in api/config/config.php
#define USE_LOCAL_SERVER true  // Set to false for production
```

Every upload sends header `X-AmpX-Api-Key` (firmware **1.0.3+**). Full API docs: `ampx.app/api/README.md`.

**Breaking changes:** If the portal/API contract changes so older flashed gateways fail, bump `FIRMWARE_VERSION` in the same change and publish OTA before (or with) the live API cutover. The API key requirement is one such break (pre-1.0.3 → 401).

**Local development stack (verified July–August 2026):**
- API: XAMPP at `D:\xampp\htdocs\ampx.app\api\` — `POST /api/v2/` with `X-AmpX-Api-Key` (see that folder’s `README.md`)
- Portal UI: http://ampx-app.local/ (WordPress vhost; not the public Cloudflare `ampx.app` host)
- InfluxDB (current): `influxdb2.ampx.app` via `api/config/config.php`; success = HTTP **201**; missing/wrong key = **401**
- InfluxDB (target): managed **Cloud Serverless** org AmpX / Energy Gateway — host `https://eu-central-1-1.aws.cloud2.influxdata.com` (AWS Frankfurt); API v3 cutover pending — see `ampx.app/api/README.md`
- Postman: `http://ampx-app.local/api/v2/` or `http://127.0.0.1/api/v2/` — enable header `X-AmpX-Api-Key`
- ESP must reach the PC on TCP **80**; Windows Wi‑Fi profile should be **Private** with Apache firewall allow rules
- Do not use port **8080** unless a separate Docker API is running

**Live stack (verified July 2026; API key gate pending Hetzner deploy):**
- API: `https://ampx.app/api/v2/` — set `#define USE_LOCAL_SERVER false`
- When enabling the key on live: deploy `api/config/config.php` + `v2/index.php` and flash/OTA gateways with matching `ampxportal_api_key` together
- Portal UI: https://ampx.app/ → Meters → gateway → View Data
- WordPress `wp-config.php` must define `AMPX_INFLUXDB_URL`, `AMPX_INFLUXDB_TOKEN`, `AMPX_INFLUXDB_ORG`, `AMPX_INFLUXDB_BUCKET` (portal plugin requires them)
- After editing live `wp-config.php` on Hetzner: flush PHP OPcache or constants may appear missing
- Create/assign the gateway in WP Admin; Influx data alone does not register it in the portal list

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
│       ├── functions_ota.ino             # HTTP pull OTA (Admin)
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
- **Active Meters**: Currently configured for 5 meters (backend expandable; HTML rows needed beyond that)
- **Server URLs**: API and OTA hosts (`ampxportal_server_local` / `_live`, `USE_LOCAL_SERVER`, firmware URLs) require a firmware rebuild
- **Gateway ID**: Changeable from Admin; stored in NVS (`DEFAULT_GATEWAY_ID` is first-boot only)
- **OTA Updates**: Admin HTTP pull from `https://ampx.app/firmware/` (Check + Update); first flash USB with 8MB dual-OTA partitions; baseline **1.0.7**
- **Admin Interface**: Gateway reboot + OTA; Gateway ID change with password

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

**Web UI Stuck on “Connecting…” (WebSocket)**

The page is served over HTTP (port 80). **Connection status: Connecting…** means the browser is waiting for the WebSocket on **port 81** (`ws://hostname:81/`). The HTTP page can load while the WebSocket is still down.

**Cause:** Modbus meter reads are blocking. If `webSocket.loop()` and `server.handleClient()` are not called around each meter read, the ESP32 cannot complete WebSocket handshakes until that work finishes — the UI can stay on Connecting for a long time (sometimes ~1–2 minutes).

**Required pattern** in the main loop (around each `handlePowerMeter()` call):
```cpp
server.handleClient();
webSocket.loop();
handlePowerMeter(currentMeterIndex);
handleWebSocket();
server.handleClient();
webSocket.loop();
```

Also keep `server.handleClient()` / `webSocket.loop()` at the end of `loop()`.

**Boot delay (separate from WebSocket):** Before `loop()` runs, `setup()` may wait on NTP (up to ~20 seconds) and meter discovery (each missing Modbus address can take ~1 second). The web server does not handle clients until `setup()` finishes.

**Cannot Connect to Setup Hotspot (Windows)**

On first boot (or when no WiFi credentials are saved), the gateway creates an open access point named `energy-gateway-{GATEWAY_ID}` (for example `energy-gateway-100001`). Connect to this network and open **http://192.168.4.1** to configure WiFi.

**Problem:** Windows shows as connected to the hotspot, but the PC does not receive an IP address (often `169.254.x.x` instead of `192.168.4.x`), so the setup page cannot be opened. The Windows Settings UI may still show “IP assignment: Automatic (DHCP)” even when DHCP is not actually active on the adapter.

**Solution:**
1. Confirm in Command Prompt with `ipconfig /all` that the Wi‑Fi adapter shows **DHCP Enabled: Yes**.
2. If it shows **No**, enable DHCP from an elevated Command Prompt or PowerShell:
   ```
   netsh interface ip set address name="WiFi 3" dhcp
   netsh interface ip set dns name="WiFi 3" dhcp
   ```
   Replace `WiFi 3` with your adapter name from `ipconfig` if different.
3. Disconnect and reconnect to the gateway hotspot, then check that you have an address such as `192.168.4.2` and gateway `192.168.4.1`.
4. Open **http://192.168.4.1** in a browser.

**Workaround:** If DHCP still fails, set a temporary static IP on the Wi‑Fi adapter: IP `192.168.4.10`, subnet mask `255.255.255.0`, gateway `192.168.4.1`.

**API Upload Failures**
- Serial success looks like: `HTTP Response Code: 201` and `Data stored successfully`
- HTTP **401**: `ampxportal_api_key` must match `AMPX_API_KEY` in `api/config/config.php` (local already enforces this)
- `connection refused` (-1) to the PC IP: check Windows Firewall / Wi‑Fi **Private** profile; Apache must accept LAN inbound on port 80 (not only localhost)
- Confirm `ampxportal_server_local` matches the PC’s current LAN IP and uses `/api/v2/` on port 80
- Confirm InfluxDB is healthy (`https://influxdb2.ampx.app/health`) — API returns 500 if storage fails
- View data (local): http://ampx-app.local/ → Meters → gateway → View Data
- View data (live): https://ampx.app/ → Meters → gateway → View Data (needs WP gateway assignment + `AMPX_INFLUXDB_*` in live `wp-config.php`)
- Misleading firmware message: “No internet connection” on connection-refused may mean LAN blocked, not WAN down

**Admin OTA reboot / Checking… stuck**
- Serial `rst:0x8 (TG1WDT_SYS_RESET)` after `Fetching firmware manifest:` = old firmware (≤1.0.5) putting `WiFiClientSecure` on the `loop()` stack — USB-flash or OTA **1.0.7+**
- Opening Admin must not fetch; only **Check for update** starts HTTPS
- Serial Monitor baud is **115200**
- Live files: `https://ampx.app/firmware/version.json` and the `.bin` must both be published and versions aligned

**Live portal “critical error” on Meters**
- Check Debug Log Manager log (path in `WP_DEBUG_LOG`), not only Apache `www_logs`
- Common cause: missing `AMPX_INFLUXDB_*` constants in live `wp-config.php`
- If constants were just added and error persists: flush PHP OPcache

**NTP `TIMEOUT!` on boot**
- `initNTP()` waits up to a max (currently ~15s) and exits early on success; increase the timeout if sync still fails
- Check router/firewall allows outbound NTP (UDP/123)

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
