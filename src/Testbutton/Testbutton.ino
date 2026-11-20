// Bench check for front-panel SW1 on Version 2.1 (Back Main Board + Front).
// SW1 shorts J3 pin 7 (BUTTON) to J3 pin 8 (GND). That net is GPIO 13
// (DevKitC header pin 15). R1 (10k) holds it high, so a press reads LOW.
// The BOOT button is GPIO 0 and will not print here.
// One line at boot proves this sketch is running and the monitor is 115200.
// After that, a line is printed only when GPIO 13 goes low.

#define BUTTON_PIN 13

void setup() {
  Serial.begin(115200);
  // Opening the monitor resets the module. The first bytes are often sent
  // before the monitor is reading, so this keeps "Setup begin" on screen.
  delay(1000);
  Serial.println("Setup begin");
  // External 10k already pulls BUTTON high. Internal pull-up covers a bare
  // DevKit with J3 unplugged, so an open pin does not float.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // If this line never appears, the monitor is on the wrong port or not 115200,
  // or this sketch was not the one uploaded.
  Serial.print("Button test ready. GPIO 13 is ");
  Serial.println(digitalRead(BUTTON_PIN) == LOW ? "LOW" : "HIGH");
}

void loop() {
  // First call is after pinMode(), so the idle level is the real pin, not a float.
  static int stable = digitalRead(BUTTON_PIN);

  int reading = digitalRead(BUTTON_PIN);
  if (reading == stable) {
    return;
  }
  stable = reading;
  // Released is HIGH. Only the press is reported.
  if (reading != LOW) {
    return;
  }
  Serial.println("Button pressed");
}
