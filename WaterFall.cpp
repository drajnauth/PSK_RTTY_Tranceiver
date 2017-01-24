/*

Program Written by Dave Rajnauth, VE3OOI to perform a FFT for the spectrum display which is referred to 
as the "waterfall"

Its based on the Fast Hartley Transform (FHT) from Open Music Labs 

The output of the FHT is slightly different than the FFT. It is as follows:
Re(k) = (x(k) + x(-k))/2
Im(k) = (x(k) - x(-k))/2
Mag(k) = sqrt((x(k)2 + x(-k)2)/2)
Phase(k) = atan2((x(k) - x(-k)), (x(k) + x(-k)))
Frequency(k) = (k)*(sample_rate)/(FHT_N)

fht_mag_lin() - This gives the magnitude of each bin in the FHT. 
It sums the squares of the imaginary and real, and then takes the square root. 
It uses a lookup table to calculate the square root, so it has limited precision. 
You can think of it as an 8b value times a 4b exponent. It covers the full 16b range, 
but only has 8b of precision at any point in that range. 
The data is taken in from fht_input[] and returned in fht_lin_out[]. 
The values are in sequential order, and there are only N/2 values total, as the FHT returns redundant data.

fht_mag_log() - This gives the magnitude of each bin in the FHT. It sums the squares of the imaginary and real, 
and then takes the square root, and then takes the log base 2 of that value. 
Therefore, the output is compressed in a logarithmic fashion, and is essentially in decibels 
(times a scaling factor). It takes no variables, and returns no variables. 
It uses a lookup table to calculate the log of the square root, and scales the output over the full 8b range 
{the equation is 16*(log2((img2 + real2)1/2))}. It is only an 8b value, and the values are taken from 
fht_input[], and returned in fht_log_out[]. The output values are in sequential order of FHT frequency bins, 
and there are only N/2 total bins, as the second half of the FFT result is redundant.

fht_mag_octave() - This outputs the RMS value of the bins in an octave (doubling of frequencies) format. 
This is more useful in some ways, as it is closer to how humans perceive sound. 
It doesn't take any variables, and doesn't return any variables. The input is taken from fht_output[] 
and returned in fht_oct_out[]. The data is represented as an 8b value of 16*log2(sqrt(mag))


See wiki.openmusiclabs.com  for more information

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"

// FHT.h can only be defined once. It defined variables which will get redeclaired if this
// if included elsewhere.
#include <FHT.h>              // Defines to complete Fast Hatley Transform
                              // There are defines that are required for this header file. They are defined in M0XPD_Controller_v0.1.h 


void FFTPeaks (unsigned char maxPeaks) 
{
// This routines searches the FFT output and identifies all peaks found in the output.
// The bin value of each peak is entered in the Peaks[] array
// maxPeaks details how many peaks to search for and must be smaller that the Peak[] array size. 
// No error checking done to validate maxPeaks!!

  unsigned char i, j;
  int maxRe;

  // Zero out the arrays and create a copy of the FFT output.
  // Subsequent processing, alters the FFT ouput so a copy must be used.
  // Whenever a peak is found, its zeroed in the FFT output so that the next highest peak can be found
  memset (Peaks, 0, sizeof(Peaks));
  memcpy ((char *)fft_tmp, (char *)fht_log_out, sizeof(fft_tmp));

  
  fft_tmp[0] = 0; // remove DC component othwise the highest peak will alwasy be at DC


  for (j=0; j<maxPeaks; j++) {
    maxRe = 0;
    for (i=2; i<FHT_N2; i++) {      // Start at bin 2 to ignore DC values
      if (maxRe < fft_tmp[i]) {
        Peaks[j] = i;
        maxRe = fft_tmp[i];
      }
    }

    // Zero out all values around the peak. Its assumed that 3 values defines the peaks.
    if ((Peaks[j]-1) > 0) fft_tmp[ Peaks[j]-1 ] = 0;
    fft_tmp[ Peaks[j] ] = 0;
    if ((Peaks[j]+1) < FHT_N2) fft_tmp[ Peaks[j]+1 ] = 0;
  }
 
}

void setupFFT (void)
{
// This routine defines the frequency associated with each bin of the FFT
// bin for a specific freqency = Frequency * FFT_Sample_Size / Sample_Rate
// Eg. for 1000 Hz, bin = 1000 * 128 / 9615 = 13

  unsigned int i;
  FreqPerBin = (double)F_SAMPLE / (double)FHT_N;     // Fequency/bin  
  for (i=0; i<FHT_N2; i++) {
    binFreq[i] = round ((double)FreqPerBin * (double)i);
  }

}


void PerformFFT (void)
{
// This routines performs the actual FFT using FHT (Fast Hatley Transform)routines from Open Music Labs
// wiki.openmusiclabs.com

  fht_window();     // window the data for better frequency response
  fht_reorder();    // reorder the data before doing the fft
  fht_run();        // process the data in the fft
//  fht_mag_octave();   // Output based on octave normilisation 
  fht_mag_log();    // Generate the logarithmic output of the fft
//  fht_mag_lin(); // Generate the linear output of the fft
}

/*
// Generate the magnitude of each bin and print it on the serial monitor
// used for debug
void print_fft(void)
{
  int i,j,largest;
  char str[FHT_N2+1];
  char linfo[6];

  str[FHT_N2] = 0;  
  largest = 0;
  // Find the largest entry which will determine how many lines
  // are needed to print the whole histogram
  for (i=0; i<FHT_N2; i++){
    if(fht_log_out[i] > largest)largest = fht_log_out[i];
  }
  // print a blank line just in case there's
  // garbage when the Serial monitor starts up
  Serial1.println("FFT Output");

  // print the histogram starting with the highest amplitude
  // and working our way back down to zero.
  for(j=largest;j >= 0;j--) {
    for(i=0;i<FHT_N2;i++) {
      // If the magnitude of this bin is at least as large as
      // the current magnitude we print an asterisk
      if(fht_log_out[i] >= j)str[i] = '*';
      // otherwise print a space
      else str[i] = ' ';
    }
    sprintf(linfo,"%3d ",j);
    Serial1.print(linfo);
    Serial1.println(str);
  }
  // print the bin numbers along the bottom
  Serial1.print("    ");
  for(i=0;i<FHT_N2;i++) {
    Serial1.print(i%10);
  }
  Serial1.println("");
  Serial1.print("    ");
  for(i=0;i<FHT_N2;i++) {
    if(i < 10)Serial1.print(" ");
    else Serial1.print((i/10)%10);
  }
  Serial1.println("");
}

*/


