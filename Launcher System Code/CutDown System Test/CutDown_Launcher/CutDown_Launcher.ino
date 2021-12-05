// Cut-Down Control Code for Launcher System
// Sends a signal upon button press, waits to receive signal and activates UI upon response
// UI includes LED strip, speaker, user interface

// Does NOT yet integrate hit-detection; any signal will be interpereted as a hit


  // IMPORTS  ---------------------------------------------------------------------------------------------------
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
#include <Wire.h>                 // Screen
#include <Adafruit_GFX.h>         // Screen
#include <Adafruit_SSD1306.h>     // Screen
#include <FastLED.h>              // LED Strip
#include <stdio.h>
#include <stdlib.h>

  // RADIO MACROS -----------------------------------------------------------------------------------------------
// TX Freq, must match RX's Freq!
#define RF95_FREQ 915.0

// Matches Adafruit wiring diagram
#define RFM95_RST     2
#define RFM95_CS      4
#define RFM95_INT     3

// Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Keeps track of the number of packets we've sent
int16_t packetnum = 0; 

  // SCREEN MACROS ----------------------------------------------------------------------------------------------
// Screen size
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Misc screen macros
#define OLED_RESET     4      // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// For display
boolean on;
int count;
int prevTime;
String inString;
String outputMsg;

  // UI MACROS --------------------------------------------------------------------------------------------------
#define LED_PIN     7         //led strip using GPIO 7 - pd7
#define SPEAKER     6         // Pin number of the speaker
const int button = 8;         // Pin number of the trigger button

#define NUM_LEDS    10        // Number of LEDs in string
#define BEATTIME 1000         // Length of the generated tone (msec)

// For LED strip
int sensorRealValue;          //resembles the timing from the potentiometer
int temp;                     //used in the loop() in order to restart the counting process
  
// LED
CRGB leds[NUM_LEDS];
int LEDColor;


  // Setup  -------------------------------------------------------------------------------------------------------
void setup() 
{
  /*
  // Serial setup ------------------------------------------------------
  Serial.begin(115200);     // Start serial connection
  while (!Serial)           // Wait for serial response
  {
    delay(1);
  }
  delay(100);
  */

  // Radio setup -------------------------------------------------------
  // Set up reset pin
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

//  Serial.println("--Module Initialized--");

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Make sure module initialized successfully
  while (!rf95.init()) 
  {
//    Serial.println("LoRa radio init failed");
    while (1);
  }
//  Serial.println("LoRa radio init OK!");

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) 
  {
//    Serial.println("setFrequency failed");
    while (1);
  }
//  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Set TX power
  rf95.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)


  // Screen setup -------------------------------------------------------
  // Make sure screen driver is properly intialized
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
//    Serial.println("Screen setup failed");
    while (1);
  }

  // Show initial display buffer contents on the screen -- the library initializes this with an Adafruit splash screen.
  display.display();                      // Show the splash screen
  delay(2000);                            // Pause for 2 seconds

  // Clear the screen buffer
  display.clearDisplay();
  
  // Set text properties
  display.setTextSize(1);                 // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);    // Draw white text

  // Define screen vars
  on = false;
  count = 0;
  inString = "";
  outputMsg = "";

  // Screen prepped
  display.setCursor(10,0);                // Set screen cursor to position 1
  display.println(F("READY"));            // Print "READY" to screen
  display.display();                      // Display the message
  delay(500);                             // Short delay


  // UI setup -------------------------------------------------------
  // Set up button
  pinMode(button,INPUT);         // define button as an input
  
  //LED setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
  // Read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  
  // Lock on timing will only be from 3-10 sec (based on FSR)
  sensorRealValue = (sensorValue / 140) + 3;      // Translates analog input to seconds
  temp = sensorRealValue;
  int LEDcount = NUM_LEDS - 1; 
  int y = sensorRealValue - 1;                    // Reverse direction of led display
  
  // Sets up the led to update it based on the timing in standby phase
  // essentially assigning an led its respective second
  while (LEDcount >= 0)
  {
    //led will be blue if led/second is already "counted"
    leds[LEDcount] = CRGB(0, 0, 255);
    LEDcount = LEDcount - 1;
  } 
  while (y >= 0)
  {
    //led will be red if it needs to counted down during the lock on phase
    leds[y] = CRGB(255, 0, 0);
    y = y - 1;
  }
  FastLED.show();
  
  // Delay efore beginning loop
  delay(1000);
}


  // Loop  ---------------------------------------------------------------------------------------------------------
void loop()
{
  // LED Defs  -------------------------------------------------------
  int LEDcount = NUM_LEDS - 1;        // Redefine num leds in order to start transition from red to blue
  int y = sensorRealValue - 1;
  int freq = 100;
  
  // Display
  display.clearDisplay();             // Clear display
  display.setCursor(10,0);            // Set screen cursor to position 1
  display.println(temp);              // Print message
  display.display();                  // Display message


  // Check trigger button  ---------------------------------------------
  if (digitalRead(button)==HIGH)      // If button is pressed, we want to TX a packet
  {
      display.setCursor(10,10);                       // Set screen cursor to position 2
      display.println(F("BUTTON PRESS"));             // Add packet to screen buffer
      display.display();
      // Build the message we want to TX  
      char radiopacket[20] = "Message #      ";               // Don't actually care what the message says at this point
      itoa(packetnum++, radiopacket+9, 10);                   // Construct packet <- need to look at itoa method
//      Serial.print("Sending "); Serial.println(radiopacket);  // Print to serial; we're sending a packet
      radiopacket[19] = 0;                                    // Handles packet number


      // TX the message
      delay(10);                                // Small delay; remove this?
      rf95.send((uint8_t *)radiopacket, 20);    // TX packet
      display.setCursor(10, 20);                // Set screen cursor to position 3
      display.println(radiopacket);             // Add packet to screen buffer
      display.display();                        // Display the screen

      // Wait for packet to finish TX
      delay(10);                                // Small delay; remove this?
      rf95.waitPacketSent();                    // Required delay; packet takes non-zero time to send

          // Check for response signal from target; this will mean we've successfully targeted and lock is occuring
      if (rf95.available())     // We've got some message in the buffer
      {    
        // Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
    
        if (rf95.recv(buf, &len))     // We've got the message
        {
          RH_RF95::printBuffer("Received: ", buf, len);     // Buffered data; this is not human readable
    
          // Print to serial
    //      Serial.print("Got: ");
    //      Serial.println((char*)buf);               // Convert to char so we can read it
    //      Serial.print("RSSI: ");
    //      Serial.println(rf95.lastRssi(), DEC);     // Get RSSI of receive signal; for launcher this isn't important, but we're grabbing it anyway 
    
          // Print to screen
            // These two lines would print the reponse from target, but we don't care what that actually says
            // at this point we just need to know that we've received something - screen space is at a premium,
            // need to print only vital info
    //      display.setCursor(10, 0);
    //      display.println((char*)buf);
          display.setCursor(10, 20);                // Set screen cursor to position 3
          display.print(F("RSSI: "));               // Print RSSI; this is our denotation that message was received
          display.println(rf95.lastRssi(), DEC);   
          display.display();                        // Display the screen
    
          // Now we can actually act on the fact that we've received a response
          leds[temp] = CRGB(0, 0, 255);     // Changing the led to red from blue when its respective second is counted down
          temp = temp - 1;
          FastLED.show();
          tone(SPEAKER,freq,BEATTIME) ;     // Build up - may integrate a freq changes based on seconds later
    
        }
        else    // Some issue with receive process
        {
          // Print error to serial
    //      Serial.println("Receive failed");
          
          // Pritn error to screen
          display.setCursor(10, 20);                // Set screen cursor to position 3
          display.print(F("RECEIVE FAIL"));         // Print RSSI; this is our denotation that message was received
          display.display();                        // Display the screen
        }
      } 
  }
  else                                // If button is not pressed, we want to return to standby
  { 
    
    // Reset timing and led strip when button is release
    temp = sensorRealValue;
    
    // Changing components (led strip and display) back to standby phase values
    while (LEDcount >= 0)
    {
      leds[LEDcount] = CRGB(0, 0, 255);
      LEDcount = LEDcount - 1;
    } 
    while (y >= 0)
    {
      leds[y] = CRGB(255, 0, 0);
      y = y - 1;
    }
    
    FastLED.show();
  }


  // Check whether we've successfully made a lock  ------------------------------
  if(temp < 0)    // When the count down has reached 0 indicating a lock on and a hit
  { 
    //maybe a delay on the bounce back from the target system
    int LEDcount = NUM_LEDS - 1;
    while (LEDcount >= 0)
    { 
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
    while (LEDcount >= 0)
    { 
      //led strip will turn of in order to save on power consumption
      leds[LEDcount] = CRGB(0, 0, 0);
      LEDcount = LEDcount - 1;
    } 
    FastLED.show();
    while (1);
  }


  // NEEDS TO BE GREAT ENOUGH TO WAIT FOR RADIO TO SWITCH FROM TX TO RX
  delay(500); 
 }
