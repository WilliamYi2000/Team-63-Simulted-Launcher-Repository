#include <SoftwareSerial.h>

#define LEDPin 12
#define rxPin 10
#define txPin 9

SoftwareSerial loraSerial(rxPin,txPin);

void setup() 
{
  loraSerial.begin(115200);
  
  loraSerial.print("AT+BAND=915000000\r\n");
  delay(100);                                       //wait for module to respond

  loraSerial.print("AT+ADDRESS=10\r\n");            //needs to be unique
  delay(100);                                       //wait for module to respond

  loraSerial.print("AT+NETWORKID=120\r\n");         //needs to be same for receiver and transmitter
  delay(100);                                       //wait for module to respond

  loraSerial.print("AT+PARAMETER=12,9,1,7\r\n");
  delay(100);                                       //wait for module to respond

  digitalWrite(LEDPin, HIGH);
}

void loop() 
{
    loraSerial.print("AT+SEND=1,4,SENT\r\n");

    delay(1000);
}
