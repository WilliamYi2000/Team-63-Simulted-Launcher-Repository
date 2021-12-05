#include <SoftwareSerial.h>

#define LEDPin 12
#define rxPin 9
#define txPin 10

SoftwareSerial loraSerial(rxPin,txPin);


void setup() {
  Serial.begin(9600);
  loraSerial.begin(115200);
  
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(100);
  
  loraSerial.print("AT\r\n");                   // Check connection: Expect return "OK"
  delay(100);

  loraSerial.print("AT+BAND=915000000\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+ADDRESS=0\r\n");   //needs to be unique
  delay(100);   //wait for module to respond

  loraSerial.print("AT+NETWORKID=0\r\n");   //needs to be same for receiver and transmitter
  delay(100);   //wait for module to respond

  loraSerial.print("AT+PARAMETER=12,9,1,7\r\n");
  delay(100);   //wait for module to respond

  Serial.println("READY");
}

void loop() 
{
    loraSerial.print("AT+SEND=1,4,SENT\r\n");
    Serial.print("Test Signal Sent - " + loraSerial.readString());

    delay(1000);
}
