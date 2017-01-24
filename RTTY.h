#ifndef _RTTY_H_
#define _RTTY_H_

unsigned char GetFreqRange (unsigned int fbin, unsigned int ebin);
char DecodeRTTY (unsigned char bitvalue);
void ResetRTTY (void);
void Pause (int dly);


void sendRTTYbit (unsigned char b) ;

char Baudot( char c, unsigned char alpha);

char setupRTTYChar (char asciichar);
void TxRTTYbit (unsigned char b); 

int roundUp(int number, int fixedBase);

#define RESETSAMPLES 50

//#define F_SAMPLE 6095        // Based on no delay in loop
//#define F_SAMPLE 8850        // Based timer
//#define F_SAMPLE 8533        // Based timer
//#define F_SAMPLE 6737

// For 40 samples, S4-830Hz is 987, S4-1000 is 1287
// For 40 samples, S6-830Hz is 10,908, S6-1000 is 15,276
// Set threshold between S4-S6 (around S5) to 5,000
//#define AUTOCORR_THRESHOLD  400000  // For IIR BPF Filter over 400000
//#define AUTOCORR_THRESHOLD  300000    // No filtering 150000
#define AUTOCORR_THRESHOLD  5000    // Assume around S5 Signal Level to Start

#define F_SAMPLE 9615
#define CORRECTION_DELAY 31     // Correction delay in us when using analog read;


#define BIN_ERROR 0xFFF 


#define RTTY_UNKNOWN 0xF0
#define RTTY_INIT 0xF0
#define RTTY_IDLE 0x0
#define RTTY_START 0x1
#define RTTY_DATA 0x2
#define RTTY_STOP 0x4

#define RTTY_FIGURES 27       //11011 bin
#define RTTY_LETTERS 31       //11111 bin

#define RTTY_TX_FIGURES 0xF6     //11110110 bin, 11 + 11011 + 0 (2 stop bits, FiguresCode, 1 Start bit)
#define RTTY_TX_LETTERS 0xFE       //11111110 bin, 11 + 11111 + 0 (2 stop bits, LettersCode, 1 Start bit)

#define BAUDOT_BITS 5
#define BAUDOT_TABLE_SIZE 33

// Free running is about 4.3ms and about 5 samples per bit.   
// Depending where sample starts it may be 4,5,6 samples. Need to accomodate.  
// For 1 bit (e.g. START bit, look for less than 4 sample to ensure detection)
// Counts below start at 0 and value must be 1 less that actual value
#define RTTY_TIMEOUT 5          // Number of samples to reset detection
#define RTTY_NULL_THRESHOLD 5   // Number of Null RTTY bits to reset detection.
#define RTTY_IDLE_THRESHOLD 35  // Idle is long string of MARK bits.
#define RTTY_LTRS_THRESHOLD 31  // Letters Code.  Need at least 5 mark bit + 2 stop bits = 7 Mark bits. Look for 7 MARK bits (7x5 = 35, need 35 MARK bits)
#define RTTY_STOP_THRESHOLD 1   // 3 Need to find 1.5 stop bits, look for 7 (1.5x5) MARK bits. Leave time to find next Start bit so look for 4 MARK bit 
#define RTTY_START_THRESHOLD 1  // 2 Need to find 1 SPACE bit, Need 5 sample but look for 3 SPACE to be in the middle of the bit.

#define RTTY_MODE 0
#define PSK_MODE 1

#define MAX_MESSAGE_SIZE 40

#define RTTY_ALPHA 1
#define RTTY_DIGIT 0

#define RTTY_SPACE_FREQUENCY 830    // Mark Frequency is 1000 Hz, Space Frequency is 830 Hz, Mid Frequency is 915Hz
#define RTTY_MARK_FREQUENCY 1000    // Mark Frequency is 1000 Hz, Space Frequency is 830 Hz, Mid Frequency is 915Hz
#define RTTY_SHIFT_FREQUENCY 170    // Shift of 170 Hz. This is the shift to make the frequency appear as 1000 Hz on receiver's waterfall

#define RTTY_BAUD_DELAY 22                    // 22 ms per bit. i.e. Baud is 45.45 and bit time is 1/45.45=22 ms 
#define RTTY_BAUD_OVERHEAD_DELAY 16           // 22 ms per bit but 6 ms overhead for frequency change so is 16ms
#define RTTY_STOPBIT_DELAY 33                 // 33 ms for stop bit. i.e 1.5 stop bits which is 22ms + 11 ms = 33ms. With 6 ms overhead, its 28ms 
#define RTTY_STOPBIT_OVERHEAD_DELAY 28        // 33 ms for stop bit. With 6 ms overhead, its 28ms 
#define RTTY_IDLE_DELAY 352                   // assume 500ms for rtty receiver to lock onto signal
#define RTTY_STOP_COUNT 1
#define RTTY_IDLE_COUNT 15

#endif // _RTTY_H_
