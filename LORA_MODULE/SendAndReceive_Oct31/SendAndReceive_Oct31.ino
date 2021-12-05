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
int prevTime;
String inString;
String outputMsg;

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

  Serial.print("AT+ADDRESS=120\r\n");
  delay(100);

  Serial.print("AT+NETWORKID=10\r\n");
  delay(100);
  
  
  on = false;
  count = 1500;
  inString = "";
  outputMsg = "";
  
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("READY"));
  display.display();
  delay(500);
  prevTime = millis();

  Serial.print("AT+SEND=121,4,PING\r\n");
  
}

void loop() 
{
  if(Serial.available() > 0)
  {  
      inString = Serial.readString();
      if(inString.indexOf("+OK") == -1)
      {
        display.clearDisplay();
        display.setCursor(0,5);
        display.println(inString.substring(0,inString.length()-9));
        display.display();
      
        delay(count);

        Serial.print("AT+SEND=121,4,PONG\r\n");
        display.setCursor(0,15);
        display.println("Sent Response");
        display.display(); 
      }
      else
      {
        display.setCursor(0,25);
        display.println("OK BLOCK");
        display.display();
      }
  }

/*
    display.clearDisplay();
    display.setCursor(0,5);
    display.println(inString);
    display.display();  
*/
}
