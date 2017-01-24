/*

Program Written by Dave Rajnauth, VE3OOI to perform various RTTY Rx and Tx processing

*/


#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"

// Baudot lookup tables. Offset is the baudot code and the contents is the ascii character
// Baudot is 5 bits and uses 2 tables with 32 entries (i.e. 2^5). One table for letters (i.e. alphabetic characters))
// and another table for figures (i.e. numbers and special characters)  
const char figures[BAUDOT_TABLE_SIZE] = {0x0,'3',0xA,'-',' ',0x7,'8','7',0xD,'$','4',0x27,',','!',':','(','5',
                  '\"',')','2','#','6','0','1','9','?','&',0x0,'.','/',';',0x0,0x0};
                      
const char letters[BAUDOT_TABLE_SIZE] = {0x0,'E',0xA,'A',' ','S','I','U',0xD,'D','R','J','N','F','C','K','T','Z','L',
                  'W','H','Y','P','Q','O','B','G',0x0,'M','X','V',0x0,0x0};

// Baudot using the LTRS and FIGRS code to switch between tables 
// #define RTTY_FIGURES 27       //11011 bin
// #define RTTY_LETTERS 31       //11111 bin


char Baudot( char c, unsigned char alpha)
{
// Routine to return the baudot code for an ascii character
// Used to translate ASCII to Baudot.  The "alpha" variable
// is used to identify the table to be use (i.e. 0 use figures, 1 use letters)
 
  unsigned char i;

  // Search baudot table
  for (i=0; i<BAUDOT_TABLE_SIZE; i++) {
    if (alpha) {                        // Search letters table
      if (letters[i] == c) {            // ASCII code found
        return i;                       // Return offset which is baudot code
      }
    } else {                            // Search letters table
      if (figures[i] == c) {            // Same as above
        return i;
      }
    }
  }
  
  return 0;                             // nothing found so return 0 (error)
}


void TxRTTYbit (unsigned char b) 
{
// Routine used by timer 4 to transmit a single RTTY bit.
// If bit is 1 turn of MARK frequency, if bit is 0 turn on SPACE frequency
// Timer 4 is executes ever 22 ms (duty cycle for RTTY 45) and carrier will be adjusted
// for next bit or if stop bits needed

  // Don't bother changing frequency if bit state has not changed
  // if if last bit was 0 and this bit is 0, no need to turn on SPACE frequency again.  Unecessary processing...the arduino is stretched as it is.
  if (b != rttyPriorState) {
    digitalWrite(TxEnable, LOW);                // Diable Tranmitter to allow carrier to drop. This is a primitive form of key click filtering. ie envelope is a raised cosine
    if (b) {                                    // Enable MARK/SPACE frequency based on bit value
      SetFrequency (rttyTransmitMarkFreq);
    } else {
      SetFrequency (rttyTransmitSpaceFreq);
    }
    digitalWrite(TxEnable, HIGH);               // Turn on tranmitter.
  }

  rttyPriorState = b;                           // Save current bit state for next iteration
}


char setupRTTYChar (char asciichar)
{
// This routine is used to process the ASCII character to be transmitter.  The routine performs several funcations:
// 1. Converts ASCII to baudot based on it being alphabetice, numeric or special character
// 2. Signals to send LTRS or FIGRS code to switch between letters and figures
// 3. Signal to sent LTRS code for idle condition (i.e. no ascii character present)
// 4. Appends start and stop bit to the baudot code (baudot is 5 bits, 1 start bit, two stop bits = 8 bits)


  char baudot, uppercase;

  // If no character present then signal to send send LTRS code
  if (!asciichar) {
      rttyFigures = RTTY_TX_LETTERS;
      baudot = RTTY_LETTERS;

  // Check for alphabetic characters (either uppercase of lowercase)
  } else if ( (asciichar>64 && asciichar<91) || (asciichar>96 && asciichar<123) ) {            // Alphabetic characters. 
      uppercase = toupper(asciichar);                                                          // Change to uppercase. If already uppercase returns uppercase

      // If last table used was letters then set signal to send letters code before sending current character
      if (rttyFigures != RTTY_TX_LETTERS) {
        rttyFigures = RTTY_TX_LETTERS;
        rttyLTRSSwitch = RTTY_TX_LETTERS;
      } else rttyLTRSSwitch = 0;                                // Already signalled LTRS 

      // Covert ASCCI to baudot
      baudot = Baudot(uppercase, RTTY_ALPHA);
      
  // Check if ascii character is numeric  
  } else if ( asciichar>47 && asciichar<58 ) {                  // Numbers 
      uppercase = asciichar;                                    // no need to make uppercase its a number

      // If last table used was FIGRS then set signal to send figures code before sending current character
      if (rttyFigures != RTTY_TX_FIGURES) {
        rttyFigures = RTTY_TX_FIGURES;
        rttyLTRSSwitch = RTTY_TX_FIGURES;
      } else rttyLTRSSwitch = 0;                                // Already signalled FIGRS 

      // Covert ASCCI to baudot
      baudot = Baudot(uppercase, RTTY_DIGIT);

  // Character is special character    
  } else {                                                      // Other characters
      uppercase = asciichar;                                    // No need to change to uppercase

      // If last table used was FIGRS then set signal to send figures code before sending current character
      if (rttyFigures != RTTY_TX_FIGURES) { 
        rttyFigures = RTTY_TX_FIGURES;
        rttyLTRSSwitch = RTTY_TX_FIGURES;
      } else rttyLTRSSwitch = 0;                                // Already signalled FIGRS 

      // Covert ASCCI to baudot
      baudot = Baudot(uppercase, RTTY_DIGIT);
      
      // This is required because receivers have "un-shift on space"
      // This means that the receive switches to LTRS (Letters) when it receives a space
      // Ensure that this system thinks its in LTRSA mode.
      if (uppercase == ' ') {                            
        rttyFigures = RTTY_TX_LETTERS;
      } else if (!baudot) {                   // if unknown (i.e. not in lookup table), then print a space
        baudot = 0x4;                         // 0x4 is a space
        rttyFigures = RTTY_TX_LETTERS;        // See explanation above
      }
  }

/*
Baudot is 5 bits and a byte is 8 bits. So can have 1 start bit and up to 2 stop bits
With RTTY, idle is mark and start bit is a space. Stop bits are 1.5 mark bits for 45.45 baud
However can use 2 stop bits because idle is mark and stop bits are mark.
i.e.

11xxxxx0
Where 11 are the stop bits, 0 is the start bit and xxxxx are the actual baudot data bits
*/
    baudot <<= 1;               // Shift left 1 bit to allow for start bit
    baudot &= 0b11111110;       // Turn off first bit. i.e. bit 0 is a space and represent start bit
    baudot |= 0b11000000;       // Turn on high order two bits to represent 2 mark bits for stop bits
    return baudot;
}




char DecodeRTTY (unsigned char bitvalue)
{
// Routine to load and frame RTTY character
// Process is 
//    IDLE  =>  START =>  DATA  =>  STOP    =>  GOTO START
//    >7xM      1S        5 bits    1.5-2M  
// Manually search for 7 iterations with Mark frequency if so, then in idle state.
// If in idle state and a start bit received, then start Timer 4 (22 ms duty cycle) to check for bits
// receive 5 bits and then search for 2 stop bits.  If 2 stop bits then start searching for start bit again  (rinse and repeat)
// The bitvalue variable is 0 for space frequency (i.e. space bit), 1 for mark frequency (i.e. mark bit) or 0xFF for unknown 

  // Process RTTY state
  switch (rttyState) {

    // Init state is first time running, idle is already running
    // Init state could also be after an error
    // Search for 5 mark bit. i.e. Mark frequency is turned on for letters threshold
    // Note in idle state LTRS code is sent
    case RTTY_IDLE:
    case RTTY_INIT:
      // Check if mark frequency is present for threshold
      if (bitvalue == 1 && rttyMark++ >= RTTY_LTRS_THRESHOLD) {       // LTRS code found so in idle state so move to next state (Start state)
        // Reset all RTTY variable used to bit detection and character framing
        rttyMark = 0;                   // Mark frequency detected counter..uses for LTRS and Stop bit detection                             
        rttySpace = 0;                  // Space frequency detected counter
        bitpos = 0;                     // bit location that is loaded in rttyChar variable
        rttyChar = 0;                   // Variable that holds the received rtty bits
        rttyLocked = false;             // Lock flag is disabled. Enabled when start, data and stop bits received
        rttyIdle = 0;                   // Idle counter. Used to detect idle condition regardless of state.  Allows state to be reset back to idle
        nortty = 0;                     // Counter to detect if no mark or space frequency detected. Allows state to be reset back to init
        rttyFigures = 1;                // Assume FIGRS received. Will be set to 0 if LTRS code received. This variable is use to identify which rtty table to use
        rttyState = RTTY_START;         // Move to Start state
      } 
      break;

    // Start State. Search for start bit
    case RTTY_START:
      // Check if mark frequency is present for start bit threshold
      if (!bitvalue && rttySpace++ >= RTTY_START_THRESHOLD) {         // Space bit found so move to data state i.e. enable timer and load bits
        DisableTimers (4);              // Reset RTTY timer   

        // So...a start bit received and 22 ms later the first data bit will be encoutered  
        // To improve chances of detecting Mark/Spec bit reset ADC and timer.  i.e. 22 ms from this point should be well within bit duty cycle and less likly 
        // to have detection fall outside of the bit duty cycle 
        // Sampling is turned on by timer 3 (22ms after this point).  In data state after bit is loaded timer 3 is enabled and sampling again started 22 ms later.  
        // Once ADC has loaded a sample, a signal is sent to processed the buffer and this routine is called to load the bit            
        StopSampling();       
        aCtr = 0;                       // Reset ADC to mark this boundary
        flags &= ~ADCDONE;
        flags &= ~REALTIME;
        EnableTimers (4, TIMER22MS);

        // Reset RTTY framing and character loading variables
        rttyMark = 0;
        rttySpace = 0;
        nortty = 0;
        bitpos = 0;
        rttyChar = 0;
        rttyState = RTTY_DATA;          // Change to data state. i.e. load 5 rtty data bite
      }

      break;

    // In data state so receive 5 data bits
    // Rtty character could be LTRS or FIGRS code or a real rtty character
    case RTTY_DATA:
    
      // Makesure bit is 0 or 1
      if (bitvalue != RTTY_UNKNOWN) {
        if (bitvalue) rttyChar |= (1<<bitpos);      // load "1" bit, don't need to load a "0" bit since rttyChar was initialized to be zero prior to processing
        bitpos++;

        // Received all 5 bits so start realtime stamping to look for STOP bits.
        // Start realtime sampling ASAP to detect number of bits.
        if (bitpos >= BAUDOT_BITS) {
          // Disable timer to start ADC and switch to realtime samping 
          DisableTimers (4);
          flags |= REALTIME;
          StartSampling ();
          rttyMark = 0;                             // Reset variable to detect stop bit
          rttySpace = 0;
          nortty = 0;
          bitpos = 0;
          rttyState = RTTY_STOP;                    // Switch to Stop state
        }
      } 
      break;

    // Stop state. Search for stop bits
    case RTTY_STOP:
      // Check if mark frequency is present for stop bits (i.e. 2 bits) threshold
      if (bitvalue == 1 && rttyMark++ >= RTTY_STOP_THRESHOLD) {
        
        // Reset all RTTY variables used for start bit detection 
        rttyMark = 0;
        nortty = 0;
        rttySpace = 0;
        bitpos = 0;
        rttyState = RTTY_START;       // Return to start state
        rttyLocked = true;            // RTTY is being decoded so set lock flag

        // Process RTTY code received
        // Switch between characters and numbers (figures)
        if (rttyChar == RTTY_FIGURES) {
          rttyFigures = 1;
        } else if (rttyChar == RTTY_LETTERS) {
          rttyFigures = 0; 

        // Not LTRS or FIGRS code so check if value within table and do a lookup
        } else if (rttyChar < BAUDOT_TABLE_SIZE) {
          if (rttyFigures) rttyChar = figures[rttyChar];    // Note Baudot code is offset in table. So ASCII character is the value at the offset (i.e. array index)
          else rttyChar = letters[rttyChar];
          return rttyChar;                                  // Return ascii character
        } else rttyLocked = false;   
      } 
      break;
      
    default:
      break;

  }

  // These checks are used to reset the state at any point in time. i.e. check for persistence of an incorrect state
  
  // If a string of not Mark or not Space, then reset back to initial state
  if (bitvalue == RTTY_UNKNOWN && nortty++ >= RTTY_NULL_THRESHOLD) {
    rttyLocked = false;
    rttyFigures = 1;
    rttyIdle = 0;
    nortty = 0;
    rttyState = RTTY_INIT;

  // Check for a string of mark bits that indicated an idle condition 
  } else if (bitvalue == 1 && rttyIdle++ >= RTTY_LTRS_THRESHOLD) {
    rttyIdle = 0;
    rttyLocked = false;
    nortty = 0;
    rttyFigures = 1;
    rttyState = RTTY_START;

  // If at least one space bit received reset idle detection. i.e. RTTY may be present  
  } else if (bitvalue == 0) {
    rttyIdle = 0;
    nortty = 0;
  } 

  return 0;
}


unsigned char GetFreqRange (unsigned int fbin, unsigned int ebin) 
{
// Routine to detect mark space frequency
// Take the autocorrelation of a captured buffered and complete a peak fit to identify the location (delay or lag) of the peak 
// Autocorrelation is a correlation of a signal with itself. 
// Lots of info on Internet about using autocorrelation for fequency determination
// This routine takes a first bin (fbin) and end bin (ebin) and performs the correlation between these two values
// The correlation at delay 0 is used to determing the threshold for the peak.  
// A peak must be at least 40% of the 0 delay value.
// The CrossCorr() routine is used to perform the correlation. This routine is also used for phase shift determination

    volatile unsigned int i;
    volatile long bin;
    volatile long k1, k2, k3, old;        // k1,k2,k3 are used for peak fitting
    
    // reset all variables
    corr0 = 0;
    corrMax = 0;
    corrDly = 0;
    corrRTTY = 0;
    k1 = k2 = k3 = 0;

    // Get the correlation value at 0 delay and store if for sLevel calculation (done elsewhere)
    corr  = CrossCorr (corrbuff, corrbuff, CORRBUFFSZ, 0);
    corrRTTY = corr;

    // If correlation is less than threshold then not a good periodic signal
    // This threshold is calculated in DecodeLoop() as corrRTTY/4 (i.e. 25% of correlation at 0 delay)
    if (corr < magThresh) return 0;

    // Calculate the thresholdfor a bin which is a little less that half of the 0 correlation value
    corr0 = (corr*4)/10;            // 40% of 0 Lag value is threshold

    // Search betwen specified values
    // initial corr value is value at delay 0 which is ok
    for (i=fbin; i<=ebin; i++) {
      if (i<=2) continue;               // Validate that value must be greater that 2 (i.e. avoid detection of maximum at 0 delay)
 
      old = corr;                       // Save previous correlation value;
      corr  = CrossCorr (corrbuff, corrbuff, CORRBUFFSZ, i);    // Calculate correlation value for delay i

      // Check it this may be a peak
      // Peak conditions:
      // 1. correlation value is at least 40% of correlation value at 0 delay
      // 2. correlation value is bigger than previous value (i.e. left side of peak)
      // 3. correlation is largest value so far
      if (corr >= corr0 && corr > old && corr > corrMax) {
        corrMax = corr;                 // Save correlation value as new max
        corrDly = i;                    // save delay value
        k1 = old;                       // define left side of peak value
        k2 = corr;                      // define centre peak value
      }

      // This is used to examine the next value after a potential peak is found
      // Used to identify the right side of peak
      if (corrDly == (i-1)) { 
        if (corr < k2) {                // Value is less than centre value so consistent for a peak
          k3 = corr;                    // define right side of peak value
        } else if (k1) {                // Something wrong, so reset peak detection
          corrMax = corrDly = k1 = k2 = k3 = 0;
        }
      }
    }

    // Do the peak fitting
    // the peak*10 is returned to account for roundoff (i.e. keep one decimal digit)
    if (corrMax && k1 < k2 && k3 < k2 && corrDly) {
      // Here is algorithm:
      // fractional bin = (k3-k1)/2/(k2*2 - k1 - k3), fraction from the max may be +/-
      // estimated peak = k2 - (k1-k3)*bin/4
      // Multiply by 10 to get fractional part accomodated
      bin = (k3 - k1)*5;        // Should be (k3-k1)/2 but x10
      bin /= (k2*2 - k1 - k3);
      corrDly =  corrDly*10 + (int)bin;
      return (corrDly);
    }

    return 0;
}




void ResetRTTY (void)
{
  // Routine to reset all RTTY variables

  
  DisableTimers (3);
  DisableTimers (4);

  // Zero buffers
  memset ((char *)adcbuff, 0, sizeof(adcbuff));
  memset ((char *)corrbuff, 0, sizeof(corrbuff));


  // Reset various RTTY variables
  rttyFigures = 1; 
  rttyChar = 0; 
  bitpos = 0; 
  rttyMark = 0; 
  rttySpace = 0; 
  rttyState = RTTY_INIT;        // Initial RTTY state
  rttyIdle = 0;
  nortty = 0;
  rttyLocked = false;

  clipctr = clipctrrst = 0;
   
  rttySpaceMag = 0;
  rttyMarkMag = 0;

  // Define the Rx frequencies and delay values
  rttySpaceFreq = RTTY_SPACE_FREQUENCY;
  rttyMarkFreq = RTTY_MARK_FREQUENCY;
  rttyMarkBin = F_SAMPLE / rttyMarkFreq;          // Expected delay is a function of sample rate and frequency (similar to FFT)
  rttySpaceBin = F_SAMPLE / rttySpaceFreq;

  // Define default threshold for decode
  if (magThresh <= 0) magThresh = AUTOCORR_THRESHOLD;
  ThreshDivider = 8;

  // Define default Tx frequencies
  // There is a small shift to make the frequency appear as 1000 Hz on receiver's waterfall
  rttyTransmitSpaceFreq = frequency_clk0 - RTTY_SHIFT_FREQUENCY + TX_FREQUENCY_OFFSET;
  rttyTransmitMarkFreq = frequency_clk0 + TX_FREQUENCY_OFFSET;

  // Reset variables to switch between LTRS and FIGRS 
  rttyPriorState = 0xF;
  rttyLTRSSwitch = 0;

  // Reset PSK flags
  flags &= ~TRANSMITRTTY;
  flags &= ~REALTIME;
  flags &= ~DECODERTTY;
  flags &= ~PROCESSINGDONE;
  flags &= ~REALTIME;
  flags &= ~BUFF1DONE;
  flags &= ~TRANSMIT_CHAR_DONE;
  flags &= ~DISPLAY_SIGNAL_LEVEL;
  flags &= ~MEASURETHRESHOLD;
  flags &= ~CLIPPING;

//  digitalWrite(RxMute, HIGH);            // Unmute receiver, unused
}


