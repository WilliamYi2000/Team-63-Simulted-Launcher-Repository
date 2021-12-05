#define ledPin 8
unsigned long lastTransmission;
const int interval=1000;
void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);
  pinMode (ledPin, OUTPUT);
  Serial.print("AT+RESET\r\n");
delay(20);

Serial.print("AT+IPR=115200\r\n");
delay(20);
Serial.print("AT+ADDRESS=1\r\n");
delay(20);

Serial.print("AT+NETWORKID=5\r\n");
delay(20);

Serial.print("AT+MODE=1\r\n");
delay(20);

Serial.print("AT+BAND=915000000\r\n");
delay(20);

Serial.print("AT+PARAMETER=10,7,1,7\r\n");
delay(20);
 pinMode(ledPin, OUTPUT);

}

void loop() { 
  // put your main code here, to run repeatedly:
  if (millis() > lastTransmission+ interval) {

    // Serial.println ("AT+SEND=2,6,Hello!");
    Serial.print ('H');
    digitalWrite (ledPin, HIGH);
    delay(3000);
    Serial.print ('L');
    digitalWrite(ledPin, LOW);
    lastTransmission=millis ();
  }

}
