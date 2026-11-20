

void initWiFi() {
  
  debugln("Inside initWiFi function. Top of function");

  //Initialize the WiFi Reset Button Pin
  //The pull-up keeps the pin high when the front ribbon is unplugged. 
  // initWiFi() can sit in the saved-network wait for a long time, so the pin has to be ready before that wait.
  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);
  
  //Create hostname string and convert to C-style string
  String hostname = "energy-gateway-" + String(GATEWAY_ID);
  WiFi.setHostname(hostname.c_str());

  // Connect to the Wi-Fi network, this is now handeled by the WiFi Mangager library...
  /*WiFi.begin(ssid, password);

  Serial.print("Now connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");//Print more dots while connecting 
  }*/

  WiFiManager wifiManager;

  String portalHead = "<script>document.addEventListener('DOMContentLoaded',function(){";
  portalHead += "var m=document.querySelector('.msg');";
  portalHead += "if(!m||m.textContent.indexOf('Trying to connect')===-1)return;";
  portalHead += "m.innerHTML='<h2>WiFi saved</h2>'";
  portalHead += "+'<p>The AmpX Energy Gateway is connecting to your network. This setup hotspot will close.</p>'";
  portalHead += "+'<p>Connect this phone or computer to the same WiFi, then open:</p>'";
  portalHead += "+'<p><strong>http://energy-gateway-";
  portalHead += String(GATEWAY_ID);
  portalHead += ".local</strong></p>'";
  portalHead += "+'<p>(or the IP of the gateway you got from your WiFi router)</p>'";
  portalHead += "+'<p>If it does not connect, reconnect to the gateway hotspot and try again.</p>';";
  portalHead += "});</script>";
  wifiManager.setCustomHeadElement(portalHead.c_str());
  wifiManager.setTitle("AmpX Energy Gateway");

  /*If you've previously connected to your WiFi with this ESP32,
  WiFi manager will more than likely not do anything.
  Uncomment this if you want to force it to delete your old WiFi details.*/
  //wifiManager.resetSettings();

  //New code added after adding a reset button
  // No saved network: first boot, admin Clear WiFi, or the reset button.
  if (!wifiManager.getWiFiIsSaved()) {
    debugln("No saved WiFi. Starting setup hotspot.");
    wifiManager.autoConnect(hostname.c_str(), "");
  } else {
    // Router missing or still booting. Stay on the saved network.
    // A failed try must not start the hotspot.
    debugln("Waiting for saved WiFi.");
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    while (WiFi.status() != WL_CONNECTED) {
      serviceWifiResetButton();
      delay(200);
    }
  }
  //Turns on the WiFi LED once either path has a connection.
  //Indicate that Wifi is successfully connected by putting on LED 3
  digitalWrite(LED_3_WIFI, HIGH);
  


  //Non blocking mode
  //wifiManager.setConfigPortalBlocking(true);

  //Set the timeout 3 minutes or more to give the WiFi router chance to startup...
  //This prevents it going into AP mode before it can connect.
  //wifiManager.setConfigPortalTimeout(180);

  /*Tries to connect to last known WiFi details
  if it does not connect it starts an access point with the specified name
  here  "AutoConnectAP"and goes into a blocking loop awaiting configuration*/
  /*
  if (!wifiManager.autoConnect(hostname.c_str(), "")) {
    debugln("Failed to connect and hit timeout...");
    //reset and try again, or maybe put it to deep sleep
    //ESP.restart();
    delay(1000);
    //TODO: JF: Fix endless loop if no signal...
  }
  */

  debugln("");
  debugln("Connected to Wi-Fi");
  
  debug("IP address: ");
  debugln(WiFi.localIP());
  
  debug("Signal strength: ");
  debugln(WiFi.RSSI());

  debug("WiFi.getHostname: ");
  debugln(WiFi.getHostname());

  debugln("Inside initWiFi function. End of function");
  debugln("");

}

  // Staff: Admin → Clear WiFi (before Shiping).
  void clearStoredWifi() {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
  }

  // LEDs must be driven low before ESP.restart(). A high pin stays high across the restart.
void turnStatusLedsOff() {
  digitalWrite(LED_1_POWER, LOW);
  digitalWrite(LED_2_METER, LOW);
  digitalWrite(LED_3_WIFI, LOW);
  digitalWrite(LED_4_INTERNET, LOW);
  digitalWrite(LED_5_SERVER, LOW);
}

// Same ending as Admin Clear WiFi. Gateway ID, meter names, and OTA data stay in NVS.
void clearWifiAndRestart() {
  debugln("Clearing stored WiFi and rebooting into AP mode...");
  clearStoredWifi();
  turnStatusLedsOff();
  delay(300);
  ESP.restart();
}

// All five LEDs are already on while the gateway is running, so turning them on
// would not show that the hold was accepted. Two off/on cycles are the confirmation.
// delay() is allowed here only because the next step is a restart.
void flashStatusLedsTwice() {
  for (int flash = 0; flash < 2; flash++) {
    turnStatusLedsOff();
    delay(200);
    digitalWrite(LED_1_POWER, HIGH);
    digitalWrite(LED_2_METER, HIGH);
    digitalWrite(LED_3_WIFI, HIGH);
    digitalWrite(LED_4_INTERNET, HIGH);
    digitalWrite(LED_5_SERVER, HIGH);
    delay(200);
  }
}

// Sampled from the WiFi wait in setup() and from loop(). A short tap can fall
// inside one Modbus read, so the pin has to stay low for 3 seconds.
// A ribbon stuck low at boot stays disarmed until the pin is seen high once.
void serviceWifiResetButton() {
  if (millis() < 2000) {
    return;
  }
  static bool armed = false;
  static bool holding = false;
  static unsigned long lowSince = 0;
  if (digitalRead(WIFI_RESET_BUTTON_PIN) == HIGH) {
    armed = true;
    holding = false;
    return;
  }
  if (!armed) {
    return;
  }
  if (!holding) {
    holding = true;
    lowSince = millis();
    return;
  }
  if (millis() - lowSince < 3000) {
    return;
  }
  flashStatusLedsTwice();
  clearWifiAndRestart();
}

//Initialize mDNS service for .local domain access
void initmDNS() {
  // Create mDNS hostname
  String mdnsHostname = "energy-gateway-" + String(GATEWAY_ID);
  
  if (MDNS.begin(mdnsHostname.c_str())) {
    debugln("mDNS responder started");
    debug("Hostname: ");
    debugln(mdnsHostname + ".local");
    
    // Add HTTP service
    MDNS.addService("http", "tcp", 80);
    
    // Add WebSocket service
    MDNS.addService("ws", "tcp", 81);
    
    debugln("mDNS services added:");
    debugln("- HTTP service on port 80");
    debugln("- WebSocket service on port 81");
  } else {
    debugln("Error setting up MDNS responder!");
  }
}