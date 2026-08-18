// HTTP pull OTA (Admin-triggered). ArduinoOTA is intentionally not used (flash size).

// TLS/TCP clients in BSS, not on the Arduino loop() stack.
// A local WiFiClientSecure in fetchFirmwareManifest() caused TG1WDT_SYS_RESET.
static WiFiClientSecure otaTlsClient;
static WiFiClient otaPlainClient;

void initOTA() {
  // No ArduinoOTA. Firmware updates are started from Admin via doOTAUpdate().
}

void saveOtaStatus(const String& status) {
  preferences.putString("ota_status", status);
  preferences.putString("ota_time", getCurrentTimestamp());
}

String getOtaStatusText() {
  String status = preferences.getString("ota_status", "");
  if (status.length() == 0) {
    return "Never";
  }
  return status;
}

String getOtaTimeText() {
  String otaTime = preferences.getString("ota_time", "");
  if (otaTime.length() == 0) {
    return "—";
  }
  return otaTime;
}

void requestOtaManifestCheck() {
  otaManifestCheckRequested = true;
}

// Returns true if serverVer > deviceVer (e.g. "1.0.6" > "1.0.5")
bool isNewerVersion(const String& serverVer, const String& deviceVer) {
  int sv[3] = {0, 0, 0};
  int dv[3] = {0, 0, 0};
  sscanf(serverVer.c_str(), "%d.%d.%d", &sv[0], &sv[1], &sv[2]);
  sscanf(deviceVer.c_str(), "%d.%d.%d", &dv[0], &dv[1], &dv[2]);
  for (int i = 0; i < 3; i++) {
    if (sv[i] > dv[i]) return true;
    if (sv[i] < dv[i]) return false;
  }
  return false;  // equal
}

void serviceOtaManifestCheck() {
  // Manual only: Admin "Check for update" sets the flag. Do not auto-fetch on a timer
  // (HTTPS TLS on loop() trips TG1WDT_SYS_RESET).
  if (!otaManifestCheckRequested) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  otaManifestCheckRequested = false;
  otaManifestLastCheckMs = millis();

  FirmwareManifest manifest;
  if (fetchFirmwareManifest(manifest)) {
        
    bool upToDate = (manifest.version == String(FIRMWARE_VERSION));
    bool newerAvailable = isNewerVersion(manifest.version, String(FIRMWARE_VERSION));

    otaStatusCache.ready = true;
    otaStatusCache.ok = true;
    otaStatusCache.available = manifest.version;
    otaStatusCache.upToDate = upToDate || !newerAvailable;  // up to date if equal OR device is newer
    otaStatusCache.updateAvailable = newerAvailable;
    otaStatusCache.status =  newerAvailable ? "Update available" : "Up to date";
    otaStatusCache.error = "";
    debug("OTA manifest OK: ");
    debugln(manifest.version);
  } else {
    otaStatusCache.ready = true;
    otaStatusCache.ok = false;
    otaStatusCache.available = "Unavailable";
    otaStatusCache.upToDate = false;
    otaStatusCache.updateAvailable = true;  // allow manual attempt
    otaStatusCache.status = "Manifest unavailable: " + manifest.error;
    otaStatusCache.error = manifest.error;
    debug("OTA manifest failed: ");
    debugln(manifest.error);
  }
}

bool fetchFirmwareManifest(FirmwareManifest& out) {
  out.ok = false;
  out.version = "";
  out.url = "";
  out.error = "";

  String manifestUrl = String(firmwareManifestURL);
  debug("Fetching firmware manifest: ");
  debugln(manifestUrl);

  const uint32_t connectTimeoutMs = 2000;
  const uint32_t transferTimeoutMs = 3000;

  HTTPClient http;
  http.setReuse(false);
  http.setConnectTimeout(connectTimeoutMs);
  http.setTimeout(transferTimeoutMs);

  int httpCode = -1;
  String body;

  bool began = false;
  yield();
  if (manifestUrl.startsWith("https://")) {
    otaTlsClient.setInsecure();
    otaTlsClient.setTimeout(transferTimeoutMs);
    began = http.begin(otaTlsClient, manifestUrl);
  } else if (manifestUrl.startsWith("http://")) {
    otaPlainClient.setTimeout(transferTimeoutMs);
    began = http.begin(otaPlainClient, manifestUrl);
  } else {
    out.error = "Unsupported manifest URL";
    return false;
  }
  
  if (!began) {
    out.error = "Manifest begin failed";
    return false;
  }
  
  debugln("Manifest GET...");
  httpCode = http.GET();
  debug("Manifest HTTP ");
  debugln(httpCode);
  if (httpCode == HTTP_CODE_OK) {
    body = http.getString();
  }
  http.end();
  otaTlsClient.stop();
  otaPlainClient.stop();
  yield();

  if (httpCode != HTTP_CODE_OK) {
    out.error = "Manifest HTTP " + String(httpCode);
    return false;
  }

  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    out.error = "Manifest JSON error";
    return false;
  }

  out.version = doc["version"] | "";
  out.url = doc["url"] | "";
  if (out.version.length() == 0) {
    out.error = "Manifest missing version";
    return false;
  }

#if USE_LOCAL_SERVER
  out.url = String(firmwareURL);
#else
  if (out.url.length() == 0) {
    out.url = String(firmwareURL);
  }
#endif

  out.ok = true;
  return true;
}

// Returns true on successful flash (caller should expect reboot).
bool doOTAUpdate() {
  debugln("Starting HTTP OTA update...");

  FirmwareManifest manifest;
  if (fetchFirmwareManifest(manifest)) {
    if (!isNewerVersion(manifest.version, String(FIRMWARE_VERSION))) {
      String msg = "Already on latest (" + manifest.version + ")";
      debugln(msg);
      saveOtaStatus(msg);
      return false;
    }
  } else {
    debug("Manifest check failed (continuing with fallback URL): ");
    debugln(manifest.error);
  }

  String downloadUrl = manifest.ok ? manifest.url : String(firmwareURL);
  debug("Firmware URL: ");
  debugln(downloadUrl);

  httpUpdate.rebootOnUpdate(false);

  t_httpUpdate_return ret;
  yield();
  if (downloadUrl.startsWith("https://")) {
    otaTlsClient.setInsecure();
    ret = httpUpdate.update(otaTlsClient, downloadUrl);
    otaTlsClient.stop();
  } else {
    ret = httpUpdate.update(otaPlainClient, downloadUrl);
    otaPlainClient.stop();
  }

  switch (ret) {
    case HTTP_UPDATE_OK: {
      String msg = "OK: update applied";
      if (manifest.ok) {
        msg += " (" + manifest.version + ")";
      }
      debugln("OTA update successful.");
      saveOtaStatus(msg);
      return true;
    }
    case HTTP_UPDATE_NO_UPDATES: {
      String msg = "No update available";
      debugln(msg);
      saveOtaStatus(msg);
      return false;
    }
    case HTTP_UPDATE_FAILED:
    default: {
      String err = httpUpdate.getLastErrorString();
      String msg = "Failed: " + err;
      debug("OTA update failed: ");
      debugln(err);
      saveOtaStatus(msg);
      return false;
    }
  }
}
