/*
  Blink

  Turns the five status LEDs on at boot.
  Hold the front-panel button (GPIO 13) for more than 1 second to flash all five twice.
  A short tap does nothing. A pin stuck low at boot is ignored until it is seen high once.
  Test leds for the Open energy gateway.

Before compiling
1. Select the correct port
2. Select the correct board.
2. Set the correct serial port boad rate: 115200
3. Set the correct flash size, 4MB or 8MB
4. Set the correct partition format.
*/

//Define the status indicating LEDs pins
//                        //XIAO//WROOM
#define LED_1_POWER     12 //2   //12 //Indicates Power is on
#define LED_2_METER     14 //3   //14 //Indicates Meter is connected via Modbus
#define LED_3_WIFI      27 //4   //27 //Indicates WiFi is connected
#define LED_4_INTERNET  26 //5   //26 //Indicates Internet is connected, this is the green LED
#define LED_5_SERVER    25 //6   //25 //Indicates succesfull communication with the Server

// Front-panel reset: GPIO 13, switch to GND. Board already has 10 kΩ pull-up + 100 nF.
// INPUT_PULLUP keeps the pin high if the ribbon is unplugged. Pressed = LOW.
#define BUTTON_PIN      13

// Drive all five status LEDs together so a flash cannot leave a mixed state.
void setAllLeds(int level) {
  digitalWrite(LED_1_POWER, level);
  digitalWrite(LED_2_METER, level);
  digitalWrite(LED_3_WIFI, level);
  digitalWrite(LED_4_INTERNET, level);
  digitalWrite(LED_5_SERVER, level);
}

// Two off/on cycles, same confirmation the gateway uses before it clears WiFi.
// delay() is safe here: this sketch has no meter reads or web server to service.
void flashStatusLedsTwice() {
  for (int flash = 0; flash < 2; flash++) {
    setAllLeds(LOW);
    delay(200);
    setAllLeds(HIGH);
    delay(200);
  }
}

// Same hold detector as serviceWifiResetButton(), with a 1 s threshold.
// Stuck-low at boot stays disarmed until the pin is seen high once.
// Fires once per hold; release re-arms the next flash.
void serviceButtonFlash() {
  static bool armed = false;
  static bool holding = false;
  static bool flashed = false;
  static unsigned long lowSince = 0;

  if (digitalRead(BUTTON_PIN) == HIGH) {
    armed = true;
    holding = false;
    flashed = false;
    return;
  }
  if (!armed || flashed) {
    return;
  }
  if (!holding) {
    holding = true;
    lowSince = millis();
    return;
  }
  // "More than 1 second": a tap under that leaves the LEDs alone.
  if (millis() - lowSince <= 1000) {
    return;
  }
  Serial.println("GPIO13 held > 1 s, flashing LEDs");
  flashStatusLedsTwice();
  flashed = true;
}

void setup() {
  // initialize LED status pins as outputs.
  pinMode(LED_1_POWER, OUTPUT);
  pinMode(LED_2_METER, OUTPUT);
  pinMode(LED_3_WIFI, OUTPUT);
  pinMode(LED_4_INTERNET, OUTPUT);
  pinMode(LED_5_SERVER, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200); // Initialize serial communication
  delay(2000); //Wait for the serial port te become available.
  Serial.println("");
  Serial.println("");

  Serial.print("LED_1_POWER: ");
  Serial.println(LED_1_POWER);

  Serial.print("LED_2_METER: ");
  Serial.println(LED_2_METER);

  Serial.print("LED_3_WIFI: ");
  Serial.println(LED_3_WIFI);

  Serial.print("LED_4_INTERNET: ");
  Serial.println(LED_4_INTERNET);

  Serial.print("LED_5_SERVER: ");
  Serial.println(LED_5_SERVER);

  Serial.print("BUTTON_PIN: ");
  Serial.println(BUTTON_PIN);
  Serial.println("");

  Serial.print("Flash Chip Size: ");
  Serial.println(ESP.getFlashChipSize()); // Get flash size in bytes

  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());

  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());

  Serial.print("Chip Cores: ");
  Serial.println(ESP.getChipCores());

  setAllLeds(HIGH);

  // GPIO 0 is the DevKit BOOT button (also active-low). Serial only, so a BOOT
  // press is not mistaken for the front-panel switch on GPIO 13.
  pinMode(0, INPUT_PULLUP);

  Serial.println("Watch this Serial at 115200. Front button should print GPIO13=0.");
  Serial.println("If only GPIO0 changes, you pressed BOOT on the DevKit, not the panel.");
}


void loop() {
  // Active-low: closed switch pulls GPIO 13 to GND (schematic R1 10 kΩ to 3.3 V).
  int gpio13 = digitalRead(BUTTON_PIN);
  int gpio0 = digitalRead(0);

  static int last13 = -1;
  static int last0 = -1;
  static unsigned long lastHeartbeat = 0;

  // Print only on change so a working press is obvious in the monitor.
  if (gpio13 != last13) {
    Serial.print("GPIO13=");
    Serial.print(gpio13);
    Serial.println(gpio13 == LOW ? " PRESSED" : " released");
    last13 = gpio13;
  }
  if (gpio0 != last0) {
    Serial.print("GPIO0=");
    Serial.print(gpio0);
    Serial.println(gpio0 == LOW ? " (DevKit BOOT pressed)" : " (DevKit BOOT released)");
    last0 = gpio0;
  }
  if (millis() - lastHeartbeat >= 1000) {
    lastHeartbeat = millis();
    Serial.print("heartbeat GPIO13=");
    Serial.print(gpio13);
    Serial.print(" GPIO0=");
    Serial.println(gpio0);
  }

  // Front panel only. GPIO 0 stays a serial check so a DevKit BOOT press is not mistaken for the panel.
  serviceButtonFlash();
}
