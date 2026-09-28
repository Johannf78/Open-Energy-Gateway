// AmpX Open Energy Gateway - ESP32-S3 bench bring-up
//
// Flash this BEFORE the main firmware on a new ESP32-S3-DevKitC-1U. It proves, in order:
//   1. The board boots and the serial monitor works (chip, flash, PSRAM report)
//   2. The five status LEDs are on the right pins (walks them 1..5, then all on)
//   3. The W5500-Lite answers on SPI and reports link up/down
//   4. (optional) The meter answers on TCP port 502
// Same pin map as open_energy_gateway.ino "Board pin map" for ESP32-S3.
//
// Board: "ESP32S3 Dev Module". USB CDC On Boot: Disabled (flash + monitor via the "UART" socket).
// Flash: 8MB or 16MB to match the module. PSRAM: OPI PSRAM. Partition: any (this sketch is tiny).
// Libraries: Ethernet (Arduino, 2.0.x) only.

#include <SPI.h>
#include <Ethernet.h>

#if !CONFIG_IDF_TARGET_ESP32S3
#error "This bring-up sketch is for the ESP32-S3 pin map. Select ESP32S3 Dev Module."
#endif

// ---- Pin map (keep in sync with open_energy_gateway.ino) ----
#define LED_1_POWER     4
#define LED_2_METER     5
#define LED_3_WIFI      6
#define LED_4_INTERNET  7
#define LED_5_SERVER    15

#define ETH_SPI_SCK_PIN   12
#define ETH_SPI_MISO_PIN  13
#define ETH_SPI_MOSI_PIN  11
#define ETH_SPI_SCS_PIN   10
// #define ETH_SPI_RST_PIN 14   // uncomment only if RST is wired

// ---- Bench network (same as main firmware defaults) ----
byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0x94, 0xB5};
IPAddress ip(192, 168, 1, 50);
IPAddress gw(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress meter_ip(192, 168, 1, 55);   // Mi550 / ME440 default used in the manual is 192.168.2.122 - change if needed
const uint16_t MODBUS_PORT = 502;

const int leds[5] = {LED_1_POWER, LED_2_METER, LED_3_WIFI, LED_4_INTERNET, LED_5_SERVER};
const char* ledNames[5] = {"POWER", "METER", "WIFI", "INTERNET", "SERVER"};

bool ethOk = false;

void printChipInfo() {
  Serial.println();
  Serial.println("==== AmpX Gateway S3 bring-up ====");
  Serial.printf("Chip: %s rev %d, %d core(s), %d MHz\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash: %u MB (%s)\n", ESP.getFlashChipSize() / (1024 * 1024),
                ESP.getFlashChipMode() == FM_QIO ? "QIO" : ESP.getFlashChipMode() == FM_DIO ? "DIO" : "other");
  Serial.printf("PSRAM: %u KB total, %u KB free %s\n", ESP.getPsramSize() / 1024, ESP.getFreePsram() / 1024,
                ESP.getPsramSize() ? "" : "<-- 0 KB: PSRAM not enabled in Tools menu, or module has none");
  Serial.printf("Heap: %u KB free\n", ESP.getFreeHeap() / 1024);
  Serial.printf("Arduino core: %s\n", ESP.getSdkVersion());
  uint64_t mac64 = ESP.getEfuseMac();   // little-endian: byte 0 is the first octet
  uint8_t* m = (uint8_t*)&mac64;
  Serial.printf("eFuse MAC: %02X:%02X:%02X:%02X:%02X:%02X (same order as esptool)\n", m[0], m[1], m[2], m[3], m[4], m[5]);
  Serial.println();
}

void ledWalk() {
  Serial.println("LED walk: each LED on for 400 ms, in front-panel order");
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) digitalWrite(leds[j], j == i ? HIGH : LOW);
    Serial.printf("  LED %d %-8s GPIO %d\n", i + 1, ledNames[i], leds[i]);
    delay(400);
  }
  for (int j = 0; j < 5; j++) digitalWrite(leds[j], HIGH);
  delay(600);
  for (int j = 1; j < 5; j++) digitalWrite(leds[j], LOW);   // leave POWER on
  Serial.println();
}

void ethernetCheck() {
  Serial.printf("W5500 on FSPI: SCK %d  MISO %d  MOSI %d  CS %d\n", ETH_SPI_SCK_PIN, ETH_SPI_MISO_PIN, ETH_SPI_MOSI_PIN, ETH_SPI_SCS_PIN);
  SPI.begin(ETH_SPI_SCK_PIN, ETH_SPI_MISO_PIN, ETH_SPI_MOSI_PIN, -1);

#ifdef ETH_SPI_RST_PIN
  pinMode(ETH_SPI_RST_PIN, OUTPUT);
  digitalWrite(ETH_SPI_RST_PIN, LOW);
  delay(2);
  digitalWrite(ETH_SPI_RST_PIN, HIGH);
  delay(150);
#endif

  Ethernet.init(ETH_SPI_SCS_PIN);
  Ethernet.begin(mac, ip, gw, subnet);
  delay(1500);

  EthernetHardwareStatus hw = Ethernet.hardwareStatus();
  if (hw == EthernetNoHardware) {
    Serial.println("  Hardware: NONE  <-- W5500 not answering. Check 3V3/GND, MOSI/MISO not swapped, CS on GPIO 10.");
    return;
  }
  Serial.printf("  Hardware: %s\n", hw == EthernetW5500 ? "W5500 detected" : hw == EthernetW5100 ? "W5100?" : hw == EthernetW5200 ? "W5200?" : "unknown");
  Serial.print("  Local IP: "); Serial.println(Ethernet.localIP());
  ethOk = true;
}

void linkAndMeterCheck() {
  static bool lastLink = false;
  bool link = (Ethernet.linkStatus() == LinkON);
  if (link != lastLink) {
    Serial.printf("  Link: %s\n", link ? "ON (cable connected)" : "OFF (no cable / meter off)");
    lastLink = link;
  }
  if (!link) { digitalWrite(LED_2_METER, LOW); return; }

  EthernetClient c;
  if (c.connect(meter_ip, MODBUS_PORT)) {
    c.stop();
    digitalWrite(LED_2_METER, HIGH);
    Serial.print("  Meter TCP 502: OPEN at "); Serial.println(meter_ip);
  } else {
    digitalWrite(LED_2_METER, LOW);
    Serial.print("  Meter TCP 502: closed / no meter at "); Serial.println(meter_ip);
  }
}

void setup() {
  for (int i = 0; i < 5; i++) { pinMode(leds[i], OUTPUT); digitalWrite(leds[i], LOW); }
  Serial.begin(115200);
  delay(1500);
  printChipInfo();
  ledWalk();
  ethernetCheck();
  Serial.println();
  Serial.println("Loop: link + meter port check every 5 s. WIFI/INTERNET/SERVER LEDs blink as a heartbeat.");
}

void loop() {
  static unsigned long t = 0;
  static bool hb = false;
  if (millis() - t > 5000) {
    t = millis();
    if (ethOk) linkAndMeterCheck();
    hb = !hb;
    digitalWrite(LED_3_WIFI, hb);
    digitalWrite(LED_4_INTERNET, !hb);
    digitalWrite(LED_5_SERVER, hb);
  }
}
