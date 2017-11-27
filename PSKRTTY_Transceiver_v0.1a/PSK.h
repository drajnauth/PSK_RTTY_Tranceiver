#ifndef _PSK_H_
#define _PSK_H_

#define VARICODE_TABLE_SIZE 128           // PSK Varicode table size

#define CROSSCORRSZ 13                    // Number of samples to cross correlate. For 1Khz signal 13 samples cause correcation
                                          // between consecutive samples to give a large negative lag(0) value
                                          // A phase shift will show up as a positive lag(0) value

#define PSK_BIN_THRESHOLD 56              // Smallest peak value to indicate a phase shift. Orig 56
#define PSK_CORRELATION_THRESHOLD -600    // Biggest value for lag(0) which indicates a phase shift. Orig 600
#define PSK_RESET_COUNT 1                 // 3 non phase samples will reset phase change detection. Orig 3

#define PSK_NO_LOCK_THRESHOLD 60         // Was 121, i.e. 11 sample buffers without a phase shift means no PSK present (i.e. 11 buffers per baud cycle x 11 buffers = 121)

#define PSK_LEVEL_RESET_COUNT 90

#define PSK_DECODE_START 11               // Sampling at 9615 and 13 sample correlation, at least 11 non phase shifts between a 00
                                          // But loose one count when phase changes so counter 12 (i.e. from 0 its 11)
#define PSK_DECODE_NOSTART 14             // A number greater than the start phase threshold

#define PSK_INIT 0xF0
#define PSK_IDLE 0x0
#define PSK_START 0x1
#define PSK_DATA 0x2
#define PSK_STOP 0x4


#define PSK_BAUD_DELAY 31                     // 32 ms per bit. i.e. Baud is 31.25 and bit time is 1/31.25=32 ms 
#define PSK_IDLE_COUNT 10                     // number of baud timeperiods for continious phase reversals
#define PSK_CHAR_GAP_COUNT 3                  // number of continious phase reversals between characters

unsigned int ConvertVaricode (unsigned int code);
unsigned int LookupVaricode (char code);
unsigned char numbits (unsigned int in);
unsigned char GetPhaseShift (void); 
char DecodePSK (unsigned char phase);
void ResetPSK (void);



#endif // _PSK_H_
