//NVS  Non-Volatile Storage (Local Permanent Storage)


//Function to load gateway ID from NVS storage
void loadGatewayId() {
    if (preferences.isKey("gateway_id")) {
      GATEWAY_ID = preferences.getInt("gateway_id", DEFAULT_GATEWAY_ID);
      debug("Loaded Gateway ID from NVS: ");
      debugln(GATEWAY_ID);
    } else {
      // First time setup - save default value
      preferences.putInt("gateway_id", DEFAULT_GATEWAY_ID);
      GATEWAY_ID = DEFAULT_GATEWAY_ID;
      debug("First time setup - saved default Gateway ID: ");
      debugln(GATEWAY_ID);
    }
  }
  
  //Function to save gateway ID to NVS storage
  void saveGatewayId(int newGatewayId) {
    preferences.putInt("gateway_id", newGatewayId);
    GATEWAY_ID = newGatewayId;
    debug("Saved new Gateway ID to NVS: ");
    debugln(GATEWAY_ID);
  }