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

// Global Vars
boolean on;

// Setup
void setup() 
{
  // Start serial connections
  Serial.begin(9600);
  loraSerial.begin(9600);

  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial) { ; }
  delay(100);

  // Reset Lora module
  pinMode(LoraReset, OUTPUT)                      // Create drain
  digitalWrite(LoraReset, LOW);                   // Open drain to ground
  delay(100);
  pinMode(LoraReset, INPUT)                       // Close drain

  // Prep Lora module
  loraSerial.print("AT+RESET\r\n");               // Software reset
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+MODE=0\r\n");              // Active Transmit/Receive mode
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+IPR=9600\r\n");            // Set baud rate to 9600
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+PARAMETER=12,9,1,7\r\n");  // Spread factor, Bandwidth, Coding Rate, Programmed Preamble
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+BAND=915000000\r\n");      // Set RF frequency
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+ADDRESS=121\r\n");         // Unique ID for this module
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+NETWORKID=10\r\n");        // ID for the connection; Needs to be same for receiver and transmitter (0-15)
  delay(100);                                     // Wait for module to respond

  loraSerial.print("AT+CRFOP=6\r\n");             // Output power (0-15) (6 = 6 dBm)
  delay(100);                                     // Wait for module to respond

  Serial.println("READY");
  on = false;
}

void loop() 
{
    String inString = loraSerial.readString();
    Serial.print(inString);

    if(inString.indexOf("R") > 0)
    {
      on = !on;
    }

    if(on)
    {
      digitalWrite(LEDPin, HIGH);
    }
    else
    {
      digitalWrite(LEDPin, LOW);
    }
}
