/*

This define is used to allocate all externally defined variables centrally.  There is another define 
that allocates the actualy variables
 
*/

#ifndef _ALLEXTERNALVARIABLES_H_
#define _ALLEXTERNALVARIABLES_H_

// These variables are defined in UART.cpp and used for Serial interface
// rbuff is used to store all keystrokes which is parsed by Execute()
// commands[] and numbers[] store all characters or numbers entered provided they
// are separated by spaces.
// ctr is counter used to process entries
// command_entries contains the total number of charaters/numbers entered
extern char rbuff[RBUFF];
extern char commands[MAX_COMMAND_ENTRIES];
extern unsigned char command_entries;
extern unsigned long numbers[MAX_COMMAND_ENTRIES];
extern unsigned char ctr;




// This is for passing signals between routines. Each bit defines a specific signal
extern volatile unsigned long flags, errorCode;
extern volatile unsigned long TermFlags;


// Correlation Buffers and Variables
// Use the largest buffer size to accomodate the data
extern volatile int adcbuff[FHT_N];        // Correlation buffers use CORRBUFFSZ and DFT buffers use DFTBUFSZ which is larger
extern volatile int corrbuff[CORRBUFFSZ];

extern volatile int adcbufflag[FHT_N];
extern volatile int corrbufflag[CORRBUFFSZ];

extern volatile long corr, corrMax, corrMin, corr0, corrAvg;
extern volatile long adcDly, deltaold, corrLevel, corrRTTY, corrPSK;
extern volatile int unsigned corrDly;
extern volatile unsigned int binMin, binMax;

extern volatile long magThresh;
extern volatile unsigned char ThreshDivider;

// ADC Sampling Variables
extern byte aLow, aHigh;
extern int si;
extern unsigned int aCtr;
extern int vLevel, sLevel, slctr;
extern int lastsi, deltasi, clipctr, clipctrrst;

// Signal Level Variables
extern volatile int digitalSignalLevel, oldDigitalLevel;
extern volatile int dlevelctr, levelctr, slevelctr, maxCorrLevel, maxvLevel;
extern volatile int oldCorrLevel, oldvLevel;

// This is a define in the FHT.h define. However it cannot be included because it defines this variable and will get a redefined error
extern int fht_input[(FHT_N)]; // FHT input data buffer
extern uint8_t fht_log_out[(FHT_N/2)]; // FHT log output magintude buffer

// Frequency Control variables
extern unsigned long frequency_clk0, frequency_clk0_tx;
extern long frequency_inc;
extern unsigned long frequency_mult;
extern unsigned long frequency_mult_old;

// RTTY Variables
extern int rttySpaceFreq;
extern int rttyMarkFreq;
extern int rttyMarkBin, rttySpaceBin, rttySpaceMag, rttyMarkMag;

extern volatile unsigned char rttyFigures, rttyChar, bitpos, rttyMark, rttySpace, rttyState, rttyIdle, nortty;
extern boolean rttyLocked;

// RTTY Transmitter Variables
extern unsigned long rttyTransmitSpaceFreq;
extern unsigned long rttyTransmitMarkFreq;
extern volatile unsigned char rttyPriorState, rttyDelay;
extern volatile char rttyLTRSSwitch;

// PSK Variables
extern boolean pskChanged, pskLocked;
extern unsigned char pskbinthresh;
extern long int pskcorrthresh, corrTotal;
extern unsigned char pskPhase;
extern unsigned int pskVaricode;
extern unsigned int pskbinlevel;
extern unsigned int levelResetCtr;
extern boolean pskChanged, pskLocked;
extern unsigned char pskResetCtr;
extern boolean decodePhaseChange;

// PSK Transmitter Variables
extern unsigned char pskSwap, pskVcodeLen;
extern unsigned int pskVcode;
extern unsigned char decodeLastPhase;
extern int decodePhaseCtr;
extern unsigned char pskState;

// FFT Variables
extern double FreqPerBin;
extern unsigned int binFreq[FHT_N2];
extern unsigned char Peaks[FHT_N2];
extern unsigned char fft_tmp[FHT_N2];
extern unsigned long FFTavg;
extern unsigned long FFTrms;

// Encoder Variables
extern volatile int enc_states[];
extern volatile int old_AB;
extern unsigned char encoderVal, encoderState;
extern unsigned int pbstate;
extern unsigned long pbreset;

// Push Button Debounce
extern volatile unsigned char pb1ctr, pb2ctr, pb3ctr, pbenable;
extern volatile unsigned int pbreusectr; 

// LCD Variables
extern unsigned int maxX, maxY, fontX, fontY, charX, charY;
extern unsigned int lcdchars, currentx, currenty;
extern unsigned int LCDErrctr;

// LED Variables
extern volatile unsigned int statusLEDctr;

// LCD Menu variables
extern unsigned char MenuSelection, MenuLevel;
extern char RootMenuOptions[MAXMENU_ITEMS][MAXMENU_LEN];
extern unsigned char RootMenuValues[MAXMENU_ITEMS];
extern char RxMenuOptions[MAXMENU_ITEMS][MAXMENU_LEN];
extern unsigned char RxMenuValues[MAXMENU_ITEMS];
extern char TxMenuOptions[MAXMENU_ITEMS][MAXMENU_LEN];
extern unsigned char TxMenuValues[MAXMENU_ITEMS];
extern char txmsg0[32];
extern char txmsg1[21];
extern char txmsg2[34];
extern char txmsg3[69];
extern char txmsg4[48];
extern char txmsg5[76];
extern char callsign[10];
extern unsigned char loadcharctr, callsignsize;

// Terminal Processing variables
extern unsigned char idle;
extern unsigned char endBuff;



// Timer Variables
extern byte adcsraReset, timsk1Reset, tccr1aReset, timsk3Reset, tccr3aReset, tccr4aReset, timsk4Reset;
extern byte tcc0areset, tccr0bReset, timsk0Reset;

// This defines the various parameter used to program Si5351 (See Silicon Labs AN619 Note)
// multisynch defines specific parameters used to determine Si5351 registers
// clk0ctl, clk1ctl, clk2ctl defined specific parameters used to control each clock
extern Si5351_def multisynth;
extern Si5351_CLK_def clk0ctl;
extern Si5351_CLK_def clk1ctl;
extern Si5351_CLK_def clk2ctl;

extern Adafruit_ILI9340 tft;



/*
/////////////////////

// This is for setting and checking various funcations
extern volatile unsigned long flags, errorCode;




*/



#endif

