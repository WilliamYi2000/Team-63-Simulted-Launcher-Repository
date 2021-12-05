void setUp() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Reyax.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
    delay(1000);
  
  Reyax.print("AT\r\n");
  delay(1000);
    /*
    * AT+PARAMETER=<Spreading Factor>,
    *<Bandwidth>,<Coding Rate>,
    *<Programmed Preamble>
    *<Spreading Factor>7~12, (default 12)
    *<Bandwidth>0~9 list as below
    *0 : 7.8KHz (not recommended, over spec.) 1 : 10.4KHz (not recommended, over spec.) 2 : 15.6KHz
    *3 : 20.8 KHz
    *4 : 31.25 KHz
    *5 : 41.7 KHz
    *6 : 62.5 KHz
    *7 : 125 KHz (default).
    *8 : 250 KHz
    *9 : 500 KHz
    *<Coding Rat>1~4, (default 1)
    *<Programmed Preamble> 4~7(default 4)
    
   */
  Reyax.print("AT+PARAMETER=12,9,1,7\r\n");
  delay(1000);   //wait for module to respond

  Reyax.print("AT+BAND=915000000\r\n");    //Bandwidth set to 915MHz
  delay(1000);   //wait for module to respond

  Reyax.print("AT+ADDRESS=1\r\n");   //needs to be unique
  delay(1000);   //wait for module to respond

  Reyax.print("AT+NETWORKID=6\r\n");   //needs to be same for receiver and transmitter
  delay(1000);   //wait for module to respond
  Reyax.print("AT+CRFOP=15\r\n");
  delay(1000);

}
