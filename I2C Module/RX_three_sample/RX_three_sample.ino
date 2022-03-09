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


// Calibration vars
int cal_num = 0;
const int set_cal = 30;
float r1_cal[set_cal];
float r2_cal[set_cal];
float r3_cal[set_cal];

float r1_runAvg;
float r2_runAvg;
float r3_runAvg;

float r1_avg;
float r2_avg;
float r3_avg;

const int sample = 5;
float r1_sam[sample];
float r2_sam[sample];
float r3_sam[sample];
float r1_savg;
float r2_savg;
float r3_savg;
int sample_count = 0;

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
    //Serial.println("----------------");
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
    {
//      RH_RF95::printBuffer("Received from 1: ", buf_1, len_1);
//      RH_RF95::printBuffer("Received from 2: ", buf_2, len_2);
      digitalWrite(LED_2, HIGH);
      
      if(cal_num < set_cal)
      {
        float r1 = rf95_1.lastRssi(); 
        float r2 = rf95_2.lastRssi();
        float r3 = rf95_3.lastRssi(); 
        
        // Print to serial
        Serial.print("CAL 1: ");
        Serial.println(r1);
        
        Serial.print("CAL 2: ");
        Serial.println(r2);
  
        Serial.print("CAL 3: ");
        Serial.println(r3);
  
        // Print to screen
        display.setCursor(10, 0);                 // Set screen cursor to position 1
        display.print("Cal RSSI: ");
        r1_cal[cal_num] = r1;                 
        display.println(r1, DEC);  
  
        display.setCursor(10, 10);                // Set screen cursor to position 2
        display.print("Cal RSSI: ");
        r2_cal[cal_num] = r2;     
        display.println(r2, DEC);  
  
        display.setCursor(10, 20);                // Set screen cursor to position 3
        display.print("Cal RSSI: ");  
        r3_cal[cal_num] = r3;  
        display.println(r3, DEC);

        cal_num += 1;
      }
      else if(cal_num == set_cal)
      {
        
        for(int i = 0; i < set_cal; i++)
        {
          Serial.print(r1_cal[i]);
          r1_avg += r1_cal[i]; 
        }
        Serial.println("");
        r1_avg = r1_avg / set_cal;
        Serial.println(r1_avg);  
        Serial.println("-----");
        delay(10);
        
        for(int j = 0; j < set_cal; j++)
        {
          Serial.print(r2_cal[j]);
          r2_avg += r2_cal[j];
        }
        Serial.println("");
        r2_avg = r2_avg / set_cal;
        Serial.println(r2_avg); 
        Serial.println("-----");
        delay(10);
       
        for(int i = 0; i < set_cal; i++)
        {
          Serial.print(r3_cal[i]);
          r3_avg += r3_cal[i]; 
        }
        r3_avg = r3_avg / set_cal;
        Serial.println("");
        Serial.println(r3_avg);  
        Serial.println("-----");
        delay(10);
               
        cal_num = cal_num + 1;
      }
/*        
      // Print to screen
      display.setCursor(10, 0);                 // Set screen cursor to position 1
      display.print(F("RSSI 1: ")); 
      int r1 = rf95_1.lastRssi();    
      r1_runAvg = (r1_runAvg + r1) / 2;
      display.println(r1, DEC);  
      Serial.println(r1_runAvg);
      

      display.setCursor(10, 10);                // Set screen cursor to position 2
      display.print(F("RSSI 2: "));
      int r2 = rf95_2.lastRssi();
      r2_runAvg = (r2_runAvg + r2) / 2;              
      display.println(r2, DEC);
      Serial.print(r2_runAvg);

      display.setCursor(10, 20);                // Set screen cursor to position 3
      display.print(F("RSSI 3: "));
      int r3 = rf95_3.lastRssi();
      r3_runAvg = (r3_runAvg + r3) / 2;              
      display.println(r3, DEC);  
      Serial.print(r3_runAvg);
*/
      else if(sample_count < sample)
      {
      r1_sam[sample_count] = rf95_1.lastRssi();    
      r2_sam[sample_count] = rf95_2.lastRssi();  
      r3_sam[sample_count] = rf95_3.lastRssi(); 
      sample_count = sample_count + 1;
      r1_savg = 0;
      r2_savg = 0;
      r3_savg = 0;
      }
      else
      {
      for(int j = 0; j < sample; j++)
        {
          Serial.print("R1_");
          Serial.print(j);
          Serial.print(": ");
          Serial.println(r1_sam[j]);
          r1_savg += r1_sam[j];
        }
      Serial.println();
      r1_savg = r1_savg / sample;
      for(int j = 0; j < sample; j++)
        {
          Serial.print("R2_");
          Serial.print(j);
          Serial.print(": ");
          Serial.println(r2_sam[j]);
          r2_savg += r2_sam[j];
        }
      Serial.println();
      r2_savg = r2_savg / sample;
      for(int j = 0; j < sample; j++)
        {
          Serial.print("R3_");
          Serial.print(j);
          Serial.print(": ");
          Serial.println(r3_sam[j]);
          r3_savg += r3_sam[j];
        }
      Serial.println();
      r3_savg = r3_savg / sample;
      
      Serial.print("Mid RSSI:");
      Serial.println(r1_savg);
      Serial.print("Mid RSSI Avg:");
      Serial.println(r1_avg);
      float mid_diff = r1_savg - r1_avg;
      Serial.print("Mid diff:");
      Serial.println(mid_diff);
      Serial.println();

      Serial.print("Right RSSI:");
      Serial.println(r2_savg);
      Serial.print("Right RSSI Avg:");
      Serial.println(r2_avg);
      float right_diff = r2_savg - r2_avg;
      Serial.print("Right diff:");
      Serial.println(right_diff);
      Serial.println();
      
      Serial.print("Left RSSI:");
      Serial.println(r3_savg);
      Serial.print("Left RSSI Avg:");
      Serial.println(r3_avg);
      float left_diff = r3_savg - r3_avg;
      Serial.print("Left diff:");
      Serial.println(left_diff);
      Serial.println();

      float delta = 0.45;
      if (abs(mid_diff) < delta){
        if (abs(left_diff) < delta){
          if(abs(right_diff) < delta){
            Serial.println("\n---HIT");
          }
          else
         {
            Serial.println("\n---MISS");
         }
        }
        else
       {
          Serial.println("\n---MISS");
       }
      }
      else
       {
          Serial.println("\n---MISS");
       }

      sample_count = 0;
      }
    }
    }
    else
    {
      Serial.println("Receive failed");
    }
  
  display.display();
  }
}
