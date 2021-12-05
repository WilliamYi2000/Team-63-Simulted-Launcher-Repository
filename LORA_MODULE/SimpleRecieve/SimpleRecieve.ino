#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LEDPin 12
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     4    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

boolean on;
int count;

void setup() 
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); 
  display.clearDisplay();
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  
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
  count = 0;
  
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("READY"));
  display.display();
  delay(500);

  Serial.println("AT+SEND=0,5,HELLO");

  delay(4000);
}

void loop() 
{
  if(Serial.available())
  {
    String inString = Serial.readString();
    display.clearDisplay();
    display.setCursor(0,10);
    display.println(inString);  

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

  display.setCursor(0,20);
  display.println(count);
  display.display();
  count++;
}
