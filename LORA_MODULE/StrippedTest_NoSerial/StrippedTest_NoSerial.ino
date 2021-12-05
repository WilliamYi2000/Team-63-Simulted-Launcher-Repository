#include <SoftwareSerial.h>

#define LEDPin 12
#define rxPin 9
#define txPin 10

SoftwareSerial loraSerial(rxPin,txPin);

boolean on;

void setup() 
{
  loraSerial.begin(115200);

  loraSerial.print("AT+FACTORY\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+BAND=915000000\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+ADDRESS=0\r\n");   //needs to be unique
  delay(100);   //wait for module to respond

  loraSerial.print("AT+NETWORKID=0\r\n");   //needs to be same for receiver and transmitter
  delay(100);   //wait for module to respond

  pinMode(LEDPin, OUTPUT);
  on = false;

  digitalWrite(LEDPin, HIGH);
  delay(1000);
  digitalWrite(LEDPin, LOW);
}

void loop() 
{
    loraSerial.print("AT+SEND=1,16,RRRRRRRRRRRRRRRR\r\n");

    if(on)
    {
      digitalWrite(LEDPin, HIGH);
    }
    else
    {
      digitalWrite(LEDPin, LOW);
    }
    on = !on;

    delay(5000);
}
