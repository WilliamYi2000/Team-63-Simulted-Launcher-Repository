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
  // Wait between messages
  delay(1000);                                                  // Delay between messages (1 sec)
  Serial.println("--------------\nTransmitting...");            // Send a message to rf95_server
  display.clearDisplay();                                       // Clear the screen buffer

  // Build the message we want to TX  
  char radiopacket[20] = "Message #      ";
  itoa(packetnum++, radiopacket+9, 10);
  Serial.print("Sending "); Serial.println(radiopacket);
  radiopacket[19] = 0;

  // TX the message
  delay(10);
  rf95.send((uint8_t *)radiopacket, 20);    // TX packet
  display.setCursor(10, 0);                 // Set screen cursor to position 1
  display.println(radiopacket);             // Add packet to screen buffer
  display.display();                        // Display the screen

  // Wait for packet to finish TX
  delay(10);
  rf95.waitPacketSent();
  
  // Now wait for a reply from receiver
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  display.setCursor(10, 10);                 // Set screen cursor to position 2

  if (rf95.waitAvailableTimeout(1000))          // 1 second timeout period
  { 
    // Should be a reply message for us before timeout period
    if (rf95.recv(buf, &len))                   // Successfully recived response
   {
      // Print to serial
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);   

      // Print to screen
      display.println((char*)buf);
      display.setCursor(10, 20);                 // Set screen cursor to position 3
      display.print(F("RSSI: "));
      display.println(rf95.lastRssi(), DEC);   
    }
    else                                        // Some issue with response packet
    {
      // Print to serial
      Serial.println("Receive failed");
      
      // Print to screen
      display.println(F("Receive failed"));
      
    }
  }
  else                                        // Did not recived response packet before timeout
  {
      // Print to serial
      Serial.println("No reply");
      
      // Print to screen
      display.println(F("No reply"));
  }
  display.display();
}
