/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.
  Test leds for the Open energy gateway.
*/

//Define the status indicating LEDs pins
//                        //C3  //WROOM
#define LED_1_POWER     2 //2   //12 //Indicates Power is on 
#define LED_2_METER     3 //3   //14 //Indicates Meter is connected via Modbus
#define LED_3_WIFI      4 //4   //27 //Indicates WiFi is connected
#define LED_4_INTERNET  5 //5   //26 //Indicates Internet is connected, this is the green LED
#define LED_5_SERVER    6 //6   //25 //Indicates succesfull communication with the Server


void setup() {
  // initialize LED status pins as outputs.
  pinMode(LED_1_POWER, OUTPUT);
  pinMode(LED_2_METER, OUTPUT);
  pinMode(LED_3_WIFI, OUTPUT);
  pinMode(LED_4_INTERNET, OUTPUT);
  pinMode(LED_5_SERVER, OUTPUT);

  Serial.begin(9600); // Initialize serial communication
  delay(2000); //Wait for the serial port te become available.
  
  Serial.print("Flash Chip Size: ");
  Serial.println(ESP.getFlashChipSize()); // Get flash size in bytes
  
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel()); 

  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision()); 

  Serial.print("Chip Cores: ");
  Serial.println(ESP.getChipCores()); 
}


void loop() {
  digitalWrite(LED_1_POWER, HIGH);
  delay(200);
  digitalWrite(LED_2_METER, HIGH);
  delay(200);
  digitalWrite(LED_3_WIFI, HIGH);
  delay(200);
  digitalWrite(LED_4_INTERNET, HIGH);
  delay(200);
  digitalWrite(LED_5_SERVER, HIGH);
  delay(200);

  digitalWrite(LED_1_POWER, LOW);
  delay(200);
  digitalWrite(LED_2_METER, LOW);
  delay(200);
  digitalWrite(LED_3_WIFI, LOW);
  delay(200);
  digitalWrite(LED_4_INTERNET, LOW);
  delay(200);
  digitalWrite(LED_5_SERVER, LOW);
  delay(200);


}
