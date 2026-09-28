# Multi-chip OTA — saved for later

**Date:** 2026-08-23  
**Status:** Designed; not implemented (except `CHIP_OTA_KEY` + `FIRMWARE_VERSION` `"1.2.0"` in the sketch)

Canonical copies (same decision):

- Spec: `D:\xampp\htdocs\ampx.app\docs\superpowers\specs\2026-08-23-multi-chip-ota-design.md`
- Plan: `D:\xampp\htdocs\ampx.app\docs\superpowers\plans\2026-08-23-multi-chip-ota.md`

## Do this next session

1. Finish chip-aware parse in `functions_ota.ino` (no `firmwareURL` fallback).
2. USB-flash **100007** with 1.2.0 **before** publishing live OTA.
3. Publish `targets`-only `version.json` + `firmware/esp32/*.bin`.
4. Do **not** publish 1.0.9 or a single-url 1.1.1 catalog.
5. Never put an S3 image at `https://ampx.app/firmware/ampx_open_energy_gateway.bin`.

S3 hardware port is a later task. OTA cannot migrate ESP32 → S3 silicon.
