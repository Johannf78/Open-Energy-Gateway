
#if MODBUS_TYPE == MODBUS_TYPE_TCPIP
  void initEthernet(){
    debugln("Starting Ethernet connection...");

    //Pin the SPI bus to the board pin map (ESP32: VSPI 18/19/23, ESP32-S3: FSPI 12/13/11).
    //The Arduino Ethernet library uses the global SPI object; on the S3 the core's default
    //pins happen to match, but calling begin() explicitly makes the wiring the same on every board.
    //SS is passed as -1 on purpose: the Ethernet library drives CS itself (Ethernet.init below);
    //attaching hardware CS as well would let the SPI peripheral toggle the pin between bytes.
    SPI.begin(ETH_SPI_SCK_PIN, ETH_SPI_MISO_PIN, ETH_SPI_MOSI_PIN, -1);

    #ifdef ETH_SPI_RST_PIN
      //Optional hardware reset of the W5500 (only if RST is wired). Datasheet: RST low >= 500us, then wait for PLL.
      pinMode(ETH_SPI_RST_PIN, OUTPUT);
      digitalWrite(ETH_SPI_RST_PIN, LOW);
      delay(2);
      digitalWrite(ETH_SPI_RST_PIN, HIGH);
      delay(150);
    #endif

    //Set the CS pin, required for ESP32 as the arduino default is different
    Ethernet.init(ETH_SPI_SCS_PIN);


    debugln("\nStarting Custom Modbus TCP Implementation");
    
    // Initialize ethernet
    debugln("Initializing Ethernet...");
    Ethernet.begin(mac, ip, gateway, subnet);
    
    // Wait for Ethernet to be ready
    delay(2000);

    //Hardware check
    debugln("Checking Ethernet hardware...");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      debugln("ERROR: No Ethernet hardware detected!");
      return;
    }
    else {
      debugln("Ethernet hardware detected!");
    }
    
    //Check if cable is connected
    if (Ethernet.linkStatus() == LinkOFF) {
      debugln("Link is OFF. Check LAN cable connection.");
      return;
    }
    else {
      debugln("Link is ON. Cable is connected. Ready to go!");
      debug("To test LAN connection, please ping: ");
      debugln(ip);
    }

    debug("Arduino IP: ");
    debugln(Ethernet.localIP());

    debug("Energy Meter IP: ");
    debugln(meter_ip);

    debug("Port: ");
    debugln(MODBUS_PORT);

    // Initialize Modbus with meter IP
    modbus_init(meter_ip);
    
    // Test network connectivity
    debugln("\nTesting network connectivity between gateway and meter on LAN cable...");
    if (modbus_test_connection()) {
      debugln("Modbus connection test successful!  We are able to communicate with the meter with modbus over TCPIP!");
      //Turn on LED 2 to indicate successful connection to energy meter.
      digitalWrite(LED_2_METER, HIGH);
    } else {
      debugln("Connection test failed!");
      debugln("Please check:");
      debugln("1. Physical network connection");
      debugln("2. IP addresses and subnet mask");
      debugln("3. No firewall blocking port 502");
      debugln("4. Energy meter is powered on and responding");
    }
    debugln("Modbus TCPIP Setup complete");
    debugln("");
  }
#endif