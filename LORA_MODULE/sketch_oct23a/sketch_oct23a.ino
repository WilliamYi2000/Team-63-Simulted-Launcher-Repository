#define ledPin 12
unsigned long lastTransmission;
const int interval = 1000;

void setup(){
    Serial.begin(9600);
    pinMode(ledPin,OUTPUT);
}

void loop()
{
    if (millis() > lastTransmission + interval)
    {
        Serial.println("AT+SEND=0,8,Testing!");
        digitalWrite(ledPin,HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        lastTransmission = millis();
    }
}
