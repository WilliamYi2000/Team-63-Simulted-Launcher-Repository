String message;
int prevTime;
bool sendON = true;
void setup() {
  // put your setup code here, to run once:
  setUp();
  pinMode(11,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  blink();
  receive();
  if (message == "ON"){
    digitalWrite(11,HIGH);
  }
  if (message == "OFF"){
    digitalWrite(11,LOW);
  }
  }


void blink() {
  if (millis() - prevTime > 2000) {
    prevTime = millis();

    if (sendON == true) {
      send("ON", 1);
      sendON = false;
    }

    else {
      send("OFF", 1);
      sendON = true;
    }
  }
}

void receive() {
  if (Serial.available() == 1){

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

void send(String Transmit, int address) {
  int TransmitLength = Transmit.length();
  String message = "AT+SEND=" + String(address) + "," + String(TransmitLength) + "," + Transmit + "\n\r";
  Serial.println(message);
}

void setUp() {
  Serial.begin(115200);

  delay(1000);

  Serial.print("AT+RESET\r\n");
  delay(500);
  Serial.print("AT+PARAMETER=12,6,1,7\r\n");
  delay(100);   //wait for module to respond

  Serial.print("AT+BAND=915000000\r\n");    //Bandwidth set to 868.5MHz
  delay(100);   //wait for module to respond

  Serial.print("AT+ADDRESS=2\r\n");   //needs to be unique
  delay(100);   //wait for module to respond

  Serial.print("AT+NETWORKID=6\r\n");   //needs to be same for receiver and transmitter
  delay(100);   //wait for module to respond
  Serial.print("AT+CRFOP=15\r\n");
  delay(1000);
}
