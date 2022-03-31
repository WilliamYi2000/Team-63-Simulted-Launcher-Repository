// Target Draft 4
// Mk 4 of complete launcher system
// Implements secreen, radio, and UI, and expects 2-way comm. with a launcher
// Synchronizes with launcher by responding to launcher signals (target does not maintain own state, state set by incoming message, or lack thereof)

// USES ONLY RADIO 1 FOR TX OF RESPONSE PACKETS TO LAUNCHER


  // IMPORTS  ----------------------------------------------------------------------------------------------------------
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
#include <Wire.h>                 // Screen
#include <Adafruit_GFX.h>         // Screen
#include <Adafruit_SSD1306.h>     // Screen
#include <FastLED.h>              // LED Strip


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


  // Module 4
#define RFM95_4_RST     26
#define RFM95_4_CS      25
#define RFM95_4_INT     3


// Misc radio macros
#define RF95_FREQ     915.0   // Radio frequency (TX Freq, must match RX's Freq!)
#define RF95_TXPOW    23      // Transmisson power (default is 13 dBmn max is 23)
#define TimeoutPeriod 1000    // Time (ms) for a message to timeout

// Instance of the radio driver
RH_RF95 rf95_1(RFM95_1_CS, RFM95_1_INT);    // Module 1
RH_RF95 rf95_2(RFM95_2_CS, RFM95_2_INT);    // Module 2
RH_RF95 rf95_3(RFM95_3_CS, RFM95_3_INT);    // Module 3
RH_RF95 rf95_4(RFM95_4_CS, RFM95_4_INT);    // Module 4

// Calibration vars
  // Calibration parameters
int cur_cal_num = 0;           // Calibration iterator for current calibration loop; counts up to cal max as we recive messages
int cal_max_num = 10;          // Calibration upper bound; total number of messages we need to successfully calibrate
bool firstCalibrationDone = false;               // Have we completed at least one calibration cycle? If not, we cannot target! Need to complete at least on calibration cycle first
  
  // Min acceptable RSSI values for hit to be detected
// Values for calibration
int r1_min;
int r2_min;
int r3_min;
int r4_min;

// Values for targeting
int r1_targeting_min;
int r2_targeting_min;
int r3_targeting_min;
int r4_targeting_min;

// Number of radios required for a hit (out of four)
int radiosForHit = 4;
int idleTime = 0;

// Number of idle loops required for timeout
int universalDelay = 100;       // Delay of every loop in ms
int timeOutLoops = 20
;          // Timeout will be universalDelay * timeOutLoops


  // SCREEN MACROS ----------------------------------------------------------------------------------------------------
// Screen size
#define SCREEN_WIDTH 128                            // OLED display width, in pixels
#define SCREEN_HEIGHT 32                            // OLED display height, in pixels

// Misc screen macros
#define OLED_RESET     4                            // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C                         // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


  // LED MACROS ------------------------------------------------------------------------------------------------------
// LED Strip
#define LED_PIN     37                              // GPIO for LED strip
#define NUM_LEDS    20                              // Total number of LEDs in LED strip
#define NUM_LEDS_LAUNCHER 10                        // Total number of LEDs on the launcher

// LED multicolor
#define LED_R 27                                    // Red
#define LED_G 28                                    // Green
#define LED_B 29                                    // Blue

int activeLEDs;                                     // Iterating variable used to change the color of a specific LED in the below array (ACTIVE - blue)
int inactiveLEDs;                                   // Iterating variable used to change the color of a specific LED in the below array (INACTIVE - red)
int ledsPerPacket = (NUM_LEDS / cal_max_num);       // Number of LEDs that light up per packet received     // TODO: Check this logic!

CRGB leds[NUM_LEDS];                                // Instance of LED driver; this controls our LED strip, each item in array is a single LED        


  // SPEAKER MACROS -------------------------------------------------------------------------------------------------
#define SPEAKER    35         // Pin number of the speaker
#define BEATTIME 1000         // Length of the generated tone (msec)

// Speaker Variables
int speakerFreqConstant = 75;     // Amount that frequency increases each time we get a hit (this is the same number the launcher is using)


  // Setup  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  // Set up PCB RGB LED; need to do this before setup methods so we can use them to denote bad state for scren
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  // Run setup methods
  screenSetup();          // Screen needs to be first since that's what we're printing debugging messages on
  UISetup();              // Setup UI second
  fourRadioSetup();       // Setup radios last
  
  // Delay before beginning loop
  delay(2000);
}


  // Loop  ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{  
  // Idle LED
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  
  if (rf95_1.available() && rf95_2.available() && rf95_3.available() && rf95_4.available())
  {
    // Clear the screen
    display.clearDisplay();

    idleTime = 0;
    
    // Working LED
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);

    // Buffers for radio data
      // Radio 1
    uint8_t buf_1[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_1 = sizeof(buf_1);
      // Radio 2
    uint8_t buf_2[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_2 = sizeof(buf_2);
      // Radio 3
    uint8_t buf_3[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_3 = sizeof(buf_3);
      // Radio 4
    uint8_t buf_4[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_4 = sizeof(buf_4);

    // We've got a message; read the message
    if (rf95_1.recv(buf_1, &len_1) && rf95_2.recv(buf_2, &len_2)  && rf95_3.recv(buf_3, &len_3) && rf95_4.recv(buf_4, &len_4))
    {
      // Read in all four messages (they should all be the same)
      char* incomingPacket_1 = (char*)buf_1;
      char* incomingPacket_2 = (char*)buf_2;
      char* incomingPacket_3 = (char*)buf_3;
      char* incomingPacket_4 = (char*)buf_4;

      // TODO: Check that all packets are the same

      // Get RSSI values for later use
      int r1_rssi = rf95_1.lastRssi();
      int r2_rssi = rf95_2.lastRssi();
      int r3_rssi = rf95_3.lastRssi();
      int r4_rssi = rf95_4.lastRssi();

      // Now that we've verified all four packets, we can just look at one packet's data
      if(incomingPacket_1[0] == 'C')              // Calibration packet
      {        
        // Look at RSSIs and calibrate
        if(cur_cal_num == 0)              // If it's our first calibration cycle
        {
           // Set min to current value
           r1_min = r1_rssi;
           r2_min = r2_rssi;
           r3_min = r3_rssi;
           r4_min = r4_rssi;
        }
        else                              // If it isn't our first calibration cycle
        {
          // Only store min if it's less than current
            // Radio 1
          if(r1_rssi < r1_min)
          {
            r1_min = r1_rssi;
          }
            // Radio 2
          if(r2_rssi < r2_min)
          {
            r2_min = r2_rssi;
          }
            // Radio 3
          if(r3_rssi < r3_min)
          {
            r3_min = r3_rssi;
          }
            // Radio 4
          if(r4_rssi < r4_min)
          {
            r4_min = r4_rssi;
          }
        }

        // Add one to cal number
        cur_cal_num += 1;

        // Print info to screen
          // State
        display.setCursor(10,0);
        display.print(F("STATE: CAL "));
        display.print(cur_cal_num);
          // Radio 1 and 2
        display.setCursor(10,10);
        display.print(F("R1: "));
        display.print(r1_min);
        display.print(F("   R2: "));
        display.print(r2_min);
          // Radio 3 and 4
        display.setCursor(10,20);
        display.print(F("R3: "));
        display.print(r3_min);
        display.print(F("   R4: "));
        display.print(r4_min);

        // Light LEDs, prep values for targeting, and send response
        if(cur_cal_num >= cal_max_num)          // We've sucessfully calibrated! Ready to target
        {
          // LEDs should all be blue
          lightLEDsOneColor(0, 0, 255);

          // Prep targeting values
          r1_targeting_min = r1_min;
          r2_targeting_min = r2_min;
          r3_targeting_min = r3_min;
          r4_targeting_min = r4_min;

          // Ready to target
          firstCalibrationDone = true;

          // TX response packet to launcher
          // Response packet = CR                         // Calibration Ready
          char radiopacket[6] = "CR   ";                  // Build calibration packet
          int16_t caliNumber_TX = cur_cal_num;            // Send current cali number
          itoa(caliNumber_TX, radiopacket+2, 10);         // Adding calibration constant to packet; calibration constant is represented as three decimals beyond a 0 (ex. 0.123) - we want to send values from 0.250 - 0.999 (Check this via testing?)
          rf95_1.send((uint8_t *)radiopacket, 6);         // SEND PACKET  (6 characters)
          delay(10);                                      // Short delay for safety
          rf95_1.waitPacketSent();                        // Pause until packet fully sent
        }
        else                                    // Not done calibrating yet
        {
          // LEDs should be blue and yellow
          lightLEDsTwoColors((cur_cal_num*ledsPerPacket), 255, 255, 0, 0, 0, 255);        // Yellow, Blue 

          // TX response packet to launcher
          // Response packet = CC                         // Calibration Continuing
          char radiopacket[6] = "CC   ";                  // Build calibration packet
          int16_t caliNumber_TX = cur_cal_num;            // Send current cali number
          itoa(caliNumber_TX, radiopacket+2, 10);         // Adding calibration constant to packet; calibration constant is represented as three decimals beyond a 0 (ex. 0.123) - we want to send values from 0.250 - 0.999 (Check this via testing?)
          rf95_1.send((uint8_t *)radiopacket, 6);         // SEND PACKET  (6 characters)
          delay(10);                                      // Short delay for safety
          rf95_1.waitPacketSent();                        // Pause until packet fully sent
        }
        
      }
      
      else if(incomingPacket_1[0] == 'T')         // Targeting packet
      {
        // Reset calibration number - must be done whenever we leave calibration state
        cur_cal_num = 0;
        
        if(!firstCalibrationDone)                           // We have NOT done a calibration phase, we can't target! Need to do that first!       
        {
          // Denote bad state on screen
          display.setCursor(10,0);
          display.println(F("STATE: TARGETING"));
          display.setCursor(10,10);
          display.println(F("Cal Incomplete!"));
          display.setCursor(10,20);
          display.println(F("Complete Cal to T"));            // Running low on characters here... can we shorten this message somehow?
          display.display();
  
          // Light LEDs to show bad state
          lightLEDsOneColor(255, 0, 255);                     // Purple (Bad state)
          delay(10);
        }
        else                                                // We have done at least one calibration phase - can continue with targeting as normal
        {
            // Denote state on screen
            display.setCursor(10,0);
            display.println(F("STATE: TARGETING"));
            display.setCursor(10,10);                         // Prepare cursor to print out Hit/Miss data
            
            // Hit detection
          // Keep track of which radios are hits and which are misses   (0 = miss, 1 = hit)
          int r1_hit = 0;
          int r2_hit = 0;
          int r3_hit = 0;
          int r4_hit = 0;
  
          // Radio 1
          if(r1_targeting_min <= r1_rssi)
          {
            display.print(F("1H "));
            r1_hit = 1;
          }
          else
          {
            display.print(F("1M "));
          }
  
          // Radio 2
          if(r2_targeting_min <= r2_rssi)
          {
            display.print(F("2H "));
            r2_hit = 1;
          }
          else
          {
            display.print(F("2M "));
          }
  
          // Radio 3
          if(r3_targeting_min <= r3_rssi)
          {
            display.print(F("3H "));
            r3_hit = 1;
          }
          else
          {
            display.print(F("3M "));
          }
  
          // Radio 4
          if(r4_targeting_min <= r4_rssi)
          {
            display.print(F("4H"));
            r4_hit = 1;
          }
          else
          {
            display.print(F("4M"));
          }
          
          // Prepare cursor to print out Hit/Miss 
          display.setCursor(10,20);

          // Determine Hit/Miss and act accordingly
          if(r1_hit + r2_hit + r3_hit + r4_hit >= radiosForHit)           // HIT
          {  
            // Unpack the rest of the message to get the number of LEDs to light up
            char passedHitNum = incomingPacket_1[1];              // Get hit num from message; should be second character         // TODO: Only reads one char! VERIFY THIS WORKS IN ALL CASES!
            int hitNum = passedHitNum - '0';                      // Convert from char to int

           // Print to screen
           display.print(F("HIT: "));
           display.print(hitNum);             

            if(hitNum == 0)     // Sucessfully targeted
            {
              // Print to screen
              display.print(F("  | TS"));                                               // Target sucessful

              // Update UI
                // LEDs
              lightLEDsOneColor(0, 255, 0);                                              // Green
                // Continue high-freq speaker tone
              tone(SPEAKER,(NUM_LEDS_LAUNCHER*speakerFreqConstant),200);                // All LEDs active so at max freq here
            }
            else                // Hit but not done targeting yet
            {
              // Update UI
                // LEDs
              lightLEDsTwoColors((hitNum*ledsPerPacket), 255, 0, 0, 0, 0, 255);          // Red, Blue 
                // Speaker
              tone(SPEAKER,((NUM_LEDS_LAUNCHER-hitNum)*speakerFreqConstant),200);        // Speaker will get higher pitched as more LEDs activate - timing is for 200 ms, may need to adjust this 
  
            }
    
            // Response packet = H                          // Hit
            char radiopacket[2] = "H";                      // Build calibration packet
            rf95_1.send((uint8_t *)radiopacket, 2);         // SEND PACKET  (2 characters)
            delay(10);                                      // Short delay for safety
            rf95_1.waitPacketSent();                        // Pause until packet fully sent
          }
          else                                                            // MISS
          {
            // Print to screen
            display.print(F("MISS"));

            // Reset LEDs
            resetLEDs();                                    // Red 
            
            // TX response packet to launcher
            // Response packet = M                          // Miss
            char radiopacket[2] = "M";                      // Build calibration packet
            rf95_1.send((uint8_t *)radiopacket, 2);         // SEND PACKET  (2 characters)
            delay(10);                                      // Short delay for safety
            rf95_1.waitPacketSent();                        // Pause until packet fully sent
          }
        }
      }
      else                                        // Unrecognzied packet
      {
        // Print state
        display.setCursor(10,0);
        display.println(F("STATE: UNREC PKT"));
        
        // Reset LEDs - this condition counts as a miss (should only occur if message is corrupted)
        resetLEDs();                   // Red 

        // Reset calibration number - must be done whenever we leave calibration state
        cur_cal_num = 0;
      }
    }
  }
  else if(idleTime >= timeOutLoops)               // Timeout        // TODO: Check # of loops to trigger this (timing)
  {
    // Idle state - not currently receiving or working on a message and we've waited long enough for timeout to occur

    // Print state
    display.clearDisplay();     // Clear the screen
    display.setCursor(10,0);
    display.println(F("STATE: IDLE - NO RX"));

    // Reset LEDs
    resetLEDs();                   // Red 
    
  }
  else                                  // Idle
  {
    // Idle state - not currently receiving or working on a message BUT not timed out

    // Don't want to clear display here, since that will wipe out info on the screen we actually want to read
    /*
    display.clearDisplay();     // Clear the screen
    display.setCursor(10,0);
    display.println(F("STATE: IDLE - NO RX"));
    */

    // Counts the number of loops we've been idle; since we know idle loops are ~100 ms we can use this to set timeout
    idleTime += 1;
  }

  // Universal delay
  delay(universalDelay);                     // TODO: Adjust this if necessary
  
  // Display screen
  display.display();
}


// HELPER METHODS ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // Screen setup -----------------------------------------------------------------------------------------------------------------------------------
void screenSetup()
{
  // Screen needs to be first since that's what we're printing debugging messages on
  
  // Make sure screen driver is properly intialized
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))        // If screen is not properly initialized
  {
    // Onboard LED purple to denote bad state
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
    
    while (1);            // Infinite loop!
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
void fourRadioSetup()
{
  // Sets up three radios; all 3 radios should be set up identically
  
  // Set up reset pins
  pinMode(RFM95_1_RST, OUTPUT);       // Define reset pin for r1
  digitalWrite(RFM95_1_RST, HIGH);    // Set reset pin for r1 high
  pinMode(RFM95_2_RST, OUTPUT);       // Define reset pin for r2
  digitalWrite(RFM95_2_RST, HIGH);    // Set reset pin for r2 high
  pinMode(RFM95_3_RST, OUTPUT);       // Define reset pin for r3
  digitalWrite(RFM95_3_RST, HIGH);    // Set reset pin for r3 high
  pinMode(RFM95_4_RST, OUTPUT);       // Define reset pin for r4
  digitalWrite(RFM95_4_RST, HIGH);    // Set reset pin for r4 high

  // Do a manual reset of radio to start
  digitalWrite(RFM95_1_RST, LOW);     // Set RST pin 1 low to reset radio 
  digitalWrite(RFM95_2_RST, LOW);     // Set RST pin 2 low to reset radio
  digitalWrite(RFM95_3_RST, LOW);     // Set RST pin 3 low to reset radio
  digitalWrite(RFM95_4_RST, LOW);     // Set RST pin 4 low to reset radio
  delay(10);                          // Wait 10 ms
  digitalWrite(RFM95_1_RST, HIGH);    // Set RST pin 1 back to high for operation
  digitalWrite(RFM95_2_RST, HIGH);    // Set RST pin 2 back to high for operation
  digitalWrite(RFM95_3_RST, HIGH);    // Set RST pin 3 back to high for operation
  digitalWrite(RFM95_4_RST, HIGH);    // Set RST pin 4 back to high for operation
  delay(10);                          // Wait 10 ms

  ////////////////////////////////

  // Make sure all modules initialized successfully
  while (!rf95_1.init())                          // Radio 1 
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 1 INIT BAD"));   // Print that radio 1 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }
  while (!rf95_2.init())                          // Radio 2 
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 2 INIT BAD"));   // Print that radio 2 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }
  while (!rf95_3.init())                          // Radio 3 
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 3 INIT BAD"));   // Print that radio 3 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }
  while (!rf95_4.init())                          // Radio 4 
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 4 INIT BAD"));   // Print that radio 4 was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop!
  }
  
  ///////////////////////////////

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95_1.setFrequency(RF95_FREQ))            // Radio 1
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 1 FREQ BAD"));   // Print that radio 1 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }
  if (!rf95_2.setFrequency(RF95_FREQ))            // Radio 2
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 2 FREQ BAD"));   // Print that radio 2 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }
  if (!rf95_3.setFrequency(RF95_FREQ))            // Radio 3
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 3 FREQ BAD"));   // Print that radio 3 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }
  if (!rf95_4.setFrequency(RF95_FREQ))            // Radio 4
  {
    display.setCursor(10,20);                 // Set screen cursor to position 3
    display.println(F("RADIO 3 FREQ BAD"));   // Print that radio 4 frequency was not initialized properly
    display.display();                        // Display the message
    while (1);                                // Infinite loop! 
  }

  ////////////////////////////////

  // Set TX power
  rf95_1.setTxPower(RF95_TXPOW, false);   // Set R1 to 23 dBm (default is 13 dBm, valid 5-23)
  rf95_2.setTxPower(RF95_TXPOW, false);   // Set R2 to 23 dBm (default is 13 dBm, valid 5-23)
  rf95_3.setTxPower(RF95_TXPOW, false);   // Set R3 to 23 dBm (default is 13 dBm, valid 5-23)
  rf95_4.setTxPower(RF95_TXPOW, false);   // Set R3 to 23 dBm (default is 13 dBm, valid 5-23)

  ////////////////////////////////

  display.setCursor(10,20);               // Set screen cursor to position 3
  display.println(F("RADIOS READY"));     // Print that radios were initialized properly
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
  
  display.setCursor(10,10);               // Set screen cursor to position 2
  display.println(F("UI READY"));         // Print that UI was initialized properly
  display.display();                      // Display the message
  delay(500);                             // Short delay
}

// Check potentiometer and reset LED strip
void resetLEDs()
{
    lightLEDsOneColor(255, 0, 0);           // Set all LEDs red
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
    // Set whole string to passed color
    for(int i = 0; i <= (NUM_LEDS-1); i++)
    {
      leds[i] = CRGB(r, g, b);
    }
    
    // Show LEDs
    FastLED.show();
}
