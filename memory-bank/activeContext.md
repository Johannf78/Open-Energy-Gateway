# Active Context - AmpX Open Energy Gateway

## Current Focus
**Aug 22, 2026:** Firmware sketch **1.0.9** — ship-mode Clear WiFi is in Admin. Local `firmware/version.json` is **1.0.9**. Live `ampx.app/firmware/` was last verified at **1.0.7** — publish the 1.0.9 `.bin` + `version.json` together when ready.

**Next project:** AmpX **API v3** / Cloud Serverless cutover (keep v2 until verified). Then live portal UX + live API key.

**Ready for next session:**
- Ship-mode done (Admin Clear WiFi; LEDs off on reboot; WiFiManager save-page copy)
- Publish live OTA **1.0.9** when you want field devices to get the button
- Later: `influxdb3_hosting_choice` (api-v3 → portal queries → firmware cutover)

Previous: Ship-mode 1.0.9; live HTTPS OTA (1.0.7); OTA WDT/stack fix; Admin Check button; 1.0.4 reboot-loop fix; Cloud Serverless spike; Portal Meter Data UX; API key auth.

## Recent Major Achievements

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

**Not done yet**: AmpX `api/v3`, firmware URL switch, portal Flux→SQL/InfluxQL against Cloud, cutover from `influxdb2.ampx.app`

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
1. **Next project:** AmpX **API v3** → Cloud Serverless (`INFLUXDB_CLOUD_*`); same payload + API key; keep v2
2. Portal: switch Meter Data queries from Flux/Influx2 to InfluxQL (or SQL) against Cloud
3. Firmware: point to `/api/v3/`, bump `FIRMWARE_VERSION` (breaking-change policy), then sunset v2/Influx2
4. Publish live OTA **1.0.9** (`.bin` + `version.json`); deploy portal UX + live API key
5. Scale HTML meters; Windows mDNS; optional NVS API key

## System Health Status
- **Firmware**: Sketch **1.0.9** (ship-mode); do not flash 1.0.3 (reboot loop) or 1.0.4/1.0.5 (Check WDT)
- **Local API**: Shared key enforced; Postman/curl verified 401/201
- **Live API**: Still open (no key gate) until Hetzner deploy
- **Influx (production)**: Still `influxdb2.ampx.app` (v2 + Flux portal)
- **Influx (target)**: Cloud Serverless org **Energy Gateway** created; not wired to AmpX yet
- **Local portal**: Meter Data scroll + 1000/30d limits + full-window CSV export verified (`ampx-app.local`)
- **Live portal**: Meters/View Data working for 100007; UX fixes not deployed yet
- **OTA hosting**: Local `version.json` **1.0.9**; live `https://ampx.app/firmware/` last verified **1.0.7** until 1.0.9 is published
