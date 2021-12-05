
void setup()
{
  Serial.begin(115200);
  pinMode(11, OUTPUT);
  
  delay(5000);

  Serial.println("AT+ADDRESS=0\r\n");
  Serial.println("AT+NETWORKID=0\r\n");
}
 
void loop()
{
  String inString;

  inString = Serial.read();
  if(inString.substring(0,6) == "+RCV=")
  {
    digitalWrite(11, HIGH);
    delay(1000);
  }
  else
  {
    digitalWrite(11, LOW);
  }
  
}
