  // IMPORTS
#include <SPI.h>                  // Radio and Screen
#include <RH_RF95.h>              // Radio
//#include <Wire.h>                 // Screen
//#include <Adafruit_GFX.h>         // Screen
//#include <Adafruit_SSD1306.h>     // Screen


  // RADIO MACROS -----------------------------------------------------------------------------------------------
// TX Freq, must match RX's Freq!
#define RF95_FREQ 915.0

// Custom wiring
  // Module 1
#define RFM95_1_RST     9
#define RFM95_1_CS      10
#define RFM95_1_INT     3

  // Module 2
#define RFM95_2_RST     7
#define RFM95_2_CS      8
#define RFM95_2_INT     2

// Instances of the radio driver
RH_RF95 rf95_1(RFM95_1_CS, RFM95_1_INT);    // Module 1
RH_RF95 rf95_2(RFM95_2_CS, RFM95_2_INT);    // Module 2


  // SCREEN MACROS ----------------------------------------------------------------------------------------------
// Screen size
//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Misc screen macros
//#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
//#define SCREEN_ADDRESS 0x3C   // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Instance of screen driver
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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
  pinMode(RFM95_1_RST, OUTPUT);
  digitalWrite(RFM95_1_RST, HIGH);
  pinMode(RFM95_2_RST, OUTPUT);
  digitalWrite(RFM95_2_RST, HIGH);

  Serial.println("--Module Initialized--");

  // Manual reset
  digitalWrite(RFM95_1_RST, LOW);
  digitalWrite(RFM95_2_RST, LOW);
  delay(10);
  digitalWrite(RFM95_1_RST, HIGH);
  digitalWrite(RFM95_2_RST, HIGH);
  delay(10);

  // Make sure module initialized successfully
  while (!rf95_1.init()) 
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
    while (!rf95_2.init()) 
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Make sure frequency is set properly - Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95_1.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while (1);
  }
    if (!rf95_2.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Set TX power
  rf95_1.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)
  rf95_2.setTxPower(23, false);   // Set to 23 dBm (default is 13 dBm)  
}


// Loop --------------------------------------------------------------------------------------------------------
int16_t packetnum = 0;  // packet counter, we increment per xmission
int RSSI_1;
int RSSI_2;
int difference;

void loop()
{  
  if (rf95_1.available() && rf95_2.available())
  {
    Serial.println("----------------");
    //display.clearDisplay();                                       // Clear the screen buffer
    
    // Should be a message for us now
    uint8_t buf_1[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_1 = sizeof(buf_1);
    
    uint8_t buf_2[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len_2 = sizeof(buf_2);

    if (rf95_1.recv(buf_1, &len_1) && rf95_2.recv(buf_2, &len_2))
    {
      RSSI_1 = rf95_1.lastRssi();
      RSSI_2 = rf95_2.lastRssi();
      difference = RSSI_1 - RSSI_2;
      
      // Print to serial received message and RSSI Radio 1
      Serial.print("Got 1: ");
      Serial.println((char*)buf_1);
      Serial.print("RSSI: ");
      Serial.println(rf95_1.lastRssi(), DEC); 

      // Print to serial received message and RSSI Radio 2
      Serial.print("Got 2: ");
      Serial.println((char*)buf_2);
      Serial.print("RSSI: ");
      Serial.println(rf95_2.lastRssi(), DEC); 
     
      //Output difference 
      Serial.print("Difference: ");
      Serial.println(difference);
    
      int precision = 4; 
      //If difference less than precision, HIT
      /*if (abs(difference) < precision)
      {
         Serial.println("Hit!");
      } else 
      {
        Serial.println("Miss");
      }*/
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  
  //display.display();
}
