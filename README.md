# Vehicle-Accident-Detection-and-Tracking-System
Embedded system project on Vehicle Accident Detection and Tracking using Tiva TM4C123GXL

This repository aims at building an embedded system that can be installed in a vehicle , through which when an accident is detected, sends an alert message to a registered mobile number along with the google maps link of the accident location. 

## Hardware used :
  1)TIVA TM4C123GXL
  <br>2)Vibration sesnor (SW420)
  <br>3)GSM module (SIM900A)
  <br>4)GPS module (U-Blox NEO-6M)
<br>
## Software used :
  1)Code Composer Studio (Version 12.7.1)
  <br>2)TivaWare (SW - TM4C software development kit)
  <br>3)Stellaris ICDI 
<br>
## Pin Configurations:
  1)Vibration Sensor : VCC=>3.3V , GND=>GND , D0=>PF4
  <br>2)GSM module : VCC=>5V , TX=>PB0 (UART1 RX) , RX=>PB1 (UART1 TX) , GND=>GND
  <br>3)GPS module : VCC=>5V , TX=>PE4 (UART5 RX) , RX=>PE5 (UART5 TX) , GND=>GND
  
 
