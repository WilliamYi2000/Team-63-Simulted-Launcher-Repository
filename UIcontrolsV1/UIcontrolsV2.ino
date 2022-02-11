//Essentially the control program for the launcher system where the target
//system can get data (timing mostly) from radio connection between the two
//Target code will only have led, display, and speaker control code for User Interface
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include <stdio.h>
#include <stdlib.h>
#define LED_PIN     7 //led strip using GPIO 7 - pd7
#define NUM_LEDS    20

#define LEDPin 12
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     4    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define BEATTIME 1000 //Length of the generated tone (msec)
#define SPEAKER 6 //Pin number of the speaker - pd6 pin
const int button = 8;            // GPIO 2 - pd8 for the button

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//for display
boolean on;
int count;
int prevTime;
String inString;
String outputMsg;
// for ledstrip
int sensorRealValue; //resembles the timing from the potentiometer
int temp; //used in the loop() in order to restart the counting process

//LED
CRGB leds[NUM_LEDS];
int LEDColor;
void setup() 
{
  pinMode(button,INPUT);         // define button as an input
  
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); 
  display.clearDisplay();
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  
  on = false;
  count = 0;
  inString = "";
  outputMsg = "";
  
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("READY"));
  display.display();
  delay(500);

  //LED setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
  // read the input on analog pin 0:
  /*int sensorValue = analogRead(A0);
  //lock on timing will only be from 3-10 sec based on FSR
  sensorRealValue = (sensorValue / 140) + 3; //translates analog input to seconds
  temp = sensorRealValue;
  int LEDcount = NUM_LEDS - 1; 
  int y = sensorRealValue - 1; //reverse direction of led display
  //sets up the led to update it based on the timing in standby phase
  //essentially assigning an led its respective second
  while (LEDcount >= 0){
    //led will be blue if led/second is already "counted"
    leds[LEDcount] = CRGB(0, 0, 255);
    LEDcount = LEDcount - 1;
  } 
  while (y >= 0){
    //led will be red if it needs to counted down during the lock on phase
    leds[y] = CRGB(255, 0, 0);
    y = y - 1;
  }
  FastLED.show();
  delay(1000);*/
}
void loop()
{
  int LEDcount = NUM_LEDS - 1; //redefine num leds in order to start transition from red to blue
  int y = sensorRealValue - 1;
  int freq = 100;
  int tempFREQ = freq;
  display.clearDisplay();
  display.setCursor(0,5);             // Start at top-left corner
  display.println(temp);
  display.display();
  if (digitalRead(button)==HIGH){ // if button is pressed
    leds[temp] = CRGB(0, 0, 255); //changing the led to red from blue when its respective second is counted down
    temp = temp - 1;
    FastLED.show();
    tone(SPEAKER,freq,BEATTIME) ; // build up
    //may integrate a freq changes based on seconds later
    tempFREQ += 50;
  }
  else { //if button is not pressed
    //also add the radio connection (hit/on target indication condition)
    
    //reset timing and led strip when button is release
    //timing will reset because trigger pulldown for consistent target acquisition or it will be a failed lock on phase
    temp = sensorRealValue;
    tempFREQ = freq;
    //changing components (led strip and display) back to standby phase values
    /*while (LEDcount >= 0){
    leds[LEDcount] = CRGB(0, 0, 255);
    LEDcount = LEDcount - 1;
    } 
    while (y >= 0){
      leds[y] = CRGB(255, 0, 0);
      y = y - 1;
    }
  FastLED.show();*/
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  //lock on timing will only be from 3-10 sec based on FSR
  sensorRealValue = (sensorValue / 140) + 3; //translates analog input to seconds
  temp = sensorRealValue;
  int LEDcount = NUM_LEDS - 1; 
  int y = sensorRealValue - 1; //reverse direction of led display
  //sets up the led to update it based on the timing in standby phase
  //essentially assigning an led its respective second
  while (LEDcount >= 0){
    //led will be blue if led/second is already "counted"
    leds[LEDcount] = CRGB(0, 0, 255);
    LEDcount = LEDcount - 1;
  } 
  while (y >= 0){
    //led will be red if it needs to counted down during the lock on phase
    leds[y] = CRGB(255, 0, 0);
    y = y - 1;
  }
  FastLED.show();
  delay(1000);
  }
  
  if(temp < 0){ //when the count down has reached 0 indicating a lock on and a hit
    //maybe a delay on the bounce back from the target system
    int LEDcount = NUM_LEDS - 1;
    while (LEDcount >= 0){ 
      //led strip will light up green for a successful hit
      //provided we have met all the nessacery conditions of locking on
      leds[LEDcount] = CRGB(0, 255, 0);
      LEDcount = LEDcount - 1;
    } 
    FastLED.show();
    //playing the Twinkle Twinkle Little Star Melody when target is hit
    tone(SPEAKER,262,BEATTIME) ; // Do
    delay(BEATTIME) ;
    tone(SPEAKER,262,BEATTIME) ; // Do
    delay(BEATTIME) ;
    tone(SPEAKER,392,BEATTIME) ; // So
    delay(BEATTIME) ;
    tone(SPEAKER,392,BEATTIME) ; // So
    delay(BEATTIME) ;
    tone(SPEAKER,440,BEATTIME) ; // La
    delay(BEATTIME) ;
    tone(SPEAKER,440,BEATTIME) ; // La
    delay(BEATTIME) ;
    tone(SPEAKER,392,BEATTIME) ; // So
    delay(BEATTIME) ;
    LEDcount = NUM_LEDS - 1;
    while (LEDcount >= 0){ 
      //led strip will turn of in order to save on power consumption
      leds[LEDcount] = CRGB(0, 0, 0);
      LEDcount = LEDcount - 1;
    } 
    FastLED.show();
    //exit(0);
    }
    //may need to add a method of code reusablity for multiple interations in the same arduino loop method
    //as of now code needs to be reseted in order to run another interation
  delay(1000); 
 }
