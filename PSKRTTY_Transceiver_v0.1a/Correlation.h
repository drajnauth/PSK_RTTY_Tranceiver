#ifndef _CORR_H_
#define _CORR_H_

// Correlation Defines
#define CORRBUFFSZ 40                     // Maximum buffer to capture for autocorrelation. Orig 40. Use 700 for buffer capture
#define CORRDC 30                         // Used to detect a DC level (i.e. no signal)
  
#define CROSSCORRSZ 13                    // Number of samples to cross correlate. For 1Khz signal 13 samples cause correcation
                                          // between consecutive samples to give a large negative lag(0) value
                                          // A phase shift will show up as a positive lag(0) value

long CrossCorr (volatile int *buff1, volatile int *buff2, int corrsize,  int lag);
unsigned char GetCorrPeak (unsigned int fbin, unsigned int ebin);
unsigned char ScaleCorr (long value);

#endif // _CORR_H_
