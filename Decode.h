#ifndef _DECODE_H_
#define _DECODE_H_


#define MIN_THRESHDIVIDER 7
#define MAX_THRESHDIVIDER 11
#define DEFAULT_THRESHDIVIDER 8



// Decoding Routines
void DecodeLoop( void );
void RTTYControl (char function);
void PSKControl (char function);
void SignalError (unsigned long errorcode);
void DisplayLevel (void);
void UpdateFrequencyData (unsigned char updateFrequency);
void SignalLevel (long rawlevel, char mode);
long fpRound (long value, int divisor);

#endif // _DECODE_H_
