// Define data type constants
const int dataTypeInt32 = 1;
const int dataTypeInt64 = 2;
const int dataTypeFloat = 3;

// Global JSON document to store meter register definitions
extern JsonDocument MeterRegisterDefs;


// This function sets up the meter register definitions in a JSON document
void setupMeterRegisters() {
    debugln("Inside setupMeterRegisters function. Top of function");
    
    // Parse the JSON string into the MeterRegisterDefs document
    //meter_registers_meatrol is defined in meter_registers.h
    DeserializationError error = deserializeJson(MeterRegisterDefs, meter_registers_meatrol);
    
    if (error) {
      debugln("Failed to parse meter_registers_meatrol JSON:");
      debugln(error.c_str());
      return;
    }
    
    debugln("MeterRegisterDefs Json loaded from string:");
    serializeJsonPretty(MeterRegisterDefs, Serial);
    
    debugln("Inside setupMeterRegisters function. End of function");
    debugln(""); // Add a newline at the end
  }

 
  //Detect the number of meters connected to the gateway.
  void detectNumberOfMeters(){
    debugln("detectNumberOfMeters...");
    debug("MODBUS_TYPE: " );
    if (MODBUS_TYPE == MODBUS_TYPE_RS485) {
      debugln("MODBUS_TYPE_RS485");
    }else{
      debugln("MODBUS_TYPE_TCPIP");
    }

    
  
    #if MODBUS_TYPE == MODBUS_TYPE_RS485
    
      uint16_t registerData[4];
      //Find number of meters, 4 max number for now.
      for (int i = 1; i <= maxNumberOfMeters; i++) {
        // Read Serial number registers 70 and 71
        if (modbus_read_registers_rs485(i, 70, 2, registerData)) {  // i is the Modbus slave ID
          uint32_t combinedValue = combineRegistersToInt32(registerData[0], registerData[1]);
          //Serial.print("Serial Number: ");
          //Serial.println(combinedValue);
          //Update the number of meters if able to read its serial number
          numberOfMeters = i;

        } else {
          Serial.println("Error reading meter: " + String(i));
          break;  // no more meters after first gap, exit the loop
        }
      } 
  
    #else
    
      // MODBUS_TYPE = MODBUS_TYPE_TCPIP
      // TCPIP is only used for handheld meters, there will always just be one.
      numberOfMeters = 1;
    
    #endif
    
    Serial.println("Number of meters detected: " + String(numberOfMeters)); 

    Serial.println("");
  }
  
  
  // One block read on whichever Modbus transport this build uses.
  // The #if stays inside so callers do not repeat the RS485/TCP split.
  bool readMeterRegisters(int meterNumber, uint16_t registerNumber, uint16_t count, uint16_t* data) {
#if MODBUS_TYPE == MODBUS_TYPE_RS485
    return modbus_read_registers_rs485(meterNumber, registerNumber, count, data);
#else
    return modbus_read_registers_tcpip(registerNumber, count, data);
#endif
  }

  // Register words to text. Each word holds two characters, high byte first; a NUL ends it.
  // Only printable ASCII other than '"' and '\' is kept, because the string goes
  // straight into the hand-built JSON in postToAmpXPortal2().
  String decodeModbusString(const uint16_t* words, int count) {
    String text = "";
    for (int i = 0; i < count; i++) {
      char pair[2] = { (char)(words[i] >> 8), (char)(words[i] & 0xFF) };
      for (int j = 0; j < 2; j++) {
        if (pair[j] == '\0') {
          text.trim();
          return text;
        }
        if (pair[j] >= 0x20 && pair[j] <= 0x7E && pair[j] != '"' && pair[j] != '\\') {
          text += pair[j];
        }
      }
    }
    text.trim();
    return text;
  }

  // The model does not change while the gateway runs, so read it once per meter.
  // A failed read is retried on the next sweeps, three times at most. A meter
  // without register 60 then posts no meter_model and the portal uses its default.
  void readMeterModelOnce(int meterNumber) {
    const uint8_t maxModelAttempts = 3;
    int index = meterNumber - 1;
    if (meterModels[index].length() > 0 || meterModelAttempts[index] >= maxModelAttempts) {
      return;
    }

    uint16_t words[METER_MODEL_WORDS];
    if (!readMeterRegisters(meterNumber, METER_MODEL_REGISTER, METER_MODEL_WORDS, words)) {
      meterModelAttempts[index]++;
      debugln("Meter " + String(meterNumber) + ": model register " + String(METER_MODEL_REGISTER) + " not read (attempt " + String(meterModelAttempts[index]) + ")");
      return;
    }

    meterModels[index] = decodeModbusString(words, METER_MODEL_WORDS);
    if (meterModels[index].length() == 0) {
      meterModelAttempts[index] = maxModelAttempts;  // Register answers but is blank; do not ask again.
    }
    debugln("Meter " + String(meterNumber) + " model: " + meterModels[index]);
  }

  //JF New combined function for RS485 and TCP
  void handlePowerMeter(int meterNumber = 1) {
    uint16_t registerData[4];
    String meterPrefix = "m" + String(meterNumber) + "_";
    
     // Add serial number to JsonDoc
     /*
    if (meterNumber == 1) JsonDoc["m1_serial_number"] = m1_serial_number;
    else if (meterNumber == 2) JsonDoc["m2_serial_number"] = m2_serial_number;
    else if (meterNumber == 3) JsonDoc["m3_serial_number"] = m3_serial_number;
    else if (meterNumber == 4) JsonDoc["m4_serial_number"] = m4_serial_number;
    */
    JsonDoc[meterPrefix + "serial_number"] = meterSerialNumbers[meterNumber - 1];

    server.handleClient();
    webSocket.loop();
    readMeterModelOnce(meterNumber);
    // "_model" so the meters page maps it to id meter_model. postToAmpXPortal2()
    // skips this key; a text value under "values" makes the API reject the post.
    JsonDoc[meterPrefix + "model"] = meterModels[meterNumber - 1];


    // Loop through all register definitions in the JSON document
    for (JsonPair kv : MeterRegisterDefs.as<JsonObject>()) {
      
      //Handle WebSocket and HTTP requests, before reading the registers
      server.handleClient();
      webSocket.loop();

      //Check the WiFi Reset Button, this is used to reset the WiFi settings and start the WiFi setup hotspot.
      //Add it here to make sure the button is checked while the meter is reading.
      serviceWifiResetButton();
      
      JsonArray registerDef = kv.value().as<JsonArray>();
      
      int registerNumber = registerDef[0];
      int numRegisters = registerDef[1];
      int dataType = registerDef[2];
      String friendlyName = registerDef[3];
      String jsonKey = registerDef[4];
      bool read_success = false;
  
  
      #if MODBUS_TYPE == MODBUS_TYPE_RS485
        if (modbus_read_registers_rs485(meterNumber, registerNumber, numRegisters, registerData)) {
          read_success=1;
        }
      #else
        if(modbus_read_registers_tcpip(registerNumber, numRegisters, registerData)){
          read_success=1;
        }
      #endif
    
  
      if (read_success){
        // Special handling for serial number
        if (registerNumber == 70) {
          uint32_t serialNum = combineRegistersToInt32(registerData[0], registerData[1]);
          /*
          if (meterNumber == 1) m1_serial_number = String(serialNum);
          else if (meterNumber == 2) m2_serial_number = String(serialNum);
          else if (meterNumber == 3) m3_serial_number = String(serialNum);
          else if (meterNumber == 4) m4_serial_number = String(serialNum);
          */
          meterSerialNumbers[meterNumber - 1] = String(serialNum);
        } else {
          // Normal handling for other registers
          processRegisters(registerData, numRegisters, dataType, friendlyName, meterPrefix + jsonKey);
        
          // Special case for energy imported total, add a duplicate for summary field
          /*
          if (registerNumber == 2512) {
            processRegisters(registerData, numRegisters, dataType, friendlyName, meterPrefix + jsonKey + "_summary");
          }
          */
        }
      } else {
        Serial.println("Error reading register " + String(registerNumber));
#if MODBUS_TYPE == MODBUS_TYPE_TCPIP
        // A short frame or a failed write already stop()s the socket. Do not
        // open port 502 again for every remaining register in this sweep.
        if (!modbusClient.connected()) {
          break;
        }
#endif
      }
    }
  
    // Display the JSON Document for Meter
    /*
    Serial.println("=== JSON Document for Meter " + String(meterNumber) + " ===");
    String jsonString;
    serializeJsonPretty(JsonDoc, jsonString);
    Serial.println(jsonString);
    Serial.println("=== JSON Size: " + String(JsonDoc.memoryUsage()) + " bytes ===");
    Serial.println();
    */

  // Leave a good TCP session open. stop() here made the next sweep call
  // connect() again. If the meter had already half-closed, that connect()
  // leaked a W5500 socket. The session is closed in the library when a
  // read or write fails.

  }

#if MODBUS_TYPE == MODBUS_TYPE_TCPIP
// Runs while the meter reply is still arriving, so a slow register cannot
// block the WebSocket handshake on port 81 for the whole wait.
void serviceNetworkWhileModbusWaits() {
  server.handleClient();
  webSocket.loop();
  //Check the WiFi Reset Button, this is used to reset the WiFi settings and start the WiFi setup hotspot.
  //Add it here to make sure the button is checked while the meter is reading.
  serviceWifiResetButton();
}
#endif

bool meterTransportReady() {
#if MODBUS_TYPE == MODBUS_TYPE_TCPIP
  if (Ethernet.linkStatus() == LinkOFF) {
    debugln("Ethernet link is OFF. Check LAN cable.");
    return false;
  }
#endif
  return true;  // RS485 has no Ethernet link to check
}

// Not used from serviceMeters(). initEthernet() has delay(2000); calling it
// every 1s when the meter is down (cable still in) freezes HTTP/WebSocket.
// setup() already calls initEthernet() / initRS485(). A failed TCP test only
// means the meter did not accept port 502 — skip the read, try again next interval.
// If W5500 recovery is needed later, do it once on link off→on, without delay()
// in the 1s path — do not hook this back into the failed-test branch.
/*
void reconnectMeter() {
#if MODBUS_TYPE == MODBUS_TYPE_RS485
  initRS485(&Serial1, RX_PIN, TX_PIN);
  debugln("RS485 Modbus initialized");
#else
  initEthernet();
  debugln("TCPIP Modbus initialized");
#endif
}
  */

void readNextMeter() {
  if (numberOfMeters == 0) {
    return;
  }

  debug("Reading Meter ");
  debugln(currentMeterIndex);

  server.handleClient();
  webSocket.loop();
  handlePowerMeter(currentMeterIndex);
  handleWebSocket();
  server.handleClient();
  webSocket.loop();

  currentMeterIndex++;
  if (currentMeterIndex > numberOfMeters) {
    currentMeterIndex = 1;
  }
}

void serviceMeters() {
  if (!meterTransportReady()) {
    digitalWrite(LED_2_METER, LOW);
    return;
  }

  if (!modbus_test_connection()) {
    debugln("Connection test failed!");
    digitalWrite(LED_2_METER, LOW);
    //reconnectMeter();
    return;
  }

#if MODBUS_TYPE == MODBUS_TYPE_RS485
  debugln("Connection test successful! We are able to communicate with the meter with modbus over RS485!");
#else
  debugln("Connection test successful! We are able to communicate with the meter with modbus over TCPIP!");
#endif
  digitalWrite(LED_2_METER, HIGH);
  readNextMeter();
}
