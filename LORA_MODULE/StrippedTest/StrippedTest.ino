#include <SoftwareSerial.h>

#define LEDPin 12
#define rxPin 9
#define txPin 10

SoftwareSerial loraSerial(rxPin,txPin);


void setup() 
{
  Serial.begin(9600);
  loraSerial.begin(115200);

  loraSerial.print("AT+FACTORY\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+BAND=915000000\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+ADDRESS=0\r\n");   //needs to be unique
  delay(100);   //wait for module to respond

  loraSerial.print("AT+NETWORKID=0\r\n");   //needs to be same for receiver and transmitter
  delay(100);   //wait for module to respond

  Serial.println("READY");
}

void loop() 
{
    loraSerial.print("AT+SEND=1,4,SENT\r\n");
    Serial.print("Test Signal Sent - " + loraSerial.readString());

    delay(100);
}
