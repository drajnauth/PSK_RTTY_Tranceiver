#ifndef _MAIN_H_
#define _MAIN_H_

#define HEADER_MESSAGE "\r\nVE3OOI PSK/RTTY Transceiver v0.1a - testing"
#define LCD_HEADER_MESSAGE "VE3OOI PSK/RTTY XCR v0.1a"


//#define FPSCALING 16384     
//#define FPSHIFT 14

//conversion Functions
#define ftok(f)         ( (int32_t)(float)( (f)*(FPSCALING) ) )

// Main Functions
void Reset (void);
void ResetFrequencies (void);
void ExecuteSerial (char *str);
void TestLEDS (void);
void StatusLED (void);


// EEPROM Routines
void EEPROMWriteCorrection(void);
void EEPROMReadCorrection(void);

// Error Codes
#define PSK_BUFFER_OVERFLOW           0x1000
#define RTTY_DATA_OVERRUN             0x1001
#define PSK_DATA_OVERRUN              0x1002
#define FREQUENCY_BAD_CHANNEL         0x1004
#define FREQUENCY_BAD_RANGE           0x1008
#define FREQUENCY_NOT_SET             0x1010
#define CANNOT_COMPLETE_DECODE_ENABLED 0x1020
#define SERIAL_BUFFER_OVERFLOW        0x1040

// Frequencies defines
#define DEFAULT_FREQUENCY           7100000
#define TX_FREQUENCY_OFFSET         990           // was 975

#define DEFAULT_FREQUENCY_INCREMENT 100
#define DEFAULT_FREQUENCY_MULTIPLIER 100
#define MAXIMUM_FREQUENCY_MULTIPLIER 100000
#define MINIMUM_FREQUENCY_MULTIPLIER 10
#define LOW_FREQUENCY_LIMIT         7000000
#define HIGH_FREQUENCY_LIMIT        7300000

// Flags
#define MUTE                  0x1
#define DISPLAY_ERROR         0x2
#define MEASURETHRESHOLD      0x4
#define ADCDONE               0x8
#define REALTIME              0x10
#define PROCESSINGDONE        0x20
#define BUFF1DONE             0x40
#define CHECKPSKVALUE         0x80
#define DECODERTTY            0x100
#define TRANSMITRTTY          0x200
#define DECODEPSK             0x400
#define TRANSMITPSK           0x800
#define TRANSMIT_CHAR_DONE    0x1000
#define MONITORSIGNAL         0x2000
#define ADCMONITOR            0x4000
#define DOFHT                 0x8000
#define USE_LCD               0x10000
#define UPDATE_FREQ           0x20000
#define TOGGLE_SAMPLING       0x40000
#define NARROW_WATERFALL      0x80000
#define PBUTTON1_PUSHED       0x100000
#define PBUTTON2_PUSHED       0x200000
#define PBUTTON3_PUSHED       0x400000
#define PBUTTON_REUSED        0x800000
#define CLIPPING              0x1000000
#define DISPLAY_SIGNAL_LEVEL  0x80000000

// Pins
#define TxEnable 13
#define RxMute   10
#define SideTone 9
#define SideToneFreq 2500
#define AUDIO_PIN A0

// LED PINS
//#define BLINKCOUNT 15000
#define BLINKCOUNT 1000
#define LED1 4
#define LED2 3
#define LED3 2

#define OKLED 4
#define LOCKLED 3
#define TXLED 2

//Push Buttons
#define PBUTTON1  8
#define PBUTTON2  11
#define TBD       10      // Same as RxMute
#define PBUTTON3  12


/* Rotary encoder Pins*/
/* Arduino Pin 7, 8
 * PortD PD7, PD6 (i.e MSB or top 2 bits)
 * Need to right shift 6 to get values as LSB (or bottom 2 bits)
 * However need to mask off bottom two bits when reading since bottom 2 bits is Tx/Rx
 * 
 */
#define ENC_A 7
#define ENC_B 6
#define ENC_PB 5
#define ENC_PORT PINH
#define ENC_PBPORT PINE
#define CW           1        // Encoder rotated clockwise
#define CCW          0        // Encoder rotated counter clockwise
#define PUSH_BUTTON_RESET 70000

#endif // _MAIN_H_
