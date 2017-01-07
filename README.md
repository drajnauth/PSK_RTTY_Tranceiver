# PSK_RTTY_Tranceiver
An Arduino based Transceiver to transmit and receive (decode) RTTY and PSK project built by Dave Rajnauth, VE3OOI

The project is meant to demonstrate that its capable to decode digital signals using limited processing capabilities of a 8 bit microcontroller.  In the case of the Arduino Mega, at 16 Mhz.

This is a project that builds on the Kanga M0XPD 40m Transmitter, Receiver and Si5351 shields to build a fully functional RTTY PSK transceiver controlled by an Arduino Mega 2560 microcontroller.  The transceiver can transmit RTTY and PSK as well we recieve and decode RTTY and PSK.

There two separate hardware boards that interfaces to the Arduino Mega. One board filters and amplifies weak audio signals from the Kanga/M0XPD receiver to be within an audio passband which is digitized and processed by the Arduino using correlation techniques.  The other board is an interface board for the ILI9340 TFT display.

The software was written and compiled using the Arduino IDE.  There are other pieces of codes (not my own) that were obtained from the internet.  Sources for all external code is noted in the source code.

This project is made available to others to use, modify, experiment and build upon for non-commercial purposes.
