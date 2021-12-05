# Team-63-Simulted-Launcher-Repository
## Radio Subsystem Readme 
* [General info](#general-info)
* [System Overview](#System-Overview)
* [File description](#file-description)

## General info
The simulated rocket launcher is intended to be used as a training device for instructing soldiers on the proper use of an electronic targeting system. The system will consist of two separate parts, a launcher and a target, each using a microcontroller and suite of sensors to send and receive data. These microcontrollers will also control peripheral systems, such as a user interface and feedback system on the launcher side, so as to notify the user of successful target acquisition, and an alarm system in the target to notify bystanders of a successful lock. The end product is intended to be used as a training device by soldiers to practice the proper use of a rocket launcher and assisted targeting system. As a device made specifically for training on targeting systems, the launcher will not fire a projectile. To test this device, successful lock will be made between the launcher and target at various ranges and with various settings of the variable timing circuit.
![image](https://github.com/WilliamYi2000/Team-63-Simulted-Launcher-Repository/blob/8612d704458e12008d0e7789cf33078425dce8ba/Project%20Diagrams/project%20overview.PNG)

## System Overview
The complete simulated rocket launcher system is divided into two main parts: the launcher and the target. Each system will contain a microcontroller serving to control the sensors and systems contained within.

The launcher is meant to be operated by a single user. The user will interact with the system both by aiming the launcher via the launcher targeting controls and by setting the length of time desired to lock the launcher onto the target. A feedback system including lights and sounds will notify the user of the status of the system, be it either currently locking on or upon a successful ‘hit’. All of these user interfaces will be driven by the launcher microcontroller, which will serve to coordinate all information collected from the user, and to send out a targeting signal upon the user activating the launcher’s firing mechanism to acquire a lock.

This targeting signal will be received by a sensor in the target system, which will send a signal notifying successful target acquisition to the target microcontroller. This microcontroller will both collect and interpret information from this target detection sensor, as well as connect to the launcher system via a radio link. This radio link will allow the launcher to notify the user of information regarding the status of the lock from the target, as well as to send timing information from the launcher itself to the target. Upon a successful ‘hit’, both the launcher feedback system and a hit notification system located in the target will activate, notifying both the user and any bystanders of a successful target acquisition.

Both the launcher and target will require internal power sources and will need to be rugged enough for extended outdoor usage. The batteries, sensors, and microcontrollers contained in each piece of the system will need to be protected from external elements, as well as be man-portable and, in the case of the launcher, easy to lift and aim. As the launcher and target are meant to operate at a distance of 500m, the target will also need to be visible enough that a user can spot it from a far distance away.

![image](https://github.com/WilliamYi2000/Team-63-Simulted-Launcher-Repository/blob/87074ff68495d6a62d8c03982d004c57835d5e32/Project%20Diagrams/system%20overview.PNG)

## File description
### I2C Module
Folder contains code that runs on the I2C LoRa RFM9x modules. 
*Tx_test* runs on the launcher to transmit a signal to the target. A more in depth description and flowchart is included in the folder readme file

*RX_Two_Modules* controls two radio modules on the target module and includes the hit detection algorithm. A more in depth flowchart and descrition is included in the folder readme file
### LORA_Module
Legacy code that was previously used to control the REYAX RYLR896 Lora radio system. This hardware is currently not in use with our system any longer.
### Launcher System Code
*CutDown System Test*

*LED_Radio_Test/LED_Radio_Test*
### Project Diagrams
### RFM9x_HelloWorld_Tx
### Screen_scroll_test
### UIcontrolsV1
