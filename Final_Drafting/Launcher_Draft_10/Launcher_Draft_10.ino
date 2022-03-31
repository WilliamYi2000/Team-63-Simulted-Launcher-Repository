// Launcher Draft 10
// Mk 10 of complete launcher system
// Implements secreen, radio, and UI, and expects 2-way comm. with a target
// Implements improvements from previous launcher versions
// Handles synchronization and communication with target

// Upgraded version of fully working system code
// Use with Target_Draft_7

  // IMPORTS  ----------------------------------------------------------------------------------------------------------
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
#include <Wire.h>                 // Screen
#include <Adafruit_GFX.h>         // Screen
#include <Adafruit_SSD1306.h>     // Screen
#include <FastLED.h>              // LED Strip


  // RADIO MACROS ------------------------------------------------------------------------------------------------------
// Pins; Matches Adafruit wiring diagram
#define RFM95_RST     2           // Reset
#define RFM95_CS      4           // Chip select
#define RFM95_INT     3           // Interrupt

// Misc radio macros
#define RF95_FREQ     915.0       // Radio frequency (TX Freq, must match RX's Freq!)
#define RF95_TXPOW    23          // Transmisson power (default is 13 dBmn, max is 23 dBmn)
#define TimeoutPeriod 1500        // Time (ms) for a message to timeout
#define TX_Delay      350         // Time (ms) we delay before sending an ACK after senind a message

// Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


  // SCREEN MACROS ----------------------------------------------------------------------------------------------------
// Screen size          (we can get 3 lines x 20 characters on screen at once)
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
int activeLEDs;                   // Iterating variable used to change the color of a specific LED in the LED array (ACTIVE - blue)
int inactiveLEDs;                 // Iterating variable used to change the color of a specific LED in the LED array (INACTIVE - red)

// Instance of LED driver
CRGB leds[NUM_LEDS];              // This controls our LED strip, each item in array is a single LED        


  // SPEAKER MACROS -------------------------------------------------------------------------------------------------
// Speaker Constants
#define SPEAKER     6             // Speaker using GPIO 6 - pd6
#define BEATTIME 1000             // Length of the generated tone (msec)

// Speaker Variables
int speakerFreqConstant = 75;     // Amount that frequency increases each time we get a hit


  // BUTTON/POTENTIOMETER MACROS ------------------------------------------------------------------------------------
// Timing pot on Analog pin 0
int timingPotRawValue;            // Stores raw input from timing potentiometer
int timingPotAdjustedValue;       // Stores adjusted value from timing potentiometer (time in seconds)

#define triggerButton     8       // Pin number of the trigger button
#define caliButton        5       // Pin number of the calibration button


  // LAUNCHER STATE MACROS -----------------------------------------------------------------------------------------
// ONLY ONE of these can be true at a time! Make sure to set old state low when new state goes high (replace these with an int?)
bool idle = true;                   // Default state: Accept UI input from user and adjust UI output accordingly, wait for trigger press                                  (goes to either calibration state or targeting state)
bool calibrating = false;           // Send calibration packet to target on trigger press; stays in this state until enough response packets from target received         (returns to idle state upon 10 response packets sucessfully RX'd, or cali button pressed again)
bool targeting = false;             // Send targeting packet to target and get response, adjust UI based on hit or miss                                                   (goes to idle state upon letting up on trigger, or hit state upon RX enough response packets in a row)
bool hit = false;                   // Set UI to HIT until user lets go of button                                                                                         (returns to idle state upon letting up on trigger)

bool firstCalibrationDone = false;               // Have we completed at least one calibration cycle? If not, we cannot send packets to target! Need to complete at least on calibration cycle first
bool calibrationPressedAndReleased = true;       // Makes sure we have released calibration button before trying to press it again; used to ensure state stability when trying to leave calibration state early
int universalDelay = 100;                        // Delay of every loop in ms

  // RADIO PACKET VARIABLES ---------------------------------------------------------------------------------------- 
#define caliPacketsRequired   10    // Number of packets required to calibrate
int caliNumber = 0;                 // The number of calibration packets we've received during a given calibration phase (Resets to 0 for every calibration phase)
int caliPackets;                    // Number of packets we need to calibrate (not necessary to define this here; this is reset whenever we enter the calibration phase, that's where any edits to # of packets should be made)    



  // Setup  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  // Run setup methods
  screenSetup();    // Screen needs to be first since that's what we're printing debugging messages on
  UISetup();        // Setup UI second so we can output radio issues on UI too
  radioSetup();     // Setup radio last

  // Delay before beginning loop
    // TODO: Do we want to play a tone or use LEDs here?
  delay(2000);
}


  // Loop  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{  
  if(idle)                                                // Base State: IDLE ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  {
    // Print state to screen
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: IDLE"));
    
    // Check Pot and LEDs
    resetLEDs();

    // Display calibration status to user
    if(firstCalibrationDone)                      // If we have done at least one calibration since last power cycle
    {
      display.setCursor(10,10);
      display.println(F("CAL COMPLETE"));
    }
    else                                          // If we have NOT done at least one calibration since last power cycle
    {
      display.setCursor(10,10);
      display.println(F("CAL INCOMPLETE"));
    }
      
    // Check buttons
    if(digitalRead(triggerButton)==HIGH)               // If trigger is held (highest priority)
    {
      // Display trigger press
      display.setCursor(10,20);
      display.println(F("TRIGGER PRESSED"));
      
      // State transition: Idle -> Targeting
      idle = false;
      targeting = true;
    }
    else if((digitalRead(caliButton)==LOW) && calibrationPressedAndReleased)           // If calibration button is pressed and trigger is NOT pressed (trigger takes precedence)
    {
      // Display calibration press
      display.setCursor(10,20);
      display.println(F("CALIBRATION PRESSED"));

      caliPackets = caliPacketsRequired;       // Packets required to calibrate
      caliNumber = 0;                          // Reset variable tracking number of received calibration packets
      calibrationPressedAndReleased = false;   // Need to release calibration button before we can use it to change state again
      
      // Play a tone
      tone(SPEAKER,400,400);
      delay(10) ;
      
      // State transition: Idle -> Calibrating
      idle = false;
      calibrating = true;
    }
    else if((digitalRead(caliButton)==HIGH) && !calibrationPressedAndReleased)      // If calibration button is released, we can use it for a state transition (This ensures stable transition to idle state from calibration state using the button)
    {
      calibrationPressedAndReleased = true;
    }

    display.display();
  }
  else if(targeting)                                    // Firing State /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  {
    // Print state to screen
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: TARGETING"));

    // Check button
    if(digitalRead(triggerButton)==HIGH)               // If button held, attempt to TX targeting packet
    {   
      if(!firstCalibrationDone)                           // We have NOT done a calibration phase, we can't target! Need to do that first!       
      {
        // Denote bad state on screen
        display.setCursor(10,10);
        display.println(F("Cal Incomplete!"));
        display.setCursor(10,20);
        display.println(F("Complete Cal to T"));            // Running low on characters here... can we shorten this message somehow?
        display.display();

        // Light LEDs to show bad state
        lightLEDsOneColor(255, 0, 255);                   // Purple (Bad state)

        // Play a tone
        tone(SPEAKER,400,300) ;
        delay(400) ;
        tone(SPEAKER,400,300) ;
        delay(10) ;
      }
      else                                                // We have done at least one calibration phase - can continue with targeting as normal
      {
        // TX targeting packet to target
        char radiopacket[5] = "T   ";                   // Build targeting packet literal
        int16_t targetingNum = inactiveLEDs;            // Number of LEDs for the target to light up red
        itoa(targetingNum, radiopacket+1, 10);          // Adding hit number
        rf95.send((uint8_t *)radiopacket, 6);           // SEND PACKET  (6 characters)
        delay(10);                                      // Short delay for safety
        rf95.waitPacketSent();                          // Pause until packet fully sent

        // Display our sent packet
        display.setCursor(10,10);
        display.println(radiopacket);
        display.display();

        // RX response packet from target
        //delay(TX_Delay);                                    // Delay needs to be long enough for receiver to interp TX'd packet and send a response; also needs to be long enough for our transmitter to swap from TX to RX; assuming RTT negligible
  
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];               // Build RX buffer
        uint8_t len = sizeof(buf);                          // Need length of buffer
        
        if (rf95.waitAvailableTimeout(TimeoutPeriod))       // Timeout length
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
            if(incomingPacket[0] == 'H')          // Hit - check first char of received message, will either be 'H' or 'M'        (Note: Needs '' to denote CHAR here, do not use "")
            {
                // Adjust LEDs accordingly
                inactiveLEDs -= 1;
                activeLEDs += 1;
                
                // Update UI
                  // LED
                lightLEDsTwoColors(inactiveLEDs, 255, 0, 0, 0, 0, 255);        // Red, Blue   
                  // Speaker         
                tone(SPEAKER,activeLEDs*speakerFreqConstant,200);             // Speaker will get higher pitched as more LEDs activate - timing is for 200 ms, may need to adjust this 
  
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
            else if(incomingPacket[0] == 'M')      // Miss - miss from target
            {
              // Print to screen
              display.setCursor(10,20);
              display.print(incomingPacket);
              display.display();

              // Light LEDs to show miss
              lightLEDsOneColor(125, 0, 0);                   // Light Red
      
              // Play a tone
              tone(SPEAKER,200,300) ;
              delay(200) ;
             
              resetLEDs();
            
            }
            else if(incomingPacket[0] == 'B')      // BAD - "bad" response from target, this happens if we try to send target packet but target has no calibration data (this might happen if we calibrate then power cycle the target)
            {
              // Denote bad state on screen
              display.setCursor(10,10);
              display.println(F("Cal Incomplete!"));
              display.setCursor(10,20);
              display.println(F("T: No Cal Data"));            // Running low on characters here... can we shorten this message somehow?
              display.display();
      
              // Light LEDs to show bad state
              lightLEDsOneColor(255, 0, 255);                   // Purple (Bad state)
      
              // Play a tone
              tone(SPEAKER,400,300) ;
              delay(400) ;
              tone(SPEAKER,400,300) ;
              delay(10) ;

              // Reset
              resetLEDs();                        // Reset LEDs
              firstCalibrationDone = false;       // Return to uncalibrated state
            }
            else                                // Unrecognized response packet - treated as miss from target (anything that isn't a hit is treated as a miss)
            {
              // Denote bad state on screen
              display.setCursor(10,10);
              display.println(F("T:BAD RESPONSE"));
              display.setCursor(10,20);
              display.println(F("RX'd unrec pkt"));
              display.display();

              // Light LEDs to show bad state
              lightLEDsOneColor(255, 0, 255);                   // Purple (Bad state)
      
              // Play a tone
              tone(SPEAKER,400,300) ;
              delay(400) ;
              tone(SPEAKER,400,300) ;
              delay(10) ;

              // Reset
              resetLEDs();
            }
          }
          else                                // RX partially recieved (Recieve failure case)
          {
            // Denote bad state on screen
            display.setCursor(10,10);
            display.println(F("RX Fail"));
            display.setCursor(10,20);
            display.println(F("No Target ACK"));
            display.display();
          
            // RX failure treated as targeting miss
            resetLEDs();
            // TODO: Do we want to play a tone here?
          }
        }
        else                                  // RX NOT received (timeout)
        {
            display.setCursor(10,20);
            display.println(F("NO RX FROM TARGET"));
            display.display();

  
            // RX not recieved treated as targeting miss            
            // Light LEDs to show miss
            lightLEDsOneColor(125, 0, 0);                   // Light Red
    
            // Play a tone
            tone(SPEAKER,200,300) ;
            delay(200) ;

            // Reset LEDs
            resetLEDs();
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
  else if(calibrating)                                    // Calibrating State //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  {
    // Print state to screen
    display.clearDisplay();
    display.setCursor(10,0);
    display.print(F("STATE: CAL "));
    display.println(caliNumber);

    // Light LEDs       // This code is hard-set on exactly 10 calibration packets. If we change that this logic will need to be modified
    lightLEDsTwoColors(caliPackets-1, 255, 255, 0, 0, 0, 255);        // Yellow, Blue  
    //lightLEDsTwoColors(caliNumber-1, 0, 0, 255, 255, 255, 0);        // Yellow, Blue   (reciprocal direction -> counts backwards)
    
    // Check button
    if(digitalRead(triggerButton)==HIGH)               // If trigger held, TX calibration packet
    {      
      // TX calibration packet to target
      
      // Calibration packet = C[Calibration Number]
      char radiopacket[5] = "C   ";                   // Build calibration packet
      int16_t caliNumber_TX = caliNumber;             // Send current cali number
      itoa(caliNumber_TX, radiopacket+1, 10);         // Adding calibration number to packet
      rf95.send((uint8_t *)radiopacket, 6);           // SEND PACKET  (6 characters)
      delay(10);                                      // Short delay for safety
      rf95.waitPacketSent();                          // Pause until packet fully sent

      // Display our sent packet
      display.setCursor(10,10);
      display.println(radiopacket);
      display.display();
      
      // RX response packet from target
      //delay(TX_Delay);           // Delay needs to be long enough for receiver to interp TX'd packet and send a response; also needs to be long enough for our transmitter to swap from TX to RX; assuming RTT negligible

      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];               // Build RX buffer
      uint8_t len = sizeof(buf);                          // Need length of buffer
      if (rf95.waitAvailableTimeout(TimeoutPeriod))                // Timeout
      { 
        if (rf95.recv(buf, &len))           // RX successfully within timeout (RX read into buf)
        {
          // Print received packet to screen
          display.setCursor(10,20);
          display.println((char*)buf);        // Need to convert to char!
          display.display();

          // If we've received a response packet, we successfully got a calibration packet out (need 10 total to calibrate)
          // TODO: Do we bother to parse this packet to verify calibration? If so, do that here
          
          // At this point, we've sucessfully received a calibration packet
          caliPackets -= 1;
          caliNumber += 1;

          // Play a tone
          tone(SPEAKER,(caliNumber)*(speakerFreqConstant),200); 
        }
        else                                // RX partially recieved (Recieve failure case)
        {
          display.setCursor(10,20);
          display.println(F("RX FAILURE"));
          display.display();

          // Calibration packets must be contiguous - can't let go of trigger or miss a packet
          caliPackets = caliPacketsRequired;       // Reset req number of cali packets
          caliNumber = 0;                          // Reset variable tracking number of received calibration packets

          // TODO: DO we want to play a tone here?
        }
      }
      else                                  // RX NOT received (timeout)
      {
          display.setCursor(10,20);
          display.println(F("NO RX FROM TARGET"));
          display.display();

          // Calibration packets must be contiguous - can't let go of trigger or miss a packet
          caliPackets = caliPacketsRequired;       // Reset req number of cali packets
          caliNumber = 0;                          // Reset variable tracking number of received calibration packets

          // TODO: DO we want to play a tone here?
      }

      // If we're done calibrating, state transition
      if(caliPackets <= 0)
      {
        // All LEDs flash blue/green - they will only be held this way for as long as the tones below last (short time), then will switch back to normal when returning to idle state
        lightLEDsOneColor(0, 255, 255);
        
        // Play a tone
        tone(SPEAKER,400,500) ;
        delay(400) ;
        tone(SPEAKER,400,500) ;
        delay(10) ;

        firstCalibrationDone = true;      // If this is our first calibration, we can now use the targeting system!
        
        // State transition: Calibrating -> Idle
        calibrating = false;
        idle = true;
      }
    }
    else if((digitalRead(caliButton)==LOW) && calibrationPressedAndReleased)           // If calibration button is pressed and trigger is NOT pressed (trigger takes precedence) we will go back to idle state (Manual leave calibration state)
    {
      calibrationPressedAndReleased = false;
      
      // State transition: Calibrating -> Idle
      calibrating = false;
      idle = true;
    }
    else if((digitalRead(caliButton)==HIGH) && !calibrationPressedAndReleased)          // If calibration button is released and trigger is NOT pressed (trigger takes precedence), we can use it for a state transition (This ensures stable transition to idle state from calibration state using the button)
    {
      calibrationPressedAndReleased = true;
    }
    
    if(digitalRead(triggerButton)==LOW)                                // If trigger released during calibration
    {
      // Calibration packets must be contiguous - can't let go of trigger or miss a packet
      caliPackets = caliPacketsRequired;       // Reset req number of cali packets
      caliNumber = 0;                          // Reset variable tracking number of received calibration packets

      // TODO: DO we want to play a tone here?
    }

    //////////// Will remain in calibration state until calibration sucessful OR calibration button pressed again
    
  }
  else if(hit)                                      // Hit state ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  {
    // Print state to screen
    display.clearDisplay();
    display.setCursor(10,0);
    display.println(F("STATE: HIT!"));
  
    // All LEDs GREEN - this will always occur at least once, since we have gotten a hit if we reach this state
    lightLEDsOneColor(0, 255, 0);    

    // Check button
    if(digitalRead(triggerButton)==HIGH)               // If trigger held, remain in hit state
    { 
        // TX targeting packet to target
        char radiopacket[5] = "T   ";                   // Build targeting packet literal; target expects packets in this state to look identical to normal targeting packets, so continue to use that format
        int16_t targetingNum = 0;                       // Number of LEDs for the target to light up red (zero for the case of a hit is what target is expecting)
        itoa(targetingNum, radiopacket+1, 10);          // Adding hit number
        rf95.send((uint8_t *)radiopacket, 6);           // SEND PACKET  (6 characters)
        delay(10);                                      // Short delay for safety
        rf95.waitPacketSent();                          // Pause until packet fully sent
  
        // Display our sent packet
        display.setCursor(10,10);
        display.println(radiopacket);
        display.display();

        // RX response packet from target
        //delay(TX_Delay);                                    // Delay needs to be long enough for receiver to interp TX'd packet and send a response; also needs to be long enough for our transmitter to swap from TX to RX; assuming RTT negligible
  
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];               // Build RX buffer
        uint8_t len = sizeof(buf);                          // Need length of buffer
        
        if (rf95.waitAvailableTimeout(TimeoutPeriod))       // Timeout length
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
            if(incomingPacket[0] == 'H')      // Hit - check first char of received message, will either be 'H' or 'M'        (Note: Needs '' to denote CHAR here, do not use "")
            {
              // All LEDs GREEN
              lightLEDsOneColor(0, 255, 0);             
              
              // Continue high-freq speaker tone
              tone(SPEAKER,(activeLEDs+1)*speakerFreqConstant,200);       // All LEDs active so at max freq here  
            }
            else                                // Miss - miss from target (anything that isn't a hit is treated as a miss)
            {
              // Print to screen
              display.setCursor(10,20);
              display.print(incomingPacket);
              display.display();

              // Light LEDs to show miss
              lightLEDsOneColor(125, 0, 0);                   // Light Red
      
              // Play a tone
              tone(SPEAKER,200,300) ;
              delay(200) ;
             
              resetLEDs();
              
              // State transition: Hit -> Idle
              hit = false;
              idle = true;
            }
          }
          else                                // RX partially recieved (Recieve failure case)
          {
            // RX failure treated as targeting miss

                          // Print to screen
              display.setCursor(10,20);
              display.print("RX FAIL");
              display.display();

              // Light LEDs to show miss
              lightLEDsOneColor(125, 0, 0);                   // Light Red
      
              // Play a tone
              tone(SPEAKER,200,300) ;
              delay(200) ;
             
              resetLEDs();
            
            // State transition: Hit -> Idle
            hit = false;
            idle = true;
          }
        }
        else                                  // RX NOT received (timeout)
        {
            display.setCursor(10,20);
            display.println(F("NO RX FROM TARGET"));
            display.display();

            // Light LEDs to show miss
            lightLEDsOneColor(125, 0, 0);                   // Light Red
    
            // Play a tone
            tone(SPEAKER,200,300) ;
            delay(200) ;
  
            // State transition: Hit -> Idle
            hit = false;
            idle = true;
        } 
    }
    else                                                // As soon as trigger released, return to idle state
    {
        // State transition: Hit -> Idle
        hit = false;
        idle = true;
    }
  }



  ////////////////////////////////

  delay(universalDelay);           // Delay between loop iterations
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
void radioSetup()           // MUST BE CALLED AFTER SCREEN AND UI SETUPS DONE
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
    display.setCursor(10,20);               // Set screen cursor to position 3
    display.println(F("RADIO INIT BAD"));   // Print that radio was not initialized properly
    display.display();                      // Display the message
    
    while(true)                             // Infinite loop! 
    {
        // Flash LEDs purple to denote bad state
        lightLEDsOneColor(255, 0, 255);                   // Purple (Bad state)
        delay(1000);
        lightLEDsOneColor(0, 0, 0);                       // LEDs Off
        delay(1000);
    }
  }

  ///////////////////////////////

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))      // If radio frequency is not properly initialized
  {
    display.setCursor(10,10);               // Set screen cursor to position 2
    display.println(F("RADIO FREQ BAD"));   // Print that radio frequency was not initialized properly
    display.display();                      // Display the message
    
    while(true)                             // Infinite loop! 
    {
        // Flash LEDs purple to denote bad state
        lightLEDsOneColor(255, 0, 255);                   // Purple (Bad state)
        delay(1000);
        lightLEDsOneColor(0, 0, 0);                       // LEDs Off
        delay(1000);
    }
  }

  ////////////////////////////////

  // Set TX power
  rf95.setTxPower(RF95_TXPOW, false);

  ////////////////////////////////

  display.setCursor(10,20);               // Set screen cursor to position 3
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
  
  display.setCursor(10,10);                // Set screen cursor to position 2
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
  timingPotAdjustedValue = (timingPotRawValue / 140) + 3;       // Translates analog input to seconds   (determined by experimentation, may need to adjust this later!)
  inactiveLEDs =  timingPotAdjustedValue - 1;                   // Number of LEDs which are to start RED
  activeLEDs = NUM_LEDS - inactiveLEDs;                         // Number of LEDs which are to start BLUE
  
  ////////////////////////////////

  // Set LED starting colors
  lightLEDsTwoColors(inactiveLEDs, 255, 0, 0, 0, 0, 255);        // Red, Blue
}


// Set LED colors (Passing LED to switch color at, RBG Color 1, RGB Color 2)
void lightLEDsTwoColors(int colorChangeLocation, int r1, int g1, int b1, int r2, int g2, int b2)
{
      // RED (inactive)
    for(int i = 0; i <= colorChangeLocation; i++)
  {
    leds[i] = CRGB(r1, g1, b1);                // Set given LED to RGB 1 
  }
    // BLUE (active, pre-counted)
  for(int i = colorChangeLocation+1; i <= (NUM_LEDS-1); i++)
  {
    leds[i] = CRGB(r2, g2, b2);                // Set given LED to RGB 2
  }

  // Show LEDs
  FastLED.show();
}

// Set LED colors (one color)
void lightLEDsOneColor(int r, int g, int b)
{
    for(int i = 0; i <= (NUM_LEDS-1); i++)
    {
      leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
}
