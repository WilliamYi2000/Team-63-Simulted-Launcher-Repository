// Lora Radio Module Control
// SENDER

// Libraries
#include <SoftwareSerial.h>

// Define Pin Macros
#define LEDPin 12
#define rxPin 10
#define txPin 11
#define LoraReset 9

// Lora software serial object
SoftwareSerial loraSerial(rxPin,txPin);

// Setup
void setup() 
{
  // Start serial connections
  Serial.begin(9600);
  loraSerial.begin(115200);

  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial) { ; }
  delay(100);

/*
  // Reset Lora module
  pinMode(LoraReset, OUTPUT);                     // Create drain
  digitalWrite(LoraReset, LOW);                   // Open drain to ground
  delay(100);
  pinMode(LoraReset, INPUT);                      // Close drain
*/

  loraSerial.print("AT+FACTORY\r\n"); 
  delay(100);

  // Prep Lora module
  loraSerial.print("AT+RESET\r\n");               // Software reset
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+MODE=0\r\n");              // Active Transmit/Receive mode
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+PARAMETER=12,9,1,7\r\n");  // Spread factor, Bandwidth, Coding Rate, Programmed Preamble
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+BAND=915000000\r\n");      // Set RF frequency
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+ADDRESS=120\r\n");         // Unique ID for this module
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+NETWORKID=0\r\n");        // ID for the connection; Needs to be same for receiver and transmitter (0-15)
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+CRFOP=15\r\n");            // Output power (0-15)
  delay(100);                                     // Wait for module to respond

  Serial.println("READY");
}

void loop() 
{
    loraSerial.print("AT+SEND=1,4,SENT\r\n");
    Serial.print("Test Signal Sent  - " + loraSerial.readString());

    delay(1000);
}
