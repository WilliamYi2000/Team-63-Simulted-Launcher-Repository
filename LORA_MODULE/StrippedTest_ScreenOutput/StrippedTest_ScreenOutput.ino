#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LEDPin 12
#define rxPin 9
#define txPin 10
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     4    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

SoftwareSerial loraSerial(rxPin,txPin);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int numSent;

void setup() 
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  
  loraSerial.begin(115200);

  loraSerial.print("AT+FACTORY\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+BAND=915000000\r\n");
  delay(100);   //wait for module to respond

  loraSerial.print("AT+ADDRESS=0\r\n");   //needs to be unique
  delay(100);   //wait for module to respond

  loraSerial.print("AT+NETWORKID=0\r\n");   //needs to be same for receiver and transmitter
  delay(100);   //wait for module to respond

  numSent = 0;
  
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("READY"));
  display.display();
  delay(500);
}

void loop() 
{
    loraSerial.print("AT+SEND=1,4,SENT\r\n");

    display.clearDisplay();
    display.setCursor(0,15);
    display.println(F("Sending..."));
    display.display();
  
    delay(1000);
}
