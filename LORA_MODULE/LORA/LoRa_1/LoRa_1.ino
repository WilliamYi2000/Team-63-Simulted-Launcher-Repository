
#include <SoftwareSerial.h>
SoftwareSerial Reyax(10, 11); // RX, TX
String message;
String received;
void setup() 
{
  // put your setup code here, to run once:
  setUp();

}

void loop() 
{
  // put your main code here, to run repeatedly:
  receive();
  if (Serial.available()>0)
  {
    String toSend = Serial.readString();
    send(toSend, 1);
    Serial.print("You: ");
    Serial.println(toSend);
  }
  //autoSend();

}
