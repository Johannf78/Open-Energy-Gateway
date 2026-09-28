//Before compiling....
//1. Make sure the correct connection type is set for the gateway, either RS485 or TCPIP, MODBUS_TYPE
//2. Set the flash size correctly to 8MB
//3. Partition scheme: custom (partitions.csv — dual OTA app slots). Alternative: "8M with spiffs (3MB APP/1.5MB SPIFFS)"
//4. Set the API to use the local server or the remote server. #define USE_LOCAL_SERVER true or false
//5. Set debug to enabled or disabled. Disable for live to increase speed, but one also lose the debug info.
//6. Select the correct COM port, check by using the function "Get Board Info"
//7. Set the serial port baud rate to: 115200 baud.
//8. Select the correct board: "ESP32 Dev Module" (WROOM-32U / WROVER field units) or "ESP32S3 Dev Module" (S3-WROOM-1U bench port)
//   ESP32S3 Dev Module settings: USB CDC On Boot = Disabled (use the "UART" USB socket of the DevKitC-1U for flashing + monitor),
//   USB Mode = Hardware CDC and JTAG, Flash Size = 8MB or 16MB (match module: N8R8 / N16R8), PSRAM = OPI PSRAM (R8 modules),
//   Partition Scheme = Custom (sketch partitions.csv is picked up automatically), Flash Mode = QIO 80MHz, Upload Speed 921600.
//   First flash of a new S3 board: Tools -> Erase All Flash Before Sketch Upload = Enabled, then set it back to Disabled.
//   Pin map is selected automatically from the board (CONFIG_IDF_TARGET_*) - see "Board pin map" below.
//9. OTA: Admin pulls https://ampx.app/firmware/ampx_open_energy_gateway.bin (first flash via USB with OTA partitions)
//   An S3 build must never be published at that single URL - S3 images go under targets.esp32s3 in version.json (see CHIP_OTA_KEY).


//There will be two variants of this gateway, one working with Modbus over RS485 and the other
//working with mobus over TCP/IP, this is setup here and used depeding on what is needed.
#define MODBUS_TYPE_RS485 1
#define MODBUS_TYPE_TCPIP 2
//Set the required Modbus type variant here to either RS485 or TCPIP
//In other words, change this depending on if the board is for RS485 or for TCPIP
//For TCPIP use MODBUS_TYPE_TCPIP, for RS485 use MODBUS_TYPE_RS485
#define MODBUS_TYPE MODBUS_TYPE_TCPIP

//NB, Also remember to change the API server from local to live if needed...

// Bump when publishing a new .bin to ampx.app/firmware/
#define FIRMWARE_VERSION "1.2.2"
//To publish OTA for gateways in the field
//Bump FIRMWARE_VERSION
//Use Sketch → Export compiled Binary (not the Upload / play button)
//FTP Copy that .bin into ampx.app\firmware\ as ampx_open_energy_gateway.bin
//Update version.json in that same folder and deploy

// OTA catalog key must match targets.<chip> in firmware/version.json.
// Derived from the board you compiled for so an ESP32 build never downloads an S3 image (and the reverse).
#if CONFIG_IDF_TARGET_ESP32S3
#define CHIP_OTA_KEY "esp32s3"
#elif CONFIG_IDF_TARGET_ESP32
#define CHIP_OTA_KEY "esp32"
#else
#error "Unsupported chip for OTA; add CHIP_OTA_KEY"
#endif

//Defining custom partiotions.
//Custom partitions is defined in the file partitions.csv
//This file can be edited using the online tool: https://thelastoutpostworkshop.github.io/ESP32PartitionBuilder/
//https://www.youtube.com/watch?v=EuHxodrye6E


//Unique Gateway ID for each gateway manufactured. To be used when adding it to a the portal under a specific user.
//This is set when manufactured and will be unique for each gateway.
//Format: 100001 increment.

//Default value - can be changed via admin web interface
#define DEFAULT_GATEWAY_ID 100001

//Global variable to store the current gateway ID (loaded from NVS)
int GATEWAY_ID = DEFAULT_GATEWAY_ID;
const char* ADMIN_PASSWORD = "1000";

/*
The following .ino files should be in the same directory as this main .ino file (AmpX-Energy-Gateway.ino).
  ampx_functions.ino
  ampx_functions_modbus.ino
  ampx_functions_network.ino
  ampx_functions_web.ino

These files are automatically included and merged with this main .ino file.
This just seperates all the functions and make this file easier to read.


For The ESP32 Node32s board do the following:
Go to File -> Preferences.
in the "Additional Boards Manager URLs"
Add the following String:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Then in the boards manager, search for: "esp32 by Espressif Systems"
Then install the board manager.

Now select the port and then 
Select "node32s" under the boards.

HTTP OTA (Admin): downloads ampx_open_energy_gateway.bin from ampx.app/firmware/
Requires dual-app OTA partitions (this sketch uses custom partitions.csv on 8MB flash).

ESP Board to be selected in Arduino IDE:
for ESP32-WROOM-32U - DUBEUYEW
  node32S
  Partition: 4MB (OTA not supported in this pass)
for ESP32-WROVER-IE-N8R8-DEVKITC-VE Espressif / 8MB modules
  ESP32 Dev Module
  Flash Size: 8MB
  Partition: custom (partitions.csv) or "8M with spiffs (3MB APP/1.5MB SPIFFS)"
*/
//Required to communicate with the RS485 Controller.
#include <HardwareSerial.h>

//Used to connect to the local WiFi network, Built-in library that comes with the ESP32 Arduino core
#include <WiFi.h>

//Install the library by tzapu, https://github.com/tzapu/WiFiManager, https://www.youtube.com/watch?v=Errh7LEEug0
#include <WiFiManager.h> //This is used to dymanically configure the wifi connection .


//ESP32 Multicast DNS (mDNS) library for ESP32.
//Built into the ESP32 Arduino core, so you don't need to install it separately from the Library Manager
//https://github.com/espressif/arduino-esp32/tree/master/libraries/ESPmDNS
#include <ESPmDNS.h>

// HTTP pull OTA (Admin). ArduinoOTA intentionally disabled to save flash.
#include <Update.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

// Arduino loop() defaults to 8KB. HTTPS (mbedTLS) plus HTTPClient on that task
// overflowed the stack and caused TG1WDT_SYS_RESET during OTA manifest fetch.
SET_LOOP_TASK_STACK_SIZE(16384);

//Time library for NTP synchronization
#include <time.h>

//Search for Preferences, Install the one by Vlodomyr Shymanskyy, https://github.com/vshymanskyy/Preferences
//Tested and working with version 2.1.0
//NVS  Non-Volatile Storage (Local Permanent Storage)
#include <Preferences.h>

//Creates a web server and serves a webpage with all the meter readings and settings.
#include <WebServer.h>

//Search for Arduino Websockets, install the one WebSockets by Markus Sattler
//Is the ESP Async WebServer@3.8.1 not maybe a better option - JF
#include <WebSocketsServer.h>
//Tested on version 2.7.1

//Search for ArduinoJson, install the one by Benoit Blanchon
#include <ArduinoJson.h>

//Built-in ESP32 library that provides HTTP client functionality for making HTTP requests to web servers and APIs.
#include <HTTPClient.h>

//SPIFFS SPI Flash File System, this is used to store the web pages in the flash memory of the ESP32.
//This adds about 30% to the 1MB program file but moves and loads the html files from the SPIFFS memomory
//#include "SPIFFS.h"

//The HTML code for the webpages are stored in a seperate file, this makes the code easier to read.
#include "web_home.h" 
#include "web_settings.h" 
#include "web_admin.h" 
#include "web_meters.h" 
#include "web_inc_header.h" 

//Define the meter registers and datatypes here in json format.
#include "meter_registers.h"





//This is an easy way to exclude all serial.print commands from production code to reduce the file size.
//Change this varialbe to enable or disable debugging
#define DEBUG 1

#if DEBUG == 1
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif




//=====================================================================================
// Board pin map - selected by the chip you compile for (Tools -> Board).
//   ESP32    : "ESP32 Dev Module"   -> ESP32-WROOM-32U / ESP32-WROVER-IE DevKitC (V2 PCB, field units)
//   ESP32-S3 : "ESP32S3 Dev Module" -> ESP32-S3-WROOM-1U DevKitC-1U (bench port, Sep 2026)
// Same sketch, same OTA catalog key logic (CHIP_OTA_KEY above). Do not mix the two maps.
//=====================================================================================
#if CONFIG_IDF_TARGET_ESP32S3
  // ESP32-S3-DevKitC-1U. Avoid: 0/3/45/46 (strapping), 19/20 (USB), 26-32 (flash),
  // 35/36/37 (octal PSRAM on N8R8/N16R8), 38 and 48 (on-board RGB LED on v1.1 / v1.0), 43/44 (UART0).
  // Status LEDs - all on the J1 header side, contiguous, so one 6-way ribbon (5 LED + GND) reaches the front board.
  #define LED_1_POWER     4
  #define LED_2_METER     5
  #define LED_3_WIFI      6
  #define LED_4_INTERNET  7
  #define LED_5_SERVER    15
  // W5500 / W5500-Lite on FSPI (the S3 default SPI pins: SCK 12, MISO 13, MOSI 11, SS 10)
  #define ETH_SPI_SCK_PIN   12  // Yellow
  #define ETH_SPI_MISO_PIN  13  // Orange
  #define ETH_SPI_MOSI_PIN  11  // Blue
  #define ETH_SPI_SCS_PIN   10  // Green  (nSS on the module)
  //#define ETH_SPI_RST_PIN 14  // Optional: module RST. Leave COMMENTED OUT unless RST is physically wired -
                                // defining it makes initEthernet() pulse the pin. The W5500 has its own pull-up.
  // RS485 (MAX485) - UART1. Reserved now so the combined RS485+Ethernet board needs no re-pinning.
  #define MAX485_DE       16  // White
  #define MAX485_RE_NEG   16
  #define RX_PIN          18  // RO Orange
  #define TX_PIN          17  // DI Yellow
#else
  // Classic ESP32 (ESP32-WROOM-32U / WROVER DevKitC) - unchanged field pin map.
  #define LED_1_POWER     12 //Indicates Power is on
  #define LED_2_METER     14 //Indicates Meter is connected via Modbus
  #define LED_3_WIFI      27 //Indicates WiFi is connected
  #define LED_4_INTERNET  26 //Indicates Internet is connected, this is the green LED
  #define LED_5_SERVER    25 //Indicates succesfull communication with the Server
  // W5500 on VSPI (hardware SPI default pins: SCK 18, MISO 19, MOSI 23)
  #define ETH_SPI_SCK_PIN   18  // Yellow
  #define ETH_SPI_MISO_PIN  19  // Orange
  #define ETH_SPI_MOSI_PIN  23  // Blue
  #define ETH_SPI_SCS_PIN   5   // Green  (nSS on the module)
  // RS485 (MAX485)
  #define MAX485_DE       4   // White
  #define MAX485_RE_NEG   4
  #define RX_PIN          16  // RO Orange
  #define TX_PIN          17  // DI Yellow
#endif
//Modbus A, Positive, Green.  Modbus B, Negative, Blue.  W5500: Red VCC 3V3, Black GND.


//Include AmpX custom written libraries for Modbus
//Saved in the D:\OneDrive\JF Data\UserData\Documents\Arduino\libraries folder
#if MODBUS_TYPE == MODBUS_TYPE_RS485

  //Custom written AmpX Modbus library for RS485
  //Saved in libary folder. On my pc:
  //On Windows: D:\OneDrive\JF Data\UserData\Documents\Arduino\libraries\ampx_modbus_rs485\src\ampx_modbus_rs485.cpp
  //On Linux: /home/username/Arduino/libraries/ampx_modbus_tcpip-main/src
  #include <ampx_modbus_rs485.h>
  // RS485 pins: see board pin map above (MAX485_DE / MAX485_RE_NEG / RX_PIN / TX_PIN)

#else

  //Custom written AmpX Modbus library for TCPIP
  //Saved in libary folder. On my pc: D:\OneDrive\JF Data\UserData\Documents\Arduino\libraries\ampx_modbus_tcpip\src\ampx_modbus_tcpip.cpp
  #include <ampx_modbus_tcpip.h>
  #include <SPI.h>
  // Ethernet pins: see board pin map above (ETH_SPI_*). SPI.begin(SCK, MISO, MOSI, CS) is called
  // explicitly in initEthernet() so the bus pins are the same on every board, not whatever the core's default is.

  //Define Ethernet Settings, Mac and IPs
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0x94, 0xB5};
  //IPAddress pc_ip(192, 168, 1, 32);   // PC IP, Assign a static IP to your PC and change it here to be the same.
  IPAddress ip(192, 168, 1, 50);     // Arduino IP
  IPAddress gateway(192, 168, 1, 1);  // Network gateway
  IPAddress subnet(255, 255, 255, 0); // Subnet mask
  IPAddress meter_ip(192, 168, 1, 55); // Energy meter IP
  //IPAddress meter_ip(192, 168, 2, 122); // Energy meter IP
#endif


//Status LED pins: see board pin map above (LED_1_POWER .. LED_5_SERVER).


//Constant data types, used in the processRegisters function.
//These are now defined in meter_registers.h
//const int dataTypeInt32 = 1;
//const int dataTypeInt64 = 2;
//const int dataTypeFloat = 3;

//NVS  Non-Volatile Storage (Local Permanent Storage)
Preferences preferences;

//Web server and websocket
#define HTTP 80
WebServer server(HTTP);
WebSocketsServer webSocket = WebSocketsServer(81);


//TODO: Improve this code use an array and do not use strings.
/*
String m1_serial_number = "";  // Meter one serial number
String m2_serial_number = "";
String m3_serial_number = "";
String m4_serial_number = "";  
*/

int numberOfMeters = 0;  // Number of meters connected, this will automatically be updated based on the number of meters detected
const int maxNumberOfMeters = 10;  // Maximum number of meters supported, this is used to limit the number of meters that can be displayed on the web page.
String meterSerialNumbers[maxNumberOfMeters]; // Array to store the serial numbers of the meters.
int currentMeterIndex = 1;  // Track which meter to read next (for staggered reading)




//The JsonDocument is used to send data to the websocket....
//StaticJsonDocument<512> JsonDoc;          //Define optimized JSON documents with minimum required size. StaticJsonDocument allocates memory on the stack (fixed at compile time)
DynamicJsonDocument JsonDoc(2048);        //DynamicJsonDocument allocates memory on the heap (dynamic at runtime)

//JSON document for meter register definitions
JsonDocument MeterRegisterDefs;

bool readSerial = false;
bool onBoot = true; //this is used to check if the device the first time it starts up, used in the loop to send read and send data immediately.


//EMONCMS, Remote energy logging, https://JsonDocs.openenergymonitor.org/emoncms/index.html
//TODO This needs to be removed, can forward to other portal from ampx portal
/*Commented out to save space
const char* emoncms_server = "http://emoncms.org";
const char* api_key = "c0526f06893d1063800d3bb966927711"; //your_API_KEY
*/

//AmpX Energy Portal, Remote energy logging. Use IP address of the server if using local server(no dns server).
// LAN IP of PC running XAMPP (port 80). Do not use :8080 unless Docker API is running.
// Note: https://ampx.app is the public site; local vhost is ampx-app.local (ESP should use IP).
const char* ampxportal_server_local = "http://192.168.2.120/api/v3/";
const char* ampxportal_server_live = "https://ampx.app/api/v3/";
// Both local and live URLs are /api/v3/ (Cloud Serverless). /api/v2/ remains on Hetzner for Influx 2 until sunset.

// Shared secret for AmpX Portal API (header X-AmpX-Api-Key). Must match AMPX_API_KEY in api/config/config.php.
const char* ampxportal_api_key = "b9e96dfb8f9c722b917f1c536ec67c373e5644eee75f267dce94b7294f0274b7";

// Which API to use local or live? - set to true for local development, false for live
#define USE_LOCAL_SERVER false

// Firmware OTA hosting (same local/live switch as API)
#if USE_LOCAL_SERVER
const char* firmwareURL = "http://192.168.2.120/firmware/ampx_open_energy_gateway.bin";
const char* firmwareManifestURL = "http://192.168.2.120/firmware/version.json";
#else
const char* firmwareURL = "https://ampx.app/firmware/ampx_open_energy_gateway.bin";
const char* firmwareManifestURL = "https://ampx.app/firmware/version.json";
#endif

struct FirmwareManifest {
  bool ok = false;
  String version;
  String url;
  String error;
};

// Cached OTA status — filled in loop(), never via HTTPClient inside a WebServer handler.
// Do not fetch from a FreeRTOS side task: WiFiClientSecure + Arduino String across cores
// caused LoadProhibited / corrupted backtrace reboot loops on 1.0.3.
struct OtaStatusCache {
  bool ready = false;
  bool ok = false;
  String available = "Not checked yet";
  String status = "Not checked yet";
  bool upToDate = false;
  bool updateAvailable = false;
  String error = "";
};
OtaStatusCache otaStatusCache;
// Only GET /ota_status?refresh (Check for update) requests a check — not Admin page load, not boot.
volatile bool otaManifestCheckRequested = false;
unsigned long otaManifestLastCheckMs = 0;

//Function prototypes, it needs to be here because it is used in the setup function.
//one needs to add a forward declaration for this function as well, as it is defined in a seperate .ino file:functions.ino
void handlePowerMeter(int meterNumber);
void postToAmpXPortal2(int meterNumber);
void setupMeterRegisters();
void detectNumberOfMeters();
void handleWebSocket();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void initNTP();
String getCurrentTimestamp();
void loadGatewayId();
void saveGatewayId(int newGatewayId);
void handleUpdateGatewayId();
void handleUpdate();
void handleAdmin();
void handleOtaStatus();
void initmDNS();
void initOTA();
bool doOTAUpdate();
void saveOtaStatus(const String& status);
String getOtaStatusText();
String getOtaTimeText();
bool fetchFirmwareManifest(FirmwareManifest& out);
bool isNewerVersion(const String& serverVer, const String& deviceVer);
void serviceOtaManifestCheck();
void requestOtaManifestCheck();
void handleClearWifi();
void clearStoredWifi();
void serviceMeters();
#if MODBUS_TYPE == MODBUS_TYPE_TCPIP
void serviceNetworkWhileModbusWaits();
#endif


void setup() {
  // initialize LED status pins as outputs.
  pinMode(LED_1_POWER, OUTPUT);
  pinMode(LED_2_METER, OUTPUT);
  pinMode(LED_3_WIFI, OUTPUT);
  pinMode(LED_4_INTERNET, OUTPUT);
  pinMode(LED_5_SERVER, OUTPUT);

  //Indicate that the power is on with a LED
  digitalWrite(LED_1_POWER, HIGH);
  //Turn off all the other LEDs (It could happen that the LED got stuck on the on state after last reset).
  digitalWrite(LED_2_METER, LOW);
  digitalWrite(LED_3_WIFI, LOW);
  digitalWrite(LED_4_INTERNET, LOW);
  digitalWrite(LED_5_SERVER, LOW);


  Serial.begin(115200); // Debug serial
  while (!Serial) {
    delay(10); // Wait for serial port to become ready.
  }
  delay(1200); //Wait some more for the serial port to become ready...
                //1200 Ok, but increase if debugging and resetting by cycling the power.
  debugln("Serial port ready. Begin setup...");
  debug("Firmware version: ");
  debugln(FIRMWARE_VERSION);

  
  // Initialize NVS  Non-Volatile Storage (Local Permanent Storage)
  initNvs();  

  // Load gateway ID from NVS storage
  loadGatewayId();
  
  // Debuging information
  debug("GATEWAY_ID: ");
  debugln(GATEWAY_ID);

  //Depending on the Modbus type set, initialise either RS485 or TCPIP
  #if MODBUS_TYPE == MODBUS_TYPE_RS485
    initRS485(&Serial1, RX_PIN, TX_PIN);
    debugln("RS485 Modbus initialized");
  #else
    initEthernet();
    debugln("TCPIP Modbus initialized");
  #endif
  
  Serial.println("Setup complete. Starting communication...");
  
  // Initialize WiFi
  initWiFi(); //Program will not continue unless WiFi is connected..

  // Initialize mDNS for .local domain access
  initmDNS();

  // Initialize NTP time synchronization (must be after WiFi)
  initNTP();

  //Initialise the local web server.
  initServer();

#if MODBUS_TYPE == MODBUS_TYPE_TCPIP
  // Meter reads wait up to 2s. Service HTTP/WebSocket during that wait.
  modbus_set_idle_callback(serviceNetworkWhileModbusWaits);
#endif

  // Initialize meter register definitions
  setupMeterRegisters();

  //Detect number of meters and set global variable, numberOfMeters.
  detectNumberOfMeters();

  // OTA: manifest checks run in loop() when Admin requests them (see serviceOtaManifestCheck).
  //initOTA();
}

void loop() {
  //JF commented out to reduce size
  //ArduinoOTA.handle();

  static unsigned long counter1 = 0;
  static unsigned long counter2 = 0;
  static unsigned long counter3 = 0;
  
  //Interval to test the meter connection and read the parameters, and update the local web page.
  const unsigned long METER_CONNECTION_INTERVAL = 1000;    // 1 second (staggered reading: 5 meters × 1s = 5s cycle)
  //Interval to post the meter data to the remote server
  const unsigned long REMOTE_SERVER_INTERVAL = 30000;    // 30 seconds
  //Interval to reboot the ESP32
  const unsigned long REBOOT_INTERVAL = 86400000;    // 24 hours

  //Built-in Arduino function that returns the number of milliseconds since the Arduino board began running the current program.
  unsigned long now = millis();

  //Test meter connection and read the parameters every METER_CONNECTION_INTERVAL milliseconds
  //If onBoot is true, read and send data immediately on boot.
  if (onBoot == true || now - counter1 > METER_CONNECTION_INTERVAL) {
    onBoot = false;
    serviceMeters();
    counter1 = now;
  }
 

  // Post meter data to remote server every 5 minutes, 300000
  //TODO: JF 2025-06-08 for testing, i decreased it to 30 seconds (30000).
  if (now - counter2 > REMOTE_SERVER_INTERVAL) {
    //Post meter data to remote server
    for (int i = 1; i <= numberOfMeters; i++) {
      //postToEmonCMS(i);
      postToAmpXPortal2(i);
    }
    counter2 = now;
  }

  //Reboot every 24 hours to ensure it keeps working, 86400000
  if (now - counter3 > REBOOT_INTERVAL) {
    //Reboot the ESP32
    //ESP.restart();
    counter3 = now;
  }


  // OTA manifest refresh on loop task (same core as WebServer) — avoids cross-core String/HTTPS crashes.
  serviceOtaManifestCheck();

  //These functions must run continuesly, so one can not include a delay in the main loop.
  server.handleClient();  //Handle webserver requests from client
  webSocket.loop();       //Handle websocket requests from client
}


