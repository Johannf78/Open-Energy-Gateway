

void initWiFi() {
  
  debugln("Inside initWiFi function. Top of function");
  
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

  /*If you've previously connected to your WiFi with this ESP32,
  WiFi manager will more than likely not do anything.
  Uncomment this if you want to force it to delete your old WiFi details.*/
  //wifiManager.resetSettings();

  //Non blocking mode
  //wifiManager.setConfigPortalBlocking(true);

  //Set the timeout 3 minutes or more to give the WiFi router chance to startup...
  //This prevents it going into AP mode before it can connect.
  //wifiManager.setConfigPortalTimeout(180);

  /*Tries to connect to last known WiFi details
  if it does not connect it starts an access point with the specified name
  here  "AutoConnectAP"and goes into a blocking loop awaiting configuration*/
  if (!wifiManager.autoConnect(hostname.c_str(), "")) {
    debugln("Failed to connect and hit timeout...");
    //reset and try again, or maybe put it to deep sleep
    //ESP.restart();
    delay(1000);
    //TODO: JF: Fix endless loop if no signal...
  }
  //Indicate that Wifi is successfully connected by putting on LED 3
  digitalWrite(LED_3_WIFI, HIGH);

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