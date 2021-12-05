void send(String Transmit, int address) {
  int TransmitLength = Transmit.length();
  String message = "AT+SEND=" + String(address) + "," + String(TransmitLength) + "," + Transmit + "\n\r";
  Reyax.println(message);
  //Serial.println(message);
}
