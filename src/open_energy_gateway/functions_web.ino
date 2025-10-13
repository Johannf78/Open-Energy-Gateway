//Initialise the server and websocket
void initServer() {
  server.on("/", handleRoot);
  // Commented out to reduce space
  // server.on("/update", HTTP_POST, handleUpdate);
  server.on("/settings", handleSettings);
  server.on("/admin", handleAdmin);
  server.on("/update_meters_name", HTTP_POST, handleChangeMetersName);
  server.on("/update_gateway_id", HTTP_POST, handleUpdateGatewayId);  // Add this line
  server.begin();
  //Initialise the websockets on port 81
  webSocket.begin();
}

//Handle the root webpage
void handleRoot() {
  //String html = "<h1>AmpX Open Energy Gateway</h1>";
  //String vol = "<h1>Voltage on L1: " + String(voltage_on_L1, 2) + "(V)</h1>";
  //html += vol;
  //the String webpage has been defined in the included file webpage.h

  //Replace the string m1_serial_number with the actual serial number, done here as it does not update regularly like values.
  /*
  webpage.replace("m1_serial_number", m1_serial_number);
  webpage.replace("m2_serial_number", m2_serial_number);
  webpage.replace("m3_serial_number", m3_serial_number);
  webpage.replace("m4_serial_number", m4_serial_number);
  */

  //This must be a server side replacement, as the javascript reacts based on this value.
  webpage.replace("numberOfMetersValue", String(numberOfMeters));

  /*
  for (int i = 1; i <= maxNumberOfMeters; i++)
  {
    String meterPrefix = "m" + String(i) + "_";
    //Copy the saved meter name from the preferences to the json doc, this is so that the name can be displayed on the page, even if the name has been changed recently.
    JsonDoc[meterPrefix + "name"] = preferences.getString(meterPrefix + "name");
  }
  */
  JsonDoc["m1_name"] = preferences.getString("m1_name");
  JsonDoc["m2_name"] = preferences.getString("m2_name");
  JsonDoc["m3_name"] = preferences.getString("m3_name");
  JsonDoc["m4_name"] = preferences.getString("m4_name");

  server.send(200, "text/html", webpage);
}

//Handle the settings webpage
void handleSettings()
{
  String page = webpage_settings;

  // Only replace numberOfMetersValue (needed for JavaScript)
  page.replace("numberOfMetersValue", String(numberOfMeters));

  // Add static values to JsonDoc for WebSocket transmission
  JsonDoc["m_gateway_id"] = String(GATEWAY_ID);
  JsonDoc["m_connected_meters_num"] = String(numberOfMeters);
  
  // Add meter names to JsonDoc
  JsonDoc["m1_name"] = preferences.getString("m1_name");
  JsonDoc["m2_name"] = preferences.getString("m2_name");
  JsonDoc["m3_name"] = preferences.getString("m3_name");
  JsonDoc["m4_name"] = preferences.getString("m4_name");
  
  // Add serial numbers to JsonDoc (already done in functions_meter.ino)
  JsonDoc["m1_serial_number"] = m1_serial_number;
  JsonDoc["m2_serial_number"] = m2_serial_number;
  JsonDoc["m3_serial_number"] = m3_serial_number;
  JsonDoc["m4_serial_number"] = m4_serial_number;

  // Add WiFi RSSI to JsonDoc
  int rssi = WiFi.RSSI();
  int percentage = (int)((float)(rssi + 100) * 1.4286);
  if (percentage < 0) percentage = 0;
  else if (percentage > 100) percentage = 100;
  JsonDoc["m_wifi_rssi"] = String(rssi) + "dBm (" + String(percentage) + "%)";

  server.send(200, "text/html", page);
}

//Handle the admin webpage
void handleAdmin()
{
  //Handle the admin page. Change the gateway ID, Server and API key, and other settings only ment for admin.
  //ToDo: Not sure if this is needed, as it can just be hard coded... Lets see...
  String page = webpage_admin;

  //Replace the gateway ID placeholder with the actual value
  page.replace("m_gateway_id", String(GATEWAY_ID));

  //send the page to the client
  server.send(200, "text/html", page);
}

void handleWebSocket() {
  String JsonString;

  serializeJson(JsonDoc, JsonString);
  //Send the JSON document to the websocket.
  webSocket.broadcastTXT(JsonString);

  //Serial.println("Sent JSON to websocket");
  //Serial.println(JsonString);
  // Pretty print with indentation of 2 spaces
  //serializeJsonPretty(JsonDoc, Serial);
}

void handleChangeMetersName() {
  String m1_name = server.arg("m1_name");
  preferences.putString("m1_name", m1_name);
  Serial.println("m1_name: " + m1_name);

  String m2_name = server.arg("m2_name");
  preferences.putString("m2_name", m2_name);
  Serial.println("m2_name: " + m2_name);

  String m3_name = server.arg("m3_name");
  preferences.putString("m3_name", m3_name);
  Serial.println("m3_name: " + m3_name);

  String m4_name = server.arg("m4_name");
  preferences.putString("m4_name", m4_name);
  Serial.println("m4_name: " + m4_name);

  //Send success response
  String successPage = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Meter Names Updated</title>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <style>
        body { background-color: #EEEEEE; font-family: Arial, sans-serif; }
        main { margin: auto; border: 3px solid black; padding: 20px; }
        .success { color: green; font-weight: bold; }
      </style>
    </head>
    <body>
      <main>
        <h1>Meter Names Updated Successfully</h1>
        <p class="success">Gateway ID has been updated to: )" + String(GATEWAY_ID) + R"(</p>
        <p><a href="/settings">Back to Settings</a> | <a href="/">Home</a></p>
      </main>
    </body>
    </html>
  )";

  server.send(200, "text/html", successPage);
}

//Handle the gateway ID update form submission
void handleUpdateGatewayId() {
  if (server.hasArg("gateway_id")) {
    String gatewayIdStr = server.arg("gateway_id");
    int newGatewayId = gatewayIdStr.toInt();
    
    //Validate the input (should be a positive integer)
    if (newGatewayId > 0) {
      saveGatewayId(newGatewayId);
      
      //Send success response
      String successPage = R"(
        <!DOCTYPE html>
        <html>
        <head>
          <title>Gateway ID Updated</title>
          <meta name='viewport' content='width=device-width, initial-scale=1'>
          <style>
            body { background-color: #EEEEEE; font-family: Arial, sans-serif; }
            main { margin: auto; border: 3px solid black; padding: 20px; }
            .success { color: green; font-weight: bold; }
          </style>
        </head>
        <body>
          <main>
            <h1>Gateway ID Updated Successfully</h1>
            <p class="success">Gateway ID has been updated to: )" + String(GATEWAY_ID) + R"(</p>
            <p><a href="/admin">Back to Admin</a> | <a href="/">Home</a></p>
          </main>
        </body>
        </html>
      )";
      server.send(200, "text/html", successPage);
    } else {
      //Invalid input
      server.send(400, "text/plain", "Invalid Gateway ID. Must be a positive integer.");
    }
  } else {
    //No gateway_id parameter
    server.send(400, "text/plain", "Missing gateway_id parameter");
  }
}
