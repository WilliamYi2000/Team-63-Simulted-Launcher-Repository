
#include <SoftwareSerial.h>
SoftwareSerial Reyax(8, 9); // RX, TX

String message;
String received;
void setup() {
  // put your setup code here, to run once:
  setUp();
  
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  receive();
  if (Serial.available()>0) {
   String toSend = Serial.readString();
    send(toSend, 2);
  Serial.println(toSend);
  }

}
