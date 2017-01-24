#ifndef _UART_H_
#define _UART_H_


#define RBUFF 80		        // Max Serail Character Buffer Size
#define MAXIMUM_DIGITS 6	  // Max numerical digits to process
#define MAX_COMMAND_ENTRIES 6 

// Control codes for terminal commands
#define CTL_A 0x1     
#define CTL_B 0x2     // Hex
#define CTL_C 0x3     // Clear Screen
#define CTL_D 0x4     // Capture Call sign
#define CTL_E 0x5     // Clear Error Code
#define CTL_F 0x6     // Stop Transmitting
#define CTL_G 0x7     // Test Tx Msg
#define CTL_H 0x8     // Help
#define CTL_I 0x9     // Root Menu
#define CTL_K 0xB     // Rx Menu
#define CTL_L 0xC     // Tx Menu
#define CTL_N 0xE     // Narrow Spectrum
#define CTL_O 0x0F    // LCD Test
#define CTL_P 0x10    // PSK Toggle
#define CTL_Q 0x11    // Info
#define CTL_R 0x12    // Toggle RTTY
#define CTL_S 0x13    // Display Slevels
#define CTL_T 0x14    // Set Thresholds
#define CTL_U 0x15    // Mute/Unmute Receiver
#define CTL_V 0x16    // Calibrate Si5351
#define CTL_W 0x17    // Wide Spectrum
#define CTL_X 0x18    // Dump ADC values to console
#define CTL_Y 0x19    
#define CTL_Z 0x1A    // Reset System

// Terminal specific flags
#define MUTERX 0x1
#define TESTMSG 0x2
#define DISPHEX 0x4
#define DISP_WATERFALL 0x8
#define DISP_NARROW_WATERFALL 0x10
#define CALLSIGN_LOADED 0x20
#define PB1_USED_ONCE 0x40

void ProcessSerial ( unsigned char serialport );
unsigned char ParseSerial ( char *str );
void ResetSerial (void);
void FlushSerialPorts (void);
void ErrorOut ( void );

void ResetSerialTerminal (void);
void ProcessSerialTerminal ( void );
void ProcessControlCharacter ( char code );
char SerialTerminalPop (void);
void DisplayHelp (unsigned char serialport);
void DisplayInfo (unsigned char serialport);

void TogglePSK (void);
void ToggleRTTY (void);
void ExecuteWaterfall (void);
void ExecuteNarrowWaterfall (void);

void StopTransmitter (void);

unsigned char ProcessSerial (void);
void CalibrateSi5351 (void);

#endif // _UART_H_




