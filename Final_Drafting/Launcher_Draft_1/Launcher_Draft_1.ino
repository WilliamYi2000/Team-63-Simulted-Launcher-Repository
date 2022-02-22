// Launcher Draft 1
// First draft of complete launcher system
// Implements secreen, radio, and UI, and expects 2-way comm. with a target
// Also implements refactorign and readability improvements from previous launcher versions


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
// Pins; Matches Adafruit wiring diagram
#define RFM95_RST     2
#define RFM95_CS      4
#define RFM95_INT     3

// Misc radio macros
#define RF95_FREQ     915.0   // Radio frequency (TX Freq, must match RX's Freq!)
#define RF95_TXPOW    23      // Transmisson power (default is 13 dBmn max is 23)

// Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


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
#define LED_PIN     7         // LED strip using GPIO 7 - pd7
#define NUM_LEDS    10        // Total number of LEDs in LED strip

int activeLEDs;               // Iterating variable used to change the color of a specific LED in the below array (ACTIVE - blue)
int inactiveLEDs;             // Iterating variable used to change the color of a specific LED in the below array (INACTIVE - red)
CRGB leds[NUM_LEDS];          // Instance of LED driver; this controls our LED strip, each item in array is a single LED        


  // SPEAKER MACROS -------------------------------------------------------------------------------------------------
#define SPEAKER     6         // Pin number of the speaker
#define BEATTIME 1000         // Length of the generated tone (msec)


  // BUTTON/POTENTIOMETER MACROS ------------------------------------------------------------------------------------
// Timing pot on Analog pin 0
int timingPotRawValue;            // Stores raw input from timing potentiometer
int timingPotAdjustedValue;       // Stores adjusted value from timing potentiometer (time in seconds)

int caliTime;

const int triggerButton = 8;      // Pin number of the trigger button
const int caliButton = 9;         // Pin number of the calibration button

  // LAUNCHER STATE MACROS -----------------------------------------------------------------------------------------
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
  screenSetup();    // Screen needs to be first since that's what we're printing debugging messages on
  radioSetup();     // Setup radio second
  UISetup();        // Setup UI last

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
    
    // Check Pot and LEDs
    resetLEDs();

    // Get Calibration Constant from Cali Pot (Move this to calibration phase? Doing it this way means it must be set BEFORE pressing cali button)
    caliConstant = 500;         // TODO: ADD POT CODE

    // Check buttons
    if(digitalRead(triggerButton)==LOW)               // If trigger is held
    {
      // Display trigger press
      display.setCursor(10,10);
      display.println(F("TRIGGER PRESSED"));
      
      // State transition: Idle -> Targeting
      idle = false;
      targeting = true;
    }
    else if(digitalRead(caliButton)==HIGH)           // If calibration button is pressed and trigger is NOT pressed (trigger takes precedence)
    {
      // Display calibration press
      display.setCursor(10,10);
      display.println(F("CALIBRATION PRESSED"));

      caliTime = 10;      // Seconds required to calibrate

      // Play a tone
      tone(SPEAKER,400,400);
      delay(10) ;
      
      // State transition: Idle -> Calibrating
      idle = false;
      calibrating = true;
    }

    display.display();
  }
  else if(targeting)                                    // Firing State
  {
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: TARGETING"));

    // Check button
    if(digitalRead(triggerButton)==LOW)               // If button held, TX targeting packet
    {      
      // TX targeting packet to target
        // Targeting packet = T|[Seconds to hit - 3]
      char radiopacket[6] = "T|   ";                  // Build targeting packet
      itoa(secondsToHit-3, radiopacket+2, 10);        // Adding secondsToHit-3 to packet; our timing range is 3-10 seconds, so if we subtract 3 here and add it back on when target RXs message we can parse a single digit (0-7) rather than two digits (3-10)
      rf95.send((uint8_t *)radiopacket, 6);           // SEND PACKET  (6 characters)
      delay(10);                                      // Short delay for safety
      rf95.waitPacketSent();                          // Pause until packet fully sent

      // Display our sent packet
      display.setCursor(10,10);
      display.println(radiopacket);
      display.display();
      
      // RX response packet from target
      delay(350);           // Delay needs to be long enough for receiver to interp TX'd packet and send a response; also needs to be long enough for our transmitter to swap from TX to RX; assuming RTT negligible

      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];               // Build RX buffer
      uint8_t len = sizeof(buf);                          // Need length of buffer
      if (rf95.waitAvailableTimeout(1000))                // 1 second timeout (Shorten this??)
      { 
        if (rf95.recv(buf, &len))           // RX successfully within timeout (RX read into buf)
        {
          display.setCursor(10,20);
          display.println((char*)buf);        // Need to convert to char!
          display.display();
        }
        else                                // RX partially recieved (Recieve failure case)
        {
          display.setCursor(10,20);
          display.println(F("RX FAILURE"));
          display.display();
        }
      }
      else                                  // RX NOT received (timeout)
      {
          display.setCursor(10,20);
          display.println(F("NO RX FROM TARGET"));
          display.display();
      }
      
    }
    else                                              // If button NOT held, leave targeting state
    {
      // State transition: Targeting -> Idle
      targeting = false;
      idle = true;
    }
  }
  else if(calibrating)                                    // Calibrating State
  {
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: CALIBRATING"));

    // Check button
    if(digitalRead(triggerButton)==LOW)               // If button held, TX calibration packet
    {      
      // TX targeting packet to target
        // Targeting packet = C|[Calibration Constant]
      char radiopacket[6] = "C|   ";                  // Build calibration packet
      itoa(caliConstant, radiopacket+2, 10);          // Adding calibration constant to packet; calibration constant is represented as three decimals beyond a 0 (ex. 0.123) - we want to send values from 0.250 - 0.999 (Check this via testing?)
      rf95.send((uint8_t *)radiopacket, 8);           // SEND PACKET  (8 characters)
      delay(10);                                      // Short delay for safety
      rf95.waitPacketSent();                          // Pause until packet fully sent

      // Display our sent packet
      display.setCursor(10,10);
      display.println(radiopacket);
      display.display();
      
      // RX response packet from target
      delay(350);           // Delay needs to be long enough for receiver to interp TX'd packet and send a response; also needs to be long enough for our transmitter to swap from TX to RX; assuming RTT negligible

      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];               // Build RX buffer
      uint8_t len = sizeof(buf);                          // Need length of buffer
      if (rf95.waitAvailableTimeout(1000))                // 1 second timeout (Shorten this??)
      { 
        if (rf95.recv(buf, &len))           // RX successfully within timeout (RX read into buf)
        {
          display.setCursor(10,20);
          display.println((char*)buf);        // Need to convert to char!
          display.display();

          // If we've received a response packet, we successfully got a calibration packet out (need 10 total to calibrate)
          caliTime = caliTime - 1;
        }
        else                                // RX partially recieved (Recieve failure case)
        {
          display.setCursor(10,20);
          display.println(F("RX FAILURE"));
          display.display();
        }
      }
      else                                  // RX NOT received (timeout)
      {
          display.setCursor(10,20);
          display.println(F("NO RX FROM TARGET"));
          display.display();
      }

      // If we're done calibrating, state transition
      if(caliTime <= 0)
      {
        // Play a tone
        tone(SPEAKER,400,500) ;
        delay(900) ;
        tone(SPEAKER,400,500) ;
        delay(10) ;
        
        calibrating = false;
        idle = true;
      }
    }
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
void radioSetup()
{
  // Set up reset pin
  pinMode(RFM95_RST, OUTPUT);         // Define reset pin
  digitalWrite(RFM95_RST, HIGH);      // Reset is active low, need to set high for operation

  // Do a manual reset of radio to start
  digitalWrite(RFM95_RST, LOW);       // Set RST pin low to reset radio
  delay(10);                          // Wait 10 ms
  digitalWrite(RFM95_RST, HIGH);      // Set RST pin back to high for operation
  delay(10);                          // Wait 10 ms

  ////////////////////////////////

  // Make sure module initialized successfully
  if(!rf95.init())                        // If radio is not properly initialized
  {
    display.setCursor(10,10);               // Set screen cursor to position 2
    display.println(F("RADIO INIT BAD"));   // Print that radio was not initialized properly
    display.display();                      // Display the message
    while (1);                              // Infinite loop! 
  }

  ///////////////////////////////

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))      // If radio frequency is not properly initialized
  {
    display.setCursor(10,10);               // Set screen cursor to position 2
    display.println(F("RADIO FREQ BAD"));   // Print that radio frequency was not initialized properly
    display.display();                      // Display the message
    while (1);                              // Infinite loop! 
  }

  ////////////////////////////////

  // Set TX power
  rf95.setTxPower(RF95_TXPOW, false);

  ////////////////////////////////

  display.setCursor(10,10);               // Set screen cursor to position 2
  display.println(F("RADIO READY"));      // Print that radio was initialized properly
  display.display();                      // Display the message
  delay(500);                             // Short delay
}

  // UI setup -----------------------------------------------------------------------------------------------------------------------------------
void UISetup()
{
  // Set up trigger
  pinMode(triggerButton,INPUT_PULLUP);         // Define trigger button as an input pullup --- THIS IS DIFFERENT FROM PERVIOUS UI VERSIONS
  pinMode(caliButton,INPUT);                   // Define calibration button as an input


  ////////////////////////////////
  
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
  // Read raw input in from potentiometer on analog pin 0
  timingPotRawValue = analogRead(A0);

  // Convert raw pot value to usable time (Lock on timing will only be from 3-10 sec, based on FSR)
  timingPotAdjustedValue = 8;       // (timingPotRawValue / 140) + 3;       // Translates analog input to seconds   (determined by experimentation, may need to adjust this later!) // TODO: ADD POT CODE
  inactiveLEDs =  timingPotAdjustedValue - 1;                   // Number of LEDs which are to start RED
  activeLEDs = NUM_LEDS - inactiveLEDs;                         // Number of LEDs which are to start BLUE

  // Get time to hit from pot
  secondsToHit = timingPotAdjustedValue;
  ////////////////////////////////

  // Set LED starting colors
    // RED (inactive)
  for(int i = 0; i <= inactiveLEDs; i++)
  {
    leds[i] = CRGB(255, 0, 0);                // Set given LED to red
  }
    // BLUE (active, pre-counted)
  for(int i = inactiveLEDs+1; i <= (NUM_LEDS-1); i++)
  {
    leds[i] = CRGB(0, 0, 255);                // Set given LED to blue
  }

  // Show LEDs
  FastLED.show();
}
