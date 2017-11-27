/*

This define is used to allocate all variables centrally.  There is another define that creates extern
references to these variables
 
*/

#ifndef _ALLVARIABLES_H_
#define _ALLVARIABLES_H_

// These variables are defined in UART.cpp and used for Serial interface
// rbuff is used to store all keystrokes which is parsed by Execute()
// commands[] and numbers[] store all characters or numbers entered provided they
// are separated by spaces.
// ctr is counter used to process entries
// command_entries contains the total number of charaters/numbers entered
char rbuff[RBUFF];
char commands[MAX_COMMAND_ENTRIES];
unsigned char command_entries;
unsigned long numbers[MAX_COMMAND_ENTRIES];
unsigned char ctr;

// This is for passing signals between routines. Each bit defines a specific signal
volatile unsigned long flags, errorCode;
volatile unsigned long TermFlags;

// Correlation Buffers and Variables
// Use the largest buffer size to accomodate the data
volatile int adcbuff[FHT_N];        // Correlation buffers use CORRBUFFSZ and DFT buffers use DFTBUFSZ which is larger
volatile int corrbuff[CORRBUFFSZ];

volatile int adcbufflag[FHT_N];
volatile int corrbufflag[CORRBUFFSZ];

volatile long corr, corrMax, corrMin, corr0, corrAvg;
volatile long adcDly, deltaold, corrLevel, corrRTTY, corrPSK;
volatile int unsigned corrDly;
volatile unsigned int binMin, binMax;

volatile long magThresh;
volatile unsigned char ThreshDivider;

// FFT Variables
double FreqPerBin;
unsigned int binFreq[FHT_N2];
unsigned char Peaks[FHT_N2];
unsigned char fft_tmp[FHT_N2];
unsigned long FFTavg;
unsigned long FFTrms;


// ADC Sampling Variables
byte aLow, aHigh;
int si;
unsigned int aCtr;
int vLevel, sLevel, slctr;
int lastsi, deltasi, clipctr, clipctrrst;

// Signal Level Variables
volatile int digitalSignalLevel, oldDigitalLevel;
volatile int dlevelctr, levelctr, slevelctr, maxCorrLevel, maxvLevel;
volatile int oldCorrLevel, oldvLevel;

// Frequency Control variables
unsigned long frequency_clk0, frequency_clk0_tx;
long frequency_inc;
unsigned long frequency_mult;
unsigned long frequency_mult_old;

// Encoder Variables
volatile int enc_states[] = {0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int old_AB = 0;
unsigned char encoderVal, encoderState;
unsigned int pbstate;
unsigned long pbreset;

// Local LCD variables
unsigned int maxX, maxY, fontX, fontY, charX, charY;
unsigned int lcdchars, currentx, currenty;
unsigned int LCDErrctr;

// LED Variables
volatile unsigned int statusLEDctr;

// Local PSK Variables
boolean pskChanged, pskLocked;
unsigned char pskResetCtr;
boolean decodePhaseChange;

// PSK Decode Variables
unsigned char pskbinthresh;
long int pskcorrthresh, corrTotal;
unsigned char pskPhase; 
unsigned int pskVaricode;
unsigned int pskbinlevel; 
unsigned int levelResetCtr;

// PSK Transmitter Variables
unsigned char pskSwap, pskVcodeLen;
unsigned int pskVcode;
unsigned char decodeLastPhase;
int decodePhaseCtr;
unsigned char pskState;

// RTTY Decode Variables
int rttySpaceFreq;
int rttyMarkFreq;
int rttyMarkBin, rttySpaceBin, rttySpaceMag, rttyMarkMag;

// RTTY Transmitter Variables
unsigned long rttyTransmitSpaceFreq;
unsigned long rttyTransmitMarkFreq;
volatile unsigned char rttyPriorState, rttyDelay;
volatile char rttyLTRSSwitch;

volatile unsigned char rttyFigures, rttyChar, bitpos, rttyMark, rttySpace, rttyState, rttyIdle, nortty;
boolean rttyLocked;


// Terminal Processing variables
unsigned char idle;
unsigned char endBuff;

// Timer Variables
byte adcsraReset, timsk1Reset, tccr1aReset, timsk3Reset, tccr3aReset, tccr4aReset, timsk4Reset;
byte tcc0areset, tccr0bReset, timsk0Reset;


// Push Button Debounce
volatile unsigned char pb1ctr, pb2ctr, pb3ctr, pbenable;
volatile unsigned int pbreusectr; 

unsigned char MenuSelection, MenuLevel;

char RootMenuOptions[MAXMENU_ITEMS][MAXMENU_LEN] = {
  {"Spectrum\0"},
  {"SLevels\0"},
  {"RX Menu\0"},
  {"CLR LCD\0"},
  {"LCD Test\0"},
  {"Help\0"},
  {"Reset\0"},
  {"INFO\0"},
};
unsigned char RootMenuValues[MAXMENU_ITEMS] = {CTL_W, CTL_S, CTL_K, CTL_C, CTL_O, CTL_H, CTL_Z, CTL_Q};

char RxMenuOptions[MAXMENU_ITEMS][MAXMENU_LEN] = {
  {"SetLevel\0"},
  {"Call SGN\0"},
  {"RTTY TxRx\0"},
  {"PSK TxRx\0"},
  {"CLR LCD\0"},
  {"Help\0"},
  {"Tx Test\0"},
  {"Top Menu"}
};
unsigned char RxMenuValues[MAXMENU_ITEMS] = {CTL_T, CTL_D, CTL_R, CTL_P, CTL_C, CTL_H, CTL_G, CTL_I};

char TxMenuOptions[MAXMENU_ITEMS][MAXMENU_LEN] = {
  {"Stop Tx\0"},
  {"CQ\0"},
  {"CQ Reply\0"},
  {"RPT\0"},
  {"Brag\0"},
  {"73\0"},
  {"LMT RESP\0"},
  {"Tx Test"},
};

unsigned char TxMenuValues[MAXMENU_ITEMS] = 
{CTL_F, TXMESSAGEID, TXMESSAGEID_INSERT_CALLSIGN, TXMESSAGEID_INSERT_CALLSIGN, TXMESSAGEID_INSERT_CALLSIGN, 
TXMESSAGEID_INSERT_CALLSIGN, TXMESSAGEID_INSERT_CALLSIGN, CTL_G};

char txmsg0[32] = "CQ CQ DE VE300I VE3OOI PSE K\r\n\0";
char txmsg1[21] = " DE VE3OOI PSE KN\r\n\0";
char txmsg2[34] = " DE VE3OOI RPT: 599 BTU VE3OOI\r\n\0";
char txmsg3[69] = " DE VE3OOI LOC: FN03CW STN: ARDUINO TRANSCEIVER 2W OCF BTU VE3OOI\r\n\0";
char txmsg4[48] = " DE VE3OOI QSL: eQSL or LOTW 73 DE VE3OOI SK\r\n\0";
char txmsg5[60] = " DE VE3OOI Using Arduino limited QSO responses BTU VE3OOI\0";

char callsign[MAXCALLSIGN];
unsigned char loadcharctr, callsignsize;




// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)
#define _sclk 52
#define _miso 50
#define _mosi 51
#define _cs 45
#define _dc 49
#define _rst 47

// Using software SPI is really not suggested, its incredibly slow
//Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _mosi, _sclk, _rst, _miso);
// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);


/*
PSK31 runs at 31.25 baud or 32 ms per symbol
For 0 bits there is a phase reversal of 180 degrees.  For a 1 bit there is no phase reversal
Varicode is used to represent characters. The code for each character is such that there are no multiple 0 bits in a row. 
If there were multiple 0 bits in a row,then it would represend an idle conditon

Whenever a character is sent, its terminsated with at least two 0 bits (i.e. two phase reversals in a row). 
Therefore the inter-character gap is marked as two phase reversals.
*/




#endif
