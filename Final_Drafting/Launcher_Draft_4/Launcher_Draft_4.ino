// Launcher Draft 4
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
//#include <stdio.h>                // Standard method library    (can we cut this out?)
//#include <stdlib.h>               // Standard method library    (can we cut this out?)


  // RADIO MACROS ------------------------------------------------------------------------------------------------------
// Pins; Matches Adafruit wiring diagram
#define RFM95_RST     2
#define RFM95_CS      4
#define RFM95_INT     3

// Misc radio macros
#define RF95_FREQ     915.0       // Radio frequency (TX Freq, must match RX's Freq!)
#define RF95_TXPOW    23          // Transmisson power (default is 13 dBmn max is 23)

// Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


  // SCREEN MACROS ----------------------------------------------------------------------------------------------------
// Screen size
#define SCREEN_WIDTH 128          // OLED display width, in pixels
#define SCREEN_HEIGHT 32          // OLED display height, in pixels

// Misc screen macros
#define OLED_RESET     4          // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C       // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


  // LED MACROS ------------------------------------------------------------------------------------------------------
// LED Constants
#define LED_PIN     7             // LED strip using GPIO 7 - pd7
#define NUM_LEDS    10            // Total number of LEDs in LED strip

// LED Variables
int activeLEDs;                   // Iterating variable used to change the color of a specific LED in the below array (ACTIVE - blue)
int inactiveLEDs;                 // Iterating variable used to change the color of a specific LED in the below array (INACTIVE - red)

// Instance of LED driver
CRGB leds[NUM_LEDS];              // This controls our LED strip, each item in array is a single LED        


  // SPEAKER MACROS -------------------------------------------------------------------------------------------------
// Speaker Constants
#define SPEAKER     6             // Pin number of the speaker
#define BEATTIME 1000             // Length of the generated tone (msec)

// Speaker Variables
int speakerFreqConstant = 75;     // Amount that frequency increases each time we get a hit


  // BUTTON/POTENTIOMETER MACROS ------------------------------------------------------------------------------------
// Timing pot on Analog pin 0
int timingPotRawValue;            // Stores raw input from timing potentiometer
int timingPotAdjustedValue;       // Stores adjusted value from timing potentiometer (time in seconds)

const int triggerButton = 8;      // Pin number of the trigger button
const int caliButton = 5;         // Pin number of the calibration button

  // LAUNCHER STATE MACROS -----------------------------------------------------------------------------------------
// ONLY ONE of these can be true at a time! Make sure to set old state low when new state goes high (replace these with an int?)
bool idle = true;                   // Default state: Accept UI input from user and adjust UI output accordingly, wait for trigger press                                  (goes to either calibration state or targeting state)
bool calibrating = false;           // Send calibration packet to target on trigger press; stays in this state until enough response packets from target received         (returns to idle state upon 10 response packets sucessfully RX'd)
bool targeting = false;             // Send targeting packet to target and get response, adjust UI based on hit or miss                                                   (goes to idle state upon letting up on trigger, or hit state upon RX enough response packets in a row)
bool hit = false;                   // Set UI to HIT until user lets go of button                                                                                         (returns to idle state upon letting up on trigger)

bool firstCalibrationDone = false;  // Have we completed at least one calibration cycle? If not, we cannot send packets to target! Need to complete at least on calibration cycle first
int caliNumber = 0;                 // The number of calibration packets we've received during a given calibration phase (Resets to 0 for every calibration phase)
int caliPackets;                    // Number of packets we need to calibrate (not necessary to define this here; this is reset whenever we enter the calibration phase, that's where any edits to # of packets should be made)    

  // RADIO PACKET VARIABLES ---------------------------------------------------------------------------------------- 
int16_t secondsToHit;               // Send to target: # of seconds to achieve hit              (will be an int 0-7)
int16_t caliConstant;               // Send to target: value for target to calibrate against    (will be an int 250-999)      // TODO: Determine what constant values we actually want to send. Maybe less than 250?


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

    // Display calibration status to user
    if(firstCalibrationDone)
    {
      display.setCursor(10,10);
      display.println(F("CAL COMPLETE"));
    }
    else
    {
      display.setCursor(10,10);
      display.println(F("CAL INCOMPLETE"));
    }
      
    // Check buttons
    if(digitalRead(triggerButton)==HIGH)               // If trigger is held
    {
      // Display trigger press
      display.setCursor(10,20);
      display.println(F("TRIGGER PRESSED"));
      
      // State transition: Idle -> Targeting
      idle = false;
      targeting = true;
    }
    else if(digitalRead(caliButton)==LOW)           // If calibration button is pressed and trigger is NOT pressed (trigger takes precedence)
    {
      // Display calibration press
      display.setCursor(10,20);
      display.println(F("CALIBRATION PRESSED"));

      caliPackets = 10;       // Packets required to calibrate
      caliNumber = 0;         // Reset variable tracking number of received calibration packets
      
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
    if(digitalRead(triggerButton)==HIGH)               // If button held, attempt to TX targeting packet
    {   
      if(!firstCalibrationDone)                           // We have NOT done a calibration phase, we can't target! Need to do that first!       
      {
        
        display.setCursor(10,10);
        display.println(F("Cal Incomplete!"));
        display.setCursor(10,20);
        display.println(F("Complete Cal to T"));
        display.display();
        
        lightLEDsBad();

      }
      else                                                // We have done at least one calibration phase - can continue with targeting as normal
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
            //Parse incoming packet
            char* incomingPacket = (char*)buf;           // Need to convert to char array
  
            // Print to screen
            display.setCursor(10,20);
            display.print(incomingPacket);
            display.display();
  
            // Decide Hit or Miss
            if(incomingPacket[0] == 'M')      // Hit        (Note: Needs '' to denote CHAR here, do not use "")
            {
                // Adjust LEDs accordingly
                inactiveLEDs -= 1;
                activeLEDs += 1;
                
               
                // Update UI
                // LED
                lightLEDs(inactiveLEDs);   
                // Speaker         
                tone(SPEAKER,activeLEDs*speakerFreqConstant,200);       // Speaker will get higher pitched as more LEDs activate - timing is for 200 ms, may need to adjust this 
  
                // If we've lit all the LEDs, we've got a successful hit!         -- NEED TO MAKE SURE THIS LINES UP WITH TARGET!!!
                if(activeLEDs == NUM_LEDS+1)
                {
                  display.println(F(" | HIT!"));
                  display.display();
                  
                  // State transition
                  targeting = false;
                  hit = true;
                }
                
            }
            else                                // Miss - miss from target
            {
                resetLEDs();
                // Do we want to play a tone here?
            }
          }
          else                                // RX partially recieved (Recieve failure case)
          {
  
            // RX failure treated as targeting miss
            resetLEDs();
            // Do we want to play a tone here?
          }
        }
        else                                  // RX NOT received (timeout)
        {
            display.setCursor(10,20);
            display.println(F("NO RX FROM TARGET"));
            display.display();
  
            // RX not recieved treated as targeting miss
            resetLEDs();
            // Do we want to play a tone here?
        } 
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
    display.print(F("STATE: CALIBRATING "));
    display.println(caliNumber);

   lightLEDsCalibrate(caliPackets);                    // TODO: Make sure this logic works 

    // Check button
    if(digitalRead(triggerButton)==HIGH)               // If trigger held, TX calibration packet
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
          // TODO: Do we bother to read this packet to verify calibration? If so, do that here
          
          // We've sucessfully received a calibration packet
          caliPackets -= 1;
          caliNumber += 1;
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
          // TODO: Do we reset calibration here upon packet not RX'd back (Want 10 contiguous packets?) That might lead to better calibration
          display.setCursor(10,20);
          display.println(F("NO RX FROM TARGET"));
          display.display();
      }

      // If we're done calibrating, state transition
      if(caliPackets <= 0)
      {
        // All LEDs flash light green - they will only be held this way for as long as the tones below last
        for(int i = 0; i <= (NUM_LEDS-1); i++)
        {
          leds[i] = CRGB(128, 255, 0);                // Set given LED to light green (TODO: This is a lime-ish green - is it ugly looking?)
        }
        FastLED.show();
        
        // Play a tone
        tone(SPEAKER,400,500) ;
        delay(400) ;
        tone(SPEAKER,400,500) ;
        delay(10) ;

        firstCalibrationDone = true;      // We can now use the targeting system!
        calibrating = false;
        idle = true;
      }
    }
    else if(digitalRead(caliButton)==LOW)           // If calibration button is pressed and trigger is NOT pressed (trigger takes precedence) we will go back to idle state (Emergency leave calibration state)
    {
      // State transition: Calibrating -> Idle
      calibrating = false;
      idle = true;
    }

    // Will remain in calibration state until calibration sucessful OR calibration button pressed again
    
  }
  else if(hit)
  {
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: HIT!"));

    // Check button
    if(digitalRead(triggerButton)==HIGH)               // If trigger held, remain in hit state
    { 
      // All LEDs GREEN
      for(int i = 0; i <= (NUM_LEDS-1); i++)
      {
        leds[i] = CRGB(0, 255, 0);                // Set given LED to green
      }
      FastLED.show();

      // Continue high-freq speaker tone
      tone(SPEAKER,activeLEDs*speakerFreqConstant,200);       // All LEDs active so at max freq here


      // TODO: Do we continue to TX packets here denoting hits? Could be easy way to make sure L and T sync up, and would allow us to stop target hit upon launcher button unpressed
      
    }
    else                                                // As soon as trigger released, return to idle state
    {
        hit = false;
        idle = true;
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
  pinMode(triggerButton,INPUT);                 // Define trigger button as an input 
  pinMode(caliButton,INPUT_PULLUP);             // Define calibration button as an input


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
  timingPotAdjustedValue = (timingPotRawValue / 140) + 3;       // Translates analog input to seconds   (determined by experimentation, may need to adjust this later!) // TODO: ADD POT CODE
  inactiveLEDs =  timingPotAdjustedValue - 1;                   // Number of LEDs which are to start RED
  activeLEDs = NUM_LEDS - inactiveLEDs;                         // Number of LEDs which are to start BLUE

  // Get time to hit from pot
  secondsToHit = timingPotAdjustedValue;
  ////////////////////////////////

  // Set LED starting colors
  lightLEDs(inactiveLEDs);

}

// Set LED colors using global variables (Be careful with side effects!) - these are RED/BLUE for targeting phase
void lightLEDs(int numReds)
{
      // RED (inactive)
    for(int i = 0; i <= numReds; i++)
  {
    leds[i] = CRGB(255, 0, 0);                // Set given LED to red
  }
    // BLUE (active, pre-counted)
  for(int i = numReds+1; i <= (NUM_LEDS-1); i++)
  {
    leds[i] = CRGB(0, 0, 255);                // Set given LED to blue
  }

  // Show LEDs
  FastLED.show();
}

// Set LED colors using global variables (Be careful with side effects!) - these are RED/YELLOW for calibration phase
void lightLEDsCalibrate(int numReds)
{
      // RED (inactive)
    for(int i = 0; i <= numReds; i++)
  {
    leds[i] = CRGB(255, 0, 0);                // Set given LED to red
  }
    // BLUE (active, pre-counted)
  for(int i = numReds+1; i <= (NUM_LEDS-1); i++)
  {
    leds[i] = CRGB(255, 255, 0);              // Set given LED to yellow
  }

  // Show LEDs
  FastLED.show();
}

// Bad State
void lightLEDsBad()
{
    // All LEDs purple to denote bad state
    for(int i = 0; i <= (NUM_LEDS-1); i++)
    {
      leds[i] = CRGB(255, 0, 255);                  // Set given LED to purple
    }
    FastLED.show();

    // Play a tone
    tone(SPEAKER,400,300) ;
    delay(400) ;
    tone(SPEAKER,400,300) ;
    delay(10) ;
}
