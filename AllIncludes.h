
/*

This define is used centrally control all includes.  
 
*/

#ifndef _ALLINCLUDES_H_
#define _ALLINCLUDES_H_


#include <stdint.h>           // Needed or Unix style definations
#include <stdio.h>            // Needed for I/O functions
#include <avr/io.h>           // Needed PIN I/O
#include <avr/interrupt.h>    // Needed for timer and adc interrupt
#include <avr/eeprom.h>       // Needed for storing calibration to Arduino EEPROM

#include <Wire.h>             // Needed to communitate I2C to Si5351
#include <SPI.h>              // Needed to communitate I2C to Si5351

#include "Main.h"   		// Main Defines for this program
#include "VE3OOI_Simple_Si5351_v1.0.h"      // VE3OOI Si5351 Routines
#include "ADC.h"              // VE3OOI ADC samping routines
#include "Decode.h"           // VE3OOI general decoding routines
#include "Encoder.h"          // VE3OOI encoder processing routines
#include "LCD_Interface.h"    // VE3OOI LCD display funcation routines  (needs LCD library)
#include "Timer.h"            // VE3OOI Timer control routines
#include "WaterFall.h"        // VE3OOI Waterfall processing routines
#include "RTTY.h"             // VE3OOI RTTY Decode Routines
#include "PSK.h"              // VE3OOI PSK Decode Routines
#include "Correlation.h"      // VE3OOI Correlation Routines
#include "UART.h"             // VE3OOI Serial Interface Routines (TTY Commands)
#include "Pbutton_menu.h"     // VE3OOI Pushbutton and Menu Support

#include "i2c.h"
#include "SPI.h"

#include "AVRMult.h"
#include "AVRDiv.h"

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

#endif // _ALLINCLUDES_H_





