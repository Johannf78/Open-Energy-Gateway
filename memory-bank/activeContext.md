# Active Context - AmpX Open Energy Gateway

## Current Focus
**Sep 28, 2026 — Live OTA catalog is 1.2.1 (single `{version,url}`).** Sketch `FIRMWARE_VERSION` is **1.2.1**. `https://ampx.app/firmware/version.json` is **1.2.1**. The downloaded `ampx_open_energy_gateway.bin` is **1,335,296** bytes and contains the ASCII string **1.2.1** (not 1.0.9). Cloudflare cache rule **Bypass firmware OTA** is **Active**: URI Path starts with `/firmware/` → Bypass cache.

**Not confirmed after that rule:** Admin on gateway **100007** still needs a Check + Update so **Current firmware version** reads **1.2.1**. Two earlier OTAs the same morning reported `OK: update applied (1.2.1)` and then booted **1.0.9**. That status line appends the **manifest** version (`saveOtaStatus` in `doOTAUpdate()`), not the string compiled into the image. Cloudflare was still serving the old `.bin`. Do not treat the field unit as 1.2.1 until Admin says so.

**100007** is Modbus **TCP/IP** (W5500), not RS485. ME537 serial **3423875005**. On 26 Sep, with voltage and current coils connected, gateway and live portal agreed (about 237–240 V; L2 idle; L1/L3 power and imported energy added up). `detectNumberOfMeters()` used to print RS485 always (`if (MODBUS_TYPE_RS485)` is `if (1)`). It now compares `MODBUS_TYPE == MODBUS_TYPE_RS485`.

**Register map** in `meter_registers.h` (this is what `setupMeterRegisters()` loads; `data/meter_registers_meatrol.json` is a copy):

| Key | Address | Words | Type |
|-----|---------|-------|------|
| `frequency` | 1024 | 2 | float |
| `power_factor_L1` | 1052 | 2 | float |
| `power_factor_L2` | 1054 | 2 | float |
| `power_factor_L3` | 1056 | 2 | float |

Do **not** send `power_factor_tot` (register 1058). API v3 allows only `mN_power_factor_L[1-3]`. One invalid key returns **400** for the whole post. The portal has no total power-factor column. `postToAmpXPortal2()` already forwards every `mN_` key. Local `/meters` still has **no** `meter_frequency` / `meter_power_factor_L*` cells, so those values do not show there. The unit `if` in `web_meters.h` checks `power_factor` before `power` (otherwise the key is labelled kW).

**Sep 28, 2026 — TCP Meter Details connects in about 5 seconds (was up to ~40).**

One Modbus TCP socket stays open for the whole register sweep, and the WebSocket is serviced between registers. Measured on the bench before the per-register `delay(100)` was removed: page status green and values up in ~5 s. That pause is now gone (`modbus_read_response()` already waits up to 2 s), which should take about another 2 s off; not re-timed yet.

**Library** `ampx_modbus_tcpip.cpp` (Arduino libraries, not the sketch tree):
- `modbus_test_connection()` connects to port 502 and **leaves the socket open** (already open → return true). It is still a TCP open, not a Modbus PDU.
- `modbus_send_request()` reuses that socket and connects only if it dropped.
- `modbus_read_response()` does **not** `stop()` on a full reply. It closes on a short frame (`bytesRead < 9`) or when the header arrived without the register bytes.
- No `delay(100)` before the read.

**Sketch** `functions_meter.ino`:
- Inside the register loop, before each read: `server.handleClient()` + `webSocket.loop()`. Do not call `handleWebSocket()` there; the JSON is still broadcast once after the sweep.
- End of `handlePowerMeter()`, TCP only: `modbusClient.stop()` once.
- `reconnectMeter()` stays **unhooked**. `initEthernet()` has `delay(2000)` and must not run on the 1 s path.

UI copy in `web_home.h`, `web_meters.h`, and `web_settings.h` now says the initial connection usually takes a few seconds (was “up to 30 seconds”).

**Sep 6, 2026 — ESP32-S3 bench port: WORKING on a stock DevKitC-1U.** Both sketches compile, flash and run; W5500 detected; WiFi AP up. Only the meter link is untested.

### Next session — start here
1. Set the meter's Ethernet to `192.168.1.55` (gateway `192.168.1.1`, mask `255.255.255.0`, Modbus-TCP enabled, port 502) so it matches the firmware defaults — the User Manual V1.0 still documents the old `192.168.2.122` pair, so **either the meter or the manual needs changing**; decide which and make them agree.
2. LAN cable W5500 ↔ meter. Expect `Link is ON`, Modbus test pass, METER LED on.
3. Then: WiFi provisioning via the AP, API post to `ampx.app/api/v3/`, and finally OTA with `CHIP_OTA_KEY = esp32s3`.
4. Consider soldering headers for the four SPI lines — dupont on breadboard caused two false "no hardware" failures today.

**Sep 11, 2026 — hardware V3 decisions that need firmware work (not yet written):**
- **Reset button on GPIO 13**, front panel, recessed pinhole. Function: **hold 3 s → clear saved WiFi credentials → restart → WiFiManager falls into AP mode** (`clearStoredWifi()` + `ESP.restart()` already exist; currently only reachable from the admin web page — which is exactly what a customer cannot reach after changing their router). Short press: reboot, or nothing — undecided.
- GPIO 13 chosen because WROVER-IE has no GPIO 16/17; 5/18/19/23 are the W5500; 12/14/25/26/27 the LEDs; 0/2/15 are strapping (GPIO 0 low at boot = bootloader mode); 21/22 kept for I²C; 32/33 kept for the RS485 variant.
- Hardware: 10 kΩ pull-up + 100 nF on the **main** board, switch to GND on the front board, `INPUT_PULLUP` also set in firmware so the pin is defined when the ribbon is unplugged.
- **Firmware still to write:** button handler with (a) LED feedback — flash all five LEDs at the 3 s threshold so the user knows to release; (b) blink the WiFi LED in AP mode to distinguish "waiting for setup" from "not working"; (c) **stuck-button guards — ignore the button for the first 2 s after boot and require a clean high→low→high transition**, otherwise a pinched ribbon wipes credentials on every boot and strands the unit in AP mode permanently.
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

Sketch `FIRMWARE_VERSION` is **"1.2.1"** and `CHIP_OTA_KEY`; `fetchFirmwareManifest()` still uses top-level `version`/`url` and `firmwareURL` fallback. `setup()` prints the version on the serial monitor after the port is ready. Live `version.json` matches **1.2.1** (published 28 Sep 2026).

Also today: TCP/IP meter connection test verified. Cable/link is in `functions_meter.ino` (`meterTransportReady` / `Ethernet.linkStatus() == LinkOFF`), not in `ampx_modbus_tcpip`. `loop()` calls `serviceMeters()`.

**Ready for next session:**
- Confirm gateway **100007** Admin shows **1.2.1** after Check + Update (catalog and `.bin` are already 1.2.1; Cloudflare `/firmware/` bypass is on).
- Confirm the next portal row for SN **3423875005** has frequency near 50 and power factor on L1–L3. Portal headings still say **W** and **Wh** while the gateway sends **kW** and **kWh**.
- Add `meter_frequency` and `meter_power_factor_L1`–`L3` cells on the local meters page if those values should show there.
- Chip-aware OTA (`targets`) is still not parsed. Do not publish an S3 image at the legacy `.bin` URL. Do not publish **1.0.9**.
- Optional: Modbus probe (registers 70–71) in TCP `modbus_test_connection()`; RS485 test is still `return true`.

Previous: multi-chip OTA spec (23 Aug); TCP/IP connection test; live Cloud cutover (1.1.1 USB); firmware local v3; portal InfluxQL; API v3.

## Recent Major Achievements

### TCP/IP meter connection test (August 2026) — Session 19
**Symptom:** TCP `modbus_test_connection()` looked always-positive. It is **not** the RS485 stub (`return true`). As of this session it was `EthernetClient.connect(meter_ip, 502)` then `stop()` — a TCP open, not a Modbus PDU. **Sep 28:** the `stop()` was removed so the sweep can reuse the socket (see top). Putting `Ethernet.linkStatus()` in `ampx_modbus_tcpip.cpp` failed in the IDE (`LinkOFF` / `Ethernet` undeclared) because that `.cpp` is parsed without Arduino Ethernet includes.

**Fix (sketch, verified 23 Aug 2026):**
- `meterTransportReady()` — TCP/IP only: `Ethernet.linkStatus() == LinkOFF` → false; RS485 always true
- `serviceMeters()` — early returns: no link → LED 2 off, skip Modbus and `initEthernet()`; test fail → LED 2 off, `reconnectMeter()`; success → LED 2 on, `readNextMeter()`
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
- Deploy helper: `ampx.app/firmware/deploy-firmware.ps1` (needs `AMPX_FTP_PASS`)

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
1. **Next:** Implement and publish chip-aware OTA **1.2.0** (`targets` only). Plan: `memory-bank/plans/2026-08-23-multi-chip-ota.md`. Do not publish **1.0.9** or single-url **1.1.1**.
2. USB-flash 100007 with 1.2.0 **before** live catalog; then migrate remaining `/api/v2/` units
3. Deploy theme **1.0.9**; then sunset v2 / Influx 2
4. Optional: 100007 voltage zeros (Modbus); TCP Modbus register probe; RS485 `modbus_test_connection()`; scale HTML meters; Windows mDNS

## System Health Status
- **Firmware**: USB **1.1.1** on **100007** (live `/api/v3/`); local sketch **1.2.1** + `CHIP_OTA_KEY` (OTA parse unfinished; TCP sweep socket Sep 28). Do not flash 1.0.3 or 1.0.4/1.0.5
- **Local + live API v3**: Key enforced; Cloud writes 201 verified
- **API v2**: Still on Hetzner → Influx 2; live portal does not read it
- **Influx (portal)**: Cloud Serverless org **Energy Gateway**
- **Portal**: Plugin **1.1.5** InfluxQL; live + local Meter Data for 100007 / 3423875005
- **Theme**: 1.0.9 local, **1.0.7** live
- **OTA hosting**: Live `version.json` **1.0.9**; next publish chip-aware **1.2.0** (not 1.1.1)
