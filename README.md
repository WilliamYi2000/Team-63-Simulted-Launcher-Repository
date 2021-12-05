# Team-63-Simulted-Launcher-Repository
## Radio Subsystem Readme 
* [General info](#general-info)
* [File description](#file-description)
* [Setup](#setup)

## General info
The simulated rocket launcher is intended to be used as a training device for instructing soldiers on the proper use of an electronic targeting system. The system will consist of two separate parts, a launcher and a target, each using a microcontroller and suite of sensors to send and receive data. These microcontrollers will also control peripheral systems, such as a user interface and feedback system on the launcher side, so as to notify the user of successful target acquisition, and an alarm system in the target to notify bystanders of a successful lock. The end product is intended to be used as a training device by soldiers to practice the proper use of a rocket launcher and assisted targeting system. As a device made specifically for training on targeting systems, the launcher will not fire a projectile. To test this device, successful lock will be made between the launcher and target at various ranges and with various settings of the variable timing circuit.
![image](https://github.com/WilliamYi2000/Team-63-Simulted-Launcher-Repository/blob/8612d704458e12008d0e7789cf33078425dce8ba/Project%20Diagrams/project%20overview.PNG)
## File description
### TX_Test
- Controls the radio module on the launcher and outputs the received messages to a connected adafruit screen for user display. 
- The file defines the I2C lora's reset pin to PCB PIN 2, Chip Select pin to PCB PIN 4, and interrupt to PCB PIN 3. 
- Sets the lora's radio frequency to 915 MHz. 
- Notifies the user if initialization has succeeded or failed and what the frequency was set to. 
- After intialization, the transciever will send a transmission signal with "Message #" and a incremented count of the number of messages that were sent previously. 
- After sending a signal, microcontroller will notify the user and the receiver waits for a response from the target module. 
- If no response was received after a short period of time, it will notify the user. 
- If a response was received, it will tell the user the message and the signal's RSSI value. 
![image](https://github.com/WilliamYi2000/Team-63-Simulted-Launcher-Repository/blob/89d4de68d7b024070be8d14b350821cecb62b5a0/I2C%20Module/Radio%20Flowcharts%20and%20Tables/Radio%20Transmitter%20Flow%20chart.png)
### RX_Two_Modules
- Controls two radio module on the target and outputs the received message, RSSI values, and if the launcher is a hit or miss to the serial monitor
- The file defines the first I2C lora's reset pin to PCB PIN 9, Chip Select pin to PCB PIN 10, and interrupt to PCB PIN 3. 
- The file defines the second I2C lora's reset pin to PCB PIN 7, Chip Select pin to PCB PIN 8, and interrupt to PCB PIN 2. 
- Sets the lora's radio frequency to 915 MHz. 
- Notifies the user if initialization has succeeded or failed and what the frequency was set to. 
- After intialization, the transcievers will wait to receive a signal. 
- After receiving a signal, microcontroller will place the signal message, message size, and RSSI values to buffers. 
- Hit detection will calculate the difference between the two radio received RSSI values and compare it to the precision value of 4
- If the difference is less than the precision value, a hit is registered
- If the difference is greater than the precision value, a miss is registered
![image](https://github.com/WilliamYi2000/Team-63-Simulted-Launcher-Repository/blob/3905327f9435ed0eaaa10a8e208a253936b562c4/I2C%20Module/Radio%20Flowcharts%20and%20Tables/Radio%20Receiver%20Flow%20chart.png)
## Setup 
You can run this project using arduino IDE and setting up the pins with arduino ATMega328p

![image](https://github.com/WilliamYi2000/Team-63-Simulted-Launcher-Repository/blob/0c6f1211317badffaf91b3de8e55cc96fa74cf83/I2C%20Module/Radio%20Flowcharts%20and%20Tables/LoRa%20pins.PNG)

