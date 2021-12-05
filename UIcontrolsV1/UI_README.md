User Interface Components:
\nLED Strip - give visual cues on the timing and target aquisition of the system
Speaker - give audio cues for build up on the locking procedure and will ring a melody when a successful hit is shown
Button - will act as the trigger in order to begin the target acquisition phase
Potentiometer - indicated by the user that sets the timing of the lock on
## User Interface Subsystem Readme 
* [General info](#general-info)
* [File description](#file-description)
* [Setup](#setup)

## General info
This folder contains the UI control code to be able to inform the user and audience on the current status of the subsystem. 
The code will handle the signal processing from the user interface components and the extract data from the user input components.
	
## File description
### UIcontrols
- Controls the radio module on the launcher and outputs the received messages to a connected adafruit screen for user display. 
- The file defines the I2C lora's reset pin to PCB PIN 2, Chip Select pin to PCB PIN 4, and interrupt to PCB PIN 3. 
- Sets the lora's radio frequency to 915 MHz. 
- Notifies the user if initialization has succeeded or failed and what the frequency was set to. 
- After intialization, the file will send a transmission signal with "Message #" and a incremented count of the number of messages that were sent previously. 
- After sending a signal, microcontroller will notify the user and the receiver waits for a response from the target module. 
- If no response was received after a short period of time, it will notify the user. 
- If a response was received, it will tell the user the message and the signal's RSSI value. 
### RX_Two_Modules
	
## Setup 
You can run this project using arduino

