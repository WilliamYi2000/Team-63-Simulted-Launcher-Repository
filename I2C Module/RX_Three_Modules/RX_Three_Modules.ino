  // IMPORTS
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
#include <Wire.h>                 // Screen
#include <Adafruit_GFX.h>         // Screen
#include <Adafruit_SSD1306.h>     // Screen


  // RADIO MACROS -----------------------------------------------------------------------------------------------
// TX Freq, must match RX's Freq!
#define RF95_FREQ 915.0

// Custom wiring
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


// Instances of the radio driver
RH_RF95 rf95_1(RFM95_1_CS, RFM95_1_INT);    // Module 1
RH_RF95 rf95_2(RFM95_2_CS, RFM95_2_INT);    // Module 2
RH_RF95 rf95_3(RFM95_3_CS, RFM95_3_INT);    // Module 3


  // SCREEN MACROS ----------------------------------------------------------------------------------------------
// Screen size
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Misc screen macros
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  // LED MACROS -------------------------------------------------------------------------------------------------
#define LED_1 13
#define LED_2 12
#define LED_3 54    // Also does ANALOG

// Setup  -------------------------------------------------------------------------------------------------------
void setup() 
{
  // LED Setup
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  
  // Serial setup ------------------------------------------------------
  Serial.begin(115200);     // Start serial connection
  while (!Serial)           // Wait for serial response
  {
    delay(1);
  }
  delay(100);


  // Screen setup -------------------------------------------------------
  // Make sure screen driver is properly intialized
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println("Screen setup failed");
    while (1);
  }

  // Show initial display buffer contents on the screen -- the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the screen buffer
  display.clearDisplay();

  // Set text properties
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);   


  // Radio setup -------------------------------------------------------
  // Set up reset pin
  pinMode(RFM95_1_RST, OUTPUT);
  digitalWrite(RFM95_1_RST, HIGH);
  pinMode(RFM95_2_RST, OUTPUT);
  digitalWrite(RFM95_2_RST, HIGH);
  pinMode(RFM95_3_RST, OUTPUT);
  digitalWrite(RFM95_3_RST, HIGH);

  Serial.println("--Module Initialized--");

  // Manual reset
  digitalWrite(RFM95_1_RST, LOW);
  digitalWrite(RFM95_2_RST, LOW);
  digitalWrite(RFM95_3_RST, LOW);
  delay(10);
  digitalWrite(RFM95_1_RST, HIGH);
  digitalWrite(RFM95_2_RST, HIGH);
  digitalWrite(RFM95_3_RST, HIGH);
  delay(10);


  // Make sure module initialized successfully
  while (!rf95_1.init()) 
  {
    Serial.println("LoRa radio 1 init failed");
    while (1);
  }
  display.setCursor(10, 0);                 // Set screen cursor to position 1
  display.print(F("Radio 1 Init good"));                
  display.display();
  
  while (!rf95_2.init()) 
  {
    Serial.println("LoRa radio 2 init failed");
    while (1);
  }
  display.setCursor(10, 10);                 // Set screen cursor to position 2
  display.print(F("Radio 2 Init good"));       
  display.display();
  
  while (!rf95_3.init()) 
  {
    Serial.println("LoRa radio 3 init failed");
    while (1);
  }
  display.setCursor(10, 20);                 // Set screen cursor to position 3
  display.print(F("Radio 3 Init good"));  
  display.display();

  
  Serial.println("All LoRa radios init OK!");

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95_1.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency 1 failed");
    while (1);
  }
  if (!rf95_2.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency 2 failed");
    while (1);
  }
  if (!rf95_3.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency 3 failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Set TX power
  rf95_1.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)
  rf95_2.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)
  rf95_3.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)
   
}


// Loop --------------------------------------------------------------------------------------------------------
int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{  
  digitalWrite(LED_2, LOW);
  
  if (rf95_1.available() && rf95_2.available() && rf95_3.available())
  {
    Serial.println("----------------");
    display.clearDisplay();                                       // Clear the screen buffer
    //digitalWrite(LED_1, HIGH);
    digitalWrite(LED_3, HIGH);
    
    // Should be a message for us now
    uint8_t buf_1[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_1 = sizeof(buf_1);
    
    uint8_t buf_2[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_2 = sizeof(buf_2);
    
    uint8_t buf_3[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_3 = sizeof(buf_3);

    if (rf95_1.recv(buf_1, &len_1) && rf95_2.recv(buf_2, &len_2)  && rf95_3.recv(buf_3, &len_3))
    {
//      RH_RF95::printBuffer("Received from 1: ", buf_1, len_1);
//      RH_RF95::printBuffer("Received from 2: ", buf_2, len_2);
      digitalWrite(LED_2, HIGH);
      
      // Print to serial
      Serial.print("Got 1: ");
      Serial.println((char*)buf_1);
      
      Serial.print("Got 2: ");
      Serial.println((char*)buf_2);

      Serial.print("Got 3: ");
      Serial.println((char*)buf_3);

      // Print to screen
      display.setCursor(10, 0);                 // Set screen cursor to position 1
      display.print(F("RSSI: "));               
      display.println(rf95_1.lastRssi(), DEC);  

      display.setCursor(10, 10);                // Set screen cursor to position 2
      display.print(F("RSSI: "));
      display.println(rf95_2.lastRssi(), DEC);  

      display.setCursor(10, 20);                // Set screen cursor to position 3
      display.print(F("RSSI: "));
      display.println(rf95_3.lastRssi(), DEC);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  
  display.display();
}
