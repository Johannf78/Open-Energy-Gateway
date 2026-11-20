# Active Context - AmpX Open Energy Gateway

## Current Focus
**Nov 19, 2026 — Firmware 1.3.0: meter manufacturer + model in the portal post. Compiled, not flashed, not published.**

The portal now identifies a meter by manufacturer + model + serial (portal ADR `AmpX-Portal/docs/adr/0002-meter-is-first-class.md`). Sketch `FIRMWARE_VERSION` is **"1.3.0"**.
- `meter_registers.h`: `METER_MANUFACTURER "Meatrol"`, `METER_MODEL_REGISTER 60`, `METER_MODEL_WORDS 10` (UTF-8, two characters per word, high byte first)
- `functions_meter.ino`: `readMeterRegisters()` (keeps the `#if MODBUS_TYPE` split inside), `decodeModbusString()` (stops at NUL; drops non-printable, `"` and `\`), `readMeterModelOnce()` — at most 3 attempts per meter, a blank answer stops retries. Called in `handlePowerMeter()` after the serial line, with `handleClient()` / `webSocket.loop()` first. Result in `meterModels[]` and `JsonDoc["m<N>_model"]`
- `functions_api.ino`: top-level `"manufacturer"` always, `"meter_model"` when known. The values loop skips keys ending `_model` (the API would 400 the whole post)
- `web_meters.h`: Model line on the meter details page
- Compiles: TCP **1,343,473** bytes, RS485 **1,325,781** bytes. **Not flashed.** Next: USB-flash 100007 (TCP) and an RS485 unit, check Serial and the portal 201 has `"identity_defaulted": false`
- The live API defaults old firmware to Meatrol / ME537, so 1.2.x keeps working. Not a breaking change; still a minor bump because the payload grew. Do not publish OTA until asked
- **20 Nov 2026:** live `/api/v3/` accepts the 1.3.0 payload (test post 201, `identity_defaulted: false`). Flash deferred by user
- **Security open item (20 Nov 2026):** `ampxportal_api_key` in `open_energy_gateway.ino` is the live gateway key, and this repo is **public** on GitHub (since commit `3e5aa2b`, Aug 2026). Fix later = move the key to a git-ignored header, rotate the key on the API, reflash every gateway in the same change (bump `FIRMWARE_VERSION`). Not started

**Oct 13, 2026 — TestLEDs button does nothing. Next session: continuity, not more firmware.**

`src/TestLEDs/TestLEDs.ino` holds all five LEDs on while GPIO 13 is high, and turns them off while GPIO 13 **or** DevKit BOOT (GPIO 0) is low. Serial 115200 prints `GPIO13=` on change and a 1 s heartbeat. User flashed it. Front-panel press still did nothing (no Serial result reported).

Schematic (bottom PCB): J3 2×4 pin 1 = 3.3 V, 2 POWER, 3 METER, 4 WIFI, 5 INTERNET, 6 SERVER, **7 BUTTON**, **8 GND**. BUTTON is DevKit pin 15 **IO13**, with R1 10 kΩ to 3.3 V and C1 100 nF to GND. LEDs are IO25 SERVER, IO26 WIFI, IO27 INTERNET, IO12 POWER, IO14 METER.

PCB screenshots (EasyEDA, silkscreen **AmpX Energy Gateway V 1.2**, one also shows **Ver 1.0**): J3 pin 1 triangle is top-left. LED stubs from the consecutive GPIO block look wired. **IO13 is on the far side of GND from IO12**, and R1/C1 sit by J3, not by IO13. The screenshot is one copper layer, so a bottom-layer track could exist. **Not confirmed.** Likely open between IO13 and J3 pin 7 — that matches “LEDs on, button does nothing.”

**Start tomorrow:** power off, continuity from DevKit **IO13** (top row, 5th pad from the USB end) to **J3 pin 7** (bottom-left of the 8 pads, opposite the triangle). Open → PCB fault; add a wire IO13 → J3-7 / R1 BUTTON pad, or fix the layout. Beep → then DC volts, black on GND: J3-7 should be ~3.3 V at rest and ~0 V while the pinhole is held. J3-7 to J3-8 should beep only while pressed. Do not change production firmware for this. `TestLEDs.ino` is uncommitted diagnostic code.

**Oct 5, 2026 — Front-panel WiFi reset button, firmware 1.2.5, tested.** Sketch `FIRMWARE_VERSION` is **"1.2.5"**. USB-flashed and the user confirmed it behaved correctly. Live catalog is still **1.2.4** (do not assume this image is published). Admin open earlier the same day was **100009** on 1.2.4; the flashed unit's ID was not restated after the test.

Button is **GPIO 13**, `INPUT_PULLUP`, switch to GND (10 kΩ and 100 nF already on the board). The same pin numbers are now in both the classic ESP32 and ESP32-S3 maps (LEDs 12/14/27/26/25, SPI 18/19/23/5, RS485 4/16/17, button 13). `TestS3Bringup.ino` still has the old S3 pins (LEDs 4/5/6/7/15, SPI 12/13/11/10). GPIO 23 and 25 do not exist on the S3 die; GPIO 26 and 27 are inside the S3 module flash bus. A DevKitC-1U cannot be wired to this shared map.

Hold behavior (`serviceWifiResetButton()` in `functions_wifi.ino`):
- Sampled from the saved-WiFi wait in `initWiFi()`, from the bottom of `loop()`, before each register in `handlePowerMeter()`, and from `serviceNetworkWhileModbusWaits()`.
- Ignore the first 2 s. Stay disarmed until the pin has been seen high (a ribbon stuck low must not wipe WiFi every boot).
- Pin must stay low for **3 seconds**. A shorter press does nothing.
- Then flash all five LEDs twice (off, on, off, on, ~200 ms), because they are already on in normal running. Then `clearWifiAndRestart()`: `clearStoredWifi()`, LEDs off, `ESP.restart()`.
- Admin **Clear WiFi** sends its page first, then calls the same `clearWifiAndRestart()`. Gateway ID, meter names, and OTA NVS stay. Next boot opens `energy-gateway-{ID}` because nothing is saved.

WiFi when the saved network is missing (`initWiFi()`):
- `getWiFiIsSaved()` false (first boot, Admin Clear WiFi, or the button) → `autoConnect()` opens the hotspot.
- Saved network present → `WiFi.mode(WIFI_STA)` + `WiFi.begin()` and wait until `WL_CONNECTED`. Do **not** open the hotspot on a failed try. The old `autoConnect()` failure path is commented out.
- The button is polled every 200 ms inside that wait, because `setup()` has not reached `loop()` yet.

Not done: blink the WiFi LED while in AP mode. 24 h reboot in `loop()` is still commented out. Publish **1.2.5** only when asked (`publish-firmware.ps1`, explicit FTP TLS). Do not publish 1.0.9, 1.2.2, or an S3 image at the legacy `.bin` URL.

**Oct 5, 2026 — earlier the same day, before the button.** Meter path still matches the Aug 23 review. `modbus_read_registers_tcpip()` does not call `modbus_test_connection()`. Failed TCP test in `serviceMeters()` turns LED 2 off and returns. It does not call `reconnectMeter()` / `initEthernet()`. Confirming **100007** installed catalog 1.2.4 was not done.

**Sep 29, 2026 — Live catalog is 1.2.4.** Sketch `FIRMWARE_VERSION` is **"1.2.4"**. `https://ampx.app/firmware/version.json` is version **1.2.4** with url `https://ampx.app/firmware/ampx_open_energy_gateway.bin`. The `.bin` is **1,341,696** bytes (Last-Modified 29 Sep 2026 08:55:05 GMT). Cloudflare `cf-cache-status` was **DYNAMIC**. Publish with `D:\xampp\htdocs\firmware\publish-firmware.ps1` after Arduino IDE **Sketch → Export compiled Binary** (ESP32 Dev Module, custom `partitions.csv`, 8 MB). The script copies the app `.bin` (not merged, bootloader, or partitions), writes `version.json` (UTF-8, no BOM), and uploads both with **explicit FTP over TLS** (`curl --ssl-reqd --ftp-pasv`) to `public_html/firmware/` on `dedivirt3789.your-server.de` as `ampxapp`. The password is `$env:AMPX_FTP_PASS` in that shell only. Plain FTP logs in and then returns **550**. `deploy-firmware.ps1` is that plain-FTP path. A gateway still on **1.2.3** can use Admin → Check for update. Whether **100007** has installed 1.2.4 was not confirmed after this publish. Admin “OK: update applied (X)” is the manifest version. Trust **Current firmware version** after reboot.

**The same image has the LAN debug page.** The Admin footer links to `GET /debug`. The sidebar does not. The page shows the last 4 KB of `debug()` / `debugln()` and reloads every 2 seconds. USB serial at 115200 still receives those lines. Direct `Serial.println` stays on USB only. `DEBUG 0` empties the macros, so the page stays blank. No file on the FAT partition. The first line can be cut off after the 4 KB buffer wraps. The page was verified earlier the same day on **100009** (`http://energy-gateway-100009.local/debug`): Modbus TCP connection test succeeded, and `postToAmpXPortal2` logged HTTP **201**, `api_version` **3.0**, serial **3423875005**. That check was a USB flash before the 1.2.4 catalog publish.

**Sep 29, 2026 — Gateway web UI stays Connected with no readings.** Confirmed on 100007 (`192.168.2.163`): WebSocket port 81 stays open and sends nothing for 50 s, while HTTP sometimes stalls ~1 s. That stall is `EthernetClient::connect()` timing out on meter port 502. `connected()` is false in `FIN_WAIT`, and `connect()` then drops the W5500 socket index without `socketClose()`. After eight abandoned sockets the chip cannot open 502, so `serviceMeters()` never reaches `handleWebSocket()`.

**Fix flashed and verified 29 Sep 2026 (sketch 1.2.3, USB, gateway 100007):** `modbus_test_connection()` and `modbus_send_request()` call `stop()` before `connect()`. A good sweep no longer `stop()`s. A failed TCP read breaks the register loop instead of reconnecting once per remaining register. `frequency` stays **1074** (Mi550 manual: FreqTotal). Do not use 1024. User confirmed readings are back. That morning the catalog was single-url **1.2.3** (`version.json` Last-Modified 07:31:49 GMT; `.bin` 1,335,392 bytes) and **100007** Admin showed Up to date. The catalog is now **1.2.4**. `targets` is still not parsed.

**Sep 29 morning — catalog was single-url 1.2.3** (bin 1,335,392 bytes, Last-Modified 07:31:49 GMT). It was replaced the same day by **1.2.4**. `fetchFirmwareManifest()` still does not read `targets`. The 28 Sep catalog was **1.2.1** (bin 1,335,296 bytes). A cached **1.0.9** image was flashed that morning while the status line said 1.2.1.

**Not confirmed after that rule:** Admin on gateway **100007** still needs a Check + Update so **Current firmware version** reads **1.2.1**. Two earlier OTAs the same morning reported `OK: update applied (1.2.1)` and then booted **1.0.9**. That status line appends the **manifest** version (`saveOtaStatus` in `doOTAUpdate()`), not the string compiled into the image. Cloudflare was still serving the old `.bin`. Do not treat the field unit as 1.2.1 until Admin says so.

**100007** is Modbus **TCP/IP** (W5500), not RS485. ME537 serial **3423875005**. On 26 Sep, with voltage and current coils connected, gateway and live portal agreed (about 237–240 V; L2 idle; L1/L3 power and imported energy added up). `detectNumberOfMeters()` used to print RS485 always (`if (MODBUS_TYPE_RS485)` is `if (1)`). It now compares `MODBUS_TYPE == MODBUS_TYPE_RS485`.

**Register map** in `meter_registers.h` (this is what `setupMeterRegisters()` loads; `data/meter_registers_meatrol.json` is a copy):

| Key | Address | Words | Type |
|-----|---------|-------|------|
| `frequency` | 1074 | 2 | float |
| `power_factor_L1` | 1052 | 2 | float |
| `power_factor_L2` | 1054 | 2 | float |
| `power_factor_L3` | 1056 | 2 | float |

Do **not** send `power_factor_tot` (register 1058). API v3 allows only `mN_power_factor_L[1-3]`. One invalid key returns **400** for the whole post. The portal has no total power-factor column. `postToAmpXPortal2()` already forwards every `mN_` key. Local `/meters` shows frequency and power factor. The unit `if` in `web_meters.h` checks `power_factor` before `power` (otherwise the key is labelled kW).

**Later — meter model from Modbus (do not build in the 1.2.3 flash).** Mi550 equipment parameters: meter model is register **60**, **10** words, **UTF8**. Serial number **70** (already read) is the next field. The sketch only decodes int32, int64, and float, so UTF8 needs a new type. Do not put that string in the API `values` object: v3 accepts only numeric keys, and an unknown key returns **400** for the whole post. Portal **Type: ME537** is hardcoded in `AMPX_Portal_InfluxDB_Detailed::parse_meters_response()`, not read from the gateway.

**Sep 28–29, 2026 — TCP socket.** Meter Details went from ~40 s to about 5 s by keeping one port-502 session open and servicing the WebSocket between registers. `delay(100)` per register is gone.

**29 Sep — 1.2.2 stopped all readings.** `connected()` is false while the W5500 socket is still in `FIN_WAIT`. `EthernetClient::connect()` then drops that socket index without `socketClose()`. After eight abandoned sockets the chip cannot open 502. The browser WebSocket stays Connected and sends nothing, and the portal gets no new rows (last good row 28 Sep 09:50:52, serial **3425875001**, 50 Hz from register **1074**). **1.2.3** calls `stop()` before every `connect()`, does not `stop()` after a good sweep, and breaks the register loop when the socket is already closed. USB-flashed on **100007**. User confirmed readings are back. The catalog was still **1.2.1** at that moment. Later the same day it was published as **1.2.3** (bin 1,335,392 bytes), then **1.2.4**.

**Library** `ampx_modbus_tcpip.cpp` (Arduino libraries, not the sketch tree):
- `modbus_test_connection()` / `modbus_send_request()`: if not connected, `stop()` then `connect()`. `stop()` frees a `FIN_WAIT` socket. A live socket is reused.
- A short `write()` (`!= 12`) calls `stop()` and returns false.
- `modbus_read_response()` does **not** `stop()` on a full reply. It closes on a short frame (`bytesRead < 9`) or a header without the register bytes. While waiting (up to 2 s) it calls `modbusIdleCallback` → `serviceNetworkWhileModbusWaits()` (`handleClient` + `webSocket.loop()`).
- No `delay(100)` before the read.

**Sketch** `functions_meter.ino`:
- Inside the register loop, before each read: `server.handleClient()` + `webSocket.loop()`. Do not call `handleWebSocket()` there; the JSON is still broadcast once after the sweep.
- On a failed TCP read, if `!modbusClient.connected()`, **break**. Do not reconnect once per remaining register.
- Do **not** `modbusClient.stop()` at the end of a good `handlePowerMeter()`.
- `reconnectMeter()` stays **unhooked**. `initEthernet()` has `delay(2000)` and must not run on the 1 s path.

UI copy in `web_home.h`, `web_meters.h`, and `web_settings.h` now says the initial connection usually takes a few seconds (was “up to 30 seconds”).

**Sep 6, 2026 — ESP32-S3 bench port: WORKING on a stock DevKitC-1U.** Both sketches compile, flash and run; W5500 detected; WiFi AP up. Only the meter link is untested.

### Next session — start here
1. Set the meter's Ethernet to `192.168.1.55` (gateway `192.168.1.1`, mask `255.255.255.0`, Modbus-TCP enabled, port 502) so it matches the firmware defaults — the User Manual V1.0 still documents the old `192.168.2.122` pair, so **either the meter or the manual needs changing**; decide which and make them agree.
2. LAN cable W5500 ↔ meter. Expect `Link is ON`, Modbus test pass, METER LED on.
3. Then: WiFi provisioning via the AP, API post to `ampx.app/api/v3/`, and finally OTA with `CHIP_OTA_KEY = esp32s3`.
4. Consider soldering headers for the four SPI lines — dupont on breadboard caused two false "no hardware" failures today.

**Sep 11, 2026 — hardware V3 reset button. Firmware written and tested Oct 5, 2026 (1.2.5).** See Current Focus. Short press does nothing. WiFi-LED blink in AP mode was not built.
- **Reset button on GPIO 13**, front panel, recessed pinhole. Hold 3 s clears saved WiFi and restarts into the setup hotspot.
- GPIO 13 chosen because WROVER-IE has no GPIO 16/17; 5/18/19/23 are the W5500; 12/14/25/26/27 the LEDs; 0/2/15 are strapping (GPIO 0 low at boot = bootloader mode); 21/22 kept for I²C; 32/33 kept for the RS485 variant.
- Hardware: 10 kΩ pull-up + 100 nF on the **main** board, switch to GND on the front board, `INPUT_PULLUP` also set in firmware so the pin is defined when the ribbon is unplugged.
- **Done in 1.2.5:** 3 s hold, double flash of all five LEDs, stuck-button guard (ignore 2 s, arm only after the pin is seen high). **Not done:** blink the WiFi LED in AP mode.
- Also noted: the 24 h auto-reboot in `loop()` is still commented out (`//ESP.restart();`).
- Front/back boards now joined by one 2×4 IDC ribbon carrying 5 LEDs + button + V + GND. Base board is USB-powered; the front status board is fed from it.
- Docs: `AmpX Products\AmpX Energy Gateway\AmpX Energy Gateway - Hardware Notes and connections.md` and `...- Prototype Build 5 - Procurement Status.md`.

### Detail
- `open_energy_gateway.ino`: new **Board pin map** block (`#if CONFIG_IDF_TARGET_ESP32S3 … #else` classic ESP32). ESP32 map unchanged; LED/ETH/RS485 defines moved out of the `MODBUS_TYPE` blocks into it.
- S3 map: LEDs 4/5/6/7/15 · W5500 SCK 12, MISO 13, MOSI 11, CS 10, RST 14 (optional) · RS485 DE 16, RX 18, TX 17 (reserved).
- `functions_ethernet.ino`: `SPI.begin(SCK, MISO, MOSI, -1)` explicit before `Ethernet.init()`; optional `ETH_SPI_RST_PIN` pulse.
- New `src/TestS3Bringup/TestS3Bringup.ino` — flash first: chip/PSRAM report, LED walk, W5500 detect, link + port 502.
- IDE: ESP32S3 Dev Module, CDC on boot **Disabled** (UART socket), OPI PSRAM, flash size per module, custom `partitions.csv`.
- **Bench result 6 Sep:** `TestS3Bringup` compiled and ran on an ESP32-S3-DevKitC-1U **N8R8** (8 MB QIO flash, 8 MB OPI PSRAM, core 3.3.x / IDF 5.5.5). LEDs 4/5/6/7/15 OK, **W5500 detected** on FSPI 12/13/11/10, IP 192.168.1.50. First failure was the four SPI wires one header pin too high (CS on 9) — count from bottom GND: 13 orange, 12 yellow, 11 blue, 10 green.
- **Main firmware runs on S3 (6 Sep):** `open_energy_gateway.ino` compiled and ran first time on the S3 — `ampx_modbus_tcpip` built without changes, NVS + gateway ID OK, **W5500 detected**, WiFiManager AP `energy-gateway-100001` at 192.168.4.1. Only `Link is OFF` (no LAN cable yet).
- Two intermittent "No Ethernet hardware detected" failures during bring-up were both **loose dupont contacts** on the breadboard, not firmware. If it ever recurs: power-cycle fully (W5500 keeps state across an ESP32 reset), then re-flash `TestS3Bringup` to split hardware from firmware.
- `ETH_SPI_RST_PIN` is now commented out in the pin map — defining it pulses a pin that is not wired. Uncomment only if RST is physically connected.
- **Still unverified on S3:** meter read over Modbus TCP; API post; OTA. `CHIP_OTA_KEY` already yields `esp32s3` — never publish an S3 .bin at the single legacy URL.
- Hardware gap: V2 bottom PCB footprint is DevKitC-32U (2×19); S3-DevKitC-1U is 2×22 → new footprint + bottom board respin before enclosure fit. **Open decision:** does production go on a DevKitC-1U carrier or the custom S3 board (LCSC parts bought Dec 2025)? That choice, plus "freeze the ESP32 fleet at date X vs. maintain two families", is worth settling before the respin — it is an exit/sellability question, not just an engineering one.
- Mirror of the pin table: `AmpX Products\AmpX Energy Gateway\AmpX Energy Gateway - Hardware Notes and connections.md`.

**Aug 23, 2026:** Multi-chip OTA **designed, not shipped**. `fetchFirmwareManifest()` still reads only top-level `version` and `url`. **Sep 28:** live catalog was published as single-url **1.2.1** anyway (that is what this firmware understands). Chip-aware `targets.esp32` is still future work. Never put an S3 `.bin` at `firmware/ampx_open_energy_gateway.bin`. Do **not** put **1.0.9** back at that URL (it posts `/api/v2/`).

Plan (pick up later): `memory-bank/plans/2026-08-23-multi-chip-ota.md`  
Spec: `ampx.app/docs/superpowers/specs/2026-08-23-multi-chip-ota-design.md`

Sketch `FIRMWARE_VERSION` is **"1.2.4"** (29 Sep 2026). `CHIP_OTA_KEY` is set. `fetchFirmwareManifest()` still uses top-level `version`/`url` and `firmwareURL` fallback. `setup()` prints the version on the serial monitor after the port is ready. Live `version.json` is **1.2.4** (published 29 Sep 2026, single url). Chip-aware `targets` is still not parsed.

Also today: TCP/IP meter connection test verified. Cable/link is in `functions_meter.ino` (`meterTransportReady` / `Ethernet.linkStatus() == LinkOFF`), not in `ampx_modbus_tcpip`. `loop()` calls `serviceMeters()`.

**Ready for next session:**
- **100007** was USB **1.2.3** on the morning of 29 Sep (Admin Up to date against the 1.2.3 catalog). Last OTA result in NVS was still **OK: update applied (1.2.1)** at **2026-09-28T07:24:06Z**. A USB flash does not change that line. The live catalog is now **1.2.4**. Applying it on 100007 was not confirmed after the publish.
- Live catalog is single-url **1.2.4**. `targets` is still not parsed. Do not put an S3 image at the legacy `.bin` URL. Do not publish **1.0.9**.
- Portal Meter Data for serial **3425875001** already showed 50 Hz and per-phase power factor on 28 Sep. Headings still say **W** / **Wh** while the gateway sends **kW** / **kWh**.
- Chip-aware OTA (`targets`) is still not parsed.
- Optional: Modbus probe (registers 70–71) in TCP `modbus_test_connection()`; RS485 test is still `return true`.

Previous: multi-chip OTA spec (23 Aug); TCP/IP connection test; live Cloud cutover (1.1.1 USB); firmware local v3; portal InfluxQL; API v3.

## Recent Major Achievements

### TCP/IP meter connection test (August 2026) — Session 19
**Symptom:** TCP `modbus_test_connection()` looked always-positive. It is **not** the RS485 stub (`return true`). As of this session it was `EthernetClient.connect(meter_ip, 502)` then `stop()` — a TCP open, not a Modbus PDU. **Sep 28:** the `stop()` was removed so the sweep can reuse the socket (see top). Putting `Ethernet.linkStatus()` in `ampx_modbus_tcpip.cpp` failed in the IDE (`LinkOFF` / `Ethernet` undeclared) because that `.cpp` is parsed without Arduino Ethernet includes.

**Fix (sketch, verified 23 Aug 2026):**
- `meterTransportReady()` — TCP/IP only: `Ethernet.linkStatus() == LinkOFF` → false; RS485 always true
- `serviceMeters()` — early returns: no link → LED 2 off, skip Modbus and `initEthernet()`; test fail → LED 2 off, **return** (do not call `reconnectMeter()` / `initEthernet()`); success → LED 2 on, `readNextMeter()`. Same-day follow-up: `modbus_read_registers_tcpip()` no longer calls `modbus_test_connection()`.
- `loop()` interval block only calls `serviceMeters()`
- Do **not** name it `handleMeters()` — that is the `/meters` HTTP handler

**Not done:** Modbus register probe in the TCP library; RS485 `modbus_test_connection()` stub.

### Live Cloud / API v3 cutover (August 2026) — Session 18
**Objective:** Point live Hetzner + firmware + portal at Cloud Serverless. Keep `/api/v2/` until the fleet is on 1.1.1.

**Deployed:**
- Live `wp-config.php` `AMPX_INFLUXDB_*` → Cloud host / Cloud token / org **Energy Gateway** / bucket `energy_metrics` (not account name AmpX, not Influx 2 org `ampx`)
- Plugin **1.1.5** InfluxQL; flush OPcache after wp-config
- Live `/api/v3/` (GET **405** `api_version` 3.0; no key **401**; keyed POST **201**; test serial `live_verify_20260822` landed in Cloud)
- Sketch **1.1.1**: both URLs `/api/v3/`; `USE_LOCAL_SERVER false`
- Meter Data page: deleted Kadence **Example Meter Info** (100001 / Test / 2724193001)

**Verified live UI (admin, 22 Aug):** 100007 / 3423875005 — 377 then 395 readings, newest ~14:17 UTC, oldest 10:52 UTC (first Cloud point). Voltages mostly 0 (meter read, not API).

**Not done:** OTA publish 1.1.1; theme 1.0.9; sunset v2; other field units still on v2 are invisible on live portal.

**Org naming:** Cloud **account** = AmpX; Cloud **organization** = Energy Gateway (ID `86a141bfd8d7f66a`). One-org quota — do not create a second org.

### Portal InfluxQL / Cloud reads (August 2026)
**Objective:** Meters / View Data / CSV read Cloud Serverless instead of Flux on `influxdb2.ampx.app`.

**Implementation:**
- `AMPX_Portal_InfluxDB_Detailed` uses `GET /query?db=energy_metrics` (InfluxQL) and parses wide JSON rows
- `wp-config.php` `AMPX_INFLUXDB_*` → Cloud host / `INFLUXDB_CLOUD_TOKEN` / org Energy Gateway (local + live)
- Plugin **1.1.5**

**Verified CLI (22 Aug 2026, local):**
- `100001` / `SN-1234567890`: 2 readings
- `100008` / `v3_plan_test`: 1 reading

**Verified live UI:** 100007 / 3423875005 Cloud table (Session 18)

### API v3 Cloud Serverless writes (August 2026)
**Objective:** Isolated `/api/v3/` clone of v2 that writes `meter_readings_detailed` to Cloud Serverless. Keep v2 on `influxdb2.ampx.app`.

**Implementation:**
- `api/v3/index.php`, `src/DataValidator.php`, `src/InfluxDBHandlerDetailed.php` (`AmpX\ApiV3`)
- Same JSON + `X-AmpX-Api-Key`; `api_version` **3.0**
- Write URL uses `INFLUXDB_CLOUD_*` with **org ID**
- Plan: `ampx.app/docs/superpowers/plans/2026-08-22-api-v3-cloud-serverless.md`

**Verified local + live (22 Aug 2026):** GET **405**; no/wrong key **401**; valid POST **201** `"api_version":"3.0"`

### Firmware `/api/v3/` (August 2026)
**1.1.0** local USB; **1.1.1** live USB on **100007**. Plan: `ampx.app/docs/superpowers/plans/2026-08-22-firmware-api-v3.md` (local slice) and `2026-08-22-live-cloud-cutover.md`.

**Compile note (ESP32 3.3.11 Windows):** stub `OpenThread/bits/align.h` → `#include_next`; merge-bin `--pad-to-size 8MB` can `MemoryError` (app `.bin` still built).


### Ship-mode / Clear WiFi (August 2026) — firmware 1.0.9
**Objective:** Before boxing a unit, erase workshop WiFi so first boot at the customer site starts WiFiManager AP mode (open AP `energy-gateway-{GATEWAY_ID}`).

**Implementation:**
- Admin card **Clear WiFi (Ship Mode)** → `POST /clear_wifi` with shared `ADMIN_PASSWORD` (same secret as Gateway ID)
- `clearStoredWifi()` calls `WiFiManager.resetSettings()` — do **not** call this from `initWiFi()` (that would wipe every boot)
- Erases WiFi credentials only; Gateway ID, meter names, and OTA NVS stay
- Status page uses ASCII hyphen (`WiFi Cleared - Rebooting`); standalone HTML has no `charset`, so a UTF-8 em dash showed as `â€"`
- All five status LEDs `LOW` before `ESP.restart()` on Clear WiFi **and** Admin Reboot (`handleRebootGateway`)
- WiFiManager `/wifisave` copy via `setCustomHeadElement()`: next steps are `http://energy-gateway-{ID}.local` or the IP from the customer router; `setCustomHeadElement` stores a pointer — keep the `String` alive until `autoConnect()` returns

**Verified:** Gateway **100007** — wrong password 403; correct password reboots into AP; save page at `192.168.4.1/wifisave`.

**Ops:** Live OTA still **1.0.7** until 1.0.9 `.bin` + `version.json` are deployed together.

### Live HTTPS OTA + WDT fix (August 2026) — firmware 1.0.7
**Symptoms (on 1.0.4/1.0.5):** Admin stuck on **Checking…**; opening Admin or **Check for update** rebooted. Serial: `Fetching firmware manifest: https://ampx.app/firmware/version.json` then `rst:0x8 (TG1WDT_SYS_RESET)` (~3s), no Guru Meditation.

**Root cause:** Local `WiFiClientSecure` on the Arduino `loop()` stack (default **8KB**) during TLS handshake. IWDT stage-2 hard-reset when the panic handler could not run. Opening Admin also auto-called `requestOtaManifestCheck()`, so merely loading `/admin` started HTTPS. Version compare was string equality, so device **1.0.6** vs server **1.0.5** showed “Update available”.

**Fix (1.0.6/1.0.7):**
- `SET_LOOP_TASK_STACK_SIZE(16384)` in `open_energy_gateway.ino`
- Static `otaTlsClient` / `otaPlainClient` in `functions_ota.ino` (BSS, not stack)
- Admin: **Check for update** → `GET /ota_status?refresh=1`; page load does **not** start TLS; no 5‑minute auto-refresh
- `isNewerVersion()` numeric `major.minor.patch`; Update Firmware only if server **>** device (`doOTAUpdate` / `handleUpdate` same guard)

**Verified Aug 18, 2026** (gateway **100008**):
- Check: `Manifest HTTP 200` / `OTA manifest OK: 1.0.5` without reboot
- Live OTA install: **OK: update applied (1.0.7)**; after Check, Current = Available **1.0.7**, status **Up to date**, Update disabled
- Live files: `https://ampx.app/firmware/version.json` + `.bin`

Do **not** ship **1.0.3** (reboot loop), **1.0.4/1.0.5** (Admin/Check WDT), or put `WiFiClientSecure` as a local in `loop()`. **1.0.7** was the live HTTPS OTA baseline; current sketch is **1.0.9** (ship-mode).

### OTA FreeRTOS reboot-loop fix (August 2026) — firmware 1.0.4
**Symptom**: After flashing **1.0.3**, gateway reboot-looped a few seconds after boot. Serial showed `EXCCAUSE: 0x02` (LoadProhibited), `Backtrace: … |<-CORRUPTED`. Gateway ID (e.g. 100008) loaded from NVS, then panic.

**Root cause**: 1.0.3 ran `otaManifestTask` on a FreeRTOS side task (core 0) that immediately fetched the **HTTPS** manifest (`USE_LOCAL_SERVER false` → `https://ampx.app/firmware/version.json`). Unsafe combo: `WiFiClientSecure` / `HTTPClient` off the Arduino loop task + Arduino `String` fields in `otaStatusCache` shared with WebServer handlers on core 1 → heap/stack corruption.

**Fix (1.0.4)**:
- Removed `xTaskCreatePinnedToCore(otaManifestTask, …)`
- `serviceOtaManifestCheck()` runs only from `loop()` (same core as WebServer)
- No boot-time auto-fetch: `otaManifestCheckRequested` starts `false`; first check only when Admin opens (`requestOtaManifestCheck`); 5‑minute refresh only after a prior check
- Handlers still must not call outbound `HTTPClient` (cache + `/ota_status` poll only)

**Verified**: USB flash 1.0.4; stable run past WiFi/meters (Aug 17, 2026). Do **not** put `WiFiClientSecure` or shared Arduino `String` caches on a FreeRTOS side task again.

### InfluxDB Cloud Serverless account (August 2026)
**Decision**: Use **managed Cloud Serverless** (not self-hosted Influx on Hetzner). Path later: Serverless → Cloud Dedicated if quotas/isolation require it. **InfluxDB 3 Cloud** (managed Enterprise) is early-access / separate product — not used yet.

**Org details** (no secrets in docs):
| Field | Value |
|-------|--------|
| Account | AmpX |
| Organization | Energy Gateway |
| Org ID | `86a141bfd8d7f66a` |
| Product | InfluxDB Cloud Serverless (v3 storage engine) |
| Provider / region | AWS `eu-central-1` (EU Frankfurt) |
| Cluster URL | `https://eu-central-1-1.aws.cloud2.influxdata.com` |
| Bucket | `energy_metrics` (ID `43bdc9cfa0531940`, retention **30 days**, schema Implicit) |
| Signup email | Company AmpX address (not personal Gmail) |

**Ops notes**:
- API token **AmpXGateway** created; value stored in local `api/config/config.php` as `INFLUXDB_CLOUD_*` (not in README/git docs). Rotate if exposed outside trusted channels.
- Writes: v1/v2-compatible line protocol; queries: **SQL** / InfluxQL (Flux not the target)
- Rough Serverless cost at 200 gateways × 30s uploads is material; prefer longer intervals (e.g. 5 min) for fleet scale
- Do not put tokens/URLs with secrets into README examples
- UI breadcrumb may still show org typo “Enegy”; profile name is Energy Gateway

**Spike verified (Aug 10, 2026)**:
- Write: `POST /api/v2/write` → **204** (line protocol → `energy_metrics`)
- Read: InfluxQL via `GET /query` → **200** after creating **DBRP** mapping `energy_metrics`/`autogen` → bucket `43bdc9cfa0531940` (DBRP id `11272fcff1de9000`)
- Helper script: `api/v2/tests/spike_cloud_serverless.php`
- Note: Cloud Serverless HTTP reads for PHP are easiest via **InfluxQL `/query`** (needs DBRP). Native **SQL** is Flight/gRPC (client libs), not simple curl for AmpX portal yet.

**Not done yet**: Sunset `/api/v2/` and `influxdb2.ampx.app`; publish OTA 1.1.1; deploy theme 1.0.9

### Portal Meter Data UX (August 2026)
**Objective**: Wide readings table usable in-viewport; clarify 1000/30-day limits; export must not be limited to the table rows.

**Theme (`ampx-portal-theme` 1.0.9)**:
- Root cause of “no scrollbar”: `.table-wrapper { overflow: hidden }` clipped columns; after `overflow-x: auto`, horizontal bar sat at bottom of ~10kpx-tall table
- Fix: `max-height` on meter-data `.table-wrapper` + `overflow-x: scroll` + visible scrollbar styling so H/V bars stay in view
- Table uses `width: max-content` + `nowrap` so columns stay readable while scrolling

**Plugin (`ampx-portal-plugin` 1.1.4)**:
- Display: Influx `range(-30d)` + `limit(1000)`; constants `READINGS_RANGE_DAYS` / `READINGS_DISPLAY_LIMIT`
- Count query for UI totals (`count_meter_readings_by_serial`, field `voltage_L1`)
- Export: `admin_post_ampx_export_meter_csv` — nonce + logged-in + `user_can_access_gateway`; fetch with `$limit = null` (full 30-day set)
- UI copy states 30-day window, 1000-row table cap, and that Export is full 30-day
- Spec/plan: `ampx.app/docs/superpowers/specs/2026-08-10-meter-data-display-limit-export-design.md`

**Verified local** (gateway **100008**, SN **3423875005**): scrollbars visible; CSV 200 with matching row counts; CLI `bin/test-meter-export-limits.php`

**Ops**: Deploy theme CSS + plugin to Hetzner for live; hard-refresh after version bump.

### Shared API Key Auth (August 2026)
**Objective**: Smallest fleet-prep change — stop open writes to `/api/v2/` before scale-up.

**Implementation**:
- Server: `AMPX_API_KEY` in `api/config/config.php`; gate in `api/v2/index.php` after Content-Type check (`hash_equals`; empty config → 500; bad/missing → 401)
- Firmware: hardcoded `ampxportal_api_key` next to API URLs; `HTTPClient` sends `X-AmpX-Api-Key` in `postToAmpXPortal2()`; introduced in **1.0.3**, current stable **1.0.7**
- Docs/tests: `api/README.md`, `test_api_v2.php`, gateway README

**Verified local**:
- curl / Postman: no key or wrong key → **401**; correct key → **201** (`ampx-app.local` / `127.0.0.1`)
- Postman: header must be checked/enabled or request is unauthenticated

**Ops / live cutover**:
- Same secret in local and live `config.php`; do not put real key in public docs (use `YOUR_KEY`)
- Flash/OTA gateways with matching key **before** enabling the check on live, or production uploads get 401 (use firmware **1.0.7+**, not 1.0.3)
- Out of scope for now: per-gateway keys, NVS/Admin key UI, rate limits, upload jitter

**Breaking change note (user confirmed Aug 2026):**
- Requiring `X-AmpX-Api-Key` breaks all pre-1.0.3 gateways (401). Accepted because **no live field gateways** existed yet.
- **Policy going forward:** every breaking API/firmware contract change must bump `FIRMWARE_VERSION` (and OTA `version.json`) in the same work; document and coordinate live deploy with flash/OTA.

### HTTP Pull OTA (August 2026)
**Objective**: Enable Admin firmware updates without USB after the first OTA-capable flash.

**Implementation**:
- `HTTPUpdate` + static `WiFiClientSecure` (`setInsecure`) for HTTPS; static `WiFiClient` for HTTP (local test)
- Admin: Check for update + Update Firmware; last OTA status/time in NVS (`ota_status` / `ota_time`)
- `POST /update` sends status page then downloads/flashes; success → NVS + reboot
- Board: **ESP32 Dev Module**, Flash **8MB**, partition **custom** (`partitions.csv` dual OTA app slots)
- Host path: `D:\xampp\htdocs\ampx.app\firmware\` + junction `htdocs\firmware` for LAN IP access
- Deploy helper (current): `D:\xampp\htdocs\firmware\publish-firmware.ps1` (explicit FTP TLS, `AMPX_FTP_PASS`). The older `deploy-firmware.ps1` uses plain FTP and returns 550.

**Verified**:
- Negative: live URL 404 → Admin `Failed: File Not Found (404)`
- Positive: local `http://{LAN_IP}/firmware/...` OTA 1.0.1 → **1.0.2**, Home/Settings OK after reboot
- USB baseline flash on COM5 with custom 8MB partitions

**Ops**: Live `public_html/firmware/` hosts **1.0.7** `.bin` + `version.json` (verified Aug 18). Keep both files in sync when publishing.
### Live Portal Meters Fix + Live API E2E (July 2026)
**Objective**: Prove gateway uploads work against live AmpX API and appear on https://ampx.app; fix Meters critical error for gateway 100007.

**Root cause (meters critical error)**:
- Live `wp-config.php` was missing `AMPX_INFLUXDB_URL` / `TOKEN` / `ORG` / `BUCKET`
- Plugin `AMPX_Portal_Config` throws if those constants are undefined when Meters loads Influx
- After adding constants, PHP **OPcache** still served the old `wp-config` until `opcache_reset()`

**Also hardened (local + deployed to live plugin)**:
- `class-ampx-portal-influxdb-detailed.php`: robust Influx annotated-CSV parse; `str_getcsv(..., ',', '"', '\\')` for PHP 8.4
- `class-ampx-portal-public.php`: try/catch around meters shortcode; admin-visible error; build marker `ampx-meters-build:2026-07-25b`

**Live ops**:
- Host: Hetzner `dedivirt3789.your-server.de`, FTP user `ampxapp`, docroot `public_html` → `/usr/www/users/ampxapp/`
- PHP **8.4**; WP debug log via Debug Log Manager path in `WP_DEBUG_LOG` (not `wp-content/debug.log`)
- Apache `www_logs/ampx.app/` is mostly scanner noise — use the Debug Log Manager file for PHP fatals
- Gateway must exist in WP Admin and be assigned to the user (Influx data alone does not register a gateway)

**Verified live**: Gateway **100007**, meter SN **2724193004**, Meters list + View Data (~74 readings); ESP posts **201** to live API.

### Local API + Portal Verification (July 2026)
**Objective**: Prove gateway uploads work against local AmpX API and appear in the local portal.

**Working path**:
1. Gateway `USE_LOCAL_SERVER true` → `http://{PC_LAN_IP}/api/v2/` (port **80**, not 8080)
2. XAMPP serves `D:\xampp\htdocs\ampx.app\api\v2\` (junction `D:\xampp\htdocs\api` → `ampx.app\api` so IP-based requests work)
3. API writes to InfluxDB (`https://influxdb2.ampx.app`, bucket `energy_metrics`, tags `gateway` / `meter` / `serial_number`)
4. Portal at **http://ampx-app.local/** (vhost `ampx-app.local` → `htdocs/ampx.app`). Public **ampx.app** is Cloudflare live site — do not confuse with local.

**Verified**: Gateway **100007**, meter **1**, serial **2724193004** → HTTP **201** on Serial; portal Meters + View Data show readings.

**Ops pitfalls**:
- Old URL `http://192.168.2.32:8080/api/v2/` is obsolete
- ESP `connection refused` to PC:80 often = Windows Firewall / network profile; Wi‑Fi **Private** + Apache inbound allow
- Firmware “No internet connection” on connection-refused is misleading
- NTP: ~15s max wait (exits early on success)
- Meter discovery: `break` on first failed Modbus ID (contiguous IDs 1..N)

**API docs**: `D:\xampp\htdocs\ampx.app\api\README.md` (gateway README links only).

### WebSocket Loop Servicing Fix (July 2026)
**Objective**: Fix multi-minute WebSocket “Connecting…” after HTTP page already loaded.

**Minimal Fix** (meter-read block): call `server.handleClient()` + `webSocket.loop()` before and after `handlePowerMeter()`.

**Related**: SoftAP DHCP on Windows; 8MB flash / OTA partition; OneDrive compile slowdowns.

### Staggered Meter Reading (January 2025)
One meter per 1s interval; sub-3s WebSocket connections; progressive UI updates.

### UI / WebSocket / 5-Meter Work (Oct–Dec 2024)
Sidebar UI, meters page, WebSocket architecture, 5-meter expansion — see progress.md / .cursorrules Sessions 1–5.

## Next Development Opportunities
1. **Tomorrow:** Publish **1.2.5** only if asked. Chip-aware `targets` is still unshipped. Plan: `memory-bank/plans/2026-08-23-multi-chip-ota.md`. Do not publish **1.0.9** or an S3 image at the legacy `.bin` URL. Optional: WiFi LED blink while the setup hotspot is up. `TestS3Bringup.ino` still uses the old S3 pin map.
2. USB-flash 100007 with 1.2.0 **before** live catalog; then migrate remaining `/api/v2/` units
3. Deploy theme **1.0.9**; then sunset v2 / Influx 2
4. Optional: 100007 voltage zeros (Modbus); TCP Modbus register probe; RS485 `modbus_test_connection()`; scale HTML meters; Windows mDNS

## System Health Status
- **Firmware**: Sketch **1.2.5** (5 Oct 2026). Reset button tested. Live catalog still **1.2.4**. **100007** was USB **1.2.3** on 29 Sep; 1.2.4 OTA apply not confirmed. Do not flash 1.0.3, 1.0.4/1.0.5, or 1.2.2 (W5500 socket leak).
- **Local + live API v3**: Key enforced; Cloud writes 201 verified
- **API v2**: Still on Hetzner → Influx 2; live portal does not read it
- **Influx (portal)**: Cloud Serverless org **Energy Gateway**
- **Portal**: Plugin **1.1.5** InfluxQL; live Meter Data for 100007 / **3425875001** (50 Hz and power factor on 28 Sep; uploads resumed after USB 1.2.3 on 29 Sep)
- **Theme**: 1.0.9 local, **1.0.7** live
- **OTA hosting**: Live `version.json` **1.2.4** (29 Sep 2026, `.bin` **1,341,696** bytes). Single `url`, no `targets`. Publish via `D:\xampp\htdocs\firmware\publish-firmware.ps1` (explicit FTP TLS).
