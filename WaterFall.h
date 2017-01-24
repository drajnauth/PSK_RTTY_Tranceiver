#ifndef _WATERFALL_H_
#define _WATERFALL_H_

// FFT Routines
void print_fft(void);
void PerformFFT (void);
int FFTSearch (unsigned int frequency, int threshold);
int FFTPeakSearch (unsigned int frequency, int threshold);
void FFTPeaks (unsigned char maxPeaks); 
void setupFFT (void);
void FFTnoise (unsigned int freq);

// FFT Defines
#define LOG_OUT 1
#define OCTAVE 0
#define OCT_NORM 0
#define LIN_OUT 0
#define FHT_N 128     // set to 128 point fht
#define FHT_N2 64     // this must be 64 or else LCD display water fall won't work
#define WINDOW 1


#endif // _WATERFALL_H_
