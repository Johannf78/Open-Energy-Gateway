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
└── functions_ota.ino       # HTTP pull OTA (Admin Check + Update)
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
- Staggered meter reading (one meter per ~1s interval) with immediate JSON broadcast
- No polling required from web interface
- Comprehensive error handling with automatic reconnection
- Connection status indicators for user feedback
- All dynamic content updates via WebSocket (no server-side string replacement)

### Non-Blocking Network Servicing (Critical)
Blocking Modbus (`modbus_test_connection`, `handlePowerMeter`) prevents WebSocket handshakes if the network stack is not serviced. **Required pattern** in the meter-read path:

```cpp
server.handleClient();
webSocket.loop();
handlePowerMeter(currentMeterIndex);
handleWebSocket();   // broadcast JSON
server.handleClient();
webSocket.loop();
```

Also keep `server.handleClient()` / `webSocket.loop()` at the end of `loop()`. UI “Connecting…” = waiting on port 81; HTTP page load does not imply WebSocket is up.

**Boot note**: `setup()` does not service clients until it returns — NTP sync and full meter discovery can delay first connection.

### Breaking-change versioning
- If a change makes previously flashed gateways fail against the portal/API, bump `FIRMWARE_VERSION` in the same change (and publish matching `version.json` for OTA).
- API key auth (Aug 2026) was such a break: pre-1.0.3 devices get **401**. Allowed then only because no field fleet existed.

### Time-series hosting pattern (August 2026)
- **Production today**: InfluxDB 2 at `influxdb2.ampx.app`; portal uses Flux
- **Target**: Managed **InfluxDB Cloud Serverless** (AmpX / Energy Gateway / AWS Frankfurt) — SQL reads, v2-compatible writes; AmpX app stays on Hetzner
- **Avoid**: Self-hosting InfluxDB on the Hetzner WordPress host for production reliability
- **Future AmpX API v3**: New endpoint writing to Cloud Serverless; keep v2 until cutover; firmware URL bump is a versioned change

### API Upload Pattern
- 30-second interval for remote server uploads (`postToAmpXPortal2`)
- JSON: `gateway_id`, `meter_id`, `serial_number`, `timestamp`, `values{mN_*}`
- Auth: HTTP header `X-AmpX-Api-Key` = sketch `ampxportal_api_key` = `AMPX_API_KEY` in `api/config/config.php` (shared fleet secret; `hash_equals` on server)
- Missing/wrong key → **401** before validation/Influx; empty server config → **500**
- Success = HTTP **201**; Influx tags: `gateway`, `meter`, `serial_number`; measurement `meter_readings_detailed`
- Configurable local vs live via `USE_LOCAL_SERVER` / `ampxportal_server_local` | `_live`
- Local = XAMPP API on LAN IP port 80; live = `https://ampx.app/api/v2/` (key gate on live only after Hetzner deploy)
- Portal reads Influx via plugin (`class-ampx-portal-influxdb-detailed.php`); UI at `/meters/?gateway_id=`

### HTTP OTA Pattern (August 2026; field baseline 1.0.7)
- Admin HTML must **not** call outbound `HTTPClient` in web handlers — that caused `ERR_CONNECTION_RESET` / hangs
- Flow: **Check for update** → `GET /ota_status?refresh=1` sets `otaManifestCheckRequested` → `loop()` runs `serviceOtaManifestCheck()` → fills `otaStatusCache` → JS polls `/ota_status` (no `refresh`). Opening `/admin` does **not** start TLS
- **Never** fetch the manifest from a FreeRTOS side task: 1.0.3 used `otaManifestTask` on core 0 with boot-time HTTPS → LoadProhibited reboot loop (**1.0.4** moved check to `loop()`)
- **Never** declare `WiFiClientSecure` as a local in `loop()` / `fetchFirmwareManifest()`: 1.0.4/1.0.5 stack overflow → `TG1WDT_SYS_RESET` on Check. Use **static** `otaTlsClient` / `otaPlainClient` + `SET_LOOP_TASK_STACK_SIZE(16384)`
- Do not share Arduino `String` objects across cores without a mutex (prefer loop-only access)
- Compare versions with `isNewerVersion()` (`major.minor.patch` numeric). Update only if server **>** device. Equal or older server → “Up to date”, button **disabled**
- Manifest fetch fail → “Unavailable”; button may stay enabled for a manual attempt
- `POST /update` re-checks; refuses if server is not newer; download URL from manifest `url` (local mode forces `firmwareURL`); flash via `HTTPUpdate`
- Host both `ampx_open_energy_gateway.bin` and `version.json` under `/firmware/` (live verified 1.0.7)
- ArduinoOTA not used; first flash via USB with dual OTA partitions, then field updates via Admin

### Portal Influx Config Pattern (WordPress)
- Plugin never hardcodes Influx credentials; `AMPX_Portal_Config` requires `AMPX_INFLUXDB_*` constants in `wp-config.php`
- Meters page fails hard (or admin error box) if those constants are missing
- On PHP 8.4 hosts: always pass `$escape` to `str_getcsv`; skip Influx `#` annotation CSV lines
- After deploying `wp-config.php` on OPcache hosts (Hetzner): flush OPcache before testing
- WP gateway table is separate from Influx: assign gateway to user in admin before portal UI shows it

### Portal Meter Data Pattern (August 2026)
- **Query window**: `range(start: -30d)` (`READINGS_RANGE_DAYS = 30`)
- **Table**: newest `READINGS_DISPLAY_LIMIT` (1000) via Flux `limit(n: 1000)`; UI must state both caps
- **Export CSV**: must not scrape the DOM — use `admin_post_ampx_export_meter_csv` with nonce + gateway ACL; Influx fetch with `$limit = null` (same 30d window, no row cap)
- **Wide tables**: theme `.meter-data-container .table-wrapper` needs `max-height` + `overflow-x: scroll` so the horizontal scrollbar stays in the viewport (bar at end of full table height is invisible)
- Avoid `.table-wrapper { overflow: hidden }` for meter readings — it clips columns with no scroll

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

### Web Interface Storage
- **Current Implementation**: HTML templates in header files
- **WebSocket Architecture**: All dynamic content updates via WebSocket JSON transmission
- **Template System**: Static HTML with JavaScript-based DOM updates
- **Memory Efficiency**: Static allocation in program memory
- **Future SPIFFS Option**: Infrastructure ready for deployment

## Error Handling Patterns

### Connection Management
- Automatic reconnection on Modbus failures
- WiFi reconnection with WiFiManager fallback
- Graceful degradation when meters are offline

### Power Supply Management
- **Development**: Direct USB connection to avoid power supply issues
- **WiFi Power Spikes**: Monitor for voltage drops during high-current operations
- **Production**: External power supply recommended for industrial deployments
- **Diagnostic**: Garbled serial output indicates power supply instability

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

## Scaling Patterns

### Current Limitations (4-Meter Hardcoded)
- **HTML Templates**: Static meter sections in `webpage.h` and `web_settings.h`
- **Global Variables**: `m1_serial_number` through `m4_serial_number` pattern
- **Server Code**: Hardcoded meter handling loops
- **JavaScript**: Static meter references in WebSocket processing

### Scaling Strategy for 10+ Meters
- **Dynamic HTML Generation**: Template-based meter section creation
- **Array-Based Data**: Replace global variables with meter arrays
- **Loop-Based Processing**: Dynamic meter handling in server and client code
- **WebSocket Scalability**: JSON-based updates support unlimited meters
- **Memory Management**: Efficient data structures for large meter counts
