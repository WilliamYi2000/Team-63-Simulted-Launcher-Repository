  // IMPORTS
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
#include <Wire.h>                 // Screen
#include <Adafruit_GFX.h>         // Screen
#include <Adafruit_SSD1306.h>     // Screen


  // RADIO MACROS -----------------------------------------------------------------------------------------------
// TX Freq, must match RX's Freq!
#define RF95_FREQ 915.0

// Matches Adafruit wiring diagram
#define RFM95_RST     2
#define RFM95_CS      4
#define RFM95_INT     3

// Instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

  // SCREEN MACROS ----------------------------------------------------------------------------------------------
// Screen size
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Misc screen macros
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Setup  -------------------------------------------------------------------------------------------------------
void setup() 
{
  // Serial setup ------------------------------------------------------
  Serial.begin(115200);     // Start serial connection
  while (!Serial)           // Wait for serial response
  {
    delay(1);
  }
  delay(100);


  // Radio setup -------------------------------------------------------
  // Set up reset pin
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.println("--Module Initialized--");

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Make sure module initialized successfully
  while (!rf95.init()) 
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Set TX power
  rf95.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)


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
  
}


// Loop --------------------------------------------------------------------------------------------------------
int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{  
  if (rf95.available())
  {
    display.clearDisplay();                                       // Clear the screen buffer
    
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      RH_RF95::printBuffer("Received: ", buf, len);

      // Print to serial
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      // Print to screen
      display.setCursor(10, 0);                 // Set screen cursor to position 1
      display.println((char*)buf);
      display.setCursor(10, 10);                // Set screen cursor to position 2
      display.print(F("RSSI: "));
      display.println(rf95.lastRssi(), DEC);   

      // Send a reply
      uint8_t data[] = "Reply from RX!";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();

      // Print to serial
      Serial.println("Sent a reply");

      // Print to screen
      display.setCursor(10, 20);                // Set screen cursor to position 3
      display.print(F("Response sent"));   
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  
  display.display();
}
