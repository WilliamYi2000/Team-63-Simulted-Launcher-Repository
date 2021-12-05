void receive() 
{
  if (Reyax.available()) 
  {
    String received = Reyax.readString();
    //Serial.print(received);
  
  
    if (received.indexOf("+RCV") >= 0)
    {
      int delimiter, delimiter_1, delimiter_2, delimiter_3;
      delimiter = received.indexOf(",");
      delimiter_1 = received.indexOf(",", delimiter + 1);
      delimiter_2 = received.indexOf(",", delimiter_1 + 1);
      delimiter_3 = received.indexOf(",", delimiter_2 + 1);
      int lengthMessage = received.substring(delimiter+1,delimiter_1).toInt();
     
      String message = received.substring(delimiter_1+1, delimiter_1 + lengthMessage+1);
      Serial.print("Recived message: ");
      Serial.println(message);
    }
  }
  else
  {
    Serial.print(received);
  }
}
