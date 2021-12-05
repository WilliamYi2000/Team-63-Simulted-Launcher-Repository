## Radio Subsystem Readme 
* [General info](#general-info)
* [File description](#file-description)
* [Setup](#setup)

## General info
The radio system is responsible for sending and receiving signals over 500 meters of distance and determining if the launcher is on target. Included in this folder is the TX_Test file and RX_Two_Modules.
	
## File description
### TX_Test
*Controls the radio module on the launcher and outputs the received messages to a connected adafruit screen for user display. 
*The file defines the I2C lora's reset pin to PCB PIN 2, Chip Select pin to PCB PIN 4, and interrupt to PCB PIN 3. 
*Sets the lora's radio frequency to 915 MHz. 
*Notifies the user if initialization has succeeded or failed and what the frequency was set to. 
*After intialization, the file will send a transmission signal with "Message #" and a incremented count of the number of messages that were sent previously. 
*After sending a signal, microcontroller will notify the user and the receiver waits for a response from the target module. 
*If no response was received after a short period of time, it will notify the user. 
*If the response was received it will tell the user the message and the signal's RSSI value. 
### RX_Two_Modules
	
## Setup 
You can run this project using arduino

