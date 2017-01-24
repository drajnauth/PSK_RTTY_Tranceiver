/*

Program Written by Dave Rajnauth, VE3OOI to perform various PSK Rx and Tx processing

PSK31 runs at 31.25 baud or 32 ms per symbol
For 0 bits there is a phase reversal of 180 degrees.  For a 1 bit there is no phase reversal
Varicode is used to represent characters. The code for each character is such that there are no multiple 0 bits in a row. 
If there were multiple 0 bits in a row,then it would represend an idle conditon

Whenever a character is sent, its terminsated with at least two 0 bits (i.e. two phase reversals in a row). 
Therefore the inter-character gap is marked as two phase reversals.

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"


// Varicode lookup table below is reversed to accomodate shifting LSB (i.e. LSB and MSB reversed)
// Offset in the table is the ASCII code
const unsigned int varicode[VARICODE_TABLE_SIZE] = {
  0x0355,  // 0 NUL
  0x036d,  // 1 SOH
  0x02dd,  // 2 STX
  0x03bb,  // 3 ETX
  0x035d,  // 4 EOT
  0x03eb,  // 5 ENQ
  0x03dd,  // 6 ACK
  0x02fd,  // 7 BEL
  0x03fd,  // 8 BS
  0x00f7,  // 9 HT
  0x0017,  // 10 LF
  0x03db,  // 11 VT
  0x02ed,  // 12 FF
  0x001f,  // 13 CR
  0x02bb,  // 14 SO
  0x0357,  // 15 SI
  0x03bd,  // 16 DLE
  0x02bd,  // 17 DC1
  0x02d7,  // 18 DC2
  0x03d7,  // 19 DC3
  0x036b,  // 20 DC4
  0x035b,  // 21 NAK
  0x02db,  // 22 SYN
  0x03ab,  // 23 ETB
  0x037b,  // 24 CAN
  0x02fb,  // 25 EM
  0x03b7,  // 26 SUB
  0x02ab,  // 27 ESC
  0x02eb,  // 28 FS
  0x0377,  // 29 GS
  0x037d,  // 30 RS
  0x03fb,  // 31 US
  0x0001,  // 32 SP
  0x01ff,  // 33 !
  0x01f5,  // 34 @
  0x015f,  // 35 #
  0x01b7,  // 36 $
  0x02ad,  // 37 %
  0x0375,  // 38 &
  0x01fd,  // 39 '
  0x00df,  // 40 (
  0x00ef,  // 41 )
  0x01ed,  // 42 *
  0x01f7,  // 43 +
  0x0057,  // 44 ,
  0x002b,  // 45 -
  0x0075,  // 46 .
  0x01eb,  // 47 /
  0x00ed,  // 48 0
  0x00bd,  // 49 1
  0x00b7,  // 50 2
  0x00ff,  // 51 3
  0x01dd,  // 52 4
  0x01b5,  // 53 5
  0x01ad,  // 54 6
  0x016b,  // 55 7
  0x01ab,  // 56 8
  0x01db,  // 57 9
  0x00af,  // 58 :
  0x017b,  // 59 ;
  0x016f,  // 60 <
  0x0055,  // 61 =
  0x01d7,  // 62 >
  0x03d5,  // 63 ?
  0x02f5,  // 64 @
  0x005f,  // 65 A
  0x00d7,  // 66 B
  0x00b5,  // 67 C
  0x00ad,  // 68 D
  0x0077,  // 69 E
  0x00db,  // 70 F
  0x00bf,  // 71 G
  0x0155,  // 72 H
  0x007f,  // 73 I
  0x017f,  // 74 J
  0x017d,  // 75 K
  0x00eb,  // 76 L
  0x00dd,  // 77 M
  0x00bb,  // 78 N
  0x00d5,  // 79 O
  0x00ab,  // 80 P
  0x0177,  // 81 Q
  0x00f5,  // 82 R
  0x007b,  // 83 S
  0x005b,  // 84 T
  0x01d5,  // 85 U
  0x015b,  // 86 V
  0x0175,  // 87 W
  0x015d,  // 88 X
  0x01bd,  // 89 Y
  0x02d5,  // 90 Z
  0x01df,  // 91 [
  0x01ef,  // 92 
  0x01bf,  // 93 ]
  0x03f5,  // 94 ^
  0x016d,  // 95 _
  0x03ed,  // 96 `
  0x000d,  // 97 a
  0x007d,  // 98 b
  0x003d,  // 99 c
  0x002d,  // 100 d
  0x0003,  // 101 e
  0x002f,  // 102 f
  0x006d,  // 103 g
  0x0035,  // 104 h
  0x000b,  // 105 i
  0x01af,  // 106 j
  0x00fd,  // 107 k
  0x001b,  // 108 l
  0x0037,  // 109 m
  0x000f,  // 110 n
  0x0007,  // 111 o
  0x003f,  // 112 p
  0x01fb,  // 113 q
  0x0015,  // 114 r
  0x001d,  // 115 s
  0x0005,  // 116 t
  0x003b,  // 117 u
  0x006f,  // 118 v
  0x006b,  // 119 w
  0x00fb,  // 120 x
  0x005d,  // 121 y
  0x0157,  // 122 z
  0x03b5,  // 123 {
  0x01bb,  // 124 |
  0x02b5,  // 125 }
  0x03ad,  // 126 ~
  0x02b7   // 127 (del)
};






unsigned int LookupVaricode (char code)
{
// Routine to convert ASCII code to varicode. Return varicode

  return (varicode[(unsigned char)code]);
}

unsigned int ConvertVaricode (unsigned int code)
{
// Routine to lookup the varicode and return the ascii code
// need to search the table entry by entry

  unsigned int i;

  // Search the varicode table
  for (i=0; i<VARICODE_TABLE_SIZE; i++) {
    if (varicode[i] == code) {
      return i;
    }
  }

  return 0xFF;  // Return error
}

unsigned char numbits (unsigned int in)
{
// Routine to identify the number of bits assocatied with varicode
// This is required to know how many bit to transmit.


  unsigned char i;
  unsigned int temp;
  temp = in;      // Destructive manipulation of varicode so use a local temp variable

  // Varicode starts and ends with 1 so each bit shift must be non zero unil last bit shifted out
  for (i=0; temp; i++) {
    temp >>= 1;
  }
  
  return i;     // Return the number of shift i.e. number of bits.
  
}


char DecodePSK (unsigned char phase)
{
// Routine to look at phase shift and frame varicode then convert to ascii
// ascii character received is returned
// The calling function must process the buffer and identify if phase was shifted. The input to this routine is 
// a flag that is toggled if there was a phase shift. 
// E.g. if last value returned by GetPhaseShift() was 0 and there was not phase shift, it returns 0
// however if there was a phase shift it returns 0xFF (i.e. not 0)  
// DecodePSK() takes the phase shift and frame the received PSK varicode character
// DecodePSK() is executed continiously whenever ADC is finished sampling.  Timer3 is running at 32ms and 
// signals DecodePSK() to decide if this was a 1 or 0 bit based on the samples processed to now.       
// DecodePSK() also convertes received varicode to ASCII

  char decodedcar;

  decodedcar = 0;

  // Check if phase shift changed since last value, if so signal that a phase change has ocured
  if (decodeLastPhase != phase) {
    decodePhaseChange = true;

    // Check count between phase shifts. If below start threshold signal a start
    // Not detection of start condition does not use timer3. However detection of
    // start condition will kick off timer 3 to measure phase shift for next bit time (bit duty cycle)
    if (abs(decodePhaseCtr-PSK_DECODE_START) <= 1) {
      pskState = PSK_START;           // Set start condition, Enter start state
    }
    decodePhaseCtr = 0;               // reset phase counter
  }

  // If no phase shift detected over an extended period, no PSK present.  No lock
  if (decodePhaseCtr++ > PSK_NO_LOCK_THRESHOLD) {
    decodePhaseCtr = 0;
    pskLocked = false;      // PSK not locked
  }

  // Act of state
  switch (pskState) {
    
    case PSK_START:                     // Start condition
      DisableTimers (3);
//      Serial1.println ("S");        // Used for debug 

      // If a start condition received and varicode received then, varicode transmission is complete
      // Note that a start condition also acts as a stop condition
      if (pskVaricode) {
//        Serial1.println (pskVaricode, BIN);     // Used for debug
        pskVaricode = ConvertVaricode (pskVaricode);      // Convert varicode to ascii
        
        if (pskVaricode <= VARICODE_TABLE_SIZE) {         // Check if lookup was sucessful
          decodedcar = (char) pskVaricode;
          pskLocked = true;
        } else {                                          // Not valid varicode, do dump character (i.e. return 0 as character) and restart
          pskLocked = false;
          decodedcar = 0;
        }
      }

      // Start condition recevied so in data state now.  S
      // Start timer and start looking for phase shift after 1 bit time elapsed
      pskState = PSK_DATA;
      flags &= ~CHECKPSKVALUE;
      decodePhaseChange = false;      // Reset Phase to detect phase shift
      pskVaricode = 0;                // The variable that will contain the bits received
      bitpos = 0;                     // Bitpos is the location of the current bit to be loaded into pskVaricode
      EnableTimers (3, TIMER32MS);    // Enable Timer 3 for bit time.  Timer sets the CHECKPSKVALUE flag to signal to check and load bit
      break;

      
    case PSK_DATA:                      // Data state...load varicode bits
      if (flags & CHECKPSKVALUE) {      // Signal from Timer to check and load bit
        flags &= ~CHECKPSKVALUE;
//        Serial1.print ("D");          // Used for debug
//        Serial1.print (bitpos);

        // No phase shift so load 1
        if ( !decodePhaseChange ) {     
          pskVaricode |= (1<<bitpos);     // Load "1" bit into position bitpos (i.e. the number of shifts to load the bit)
//         Serial1.println (" 1");        // Used for debug

        // Phase shift so 0 bit, not need to load it because the pskVaricode variable is zero. Just need to set the "1" bits
        } else {                          
          decodePhaseChange = false;
//          Serial1.println (" 0");       // Used for debug
        }
        bitpos++;                         // increment bit counter for next bit
      }

      break;
  }
 
  decodeLastPhase = phase;                // Remember this phase. Used to determine if phase changed from last phase state

  return decodedcar;                      // Return varicode.  If varicode is zero, then nothing received
}



unsigned char GetPhaseShift (void) 
{
// Routing to process the ADC buffers to identify a phase shift in carrier

//  corr0  = CrossCorr (corrbuff, corrbufflag, CROSSCORRSZ, 0);   // Not used anymore, using GetCorrPeak instead

  // GetCorrPeak() is used to perform the cross correlation between the buffers and 
  // return the delay for the peak.  The routine only searches between delay 0 and 8. If delay > 8 its not a 1000 hz carrier
  // The routine also sets the corr0 value (correlation sum at delay 0).  Ideally corr0 should be positive if both buffers in phase
  // and negative if both buffers out of phase.
  // The delay where the peak is located (i.e. corrDly) also shifts depending where the phase shift ocures in the buffers
  corrDly = GetCorrPeak (0, 8);

  // binMax is used to identify the bin threshold.
  if (corr0 > magThresh) {
    if (binMax < corrDly) binMax = corrDly;                 // Get the max bin delay for peak regardless of phase        
    if (corrDly && binMin > corrDly) binMin = corrDly;      // If in phase always use this value for peak. Ignore previous statement
  } 

  // The following is used for calculation of various theresholds and signal level calculation
  if (corrMin > corr0) corrMin = corr0;
  if (corrMax < corr0) corrMax = corr0;


  // The average correlation level (at delay 0) is used to set the correlation threshold
  corrTotal += corr0;       // total used for average calculation
  
  // If number of evaluation exceeds threshold then reset all values evaluated
  if (levelResetCtr++ >= PSK_LEVEL_RESET_COUNT) {     // Calculate average and reset values
    corrAvg = corrTotal/PSK_LEVEL_RESET_COUNT;
    corrTotal = 0;
//    binMin = 200;
    binMax = 0;
    corrMax = -600000;
    corrMin = 600000;
    levelResetCtr = 0;
  }


  // Positive sum at delay 0 and peak below threshold (default set to PSK_BIN_THRESHOLD) is strongest indication of phase change
  // pskbinthreshold used to allow for automtic adjustment of threshold.  Not used.
  // Note that peak threshold is usually 17 (i.e. delay of 1.7, delay is x10) less than max bin observered 
  // The peak delay threshold is used to identify phase shifts
  // If the peak delay value is below the threshold, then its potentially a phase shift
  if (corr0 > 0 && corrDly < pskbinthresh) {
    if ( !(pskChanged) ) {          // Set phase shift if no shift detected previousle
      pskChanged = true;            // Uses only in this routine to identify if there was a phase shift
      pskPhase = ~pskPhase;         // Toggle phase (0 of 0xFF). If values changes then phase shift occured. This is returned to calling program
//      Serial1.println ("A");      // Used for debug
    }
    
  // Positive sum at delay 0 with peak not below threshold is the next strongest indication of phase change
  } else if (corr0 > 0) {           // Same as above
    if ( !(pskChanged) ) {
      pskChanged = true;
      pskPhase = ~pskPhase;
//      Serial1.println ("B");
    }
    
  // Correlation sum is above threshold and peak below threshold is next strongest indication
  } else if (corr0 > magThresh && corrDly < pskbinthresh){
    if ( !(pskChanged) ) {          // Same as above
      pskChanged = true;
      pskPhase = ~pskPhase;
//      Serial1.println ("C");
    }

  // Correlation sum is above threshold with peak not below threshold is the weakest indication of phase change
  } else if (corr0 > magThresh) {
    if ( !(pskChanged) ) {          // Same as above
      pskChanged = true;
      pskPhase = ~pskPhase;
//      Serial1.println ("D");
    }


  // If there are minimum number of consective non-phase shift (i.e. negative correlation values) values present after a phase shift, then reset phase shift detection
  } else if (pskChanged && corr0 < magThresh) {
    if (pskResetCtr++ > PSK_RESET_COUNT){  // Orig pskResetCtr++ > PSK_RESET_COUNT - i.e. 3 detections
      pskResetCtr = 0;
      pskChanged = false;
//      Serial1.println ("R");      // Used for debug
    }

    
  }
  
  return pskPhase;
  
}


void ResetPSK (void)
{
// Routine to reset PSK variables
  
  DisableTimers (3);
  DisableTimers (4);

  // Zero buffers
  memset ((char *)adcbuff, 0, sizeof(adcbuff));
  memset ((char *)adcbufflag, 0, sizeof(adcbufflag));
  
  memset ((char *)corrbuff, 0, sizeof(corrbuff));
  memset ((char *)corrbufflag, 0, sizeof(corrbufflag));

  pskPhase = 0;

  pskVaricode = 0;
  bitpos = 0;

  pskChanged = false;
  pskLocked = false;
  decodePhaseChange = false;
  pskResetCtr = 0;

  clipctr = clipctrrst = 0;

  decodeLastPhase = decodePhaseCtr = 0;
  pskState = PSK_INIT;          // Initial PSK State

  // Reset delay measurement parameter
  binMin = 200;
  binMax = 0;

  // Set default correlation values
  corrMax = -600000;
  corrMin = 600000;

  // Set default parameters
  pskbinthresh = PSK_BIN_THRESHOLD;

  // Keep the prior thresholds unless they are obviously incorrect, then reset to default
  if (magThresh <= 0) magThresh = PSK_CORRELATION_THRESHOLD;
  if (ThreshDivider <= MIN_THRESHDIVIDER || ThreshDivider > MAX_THRESHDIVIDER) ThreshDivider = DEFAULT_THRESHDIVIDER;

  corrAvg = 0;
  corrTotal = 0;

  levelResetCtr = 0;

  pskSwap = 1;

  frequency_clk0_tx = frequency_clk0 + TX_FREQUENCY_OFFSET;

  // Reset PSK flags
  flags &= ~TRANSMITPSK;
  flags &= ~REALTIME;
  flags &= ~MEASURETHRESHOLD;
  flags &= ~PROCESSINGDONE;
  flags &= ~REALTIME;
  flags &= ~DECODEPSK;
  flags &= ~BUFF1DONE;
  flags &= ~CHECKPSKVALUE;
  flags &= ~TRANSMIT_CHAR_DONE;
  flags &= ~DISPLAY_SIGNAL_LEVEL;
  flags &= ~CLIPPING;

//  digitalWrite(RxMute, HIGH);            // Unmute receiver, Not used
}


