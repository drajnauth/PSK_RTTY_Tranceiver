/*

Routines Written by Dave Rajnauth, VE3OOI to control ADC and sampling of audio data containing PSK/RTTY signals 

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"


//////////////////////////////////
//  ADC ISR  - Used to Sample at a rate of 9615 Hz
//////////////////////////////////
ISR(ADC_vect)
{
  // Gather ADC samples and fill buffer for processing
  aLow = ADCL;
  aHigh = ADCH;
  si = (aHigh << 8) | aLow; // get 10 bit adc value
  si -= 0x0200;             // form into a signed int
  //    si += 9;

  CheckForClip ();

  // ---------------  Fill RTTY Auto-Correlation Bufffer
  if (flags & DECODERTTY) {
    
    // Check if buffer full
    if (aCtr < CORRBUFFSZ) {
      adcbuff[aCtr++] = si;   // put data into buffer
      
    } else if ( !(flags & ADCDONE) ) {
      flags |= ADCDONE;
      memcpy ((char *)corrbuff, (char *)adcbuff, sizeof(corrbuff));
      aCtr = 0;
      if (!(flags & REALTIME)) StopSampling();

    // Overrun condition
    }  else {
      flags |= DISPLAY_ERROR;
      errorCode = RTTY_DATA_OVERRUN;
//      aCtr = 0;
//      flags &= ~ADCDONE;
//      flags &= ~MEASURETHRESHOLD;
    }

    // ---------------  Fill PSK Cross-Correlation Bufffers
    // MUST have two buffers back to back (i.e contigious buffers)..so wait for flag to clear
  } else if (!(flags & ADCDONE) && flags & DECODEPSK) {

    // Fill First buffer
    if (aCtr < CROSSCORRSZ) {
      adcbufflag[aCtr++] = si;   // put data into 1st buffer

    // Fill second buffer
    } else if ( aCtr >= CROSSCORRSZ && aCtr < CROSSCORRSZ * 2) {
      
      // Transfer first buffer if not completed previously then start filling 2nd buffer
      if (!(flags & BUFF1DONE) && (flags & PROCESSINGDONE)) {
        memcpy ((char *)corrbufflag, (char *)adcbufflag, sizeof(corrbufflag));
        flags |= BUFF1DONE;
        
      // Overflow condition
      } else if (!(flags & BUFF1DONE) && !(flags & PROCESSINGDONE)) {
          flags |= DISPLAY_ERROR;
          errorCode = PSK_BUFFER_OVERFLOW;
      }
      adcbuff[aCtr - CROSSCORRSZ] = si;  // put real data into 2nd buffer
      aCtr++;

    // Signal both buffers full
    } else if ( aCtr >= CROSSCORRSZ * 2) {
      if ( flags & PROCESSINGDONE ) {
        memcpy ((char *)corrbuff, (char *)adcbuff, sizeof(corrbuff));
        flags |= ADCDONE;
        flags &= ~BUFF1DONE;
        aCtr = 0;
        if (!(flags & REALTIME)) StopSampling();

      // Overflow condition
      } else {
          flags |= DISPLAY_ERROR;
          errorCode = PSK_BUFFER_OVERFLOW;
      }
    }

  // Overrun condition
  } else if ((flags & ADCDONE) && (flags & DECODEPSK)) {
    flags |= DISPLAY_ERROR;
    errorCode = PSK_DATA_OVERRUN;
//    aCtr = 0;
//    flags &= ~ADCDONE;
//    flags |= PROCESSINGDONE;
//    flags &= ~MEASURETHRESHOLD;

  // ---------------  Fill Descrete FFT Buffer
  } else if (flags & DOFHT) {
    // Check if buffer full
    if (aCtr < FHT_N) {
      adcbuff[aCtr++] = si;   // put data into buffer
      
    } else if ( !(flags & ADCDONE) ) {
      flags |= ADCDONE;
      memcpy ((char *)fht_input, (char *)adcbuff, sizeof(fht_input));
      memcpy ((char *)corrbuff, (char *)adcbuff, sizeof(corrbuff));
      aCtr = 0;

    // Overrun condition...don't do anything...don't care in this mode
    }  else {
      aCtr = 0;
    }

  // ---------------  Fill Buffer for Console Dump
  } else if (flags & ADCMONITOR) {
    // Check if buffer full
    if (aCtr < FHT_N) {
      adcbuff[aCtr++] = si;   // put data into buffer
    } else if ( !(flags & ADCDONE) ) {
      flags |= ADCDONE;
      memcpy ((char *)fht_input, (char *)adcbuff, sizeof(fht_input));
      aCtr = 0;
      
    // Overrun condition...don't do anything...don't care in this mode
    }  else {
      aCtr = 0;
    }
  }


}

void CheckForClip (void)
{
// This function is used to check if sucessive sampled values are the same
// When samples values remain the same for one wavelength, then its clipping
  
  if (si > vLevel) vLevel = si;     // Set vLevel to the max value

  // Need to check is sampled value is an actual audio signal by checking against a min threshold
  // Next check if last value also exceeds min threshold 
  // If thresholds are met, then check the difference between values to see if they fall below a
  // difference threshold. 
  if (si > ADC_CLIPPING_THRESHOLD && lastsi > ADC_CLIPPING_THRESHOLD || si < -(ADC_CLIPPING_THRESHOLD) && lastsi < -(ADC_CLIPPING_THRESHOLD) ) {

    deltasi = si ^ lastsi;                    // XOR to find the difference. Only care about differences in lower 2 bits
    
    if (deltasi < MIN_ADC_DELTA) {            // check if below threshold
      if (clipctr++ > MAX_CLIP_COUNT) {       // Debounce.  Check for sucessive flat topping
        flags |= CLIPPING;                    // Set Clip flag.  LEDs turn on elsewhere
        clipctr = clipctrrst = 0;             // Reset clipping reset counter
      }
    }
  } else {                                    // Reset logic
    if ( flags & CLIPPING ) {                 //  If clipping detected
      if (clipctrrst++ > MAX_CLIP_RESET_COUNT) {      // See if state has not changed 
        flags &= ~CLIPPING;                   // Reset Clipping singal
        clipctr = clipctrrst = 0;
      }
    } 
  }

  // Reset to allow for updates
  if (slctr++ > ADC_RESET_COUNT) {
    vLevel = 0;
    slctr = 0;
//    clipctr = clipctrrst = 0;           // Remove this because LED did not stay on long enough
//    flags &= ~CLIPPING;
  }
  lastsi = si;                        // Save current state for comparision on next iteration

}







void ToggleSampling (unsigned char mode)
{
// Routing to toggle the ADC on/off for acquiring samples.
  
  if (flags & REALTIME) {
    if (mode) {
      StartSampling();
    } else {
      StopSampling();
    }
  }
}


void StartSampling (void)
{
// Routine to enable Sampling

  // First rest all associated variables
  aCtr = 0;
  lastsi = 0;
  deltasi = 0;
  clipctr = 0;
  vLevel = 0;
  sLevel = 0;
  slctr = 0;

  flags &= ~ADCDONE;
  EnableADC();
}

void StopSampling (void)
{
// Routine to turn off ADC and Sampling

  aCtr = 0;
  ADCSRA = 0;
}


void EnableADC (void)
{
// Function to enable the ADC to acquire samples.  Currently using free running (9615 Hz)

  DIDR0 = (1 << ADC0D); // turn off the digital input for adc0
  ADMUX = (1 << REFS0); // AVCC is ref, A0 as input
  //    ADMUX |= (1<<ADLAR);   // Right justify (ie. drop last 2 bits or divide by 4) to get 8 bits samples (ADCH has 8 bit sample)
  ADCSRA = 0;           // Reset
  ADCSRB = 0;           // Free Running
  ADCSRA |= (1 << ADATE); // Enable Auto Trigger Enable
  //    ADCSRA |= (1<<ADPS2) | (1<<ADPS1);                 // Enable Prescaler 64 (250 Khz)
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable Prescaler 128 (125 Khz)
  ADCSRA |= (1 << ADEN);      // Enable ADC
  ADCSRA |= (1 << ADIE);      // Enable Interrupt
  ADCSRA |= (1 << ADSC);      // Start Conversion

}


