// HTTP pull OTA (Admin-triggered). ArduinoOTA is intentionally not used (flash size).

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

void serviceOtaManifestCheck() {
  const unsigned long OTA_MANIFEST_INTERVAL_MS = 300000;  // refresh every 5 minutes
  unsigned long now = millis();
  bool due = otaManifestCheckRequested ||
             (otaManifestLastCheckMs == 0) ||
             (now - otaManifestLastCheckMs > OTA_MANIFEST_INTERVAL_MS);
  if (!due) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  otaManifestCheckRequested = false;
  otaManifestLastCheckMs = now;

  FirmwareManifest manifest;
  if (fetchFirmwareManifest(manifest)) {
    bool upToDate = (manifest.version == String(FIRMWARE_VERSION));
    otaStatusCache.ready = true;
    otaStatusCache.ok = true;
    otaStatusCache.available = manifest.version;
    otaStatusCache.upToDate = upToDate;
    otaStatusCache.updateAvailable = !upToDate;
    otaStatusCache.status = upToDate ? "Up to date" : "Update available";
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

// Runs on core 0 so a slow/hanging HTTPClient cannot stall WebServer on loop()/core 1.
void otaManifestTask(void* param) {
  (void)param;
  for (;;) {
    serviceOtaManifestCheck();
    vTaskDelay(pdMS_TO_TICKS(500));
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

  if (manifestUrl.startsWith("https://")) {
    WiFiClientSecure secureClient;
    secureClient.setInsecure();
    secureClient.setTimeout(transferTimeoutMs);
    if (!http.begin(secureClient, manifestUrl)) {
      out.error = "Manifest begin failed";
      return false;
    }
    httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      body = http.getString();
    }
    http.end();
  } else {
    // Explicit connect timeout — HTTPClient alone often hangs inside WebServer context.
    String host;
    String path = "/";
    uint16_t port = 80;
    if (manifestUrl.startsWith("http://")) {
      String rest = manifestUrl.substring(7);
      int slash = rest.indexOf('/');
      String hostPort = (slash >= 0) ? rest.substring(0, slash) : rest;
      path = (slash >= 0) ? rest.substring(slash) : "/";
      int colon = hostPort.indexOf(':');
      if (colon >= 0) {
        host = hostPort.substring(0, colon);
        port = (uint16_t)hostPort.substring(colon + 1).toInt();
      } else {
        host = hostPort;
      }
    } else {
      out.error = "Unsupported manifest URL";
      return false;
    }

    WiFiClient client;
    if (!client.connect(host.c_str(), port, connectTimeoutMs)) {
      out.error = "Manifest connect failed";
      return false;
    }
    client.setTimeout(transferTimeoutMs);
    if (!http.begin(client, manifestUrl)) {
      out.error = "Manifest begin failed";
      client.stop();
      return false;
    }
    httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      body = http.getString();
    }
    http.end();
    client.stop();
  }

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
    if (manifest.version == String(FIRMWARE_VERSION)) {
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
  if (downloadUrl.startsWith("https://")) {
    WiFiClientSecure secureClient;
    secureClient.setInsecure();
    ret = httpUpdate.update(secureClient, downloadUrl);
  } else {
    WiFiClient client;
    ret = httpUpdate.update(client, downloadUrl);
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
