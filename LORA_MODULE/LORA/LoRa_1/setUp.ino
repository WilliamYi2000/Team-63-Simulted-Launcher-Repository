void setUp() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Reyax.begin(115200);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(1000);
  
  Reyax.print("AT\r\n");
  delay(1000);

  Reyax.print("AT+BAND=915000000\r\n");
  delay(1000);   //wait for module to respond

  Reyax.print("AT+ADDRESS=0\r\n");   //needs to be unique
  delay(1000);   //wait for module to respond

  Reyax.print("AT+NETWORKID=0\r\n");   //needs to be same for receiver and transmitter
  delay(1000);   //wait for module to respond

}
