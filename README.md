🔒 Dual Microcontroller-Based Door Locker Security System
Project Description
This project develops a secure and efficient smart door control system utilizing a dual-microcontroller architecture. The system features password authentication, data storage in external EEPROM, and integration with a PIR sensor, H-bridge motor control, and a buzzer for enhanced security and functionality. The primary objective is to provide a robust and user-friendly access control solution. 

Key Features
Dual Microcontroller Design: Employs an HMI_ECU (Human-Machine Interface) and a Control_ECU for distributed processing and enhanced reliability. 

Password Authentication: Secure password verification and storage in external EEPROM. 

User Interface: Intuitive interaction via an LCD and keypad for password entry and system management. 

UART Communication: Seamless data exchange between the HMI_ECU and Control_ECU using UART. 
Motorized Door Control: The door's locking/unlocking mechanism is controlled by a motor driven by an H-bridge circuit. 

Buzzer Alerts: Provides auditory warnings for incorrect password attempts and other system alerts. 

PIR Motion Detection: Integrates a PIR sensor to detect motion, influencing door operation states. 

Password Management: Users can change the system password after successful authentication. 
Security Lockout: The system implements a 1-minute lockout period with an active buzzer and error message on the LCD after three consecutive incorrect password attempts. 

Hardware Components
The system's functionality relies on the following interconnected hardware:

HMI_ECU Connections 
LCD (8-bit mode):
RS pin: PC0
E (Enable) pin: PC1
Data Pins (D0-D7): Port A (PA0 to PA7)
Keypad (4x4):
Rows: PB0, PB1, PB2, PB3
Columns: PB4, PB5, PB6, PB7
UART Communication:
TXD: Connected to RXD of Control_ECU
RXD: Connected to TXD of Control_ECU
Control_ECU Connections 
External EEPROM (I2C Communication):
SCL (Serial Clock Line): PC0
SDA (Serial Data Line): PC1
Buzzer: Connected to PC7
H-bridge Motor Driver:
Input 1: PD6
Input 2: PD7
Enable1: PB3/OC0
Motor: Connected to the H-bridge motor driver.
PIR Motion Sensor: Connected to PC2
System Architecture
The project adheres to a layered model architecture to organize the software components, promoting modularity and reusability. 

HMI_ECU Software Layers :
Application Layer
Keypad Driver
LCD Driver
GPIO Driver
Timer Driver
UART Driver
Control_ECU Software Layers 
Application Layer
PIR Driver
Motor Driver
Buzzer Driver
EEPROM Driver
GPIO Driver
PWM Driver
UART Driver
Timer Driver
I2C Driver


Getting Started
To deploy and test this project:

Clone the repository to your development environment.
Open the project files within your AVR microcontroller IDE.
Build the project to compile the source code for both ECUs.
Flash the respective compiled firmware onto each ATmega32 microcontroller (HMI_ECU and Control_ECU).
Connect all hardware components as specified in the "Hardware Components" section to ensure proper system operation.
