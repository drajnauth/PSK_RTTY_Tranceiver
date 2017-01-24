/*

Routines Written by Dave Rajnauth, VE3OOI to perform correlation for frequency and phase detection for 
RTTY and PSK demodulation

*/


#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"

long AutoCorr (int lag)
{
// Performs an autocorrelation on the corrbuff[] array based on the specified delay.
// The total is return as a long value which can be positive or negative
// It uses the AVR multipilication accelerator for speed.

// By specifying the lag value the correlation can be used to only correlate specific parts of the 
// array for optimization and speed.

  int i, dcplus, dcminus;
  volatile long total;

// The dcplus and dcminus variables are used to trace the number of positive and negative values
// The idea was that if the buffer was composed of primilary positive or negative values.  If this value 
// exceeds threshold then its not a periodic signal. It primarly a DC level and ignore it
  
  dcplus = dcminus = total = 0;
  for (i=0; i<CORRBUFFSZ-lag; i++) {
    // Check for DC level and if present end processing.
    if (corrbuff[i] < 0) dcminus++;
    else if (corrbuff[i] > 0) dcplus++;
    if (dcplus >= CORRDC || dcminus >= CORRDC) return 0;

    // Perform the multiplication
    total += muls16x16_32(corrbuff[i], corrbuff[i+lag]);
  }
  return total;
}



long CrossCorr (volatile int *buff1, volatile int *buff2, int corrsize,  int lag)
{
// This routine performs a cross correlation on two buffers. Buffers are multiplied togther and summed 
// then shifted.  
// The value of lag and corrsize determined the bounds of the array which should be correlated.
// It uses the AVR multipilication accelerator for speed.

  int j, k;
  volatile long total;
  
  total = 0;
  for (j=0; j<corrsize; j++) {
    k = lag + j;
    if (k < corrsize) {
      total += muls16x16_32(buff1[k], buff2[j]);
    } else {
      j = corrsize;
    }
  }
  return  total;
}


unsigned char GetCorrPeak (unsigned int fbin, unsigned int ebin) 
{
// This routine performs a cross correlation between two arrays and then identifies any positive peaks
// It also includes a peak fitting algorithm used to identify the centre of the peak and the actual peak value
// This routine is used primarly to identify phase shifts. The two most important values are corr0 and 
// the peak location corrDly. The cross correlation is performed on two buffers corrbuff[] and corrbufflag[].
// It is assumes that corrbuff[] and corrbufflag[] is part of a larger buffer that's split into two halves

// With this type of correlation, when a phase shift occures the sum at the positive peak is larger that the initial corr0
// sum.  With an autocorrelation the corr0 sum is larger that the sum at the positive peak



    volatile unsigned int i;
    volatile long bin;
    volatile long k1, k2, k3, old;
    unsigned char sucess;
    
    corr0 = 0;
    corrMax = 0;
    corrDly = 0;
    sucess = 0;
   
    k1 = k2 = k3 = 0;

// First get the correlation value for zero delay. This is used to set the threshold of the peak
    corr0  = CrossCorr (corrbuff, corrbufflag, CROSSCORRSZ, 0);

    if (fbin > 1) corr  = CrossCorr (corrbuff, corrbufflag, CROSSCORRSZ, fbin-1);  
    else  corr = corr0;
    
    for (i=fbin; i<=ebin; i++) {
      old = corr;
      corr  = CrossCorr (corrbuff, corrbufflag, CROSSCORRSZ, i);      // Do a correlation for this delay

      // Identify if this is potentially a peak
      if (corr >= corr0 && corr > old && corr > corrMax) {
        corrMax = corr;
        corrDly = i;
        k1 = old;
        k2 = corr;
      }

      // This only get executes when a prior potential for a peak was identified
      // That is the last iteration defined a potential value for peak (i.e corrDly and corrMax defined)
      // Check if this value is less that prior values (i.e. right side of peak) check for downhill
      if (corrDly == (i-1)) {
        if (corr < k2) {
          k3 = corr;
          sucess = 1;
        } else if (k1) {            // This is to catch any errors where its now downhill AND last value was 
                                    // identified as a potential peak. If so reset peak detection
          corrMax = corrDly = k1 = k2 = k3 = 0;
          sucess = 0;
        }
      }
    }

    // Do the peak fitting
    // the peak*10 is returned to account for roundoff (i.e. keep one decimal digit)
    if (sucess && corrMax && (k1 < k2) && (k3 < k2) && corrDly) {
      // Here is algorithm:
      // fractional bin = (k3-k1)/2/(k2*2 - k1 - k3), fraction from the max may be +/-
      // estimated peak = k2 - (k1-k3)*bin/4
      // Multiply by 10 to get fractional part accomodated
      bin = (k3 - k1)*5;    // Should be (k3-k1)/2 but x10
      bin /= (k2*2 - k1 - k3);
      corrDly =  corrDly*10 + (int)bin;
      return (corrDly);
    }

    return 0;
}

unsigned char ScaleCorr (long value)
{
// This routine was developed to identify the number of bits used by a long values
// The puspose was to use this information to scale long values. 
// Its not used in current implementation. Let here just in case its needed
// Should be removed, complier will optimize it out

  unsigned char i;
  // Scale value down to a 16 bit value. Note this is a signed value and need to scale n-1 bit value (i.e. 32bit is -32768 to 32768)
  for (i=24; i>=16; i--) {

    // For positive number just bit shit to find out the value
    if (value > 0) {
      if (value>>i >= 1) return (i);

    // For negative numbers need to switch to 2's complement to determine number of bit.
    // For positive to negative pos = ~neg + 
    // For negative to positive neg = ~pos + 1
    } else if (value < 0) { 
      if (  (~value+1)>>i >= 1) return (i);        
    }
  }
  return 0;
  
}
