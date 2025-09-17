//Used to connect to the local WiFi network, Built-in library that comes with the ESP32 Arduino core
#include <WiFi.h>

//Install the library by tzapu, https://github.com/tzapu/WiFiManager, https://www.youtube.com/watch?v=Errh7LEEug0, Tested with version 2.0.17
#include <WiFiManager.h> //This is used to dymanically configure the wifi connection .

//Tested with 
//Arduino IDE 2.3.6
//esp by Esprissif Systems version 3.1.3 Boards manager. 


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


//Define the status indicating LEDs pins
//                        //C3  //WROOM
#define LED_1_POWER     12 //2   //12 //Indicates Power is on 
#define LED_2_METER     14 //3   //14 //Indicates Meter is connected via Modbus
#define LED_3_WIFI      27 //4   //27 //Indicates WiFi is connected
#define LED_4_INTERNET  26 //5   //26 //Indicates Internet is connected, this is the green LED
#define LED_5_SERVER    25 //6   //25 //Indicates succesfull communication with the Server



void initWiFi() {
  
  debugln("Inside initWiFi function. Top of function");
  
  WiFi.setHostname("AmpX-Energy-Gateway");

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
  if (!wifiManager.autoConnect("AmpX-Energy-Gateway-AP", "")) {
    debugln("Failed to connect and hit timeout...");
    //reset and try again, or maybe put it to deep sleep
    // ESP.restart();
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




void setup() {

    // initialize LED status pins as outputs.
  pinMode(LED_1_POWER, OUTPUT);
  pinMode(LED_2_METER, OUTPUT);
  pinMode(LED_3_WIFI, OUTPUT);
  pinMode(LED_4_INTERNET, OUTPUT);
  pinMode(LED_5_SERVER, OUTPUT);

 
  Serial.begin(9600); // Debug serial
  while (!Serial) {
    delay(10); // Wait for serial port to become ready.
  }
  delay(1200); //Wait some more for the serial port to become ready...
  debugln("Serial port ready. Begin setup...");

  initWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:

}
