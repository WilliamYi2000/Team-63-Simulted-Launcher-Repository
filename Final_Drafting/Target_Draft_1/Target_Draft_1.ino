// Target Draft 1
// First draft of complete target system
// Implements secreen, radio, and UI, and expects 2-way comm. with a launcher


  // IMPORTS  ----------------------------------------------------------------------------------------------------------
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
#include <Wire.h>                 // Screen
#include <Adafruit_GFX.h>         // Screen
#include <Adafruit_SSD1306.h>     // Screen
#include <FastLED.h>              // LED Strip
#include <stdio.h>                // Standard method library    (can we cut this out?)
#include <stdlib.h>               // Standard method library    (can we cut this out?)


  // RADIO MACROS ------------------------------------------------------------------------------------------------------
// Pins; Custom wiring
  // Module 1
#define RFM95_1_RST     49
#define RFM95_1_CS      53
#define RFM95_1_INT     19

  // Module 2
#define RFM95_2_RST     23
#define RFM95_2_CS      22
#define RFM95_2_INT     18

  // Module 3
#define RFM95_3_RST     4
#define RFM95_3_CS      24
#define RFM95_3_INT     2

/*
  // Module 4
#define RFM95_3_RST     26
#define RFM95_3_CS      25
#define RFM95_3_INT     3
*/

// Misc radio macros
#define RF95_FREQ     915.0   // Radio frequency (TX Freq, must match RX's Freq!)
#define RF95_TXPOW    23      // Transmisson power (default is 13 dBmn max is 23)

// Instance of the radio driver
RH_RF95 rf95_1(RFM95_1_CS, RFM95_1_INT);    // Module 1
RH_RF95 rf95_2(RFM95_2_CS, RFM95_2_INT);    // Module 2
RH_RF95 rf95_3(RFM95_3_CS, RFM95_3_INT);    // Module 3

// Calibration vars
  // Calibration parameters
int cal_num = 0;      // Calibration iterator; counts up to cal max as we recive messages
int cal_max = 10;     // Calibration upper bound; total number of messages we need to successfully calibrate

  // Calibrations averages
float r1_cal[10];       // Array for holding RSSI values of calibration messages for radio 1
float r2_cal[10];       // Array for holding RSSI values of calibration messages for radio 2
float r3_cal[10];       // Array for holding RSSI values of calibration messages for radio 3

float r1_avg;           // Varialbe for average of r1_cal
float r2_avg;           // Varialbe for average of r2_cal
float r3_avg;           // Varialbe for average of r3_cal

float r1_runAvg;        // Varialbe for average of r1_cal
float r2_runAvg;        // Varialbe for average of r2_cal
float r3_runAvg;        // Varialbe for average of r3_cal


  // SCREEN MACROS ----------------------------------------------------------------------------------------------------
// Screen size
#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 32      // OLED display height, in pixels

// Misc screen macros
#define OLED_RESET     4      // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


  // LED MACROS ------------------------------------------------------------------------------------------------------
// LED Strip
#define LED_PIN     37        // GPIO for LED strip
#define NUM_LEDS    20        // Total number of LEDs in LED strip

// LED multicolor
#define LED_R 27    // Red
#define LED_G 28    // Green
#define LED_B 29    // Blue

int activeLEDs;               // Iterating variable used to change the color of a specific LED in the below array (ACTIVE - blue)
int inactiveLEDs;             // Iterating variable used to change the color of a specific LED in the below array (INACTIVE - red)
CRGB leds[NUM_LEDS];          // Instance of LED driver; this controls our LED strip, each item in array is a single LED        


  // SPEAKER MACROS -------------------------------------------------------------------------------------------------
#define SPEAKER    35         // Pin number of the speaker
#define BEATTIME 1000         // Length of the generated tone (msec)


  // TARGET STATE MACROS -----------------------------------------------------------------------------------------
// ONLY ONE of these can be true at a time! Make sure to set old state low when new state goes high (replace these with an int?)
bool idle = true;
bool calibrating = false;
bool targeting = false;
bool hit = false;

int16_t secondsToHit;
int16_t caliConstant;

  // Setup  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  // Run setup methods
  screenSetup();          // Screen needs to be first since that's what we're printing debugging messages on
  threeRadioSetup();      // Setup radios second
  UISetup();              // Setup UI last

  // Delay before beginning loop
  delay(2000);
}


  // Loop  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{  
  if(idle)                                                // Base State: IDLE
  {
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: IDLE"));
    
    // TODO: Idle logic
      // Wait for message from launcher - parse message, send response, and move to targeting or calibration state based on message content
      // May want to re-design this state, since we probably want to use the RSSI/information from this message

    display.display();
  }
  else if(targeting)                                    // Firing State
  {
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: TARGETING"));

    // TODO: Targeting logic
      // Recieve message from launcher, run hit detection algorithm, send response (HIT/MISS), and set up LEDs

    display.display();
    
  }
  else if(calibrating)                                    // Calibrating State
  {
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: CALIBRATING"));
    
    // TODO: Calibration logic
      // Receive message from launcher, record RSSI value for each radio, send response

    display.display();
    
  }
  
  ////////////////////////////////

  delay(200);           // Delay between loop iterations
}


// HELPER METHODS ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // Screen setup -----------------------------------------------------------------------------------------------------------------------------------
void screenSetup()
{
  // Screen needs to be first since that's what we're printing debugging messages on
  
  // Make sure screen driver is properly intialized
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))        // If screen is not properly initialized
  {
    while (1);            // Infinite loop! TODO: If we have a debug LED on PCB, use that to denote this case
  }

  ////////////////////////////////

  // Show initial display buffer contents on the screen -- the library initializes this with an Adafruit splash screen.
  display.display();                      // Show the splash screen
  delay(1000);                            // Pause for 1 second

  // Clear the screen buffer
  display.clearDisplay();

  ////////////////////////////////
  
  // Set text properties
  display.setTextSize(1);                 // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);    // Draw white text

  // Screen prepped
  display.setCursor(10,0);                // Set screen cursor to position 1
  display.println(F("SCREEN READY"));     // Print "SCREEN READY" to screen
  display.display();                      // Display the message
  delay(500);                             // Short delay
}

  // Radio setup -----------------------------------------------------------------------------------------------------------------------------------
void threeRadioSetup()
{
  // Sets up three radios; all 3 radios should be set up identically
  
  // Set up reset pins
  pinMode(RFM95_1_RST, OUTPUT);       // Define reset pin for r1
  digitalWrite(RFM95_1_RST, HIGH);    // Set reset pin for r1 high
  pinMode(RFM95_2_RST, OUTPUT);       // Define reset pin for r2
  digitalWrite(RFM95_2_RST, HIGH);    // Set reset pin for r2 high
  pinMode(RFM95_3_RST, OUTPUT);       // Define reset pin for r3
  digitalWrite(RFM95_3_RST, HIGH);    // Set reset pin for r3 high

  // Do a manual reset of radio to start
  digitalWrite(RFM95_1_RST, LOW);     // Set RST pin low to reset radio 
  digitalWrite(RFM95_2_RST, LOW);     // Set RST pin low to reset radio
  digitalWrite(RFM95_3_RST, LOW);     // Set RST pin low to reset radio
  delay(10);                          // Wait 10 ms
  digitalWrite(RFM95_1_RST, HIGH);    // Set RST pin back to high for operation
  digitalWrite(RFM95_2_RST, HIGH);    // Set RST pin back to high for operation
  digitalWrite(RFM95_3_RST, HIGH);    // Set RST pin back to high for operation
  delay(10);                          // Wait 10 ms

  ////////////////////////////////

  // Make sure all modules initialized successfully
  while (!rf95_1.init())                          // Radio 1 
  {
    display.setCursor(10,10);                 // Set screen cursor to position 2
    display.println(F("RADIO 1 INIT BAD"));   // Print that radio 1 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }
  while (!rf95_2.init())                          // Radio 2 
  {
    display.setCursor(10,10);                 // Set screen cursor to position 2
    display.println(F("RADIO 2 INIT BAD"));   // Print that radio 2 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }
  while (!rf95_3.init())                          // Radio 3 
  {
    display.setCursor(10,10);                 // Set screen cursor to position 2
    display.println(F("RADIO 3 INIT BAD"));   // Print that radio 3 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }

  ///////////////////////////////

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95_1.setFrequency(RF95_FREQ))            // Radio 1
  {
    display.setCursor(10,10);                 // Set screen cursor to position 2
    display.println(F("RADIO 1 FREQ BAD"));   // Print that radio 1 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }
  if (!rf95_2.setFrequency(RF95_FREQ))            // Radio 2
  {
    display.setCursor(10,10);                 // Set screen cursor to position 2
    display.println(F("RADIO 2 FREQ BAD"));   // Print that radio 2 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }
  if (!rf95_3.setFrequency(RF95_FREQ))            // Radio 3
  {
    display.setCursor(10,10);                 // Set screen cursor to position 2
    display.println(F("RADIO 3 FREQ BAD"));   // Print that radio 3 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }

  ////////////////////////////////

  // Set TX power
  rf95_1.setTxPower(RF95_TXPOW, false);   // Set R1 to 23 dBm (default is 13 dBm, valid 5-23)
  rf95_2.setTxPower(RF95_TXPOW, false);   // Set R2 to 23 dBm (default is 13 dBm, valid 5-23)
  rf95_3.setTxPower(RF95_TXPOW, false);   // Set R3 to 23 dBm (default is 13 dBm, valid 5-23)

  ////////////////////////////////

  display.setCursor(10,10);               // Set screen cursor to position 2
  display.println(F("RADIOs READY"));     // Print that radios were initialized properly
  display.display();                      // Display the message
  delay(500);                             // Short delay
}

  // UI setup -----------------------------------------------------------------------------------------------------------------------------------
void UISetup()
{

  // Set up LED strip
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  ////////////////////////////////

  resetLEDs();

  ////////////////////////////////
  
  display.setCursor(10,20);                // Set screen cursor to position 3
  display.println(F("UI READY"));         // Print that UI was initialized properly
  display.display();                      // Display the message
  delay(500);                             // Short delay
}

// Check potentiometer and reset LED strip
void resetLEDs()
{
    // TODO: LED Reset Logic; need to get num active LEDs from launcher first!
}
