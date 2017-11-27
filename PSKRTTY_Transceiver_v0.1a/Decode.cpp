/*

Routines Written by Dave Rajnauth, VE3OOI to control decoding of RTTY and PSK signals in the audio passband 

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"

//  PORTG ^= (1<<PING5);        //PG5 is D4
//  PORTE ^= (1 << DDE5);       //PE5 is D3
//  PORTE ^= (1 << PINE4);      //PE4 is D2


void DecodeLoop( void ) 
{
// This routine is the main loop for any Rx related activity such as decoding RTTY, PSK, displaying waterfall
// or displaying raw ADC values captured.

  unsigned int i;
  char currentChar;     // Current decode ASCII character

  // ADC sample ready so process buffer captures
  if (flags & ADCDONE) {
    
    // Decode PSK
    if (flags & DECODEPSK) {
      // First lets perform data acquisition and processing in parallel.  
      // This assums that the time to sample/acquire data is longer that time to process last buffer captures
      // For a 8 bit 16Mhz Arduino, this periodically overflows and causes character to be missed of bogus characters 
      // to be decoded.  Arduino needs to be faster!!
      // Need to signal ADC sampling interupt when processing is done so that it can transfer the captued 
      // raw ADC samples to the correlation buffers (corrbuff[] and corrbufflag[])
      // If the ADC need to transfer the values and processing is not complete, its a overrun and an error 
      // is generated and the current buffer is kept (i.e. subsequent buffers not captured until processing is done)
      flags &= ~ADCDONE;
      flags &= ~PROCESSINGDONE;         

      // Process the buffer and identify if phase was shifted. The returned value is a flag that is toggled
      // if there was a phase shift. 
      // E.g. if last value returned by GetPhaseShift() was 0 and there was not phase shift, it returns 0
      // however if there was a phase shift it returns 0xFF (i.e. not 0)  
      // DecodePSK() take the phase shift and frame the received PSK varicode character
      // This loop is executed continiously whenever ADC is finished sampling.  Timer3 is running at 32ms and 
      // signals DecodePSK() to decide if this was a 1 or 0 bit based on the samples processed to now.       
      // DecodePSK() also convertes received varicode to ASCII
      i = (int) GetPhaseShift ();
      currentChar = DecodePSK ((unsigned char)i);
      flags |= PROCESSINGDONE;    // Signal ADC interupt that its ok to tranfer ADC raw buffer to correlation buffers

      // Can either display signal levels or display received characters.
      // Arduino does not have the horsepower to do both. Also the LCD screen is far
      // to slow to allow updates such as this.  Since LCD using I2C, a faster Arduino may 
      // make this possible but with 16Mhz arduino LCD updates are VERY slow!! 
      if (flags & DISPLAY_SIGNAL_LEVEL) {   // Calculate signal levels and display on LCD
        SignalLevel (corrAvg, 'P');
        LCDDisplayLevel ();

      // If character present and PSK appears to be synchronized, then display current character
      } else if (pskLocked && currentChar) {  // Display decoded character on LCD
//        Serial1.print (currentChar);
        LCDDisplayCharacter(currentChar);
        LoadCallSign (currentChar);           // This is supposed to capture the call sign...work in progress
      } 

      // Update threashold value base on the average correlation value for 0 delay. Should be negative
      // Cycle through various dividers (between MIN_THRESHDIVIDER and MAX_THRESHDIVIDER).  This give
      // more granulatity in identifing appropriate threshold value.
      if ( flags & MEASURETHRESHOLD ) {
        if (corrAvg) {
          magThresh = corrAvg / ThreshDivider;
//          magThresh = corrAvg / 10;           // Older artifact, originally using /10 but this new approach seem to be better
          SignalLevel (corrAvg, 'P');
          LCDDisplayLevel ();
          Serial1.print (ThreshDivider);
          Serial1.print (" ");
          Serial1.print (corrAvg);
          Serial1.print (" ");
          Serial1.println (magThresh);
          ThreshDivider++;
          if (ThreshDivider > MAX_THRESHDIVIDER) ThreshDivider = MIN_THRESHDIVIDER;
          flags &= ~MEASURETHRESHOLD;
        }

        // Identify the peak threashold, Threshold is usually 17 (i.e. delay of 1.7, delay is x10) less than max bin observered 
        // The peak delay threshold is used to identify phase shifts
        // If the peak delay value is below the threshold, then its potentially a phase shift
        if (binMax > 17) {
          pskbinthresh = binMax - 17;
        }
      }
   
    // Decode RTTY
    } else if ( flags & DECODERTTY ) {
      // GetFreqRange() Performs an autocorrelation between the delay valuses specified and defines the corrDly
      // which identified the delay (x10) at which a correlation peak appears. The delay is x10 to account
      // for decimal numbers.  
      GetFreqRange (rttyMarkBin - 2, rttySpaceBin + 2);

      currentChar = 0;

      // Check if the delay value calculated by GetFreqRange() is close to the Mark or Space frequency
      // The DecodeRTTY() function takes the bit and assembles the RTTY baudot code.  It used Timer4 which 
      // signals DecodeRTTY() every 22ms to load the bit.  
      // If a start bit, 5 data bits and at least 2 stop bits received, then DecodeRTTY() converts the 5 data bits 
      // from baudot to ASCII and return it      
      if (abs(corrDly - rttySpaceBin * 10) <= 10) {
        currentChar = DecodeRTTY (0);
      } else if ( abs(corrDly - rttyMarkBin * 10) <= 10) {
        currentChar = DecodeRTTY (1);
      } else {
        currentChar = DecodeRTTY (RTTY_UNKNOWN);
      }
      flags &= ~ADCDONE;                // Signal the ADC samping interrupt to continue processing

    
      // Can either display signal levels or display received characters.
      // Arduino does not have the horsepower to do both. Also the LCD screen is far
      // to slow to allow updates such as this.  Since LCD using I2C, a faster Arduino may 
      // make this possible but with 16Mhz arduino LCD updates are VERY slow!! 
      if (flags & DISPLAY_SIGNAL_LEVEL) {
        SignalLevel (corrRTTY, 'R');
        LCDDisplayLevel ();

      // If character present and RTTY appears to be synchronized, then display current character
      } else if (rttyLocked && currentChar) {
        LCDDisplayCharacter(currentChar);
        LoadCallSign (currentChar);     // This is supposed to capture the call sign...work in progress
      }

      // Update threashold value base on the average correlation value for 0 delay. Should be positive
      // Simply take 1/2 of the average RTTY correlation value.
      if (flags & MEASURETHRESHOLD) {
        magThresh = corrRTTY >> 2;
        SignalLevel (corrRTTY, 'R');
        LCDDisplayLevel ();
        Serial1.print (corrRTTY);
        Serial1.print (" ");
        Serial1.println (magThresh);
        flags &= ~MEASURETHRESHOLD;
      } 


    // Display Waterfall - Perform DFT and dislay spectrum on LCD
    // Since this is using an FFT much more data needs to be catured that for a correlation and hence
    // this is executed much slower
    } else if (flags & DOFHT) {

      // Stop data acquisition and perform the FFT
      StopSampling();         
      PerformFFT();

      // Check mode of display
      if (flags & NARROW_WATERFALL) {

        // Narrow band mode so disply FFT bins and correlation peak delays side by side for comparison
        // Calculate delay for the correlation peak as if it were RTTY 
        // Also display the Bin for the FFT peak
        magThresh = AUTOCORR_THRESHOLD;
        GetFreqRange (rttyMarkBin - 2, rttySpaceBin + 2);
        LCDDisplayPassbandWaterfall (); 

      // Regular wide band mode so display spectrum on LCD (VERY SLOW!!!)
      } else {
        LCDDisplayWaterFall (); 
      }     
      // Treat as if it were RTTY and calculate and display the signal levels
      //SignalLevel() needs RTTY to be synchronized
      SignalLevel (corrRTTY, 'R');
      LCDDisplayLevel ();

      // Enable capture of another sample and continue
      flags &= ~ADCDONE;
      StartSampling();

    } else if (flags & ADCMONITOR) {
      for (i = 0; i < FHT_N; i++) Serial1.println (fht_input[i]);
      flags &= ~ADCDONE;

    }
  } // ADC DONE

  // Completed all process and data is being captured by ADC, check if rotary encoder engaged to change frequency
  // If engaged, then update frequency data on display
  // Note since updating the LCD is slow, its required to stop data acquisition when updating the LCD
  UpdateFrequencyData (1);        // Update frequency display and change frequency of OSC

  // If an errow condition was thrown, display the error code.
  // Note that for PSK, there are numerious OVERRUNS and there would be a barrage of PSK errors displayed on LCD
  // This has a catastrophic effect on PSK decode because updating the LCD with an error code is slow and 
  // caused more OVERRUNS and PSK bascially stops working.  
  // I was using SignalError() which display the error on serial1 but this also has a negative effect on
  // PSK decoding. So for the time being, PSK errors are not displayed.
  if (flags & DISPLAY_ERROR) {
    if (flags & DECODERTTY ) LCDSignalError (errorCode);
//    else SignalError (errorCode);                       // Display PSK error on Serial1
    flags &= ~DISPLAY_ERROR;

  // This is used to autoclear the error message on the LCD otherwise you can't tell if a new
  // error code is present
  } else if (errorCode && LCDErrctr++ > LCD_CLEAR_ERROR_THRESHOLD) {
    if (flags & DECODERTTY) LCDSignalError (0);
    errorCode = 0;
    LCDErrctr = 0;
  }

  
}


void UpdateFrequencyData (unsigned char updateFrequency)
{

  // encoderState is used to signal if the encoder pushbutton was engaged to change the frequency
  // increment. That is the increment/decrement to apply to the frequency when to encoder is rotated
  // Because the LCD is slow, this caused PSK/RTTY character to be dropped or misintrepreted.  
  // Arduino just does not have the horsepower
  if (encoderState & 0x1) {
    LCDDisplayFrequencyIncrement ();            // Update LCD with increment
        encoderState &= ~0x1;                   // Clear the signal
  }
  
  if (encoderState & 0x2) {
    // Encoder was rotated to update the various frequencies used to Rx and Tx
    rttyTransmitSpaceFreq = frequency_clk0 - RTTY_SHIFT_FREQUENCY + TX_FREQUENCY_OFFSET;
    rttyTransmitMarkFreq = frequency_clk0 + TX_FREQUENCY_OFFSET;
    frequency_clk0_tx = frequency_clk0 + TX_FREQUENCY_OFFSET;

    // If the receiver or the waterfall is running then change frequency
    // Updating the frequency takes some time (calculating Si5351 dividers and I2C communications) and
    // will cause RTTY/PSK decode errors. Arduino horsepower thing....
    if (flags & REALTIME || flags & DOFHT) {
      if (updateFrequency) SetFrequency (frequency_clk0);
    } else {
      if (updateFrequency) SetFrequency (frequency_clk0_tx);
    }

    // Finally update the frequency on the LCD and clear the signal
    LCDDisplayFrequency ();
    encoderState &= ~0x2;
  }
  
}

void SignalLevel (long rawlevel, char mode)
{

// This routine is a work in progress. Its supposed to calcuate the dbM of the recieved signal (via ADC value)
// as well as display the strength of the correlation

// Still working on it....

//  double ratio;

#define POWER 256
#define ALPHA 192
 
  if (mode == 'R') {   // && rttyLocked
    
    // digitalSignalLevel is supposed to be a value that track the correlation level and display a value
    // indicated how far off the max it it. It help identify max correlation level. 10 is the max and another 
    // below 10 is a relative difference from the maximum
    // Its a work in progress....
    oldDigitalLevel = digitalSignalLevel;
    if (rawlevel >= magThresh) {
      digitalSignalLevel =  (10*(rawlevel-magThresh))/rawlevel;
    } else {
      digitalSignalLevel =  (10*(rawlevel-magThresh))/magThresh;
    }

    // Smooth out the values
    digitalSignalLevel = (ALPHA * digitalSignalLevel + (POWER - ALPHA) * oldDigitalLevel )/ POWER;
    
    // Smooth out the values
    oldCorrLevel = (ALPHA * rawlevel + (POWER - ALPHA) * oldCorrLevel )/ POWER;    
    if (oldCorrLevel > maxCorrLevel) {
      maxCorrLevel = oldCorrLevel;
    }

    // Round the corrLevel - another form of smoothing
    // if below 1000 round to nearest 100
    // if between 1000 and 2000 round to nearest 200
    // if over 2000 round to the nearest 500
    if (corrLevel < 1000) corrLevel = roundUp(maxCorrLevel, 100);
    else if (corrLevel < 2000) corrLevel = roundUp(maxCorrLevel, 200);
    else corrLevel = roundUp(maxCorrLevel, 500);
    
    // This is the update interval...another form of smoothing
    if (levelctr++ > 10) {
      levelctr = 0;
      maxCorrLevel = 0;
    }
    
  } else if (mode == 'P') {    // && pskLocked

    // digitalSignalLevel is supposed to be a value that track the correlation level and display a value
    // indicated how far off the max it it. It help identify max correlation level. 10 is the max and another 
    // below 10 is a relative difference from the maximum
    // Its a work in progress....
    oldDigitalLevel = digitalSignalLevel;
    if (rawlevel >= magThresh) {
      digitalSignalLevel =  (10*(magThresh - rawlevel))/magThresh;
      if (digitalSignalLevel > 0) digitalSignalLevel = -digitalSignalLevel; 
    } else {
      digitalSignalLevel =  (10*(rawlevel - magThresh))/rawlevel;
      if (digitalSignalLevel < 0) digitalSignalLevel = -digitalSignalLevel; 
    }

    // Smooth out the values
    digitalSignalLevel = (ALPHA * digitalSignalLevel + (POWER - ALPHA) * oldDigitalLevel )/ POWER;
    
    oldCorrLevel = (ALPHA * rawlevel + (POWER - ALPHA) * oldCorrLevel )/ POWER;    
    if (oldCorrLevel < maxCorrLevel) {
      maxCorrLevel = oldCorrLevel;
    }
   
    // Round the corrLevel - another form of smoothing
    // if below 1000 round to nearest 100
    // if between 1000 and 2000 round to nearest 200
    // if over 2000 round to the nearest 500
    if (corrLevel < 0) corrLevel = -corrLevel;
    if (corrLevel < 1000) corrLevel = roundUp(maxCorrLevel, 100);
    else if (corrLevel < 2000) corrLevel = roundUp(maxCorrLevel, 200);
    else corrLevel = roundUp(maxCorrLevel, 500);
    
    // This is the update interval...another form of smoothing
    if (levelctr++ > 10) {
      levelctr = 0;
      maxCorrLevel = AUTOCORR_THRESHOLD;
    }
    
  } else {            // Not in PSK or RTTY decode
    digitalSignalLevel = 0;
    corrLevel = 0;
  }

  // this is the dbM calculation.  I wanted to show the sLevel here (i.e. S9 us -73 dbM etc)
  // It almost impossible to get the actualy RF signal strength because amplification is added
  // RF amp is is also variable on the receiver I'm using.
  oldvLevel = (ALPHA * vLevel + (POWER - ALPHA) * oldvLevel )/POWER;  
  if (oldvLevel > maxvLevel) {
    maxvLevel = oldvLevel;
    if (oldvLevel) sLevel = 10*log10(oldvLevel*5);        // This is slow. Need to find an ASM version of log10().
    else sLevel = 0;
  }
  
  // Update interval
  if (slevelctr++ > 20) {
    slevelctr = 0;
    maxvLevel = 0;
    vLevel = 0;
  }

}


void RTTYControl (char function)
{
// This function is used to control RTTY Rx
  
  // Disable RTTY
  if (function == 'D') {
    DisableTimers (1);              // Timer 1 is for 5ms for Rotary
    StopSampling();
    flags &= ~REALTIME;
    flags &= ~MEASURETHRESHOLD;
    flags &= ~DECODERTTY;

  // if no sub commands then start RTTY decode.
  } else {
    ResetRTTY();
    flags |= REALTIME;
    flags |= DECODERTTY;
    levelctr = 0;
    maxCorrLevel = 0;
    EnableTimers (1, TIMER3MS);         // Timer 1 is for Rotary 
    StartSampling ();
  }

}


void PSKControl (char function)
{
// This function is used to control PSK Rx

  // Disable PSK
  if (function == 'D') {
    DisableTimers (1);              // Timer 1 is for 5ms for Rotary
    StopSampling();
    flags &= ~MEASURETHRESHOLD;
    flags &= ~PROCESSINGDONE;
    flags &= ~REALTIME;
    flags &= ~DECODEPSK;
    flags &= ~BUFF1DONE;
    flags &= ~CHECKPSKVALUE;

  // if no sub commands then start dPSKecode.
  } else {
    ResetPSK();
    flags |= PROCESSINGDONE;
    flags |= REALTIME;
    flags |= DECODEPSK;
    flags &= ~BUFF1DONE;
    flags &= ~CHECKPSKVALUE;
    levelctr = 0;
    maxCorrLevel = 0;
    EnableTimers (1, TIMER3MS);         // Timer 1 is for Rotary 
    StartSampling ();

  }
}


void SignalError (unsigned long errorcode)
{
// Routine similar to LCD routine to display errors except error is displayed on Serial1
 
  switch (errorcode) {
    case SERIAL_BUFFER_OVERFLOW:
      Serial1.println((char *)"SOVF");
      break;

    case RTTY_DATA_OVERRUN:
      Serial1.println((char *)"RBUF");
      break;
      
    case PSK_BUFFER_OVERFLOW:
      Serial1.println((char *)"PBUF");
      break;

    case PSK_DATA_OVERRUN:
      Serial1.println((char *)"PND");
      break;

    case FREQUENCY_BAD_CHANNEL:
      Serial1.println((char *)"BC");
      break;

    case FREQUENCY_BAD_RANGE:
      Serial1.println((char *)"BR");
      break;

    case CANNOT_COMPLETE_DECODE_ENABLED:
      Serial1.println((char *)"CC");
      break;

    case FREQUENCY_NOT_SET:
      Serial1.println((char *)"FNS");
      break;

  }
  
}

void DisplayLevel (void)
{
// Routine similar to LCD routine to display signal levels except signal data is displayed on Serial1

  ToggleSampling (0);
  Serial1.print((long)sLevel); 
  Serial1.print ("\t");
  Serial1.print((long)digitalSignalLevel); 
  Serial1.print ("\t");
  Serial1.print((long)corrLevel);  
  Serial1.println("");
  
  ToggleSampling (1);

}


int roundUp(int number, int fixedBase) {
// Funtion to round a number up to nearest 10's
// Code taken from a Blog - can't recall which one.
  
  if (fixedBase != 0 && number != 0) {
    int sign = number > 0 ? 1 : -1;
    int baseSign = fixedBase > 0 ? 1 : 0;
    number *= sign;
    int fixedPoint = (number + baseSign * (fixedBase - 1)) / fixedBase;
    number = fixedPoint * fixedBase;
    number *= sign;
  }
  return number;
}


