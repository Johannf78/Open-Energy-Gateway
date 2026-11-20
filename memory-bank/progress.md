# Progress Status - AmpX Open Energy Gateway

## ✅ Completed Features

### Core Infrastructure
- [x] **ESP32 Platform Setup**: Arduino IDE configuration with ESP32 board support
- [x] **Modular Architecture**: 9 separate `.ino` files for clean code organization
- [x] **Hardware Variants**: Conditional compilation for RS485 and TCP/IP communication
- [x] **Debug System**: `DEBUG` macros print to USB and a fixed 4 KB buffer; `GET /debug` shows that buffer on the LAN. Admin footer links to it. Verified 29 Sep 2026 on gateway 100009. Included in live catalog **1.2.4**.

### Meter identity (1.3.0 — Nov 19, 2026, compiled only)
- [x] Post sends `manufacturer` (constant) and `meter_model` (register 60, read once per meter, max 3 tries)
- [x] TCP and RS485 compile
- [ ] USB flash on 100007 and an RS485 gateway; confirm `identity_defaulted: false` in the 201
- [ ] Publish OTA only when asked

### Communication Systems
- [x] **WiFi Management**: WiFiManager integration for easy network setup
- [x] **Modbus Communication**: Support for both RS485 and TCP/IP protocols
- [x] **Web Server**: HTTP server with multiple endpoints (/, /settings, /admin, /debug)
- [x] **WebSocket Architecture**: Comprehensive real-time updates for all dynamic content
- [x] **Connection Status Indicators**: Real-time connection status and last update timestamps
- [x] **WebSocket Error Handling**: Robust reconnection logic with retry limits
- [x] **NTP Synchronization**: Accurate timestamp generation for data logging
- [x] **mDNS Service**: .local domain access with hostname `energy-gateway-{GATEWAY_ID}.local`

### Data Management
- [x] **Meter Register Definitions**: JSON-based register mapping for Meatrol meters
- [x] **Type Conversion System**: Proper handling of int32, int64, and float data types
- [x] **JSON Data Pipeline**: Unified data model with `DynamicJsonDocument`
- [x] **Non-Volatile Storage**: Preferences system for persistent settings
- [x] **Web Interface**: HTML templates in header files (SPIFFS infrastructure ready for future)

### User Interface
- [x] **Real-Time Dashboard**: Live meter readings with WebSocket-based 3-second updates
- [x] **Modern Sidebar Layout**: Professional left sidebar navigation with flexbox design
- [x] **Energy Summary Table**: L1, L2, L3, Total columns for active energy imported
- [x] **Power Summary Table**: L1, L2, L3, Total columns for active power
- [x] **Meters Detail Page**: Single meter view accessible via /meters?id=N query string
- [x] **Settings Interface**: WebSocket-powered meter naming and configuration
- [x] **Admin Interface**: Gateway configuration page structure
- [x] **Connection Status Display**: Real-time WebSocket connection status indicators
- [x] **Last Update Timestamps**: Automatic timestamp updates on all pages
- [x] **Status Indicators**: 5 LEDs for comprehensive system health monitoring
- [x] **Mobile Responsive**: Clean table-based layout for mobile devices

### API Integration
- [x] **AmpX Portal Integration**: Structured JSON uploads every 30 seconds
- [x] **Dual Server Support**: Local development and live production endpoints
- [x] **Shared API Key Auth**: `X-AmpX-Api-Key` on `/api/v2/` and `/api/v3/` (local + live v3 verified August 2026)
- [x] **Data Formatting**: Proper timestamp and meter identification in API calls
- [x] **Error Handling**: HTTP client error management and retry logic

### Operational Features
- [x] **Auto-Discovery**: Automatic meter detection during startup
- [x] **Multi-Meter Support**: Currently configured for 5 active meters, tested and operational
- [x] **Status Monitoring**: Continuous health checking with LED feedback
- [x] **Automatic Reboot**: 24-hour reboot cycle for system stability
- [x] **Gateway Reboot Function**: Web-based reboot button with countdown, auto-redirect, and LEDs off before restart
- [x] **Ship-mode / Clear WiFi**: Admin password + confirm; `resetSettings()`; reboot into AP; WiFi only (Gateway ID kept)
- [x] **Front-panel reset button (5 Oct 2026, firmware 1.2.5)**: GPIO 13, hold 3 s, flash all five LEDs twice, then the same clear-and-restart as Admin Clear WiFi. Short press does nothing. A saved network that is missing is retried and does not open the hotspot. USB-flashed and the user confirmed it worked. Not published. Live catalog remains **1.2.4**.
- [x] **WebSocket Event Handling**: Complete event handler with connection/disconnection tracking
- [x] **Staggered Meter Reading**: Sequential 1-second reads for <3-second WebSocket connections
- [x] **Optimized Performance**: Sub-3-second initial connection, progressive data updates
- [x] **TCP/IP meter service (`serviceMeters`)**: Ethernet `LinkOFF` in the sketch; skip read and `initEthernet()` when the cable is down. Failed port-502 test: LED 2 off, return — do not `reconnectMeter()` (`initEthernet()` `delay(2000)`). `modbus_read_registers_tcpip()` does not call `modbus_test_connection()`. Verified Aug 23 2026; still true in 1.2.4.

## 🔄 Current Capabilities

### Working Systems
- **Hardware Support**: Both RS485 (MAX485) and TCP/IP (W5500) variants operational
- **Meter Communication**: Successfully reads serial numbers, voltage, current, power, and energy
- **Web Interface**: Fully functional dashboard with real-time updates
- **Network Integration**: WiFi connectivity with automatic configuration
- **Data Logging**: Continuous uploads to AmpX Portal with proper formatting

### Tested Functionality
- **Modbus Register Reading**: All register types (int32, int64, float) properly converted
- **WebSocket Performance**: Staggered meter reads plus `handleClient`/`webSocket.loop` around each read (July 2026)
- **Progressive Data Updates**: Meters populate sequentially every 1 second
- **WebSocket Event Handler**: Fully implemented with connection tracking and error handling
- **API Communication**: Successful data transmission to remote servers
- **Fault Recovery**: Automatic reconnection on communication failures
- **Memory Management**: Stable operation within ESP32 memory constraints
- **5-Meter Operation**: Successfully tested with 5 meters reading and displaying real-time data

## 📋 Known Limitations

### Current Constraints
- **Active Meter Limit**: 5 meters actively configured and operational (vs 32 theoretical maximum)
- **Server URLs**: Local/live API and OTA URLs are compile-time (`USE_LOCAL_SERVER`, `ampxportal_server_*`, `firmwareURL`); Gateway ID is Admin + NVS, not hardcoded after first boot
- **Admin Interface**: Gateway reboot + HTTP OTA (Check + Update) + Clear WiFi (ship-mode); Gateway ID change with password
- **Windows mDNS**: .local domain resolution not working on Windows (use IP address as workaround)
- **Live OTA binary**: Live `https://ampx.app/firmware/version.json` is **1.2.4** (29 Sep 2026, `.bin` 1,341,696 bytes). Sketch is **1.2.5** and is not in that catalog yet. Do not put **1.0.9** or an S3 image back at that URL.
- **Live API key**: Live `/api/v3/` gated (401/201). `/api/v2/` still present for Influx 2.

### Technical Debt
- **HTML Templates**: Static meter sections (5 meters) in web files - need dynamic generation for scaling beyond 5
- **String Operations**: Some inefficient string concatenation in API functions
- **TCP `modbus_test_connection()`**: still a TCP open on port 502, not a Modbus PDU. Sep 28 it leaves the socket open for the sweep. Cable check lives in `meterTransportReady()`. RS485 test is still `return true`.
- **Error Recovery**: Basic retry logic, could be enhanced
- **Configuration Management**: Gateway ID via Admin/NVS; WiFi via WiFiManager; API/OTA URLs still compile-time
- **Documentation**: Some TODO comments indicate pending improvements

### Resolved Technical Debt
- ✅ **Global Variables**: Converted from hardcoded `m1_serial_number` variables to `meterSerialNumbers[]` array
- ✅ **Backend Scalability**: All backend functions use dynamic loops with `maxNumberOfMeters` constant
- ✅ **WebSocket Event Handler**: Fully implemented with proper connection tracking
- ✅ **WebSocket Performance**: Optimized with staggered meter reads - <3-second connections achieved (was 12-13 seconds)
- ✅ **Staggered Meter Reading**: Sequential 1-second intervals instead of batched 5-second reads
- ✅ **WebSocket Loop Servicing (July 2026)**: Restored `server.handleClient()` + `webSocket.loop()` before/after `handlePowerMeter()` — fixes multi-minute “Connecting…” when Modbus blocked handshakes
- ✅ **Boot discovery early-exit**: `detectNumberOfMeters()` breaks on first missing Modbus ID (contiguous 1..N)
- ✅ **NTP max wait**: tuned (~15s; exits early on success) after 5s proved too short
- ✅ **Local API E2E (July 2026)**: ESP → `http://{LAN_IP}/api/v2/` → InfluxDB → `ampx-app.local` portal View Data (gateway 100007 verified)
- ✅ **Live API E2E (July 2026)**: ESP → `https://ampx.app/api/v2/` → Influx → live portal Meters + View Data (100007 / SN 2724193004)
- ✅ **Live meters critical error**: Added `AMPX_INFLUXDB_*` to live `wp-config.php`; flushed OPcache; PHP 8.4 CSV/`str_getcsv` harden in portal plugin
- ✅ **HTTP Pull OTA (August 2026)**: Admin `POST /update` via `HTTPUpdate`; NVS last status; local E2E 1.0.1→1.0.2; ArduinoOTA left disabled
- ✅ **Shared API Key (August 2026)**: Local `X-AmpX-Api-Key` / `AMPX_API_KEY`; firmware `ampxportal_api_key` + 1.0.3; Postman/curl 401/201 verified
- ✅ **InfluxDB Cloud Serverless org (August 2026)**: AmpX / Energy Gateway / AWS Frankfurt (`eu-central-1-1.aws.cloud2.influxdata.com`)
- ✅ **API v3 local writes (August 2026)**: Isolated `/api/v3/` → Cloud Serverless; 201 + InfluxQL (`v3_plan_test`); v2 still 201 to `influxdb2.ampx.app`
- ✅ **Portal InfluxQL local reads (August 2026)**: Plugin 1.1.5; local `AMPX_INFLUXDB_*` → Cloud; CLI verified 100001 / SN-1234567890 and 100008 / v3_plan_test
- ✅ **Firmware local `/api/v3/` (August 2026)**: Sketch 1.1.0; USB 100007 Serial 201; local portal Cloud rows for 3423875005
- ✅ **Live Cloud cutover (August 2026)**: Live `/api/v3/` + plugin 1.1.5 + Cloud wp-config; firmware **1.1.1** on 100007; live Meter Data Cloud rows; example 100001 page HTML removed
- ✅ **Portal Meter Data UX (August 2026)**: Theme horizontal scroll (max-height viewport); plugin display 1000/30d with clear copy; server CSV export full 30d (`admin_post`); local verified gateway 100008
- ✅ **OTA reboot-loop fix (August 2026)**: 1.0.3 FreeRTOS HTTPS manifest task → LoadProhibited reboot loop; **1.0.4** manifest check on `loop()` only
- ✅ **Live HTTPS OTA (August 2026)**: 1.0.4/1.0.5 Check → `TG1WDT_SYS_RESET` (stack `WiFiClientSecure`); **1.0.7** static TLS client + 16KB loop stack + Check button + numeric version compare; live OTA on gateway 100008 Aug 18
- ✅ **Ship-mode / Clear WiFi (August 2026)**: Admin `POST /clear_wifi` + `ADMIN_PASSWORD`; `clearStoredWifi()` / `resetSettings()`; LEDs off before reboot (Clear WiFi and Admin Reboot); WiFiManager save-page next-step copy; sketch **1.0.9**
- ✅ **TCP/IP connection test (August 2026)**: `serviceMeters()` + `meterTransportReady()` (`LinkOFF` in sketch, not the AmpX TCP library); hardware verified 23 Aug; do not put `Ethernet.linkStatus()` in `ampx_modbus_tcpip.cpp`
- ✅ **TCP sweep socket + WS between registers (28 Sep 2026)**: one port-502 session per `handlePowerMeter()`; `webSocket.loop()` inside the register loop; per-register `delay(100)` removed. Bench before that pause was removed: Meter Details ~5 s (was up to ~40 s).
- ✅ **Meatrol frequency + power factor in the register map (28 Sep 2026)**: `frequency` **1074** (Mi550 manual FreqTotal; FreqA/B/C are 1068/1070/1072). `power_factor_L1`–`L3` 1052/1054/1056. No `power_factor_tot` (register 1058; API v3 **400**s the whole post). `detectNumberOfMeters()` debug compares `MODBUS_TYPE`.
- ✅ **W5500 socket leak fixed (29 Sep 2026, firmware 1.2.3)**: 1.2.2 called `connect()` while the previous socket was still in `FIN_WAIT`. `EthernetClient::connect()` drops that index without `socketClose()`. After eight leaks, port 502 never opens: local page stays Connected with no readings, and the portal gets no new rows (last row 28 Sep 09:50:52, serial **3425875001**). 1.2.3 calls `stop()` before `connect()`, keeps a good session open, and breaks the register loop when the socket is already closed. USB-flashed on **100007**. User confirmed readings are back. Frequency register stays **1074**. Do not republish **1.2.2**.
- ✅ **LAN /debug page (29 Sep 2026)**: `debug()` / `debugln()` copy into a 4 KB RAM buffer and still print on USB 115200. `GET /debug` reloads every 2 seconds. Admin footer links to it; the sidebar does not. Direct `Serial.println` is USB only. No FAT log file. Verified on gateway **100009**: Modbus TCP test OK, portal post HTTP **201** / `api_version` 3.0. Shipped in catalog **1.2.4**.
- ✅ **Live OTA catalog 1.2.4 (29 Sep 2026)**: `public_html/firmware/version.json` + `.bin` (**1,341,696** bytes, ASCII **1.2.4**). Uploaded with `D:\xampp\htdocs\firmware\publish-firmware.ps1` (explicit FTP over TLS). Cloudflare `cf-cache-status` **DYNAMIC**. Earlier the same day the catalog was **1.2.3** (1,335,392 bytes). On 28 Sep it was **1.2.1** (1,335,296 bytes), and a cached **1.0.9** image was flashed while the status line said 1.2.1. **100007** apply of 1.2.4 was not confirmed after the publish.

### Remaining Performance / Ops Notes
- Page copy says “a few seconds”; values still arrive together when `handleWebSocket()` runs after the sweep
- Update `ampxportal_server_local` when the PC LAN IP changes
- Ensure Windows Wi‑Fi profile Private + Apache firewall allow so ESP can reach PC:80
- Live portal: after editing `wp-config.php` on Hetzner, flush PHP OPcache (constants can look “missing” until then)
- Live debug: Debug Log Manager path in `WP_DEBUG_LOG`, not `wp-content/debug.log`
- Do not reintroduce FreeRTOS OTA / `WiFiClientSecure` side tasks, cross-core Arduino `String` caches, or a **local** `WiFiClientSecure` on `loop()`
- Do not publish OTA **1.0.9** (v2) or an S3 image at `ampx_open_energy_gateway.bin`. Live catalog is single-url **1.2.4** because `fetchFirmwareManifest()` does not read `targets`. Chip-aware parse is still the plan: `memory-bank/plans/2026-08-23-multi-chip-ota.md`. Publish with `publish-firmware.ps1` (explicit FTP TLS). Plain FTP returns 550.
- Purge or bypass Cloudflare for `/firmware/` on every publish. Admin “OK: update applied (X)” is the manifest version, not the booted image.
- 100007 (TCP/IP, ME537 serial **3425875001**) showed real volts, 50 Hz, and power factor on 28 Sep. Uploads stopped at 09:50:52 on firmware 1.2.2 (W5500 socket leak) and resumed after USB **1.2.3** on 29 Sep.

## 🎯 Immediate Development Opportunities

### High Priority
1. **Sketch is 1.2.5** (reset button, tested 5 Oct 2026). Live catalog is still **1.2.4**. Publish 1.2.5 only when asked. Chip-aware `targets` is still unshipped. Do not publish 1.0.9, 1.2.2, or an S3 `.bin` at the legacy URL.
2. **Migrate remaining live gateways** off `/api/v2/` (USB 1.2.0 after chip-aware firmware, not OTA 1.0.9)
3. **Deploy theme 1.0.9** (meter-data table scroll)
4. **Sunset v2 / Influx 2** after no gateway posts `/api/v2/`
5. **Scale to 10 Meters**: Add HTML sections for meters 6-10 (backend already supports this)
6. **Windows mDNS Resolution**: Troubleshoot and fix .local domain access on Windows

### Medium Priority
1. **Additional Meter Support**: Extend beyond Meatrol to other manufacturers
2. **Data Persistence**: Local data logging capabilities
4. **Alert System**: Email/SMS notifications for fault conditions

### Future Enhancements
1. **Historical Data**: Database integration for trend analysis
2. **Advanced Dashboard**: Charts and visualization capabilities
3. **Mobile App**: Native mobile application for remote monitoring
4. **Protocol Extensions**: Support for additional industrial protocols

## 🔧 System Health Assessment

### Strengths
- **Production Ready**: Stable codebase suitable for industrial deployment
- **Comprehensive**: Full feature set from hardware to cloud integration
- **Maintainable**: Clean modular architecture supports ongoing development
- **Documented**: Extensive inline documentation and comprehensive README
- **Power Supply Validated**: Resolved ESP32 reset issues, confirmed stable operation

### Performance Metrics
- **Update Frequency**: 3-second real-time updates
- **API Transmission**: 30-second intervals to remote server
- **Uptime Target**: 24-hour operation cycles with automatic recovery
- **Memory Usage**: Optimized for ESP32 platform constraints

### Deployment Status
- **Development**: Fully functional development environment
- **Testing**: Suitable for industrial testing and validation
- **Production**: Ready for commercial deployment with current feature set
- **Scaling**: Architecture supports horizontal scaling across multiple sites

## 📈 Success Metrics

### Achieved Targets
- ✅ Real-time meter communication
- ✅ Web interface functionality
- ✅ API integration working
- ✅ Multi-protocol support
- ✅ Automatic network configuration
- ✅ Industrial-grade reliability patterns

### Next Milestones
- 🎯 **Done 5 Oct:** Sketch **1.2.5** reset button, USB-flashed and tested. Live catalog is still the 29 Sep **1.2.4** image (1,341,696 bytes). Publishing 1.2.5 was not done.
- 🎯 **Done 29 Sep:** Live catalog **1.2.4**. **100007** was USB **1.2.3**; installing 1.2.4 was not confirmed.
- 🎯 **Later:** read meter model from Modbus register **60** (UTF8, 10 words). Do not post it in API `values`. Portal type is still hardcoded `ME537`.
- 🎯 Chip-aware `targets` parse for the next catalog
- 🎯 Sunset Influx 2 / `/api/v2/`
- 🎯 32-meter capacity utilization
- 🎯 Enhanced fault tolerance
- 🎯 Extended meter manufacturer support
