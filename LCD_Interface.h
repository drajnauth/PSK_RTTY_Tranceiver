#ifndef _LCD_INTERFACE_
#define _LCD_INTERFACE_

// Specific LCD display parameters
#define MIN_X 0
#define MAX_X 235
#define MIN_Y 0
#define MAX_Y 320

#define TITLE_START_Y 0                   
#define TITLE_END_Y 16        
#define TITLE_WIN_SIZE 16

#define WF_START_Y 17                   
#define WF_END_Y 106        
#define WF_WIN_SIZE 89

#define DATA_START_Y 107
#define DATA_END_Y 160
#define DATA_WIN_SIZE 53

#define DECODE_START_Y 161
#define DECODE_END_Y 320
#define DECODE_WIN_SIZE 159

#define MAX_WF_HEIGHT 87        // i.e. WF_END_Y - WF_START_Y = 106-17 = 89. Make it a little smaller that actual height    
#define MAX_WF_VALUE 100       // Max FFT value
               
#define WF_BIN_WIDTH 5
#define PB_BIN_WIDTH 15  
                   
#define MAX_DECODE_CHARACTERS 190
#define MAX_DECODE_X 228
#define MAX_DECODE_Y 305
  
#define SPACE_FREQUENCY_BIN 11
#define MARK_FREQUENCY_BIN 13        // Bin for 1000 Hz  

#define SPACE_FREQUENCY_CBIN 12
#define MARK_FREQUENCY_CBIN 10       // Bin for 1000 Hz  

#define SPACE_FREQUENCY_XVALUE 82
#define MARK_FREQUENCY_XVALUE 96
#define FREQUENCY_XWIDTH 2


#define LCD_UPDATE_THRESHOLD 100

#define LCD_CLEAR_ERROR_THRESHOLD 10000

// The following are used for testing
void LCDDisplayTest(); 
//void LCDLoopTest (void)

// LCD Routines
void LCDBlank (void);
void LCDDisplaySetup(); 
void LCDPrintChar (char value);
void LCDSignalError (unsigned long errorcode);
void LCDDisplayFrequencyIncrement (void);
void LCDDisplayFrequency (void);
void LCDDisplayMode (char *mode);
void LCDDisplayCharacter (char value);
void LCDClearDisplayWindow (void);
void LCDDisplayLevel (void);
void LCDDisplayWaterFall (void);
void LCDDisplayPassbandWaterfall (void);
void LCDDrawWaterfallWindowMarkers (unsigned char narrow);
void LCDClearWaterfallWindow (void);
void LCDDisplayInfo (void);


void PrintCursorLocation (void);

#endif // _LCD_INTERFACE_
