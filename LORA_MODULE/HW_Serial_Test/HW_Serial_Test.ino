String message;
int prevTime;

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
  
  
  pinMode(12, OUTPUT);
}


void loop() 
{
  receive();
  if (message == "ON")
  {
    digitalWrite(12,HIGH);
  }
  if (message == "OFF")
  {
    digitalWrite(12,LOW);
  }
}


void receive() 
{
  if (Serial.available() == 1)
  {

  String readString = Serial.readString();

  int delimiter, delimiter_1, delimiter_2, delimiter_3;
  delimiter = readString.indexOf(",");
  delimiter_1 = readString.indexOf(",", delimiter + 1);
  delimiter_2 = readString.indexOf(",", delimiter_1 + 1);
  delimiter_3 = readString.indexOf(",", delimiter_2 + 1);
  int lengthMessage=readString.substring(delimiter_1+1,delimiter_2).toInt();
  
  String message = readString.substring(delimiter_2 + 1 , lengthMessage);
  }
}

void send(String Transmit, int address) 
{
  int TransmitLength = Transmit.length();
  String message = "AT+SEND=" + String(address) + "," + String(TransmitLength) + "," + Transmit + "\n\r";
  Serial.println(message);
}
