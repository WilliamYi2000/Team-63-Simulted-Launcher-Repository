
#define LEDPin 12
#define LEDPin2 8

boolean on;
boolean recieve;

void setup() 
{
  Serial.begin(115200);

  Serial.print("AT+FACTORY\r\n");
  delay(100);   //wait for module to respond

  Serial.print("AT+BAND=915000000\r\n");
  delay(100);   //wait for module to respond

  Serial.print("AT+ADDRESS=0\r\n");   //needs to be unique
  delay(100);   //wait for module to respond

  Serial.print("AT+NETWORKID=0\r\n");   //needs to be same for receiver and transmitter
  delay(100);   //wait for module to respond

  delay(1000);

//  Serial.print("AT+SEND=1,4,SENT\r\n");

//  delay(5000);

  on = false;
  recieve = false;
  pinMode(LEDPin, OUTPUT);
  pinMode(LEDPin2, OUTPUT);
  digitalWrite(LEDPin2, LOW);
}

void loop() 
{
  Serial.print("AT+SEND=0,4,SENT\r\n");

  String inString = Serial.readString();

  if(inString.indexOf("TEST") > 0)
  {
    recieve = true;
  }

  if(on)
  {
    digitalWrite(LEDPin, HIGH);
  }
  else
  {
    digitalWrite(LEDPin, LOW);
  }


  if(recieve)
  {
    digitalWrite(LEDPin2, HIGH);
  }
  
  on = !on;
}
