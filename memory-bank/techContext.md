# Technical Context - AmpX Open Energy Gateway

## Development Environment

### Platform & IDE
- **Primary IDE**: Arduino IDE (recommended)
- **Alternative**: PlatformIO
- **Target Board**: ESP32 Node32S (ESP32 Wroom 32U)
- **Board Package**: ESP32 by Espressif Systems
- **Board Manager URL**: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

### Power Supply Requirements
- **Development**: Direct PC USB connection required (avoid USB hubs)
- **WiFi Operations**: Peak current draw 200-300mA during connection attempts
- **Production**: Consider external power supply for industrial deployments
- **Stability**: USB hubs can cause voltage drops and power supply noise

### Build Configuration
- **Partition Scheme**: "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)" or "Huge APP (3MB No OTA/1MB SPIFFS)"
- **Serial Baud Rate**: 115200 (sketch `Serial.begin(115200)`)
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
- **Repository**: [AmpXModbus](https://github.com/Johannf78/AmpXModbus)
- **RS485 Variant**: `ampx_modbus_rs485.h` (custom AmpX library)
- **TCP/IP Variant**: `ampx_modbus_tcpip.h` (custom AmpX library)
- **Installation**: Clone repository and copy to `Documents/Arduino/libraries/` folder

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
- **AP Mode**: `energy-gateway-{GATEWAY_ID}` (open / no password) — first boot and after Admin Clear WiFi
- **Hostname / mDNS**: `energy-gateway-{GATEWAY_ID}` (`.local` often fails on Windows; use IP)
- **Config portal**: `http://192.168.4.1` (WiFiManager); save-page copy customized in `initWiFi()`
- **Web Server**: Port 80
- **WebSocket**: Port 81
- **Admin password**: `ADMIN_PASSWORD` in `open_energy_gateway.ino` (Gateway ID + Clear WiFi); do not put the value in public docs

### API Endpoints
```cpp
// Local: PC LAN IP + XAMPP on port 80 (update IP when DHCP changes)
const char* ampxportal_server_local = "http://192.168.2.120/api/v3/";
const char* ampxportal_server_live = "https://ampx.app/api/v3/";
const char* ampxportal_api_key = "...";  // must match AMPX_API_KEY in api/config/config.php
#define USE_LOCAL_SERVER false  // 1.1.1 production Cloud path; true → LAN /api/v3/
```

### Local AmpX stack (July–August 2026)
- **API code**: `D:\xampp\htdocs\ampx.app\api\` — v2 `POST /api/v2/` (Influx 2); v3 `POST /api/v3/` (Cloud Serverless, local + live) — docs: `api/README.md`
- **Auth**: Required header `X-AmpX-Api-Key` (local + live v3 verified August 2026); value from `AMPX_API_KEY` / `ampxportal_api_key`
- **Portal (WordPress)**: http://ampx-app.local/ (hosts → 127.0.0.1; Apache vhost DocumentRoot `htdocs/ampx.app`)
- **LAN access to API**: junction `D:\xampp\htdocs\api` → `ampx.app\api` so `http://{LAN_IP}/api/v2/` works
- **LAN access to firmware**: junction `D:\xampp\htdocs\firmware` → `ampx.app\firmware` so `http://{LAN_IP}/firmware/*.bin` works
- **InfluxDB (legacy v2)**: `https://influxdb2.ampx.app`, org `ampx`, bucket `energy_metrics`, measurement `meter_readings_detailed`
- **Success**: HTTP **201** + portal meters page for the gateway; **401** if key missing/wrong
- **Postman**: Local URL `http://ampx-app.local/api/v2/` or `http://127.0.0.1/api/v2/`; enable `X-AmpX-Api-Key` header
- **Do not use**: `:8080` unless a Docker API container is running; public DNS `ampx.app` ≠ local vhost

### InfluxDB Cloud Serverless (production portal + v3 — August 2026)
- **Product**: Managed InfluxDB Cloud Serverless (not self-hosted on Hetzner; not early-access “InfluxDB 3 Cloud”)
- **Account**: AmpX · **Organization**: Energy Gateway · **Org ID**: `86a141bfd8d7f66a`
- **Region**: AWS `eu-central-1` (EU Frankfurt)
- **Host**: `https://eu-central-1-1.aws.cloud2.influxdata.com`
- **Bucket**: `energy_metrics` (ID `43bdc9cfa0531940`, retention 30 days)
- **Docs**: https://docs.influxdata.com/influxdb3/cloud-serverless/
- **Writes**: v1/v2-compatible `/api/v2/write` + line protocol (spike **204** OK)
- **Queries (HTTP/PHP-friendly)**: InfluxQL on `GET /query?db=energy_metrics` — requires **DBRP** mapping (created: database `energy_metrics` / rp `autogen` → bucket id `43bdc9cfa0531940`)
- **Queries (native SQL)**: Flight+gRPC client libraries (not used in spike)
- **Token**: local `INFLUXDB_CLOUD_TOKEN` in `api/config/config.php` (v2 still uses legacy `INFLUXDB_*`); rotate if exposed
- **Spike script**: `api/v2/tests/spike_cloud_serverless.php`
- **AmpX wiring**: Local + live `/api/v3/` writes + portal InfluxQL reads verified 22 Aug 2026 (100007 / 3423875005); never put Cloud token in README
- **Upgrade path if needed**: Cloud Dedicated (sales), not DIY Core on the WP host

### Live AmpX portal (July 2026)
- **Site**: https://ampx.app/ (Hetzner `dedivirt3789.your-server.de`, FTP `ampxapp`, PHP **8.4**)
- **API**: `https://ampx.app/api/v3/` (Cloud); `https://ampx.app/api/v2/` still on Influx 2. Live v3 key gate verified GET **405**, POST no key **401**, keyed POST **201**.
- **Portal plugin**: `ampx-portal-plugin` (**1.1.5**) — InfluxQL against Cloud (`AMPX_Portal_InfluxDB_Detailed`)
- **Portal theme**: `ampx-portal-theme` (**1.0.9** local, **1.0.7** live) — table scroll CSS not on live yet
- **Meter Data page**: `/meter-data/?meter_sn=&gateway_id=` — table ≤1000 newest / 30d; Export CSV = full 30d via `admin-post.php?action=ampx_export_meter_csv`
- **Required in live `wp-config.php`** (before “stop editing”):
  - `AMPX_INFLUXDB_URL`, `AMPX_INFLUXDB_TOKEN`, `AMPX_INFLUXDB_ORG`, `AMPX_INFLUXDB_BUCKET`
  - Local XAMPP `wp-config.php` already had these; live was missing them → WP critical error on Meters
- **OPcache**: After changing live `wp-config.php`, flush OPcache or PHP keeps old defines
- **Debug log**: `WP_DEBUG_LOG` → Debug Log Manager file under `wp-content/uploads/debug-log-manager/` (not `wp-content/debug.log`)
- **Gateway registry**: Create/assign gateway in WP Admin; Influx alone does not list a gateway
- **Design docs**: `ampx.app/docs/superpowers/specs/2026-08-10-meter-data-display-limit-export-design.md`

## Memory Management

### File System
- **Current**: HTML templates in header files (`webpage.h`, `web_settings.h`, `web_admin.h`)
- **Future SPIFFS Option**: Infrastructure ready with tools and data folder
- **Tools Available**: `create_spiffs.bat`, `upload_spiffs.bat`, `spiffsgen.py`
- **Web Assets Ready**: `index.html`, `settings.html`, `admin.html`, `meter_registers_meatrol.json`

### Storage Optimization
- Conditional debug output compilation
- Static string templates with runtime replacement
- Optimized JSON document sizing
- HTTP OTA enabled (ArduinoOTA not used); sketch ~1.3MB fits dual ~2.1MB OTA app slots in `partitions.csv`

## Timing & Performance

### Update Intervals
```cpp
const unsigned long METER_CONNECTION_INTERVAL = 1000;   // 1 second (staggered: one meter per tick)
const unsigned long REMOTE_SERVER_INTERVAL = 30000;     // 30 seconds  
const unsigned long REBOOT_INTERVAL = 86400000;         // 24 hours
```

### Processing Pipeline
1. **Meter Reading**: One meter per 1-second interval (full cycle ≈ numberOfMeters seconds)
2. **Network service**: `server.handleClient()` + `webSocket.loop()` before/after each meter read (required for responsive WS)
3. **WebSocket Update**: Immediate broadcast after each meter read
4. **API Upload**: 30-second intervals
5. **Status Monitoring**: Continuous LED status updates

### Flash / OTA (August 2026)
- Target: **8MB** modules — Arduino IDE **ESP32 Dev Module**, Flash **8MB**, Partition **custom** (`src/open_energy_gateway/partitions.csv` dual OTA apps) or **8M with spiffs**
- Current firmware: **`FIRMWARE_VERSION` 1.1.1** (`ampxportal_server_live` = `/api/v3/`, `USE_LOCAL_SERVER false` on 100007). Do not ship 1.0.3 reboot loop, or 1.0.4/1.0.5 Check WDT. Live OTA `version.json` still **1.0.9** (v2) — next publish **1.1.1**, never 1.0.9.
- `SET_LOOP_TASK_STACK_SIZE(16384)` — default 8KB `loop()` stack overflows during HTTPS TLS
- Admin HTTP pull: `https://ampx.app/firmware/ampx_open_energy_gateway.bin` via `HTTPUpdate` + **static** `WiFiClientSecure`
- Manifest check: `serviceOtaManifestCheck()` from **`loop()` only** after **Check for update**; no FreeRTOS OTA task; no Admin-load or boot-time HTTPS fetch
- Local hosting: `D:\xampp\htdocs\ampx.app\firmware\` + junction `D:\xampp\htdocs\firmware` for LAN IP downloads
- Deploy live: `firmware/deploy-firmware.ps1` with `AMPX_FTP_PASS` (Hetzner `ampxapp`)
- First install still requires USB flash with OTA partitions; thereafter Admin Update Firmware
- ArduinoOTA intentionally disabled (flash size / unused for field updates)

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
- Serial output at **115200** baud
- Configurable debug levels
- WiFi signal strength reporting
- Modbus communication status
