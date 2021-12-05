#define LEDPin 12

boolean on;

void setup() 
{
  Serial.begin(115200);

  Serial.print("AT+FACTORY\r\n");
  delay(100);

  Serial.print("AT+BAND=915000000\r\n");
  delay(100);

  Serial.print("AT+ADDRESS=1\r\n");
  delay(100);

  Serial.print("AT+NETWORKID=0\r\n");
  delay(100);
  
  
  on = false;
  pinMode(LEDPin, OUTPUT);

}

void loop() 
{
  if(Serial.available() > 0)
  {
    String inString = Serial.readString();

    if(inString.indexOf("R") > 0)
    {
      on = !on;
    }
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
