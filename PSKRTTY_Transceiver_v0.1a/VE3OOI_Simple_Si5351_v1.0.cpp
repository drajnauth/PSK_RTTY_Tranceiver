#include "Arduino.h"

#include "VE3OOI_Simple_Si5351_v1.0.h"      // VE3OOI Si5351 Routines
#include "i2c.h"

// This defines the various parameter used to program Si5351 (See Silicon Labs AN619 Note)
// multisynch defines specific parameters used to determine Si5351 registers
// clk0ctl, clk1ctl, clk2ctl defined specific parameters used to control each clock
Si5351_def multisynth;
Si5351_CLK_def clk0ctl;
Si5351_CLK_def clk1ctl;
Si5351_CLK_def clk2ctl;

// These are variables used by the various routines.  Its globally defined to conserve ram memory
unsigned long temp;
unsigned char base;
unsigned char clkreg;

/*
The way the Si5351 works (in a nutshell) is the a PLL frequency is generated based on the Crystal Frequency (XTAL).  A multisyncth multiplier (called Feedback Multisynth Divider
but I refer to is at the PLL multisynth multiplier) is used to generate the PLL frequency. The PLL frequency MUST be between 600 Mhz and 900 Mhz!!. So for 25 Mhz clock the multipler must 
be between 24 and 36

The output clock (clk0, clk1, clk2) take a PLL frequency and applied another multisyncth divider (called Output Multisynth Divider) to get the desired frequency.

So,
PLL Frequency = XTAL_Frequency * (PLL_MS_A + PLL_MS_B / PLL_MS_C), where A, B and C is the PLL Multisynch multiplier. The PLL divider can be between 8 +1/1048575 to 900.

Clock Output Frequency = Desired Frequency = PLL_Frequency / (MS_A + MS_B / MS_C), where A, B, C are a seperate other multisynth divider (Output Multisynth Divider)
The MSN divider can be between 15 +1/1048575 to 90

Therefore its critical to get the B/C ratio to represent the fractional component of the divider/multiplier.  
For example, to generate a PLL frequency of 612500000 Hz (612.5 Mhz), we need to multiply the 25Mhz crystal frequency by 24.5 or (24 + 1/2) so B/C = 1/2

The FareyFraction () routine take a decimal number (e.g. 0.5) and generates approprite B & C values to represent the decimal number.
The RationalNumberAppoximation() routine....

The A, B and C values are encoded and then written to various Si5351 registers.

A special note about drive current (2, 4, 6, or 8 mA).  To get 50R output impedence you must use 8 mA.  Using lower drives causes the output impedence to increase above 50R.

Si5351_def is structure that defines the following "multisynth" structure
=========================================================================
The multsynth structure is used to define parameters used to configure PLL and Multisynth dividers.  It does not 
represent a running state.  

multisynth.Fxtal is the reference crystal clock
multisynth.Fxtalcorr is the adjusted/corrected reference crystal clock based on correction/calibration value
multisynth.correction correction for xtal in parts per 10 million
multisynth.PLL is the PLL to use either 'A' or 'B'
multisynth.MS_Fout is the output frequency
multisynth.PLL_Fvco is the PLL Clock Frequency (Max 900 MHz)

multisynth.ClkEnable is a parameter used to enable or disable the clock being configured

multisynth.MSN_P1 is PLL Feedback Multisynth Divider
multisynth.MSN_P2 is PLL Feedback Multisynth Divider
multisynth.MSN_P3 is PLL Feedback Multisynth Divider
multisynth.PLL_a is PLL Feedback Multisynth Divider
multisynth.PLL_b is PLL Feedback Multisynth Divider
multisynth.PLL_c is PLL Feedback Multisynth Divider

multisynth.PLL_Fvco = multisynth.Fxtal * (multisynth.PLL_a + multisynth.PLL_b/multisynth.PLL_c)
where a, b, c are fractional dividers for PLL frequency
a is multiplier, b is numerator and c is demonator (a+b/c)

multisynth.MS_a is Output Multisynth Divider
multisynth.MS_b is Output Multisynth Divider
multisynth.MS_c is Output Multisynth Divider
multisynth.MS_P1 is Output Multisynth Divider
multisynth.MS_P2 is Output Multisynth Divider
multisynth.MS_P3 is Output Multisynth Divider

multisynth.MS_Fout = multisynth.PLL_Fvco * (multisynth.MS_a + multisynth.MS_b/multisynth.MS_c)

To convert a, b and c to P1, P2 and P3 the following calculation is done for either PLL multisynch divider or output multisynch output
temp = (128*multisynth.MS_b)/multisynth.MS_c;      // Note that 128*multisynth.MS_b is done first to have the most accurate integer value
multisynth.MS_P1 = 128 * multisynth.MS_a + temp - 512;
multisynth.MS_P2 = 128 * multisynth.MS_b - multisynth.MS_c * temp;
multisynth.MS_P3 = multisynth.MS_c;
where a, b, c are fractional dividers for output
a is multiplier, b is numerator and c is demonator (a+b/c)

multisynth.R_DIV R_DIV MUST be used for frequencies below 500 Hz (to min of 8 Khz).  These routines used R_DIV for frequencies below 1 Mhz.  R_DIV indicates that the output frequency should be divided by 1, 2, 4, 8,....128. 
R_DIV = 0 means divide by 1 (the default configuration).  Bascially the output frequecy is multiplied by an interger to get the frequency above
1 Mhz, then the R_DIV is applied to bring the frequency back to the frequency.
For example, for 800 Khz:
- The frequency output is multiplied by 4 (4x800=3.2 Mhz). That is the PLL and Output Multisynth for Clock is set to 3.2 Mhz
- The R_DIV is set to 4 so the actual output frequency is 3.2/4 = 800 Khz
- This may see silly but its necessary for the Si5351 below 500 Khz.  This was done below 1 Mhz for convenience.

multisynth.MS_DIVBY4 is a similar divider that is used for frequencies above 150 Mhz to 160 Mhz.  The Multisynth cannot generate frequencies easily above 150 Mhz (its too small). 
So you need to setup the PLL and output multisynch for 4xDesiged frequency.  PLL frequencies must be between 600 to 900 Mhz.  4x150 Mhz is 600 Mhz.  So, we setup the PLL and Multisynch 
to generate 4x the desired fequency and then apply the divide by 4 (MS_DIVBY4) at the end to bring the frequency back to the desired frequency.

multisynth.ClkEnable is a parameter that is used to enable and disable the clock.

Si5351_CLK_def is structure that defines clock control (clk0ctl, clk1ctl, clk2ctl) structure as follows
=======================================================================================================
The clock control structure is used to define parameters for the current running state of the clock.
clk0ctl, clk1ctl and clk2ctl structure are all the same except they are dedicated to the specific clock. 
At any point in time, these structures defines how the clock is configured.

clk0ctl.PLL is the PLL that is assigned to the clock. Either "A" or "B"
clk0ctl.mAdrive is the actual mA for the output i.e. 2, 4, 6, 8.  The UpdateDrive() routine converts to appropriate register value
clk0ctl.phase is the phase from 0 to clk0ctl.maxangle. Each binary value increments the delay for the output frequency by 1 / (4 x clk0ctl.PLLFreq)
clk0ctl.maxangle is the maximum phase based on the PLL Frequency used to generate the output frequency
clk0ctl.reg is the control register (16,17,18) value that was last written to the Si5351
clk0ctl.freq is the output frequency for the running clock
clk0ctl.PLLFreq is the PLL Frequency that is used to generate the output frequency
*/

void ResetSi5351 (unsigned int loadcap)
// This routine zeros out all structures and write 0 to various control Si5351 registers to reset the chip
// Note when switching between > 150 Mhz to < 150 Mhz, a reset is required!
// It also sets the crystal load capacitance and the Crystal frequency.
// For the Adafruit module, its 25 Mhz. There are no other Adafruit modules.
{
  unsigned char i;
  
  // Zero all clk registers and multisynth variables
  memset ((char *)&clk0ctl, 0, sizeof(clk0ctl));
  memset ((char *)&clk1ctl, 0, sizeof(clk1ctl));
  memset ((char *)&clk2ctl, 0, sizeof(clk2ctl));
  memset ((char *)&multisynth, 0, sizeof(multisynth));

  i2cInit();

  // Disable clock outputs
  Si5351WriteRegister (SIREG_3_OUTPUT_ENABLE_CTL, 0xFF);  // Each bit corresponds to a clock outpout.  1 to disable, 0 to enable

  // Power off CLK0, CLK1, CLK2
  Si5351WriteRegister (SIREG_16_CLK0_CTL, 0x80);      // Bit 8 must be set to power down clock, clear to enable. 1 to disable, 0 to enable
  Si5351WriteRegister (SIREG_17_CLK1_CTL, 0x80);
  Si5351WriteRegister (SIREG_18_CLK2_CTL, 0x80);

  // Zero ALL multisynth registers.
  for (i = 0; i < SI_MSREGS; i++) {
    Si5351WriteRegister (SIREG_26_MSNA_1 + i, 0x00);
    Si5351WriteRegister (SIREG_34_MSNB_1 + i, 0x00);
    Si5351WriteRegister (SIREG_42_MSYN0_1 + i, 0x00);
    Si5351WriteRegister (SIREG_50_MSYN1_1 + i, 0x00);
    Si5351WriteRegister (SIREG_58_MSYN2_1 + i, 0x00);
  }

  // Set Crystal Internal Load Capacitance. For Adafruit module its 8 pf
  Si5351WriteRegister (SIREG_183_CRY_LOAD_CAP, loadcap);

  // Define XTAL frequency. For Aadfruit it 25 Mhz.
  multisynth.Fxtal =  SI_CRY_FREQ_25MHZ;
}


void DisableSi5351Clocks ( void )
// This routine turns off CLKs by setting the corresponding bit in the CLK control register
{
  // Power off CLK0, CLK1, CLK2
  Si5351WriteRegister (SIREG_16_CLK0_CTL, 0x80);      // Bit 8 must be set to power down clock, clear to enable. 1 to disable, 0 to enable
  Si5351WriteRegister (SIREG_17_CLK1_CTL, 0x80);
  Si5351WriteRegister (SIREG_18_CLK2_CTL, 0x80);

}


void SetFrequency (unsigned long freq)
// This is the detailed call to configure a frequency.  It requires Clk (0,1,2), PLL (A or B), pllfreq (0 for autodetermine or 600-900 Mhz), phase (0-max angle), mAdrive (2,4,6,8 mA)
// See note above about phase configuration and programming note AN619
// Note that SI_XTAL can be use instead of PLL "A" or "B".  This simply passes crystal frequency to the output (i.e. output is 25 Mhz and multiplier and dividers are not used).
{
  // Validate frequency limits
  if (!freq || freq > SI_MAX_OUT_FREQ || freq < SI_MIN_OUT_FREQ) {
    Serial1.println ("Bad Freq");
    return;
  }
  


  CalculateDividers (freq);
  ProgramSi5351();

}


void ProgramSi5351 (void)
{
  unsigned char Si5351RegBuffer[10];
  
  // Zero all Buffer used for repeated write of all registers
  memset ((char *)&Si5351RegBuffer, 0, sizeof(Si5351RegBuffer));

  base = SIREG_26_MSNA_1;                        // Base register address for PLL A

  //Load the buffer with MSN register data
  Si5351RegBuffer[0] = (multisynth.MSN_P3 & 0x0000FF00) >> 8;
  Si5351RegBuffer[1] = (multisynth.MSN_P3 & 0x000000FF);
  Si5351RegBuffer[2] = (multisynth.MSN_P1 & 0x00030000) >> 16;
  Si5351RegBuffer[3] = (multisynth.MSN_P1 & 0x0000FF00) >> 8;
  Si5351RegBuffer[4] = (multisynth.MSN_P1 & 0x000000FF);
  Si5351RegBuffer[5] = ((multisynth.MSN_P3 & 0x000F0000) >> 12) |
                       ((multisynth.MSN_P2 & 0x000F0000) >> 16);
  Si5351RegBuffer[6] = (multisynth.MSN_P2 & 0x0000FF00) >> 8;
  Si5351RegBuffer[7] = (multisynth.MSN_P2 & 0x000000FF);
  
  // Write the data to the Si5351
  Si5351RepeatedWriteRegister(base, 8, Si5351RegBuffer);
  
//  Si5351WriteRegister( base++, (multisynth.MSN_P3 & 0x0000FF00) >> 8);
//  Si5351WriteRegister( base++, (multisynth.MSN_P3 & 0x000000FF));
//  Si5351WriteRegister( base++, (multisynth.MSN_P1 & 0x00030000) >> 16);
//  Si5351WriteRegister( base++, (multisynth.MSN_P1 & 0x0000FF00) >> 8);
//  Si5351WriteRegister( base++, (multisynth.MSN_P1 & 0x000000FF));
//  Si5351WriteRegister( base++, ((multisynth.MSN_P3 & 0x000F0000) >> 12) |
//                       ((multisynth.MSN_P2 & 0x000F0000) >> 16) );
//  Si5351WriteRegister( base++, (multisynth.MSN_P2 & 0x0000FF00) >> 8);
//  Si5351WriteRegister( base, (multisynth.MSN_P2 & 0x000000FF));

  // Reset PLLA (bit 5 set) & PLLB (bit 7 set)
  Si5351WriteRegister (SIREG_177_PLL_RESET, SI_PLLA_RESET | SI_PLLB_RESET );

  // Set the base register for the Multisynth diveder for the clock
  // clkreg is the actual data that will be written to the clock control register and we need to build it up based on parameters passed to this routine
  // We first restore the last clock control register for the clock being configured.
  // Note that when the ResetSi5351() is called all registered are zeroed
  base = SIREG_42_MSYN0_1;    // Base register address for Out 0
  clkreg = clk0ctl.reg;
  memset ((char *)&Si5351RegBuffer, 0, sizeof(Si5351RegBuffer));

  Si5351RegBuffer[0] = (multisynth.MS_P3 & 0x0000FF00) >> 8;
  Si5351RegBuffer[1] = (multisynth.MS_P3 & 0x000000FF);
  Si5351RegBuffer[2] = ( ((multisynth.MS_P1 & 0x00030000) >> 16) |
                         ((multisynth.R_DIV & 0x7) << 4) |
                         ((multisynth.MS_DIVBY4 & 0x3) << 2)) ;
  Si5351RegBuffer[3] = (multisynth.MS_P1 & 0x0000FF00) >> 8;
  Si5351RegBuffer[4] = (multisynth.MS_P1 & 0x000000FF);
  Si5351RegBuffer[5] = ((multisynth.MS_P3 & 0x000F0000) >> 12) |
                       ((multisynth.MS_P2 & 0x000F0000) >> 16);
  Si5351RegBuffer[6] = (multisynth.MS_P2 & 0x0000FF00) >> 8;
  Si5351RegBuffer[7] = (multisynth.MS_P2 & 0x000000FF);


  
  // Write the values to the corresponding register
  Si5351RepeatedWriteRegister(base, 8, Si5351RegBuffer);
  
//  Si5351WriteRegister( base++, (multisynth.MS_P3 & 0x0000FF00) >> 8);
//  Si5351WriteRegister( base++, (multisynth.MS_P3 & 0x000000FF));
//  Si5351WriteRegister( base++, (
//                         ((multisynth.MS_P1 & 0x00030000) >> 16) |
//                         ((multisynth.R_DIV & 0x7) << 4) |
//                         ((multisynth.MS_DIVBY4 & 0x3) << 2)) );
//  Si5351WriteRegister( base++, (multisynth.MS_P1 & 0x0000FF00) >> 8);
//  Si5351WriteRegister( base++, (multisynth.MS_P1 & 0x000000FF));
//  Si5351WriteRegister( base++, ((multisynth.MS_P3 & 0x000F0000) >> 12) |
//                       ((multisynth.MS_P2 & 0x000F0000) >> 16) );
//  Si5351WriteRegister( base++, (multisynth.MS_P2 & 0x0000FF00) >> 8);
//  Si5351WriteRegister( base, (multisynth.MS_P2 & 0x000000FF));

/*
Reg 16-18: Power up clock, set fractional mode, set PLLA, set MultiSynth 0 as clock source, current output
Reg 165-167: Sets phase

For clock control register, the bits are as follows
  7  Power down (o power up)
  6  MultiSynth 0 Integer Mode (0 Fractional mode, 1 Intefer mode)
  5  MultiSynth Source (0 for PLLA, 1 for PLLB)
  4  Clock Invert
  3  set to 1 for MultiSynth source
  2  set to 1 for MultiSynth source
  1  set to 1 for 8mA source (50R output impedence)
  0  set to 1 for 8mA source (50R output impedence)
*/

  // clkreg is the actual data that will be written to the clock control register and we need to build it up based on parameters 
  clkreg &= SI_CLK_CLR_DRIVE;          // Clear original mA drive setting

  // Define the source for the clock. It can be PLLA, PLLB or XTAL pass through. XTAL passthrough simply take the clock frequency and passes it through
  clkreg &= ~SI_CLK_SRC_PLLB;         // Set to use PLLA i.e. clear using PLLB define

  clkreg |= SI_CLK_MS_INT;            // Set MSx_INT bit for interger mode
  clkreg |= SI_CLK_SRC_MS;            // Set CLK to use MultiSyncth as source

  // The bit values that are written to the register is different from the
  // interger ma numbers.  For example 2ma, a value of 0 is written into bits 0 & 1
  // For 6mA, a value of 4 is written into bits 0 & 1 of clock control register
  // mAdrive is the interger value for drive current (i.e. 2, 4, 6 8 mA)
  // "SI_CLK_2MA" is the actual value that is used to set appropriate bits in the clock control register
  // clkreg is the variable that has the actual data that will be written to the clock control register
  clkreg |= SI_CLK_8MA;
 
  // Define clk setting in corresponding CLK structure. Then update the clock
  // Save the current control register value so that can then change drive and phase 
  // on the fly by simply updating the register with corresponding value
  clk0ctl.PLL = multisynth.PLL;
  clk0ctl.PLLFreq = multisynth.PLL_Fvco;
  clk0ctl.mAdrive = SI_CLK_8MA;
  clk0ctl.freq = multisynth.MS_Fout;
  clk0ctl.reg = clkreg;
 
 
  // Update clk control based on above settings
  UpdateClkControlRegister ();
 
  multisynth.ClkEnable &= ~SI_ENABLE_CLK0;       // Enable clk0, bit must be cleared to enable
  
  // The ResetSi5351() routine disables all output clocks and they need to be enabled.  Below enables the specific clock referenced in this routine
  Si5351WriteRegister (SIREG_3_OUTPUT_ENABLE_CTL, multisynth.ClkEnable);
}


void CalculateDividers (unsigned long freq)
{
  unsigned long numerator, remainder, b, c;
  uint64_t numerator64;

  // Use the higher PLL frequency possible (one less calculation to make)
  multisynth.PLL_Fvco = SI_MAX_PLL_FREQ;

  // define the fequency to be used. This variable is used by other routines.
  multisynth.MS_Fout = freq;
  multisynth.PLL = SI_PLL_A;

/*
Process to program Si5351 in simple mode
1. Use Max PLL freq which is 900 Mhz

2. Identify Multisynth dividers to generate 900 Mhz from Crystal frequency
a) However need to first correct Xtal freq.
b) Then figure out PLL divider
i.e. PLL_Freq/Xtal_freq gives divider to be converted to a+b/c (PLL_a, PLL_b, PLL_c)

3) Once the PLL frequency is known can calculate the clock Multisynth divider 
necessary to convert the PLL frequency to the desired output frequency.
i.e. PLL_Freq/Freq gives divider which must be converted to a+b/c (MS_a, MS_b, MS-c)

Need to convert fractional part of divider into b/c. Can use Farey or other rational 
number approximation.
*/

// Step 1. Calculate PLL Frequency Divider
  // Calculate the corrected/calibrated crystal frequency that will be used to calculate the PLL frequency
  multisynth.Fxtalcorr = multisynth.Fxtal + (long) ((double)(multisynth.correction / 10000000.0) * (double) multisynth.Fxtal);

  // Calculate the multipler (quotient) to get the PLL frequency
  multisynth.PLL_a = multisynth.PLL_Fvco / multisynth.Fxtalcorr;
/*
The basic approach is that x/y will result in a quotent q and remainder r/y (e.g. 3/2 is 1.5 Quotient is 1, remainder is 1/2). 
We need to get the remainder from the division and then figure out the most accurate fraction based on the largest number that can be used (i.e. SI_MAXIMUM_DENOMINATOR).
To do this take the remainder and do a linear extrapolation to make it a   

Because the remainder would be a large number then multiplying, it by another large number (e.g. SI_MAXIMUM_DENOMINATOR) 
may result in a 32bit integer to overflow.  So we need to use a 64bit integer which is not fully supported in Arduino.

So define a 64bit interger and completed the math, then convert back to a 32 bit integer.

*/  
  // Calculate the remainder for the multiplier to get the PLL frequency
  remainder = multisynth.PLL_Fvco % multisynth.Fxtalcorr;
  numerator64 = remainder;
  numerator64 *= SI_MAXIMUM_DENOMINATOR;

  do_div(numerator64, multisynth.Fxtalcorr); 
  
  numerator = (unsigned long)numerator64;
  
//  RationalNumberApproximation(numerator, SI_MAXIMUM_DENOMINATOR, (SI_MAXIMUM_DENOMINATOR - 1), SI_MAXIMUM_DENOMINATOR, &b, &c);   
//  multisynth.PLL_b = b;
//  multisynth.PLL_c = c;
  multisynth.PLL_b = numerator;
  multisynth.PLL_c = SI_MAXIMUM_DENOMINATOR;
  
//  FareyFraction (result, &multisynth.PLL_b, &multisynth.PLL_c);

// Encode Fractional PLL Feedback Multisynth Divider into P1, P2 and P3
  temp = (128 * multisynth.PLL_b) / multisynth.PLL_c;
  multisynth.MSN_P1 = 128 * multisynth.PLL_a + temp - 512;
  multisynth.MSN_P2 = 128 * multisynth.PLL_b - multisynth.PLL_c * temp;
  multisynth.MSN_P3 = multisynth.PLL_c;


// Step 2. Calculate clock divider  
  // The PLL clock frequency is based on an interger multiple of the desired frequency
  // multisynth.PLL_Fvco = multisynth.MS_Fout * multisynth.SimpleDivider
  // so (a+b/c) is a = multisynth.SimpleDivider, b=0, c=1
  // encode A, B and C for multisynth divider into P1, P2 and P3
  // Calculate the multipler to get the maximum PLL frequency
  // The divider below is used to calculate the multisynch dividers (a+b/c) to get the clock frequency from the PLL frequency calculated below

  // Calculate the multipler (quotient) to get the Output frequency
  multisynth.MS_a = multisynth.PLL_Fvco /freq;
  
  // Calculate the remainder for the multiplier to get the Output frequency
  remainder = multisynth.PLL_Fvco % freq;
  numerator64 = remainder;
  numerator64 *= SI_MAXIMUM_DENOMINATOR;

  do_div(numerator64, freq); 
  
  numerator = (unsigned long)numerator64;
  
//  RationalNumberApproximation(numerator, SI_MAXIMUM_DENOMINATOR, (SI_MAXIMUM_DENOMINATOR - 1), SI_MAXIMUM_DENOMINATOR, &b, &c);   
//  multisynth.MS_b = b;
//  multisynth.MS_c = c;
  multisynth.MS_b = numerator;
  multisynth.MS_c = SI_MAXIMUM_DENOMINATOR;

  temp = (128 * multisynth.MS_b) / multisynth.MS_c;
  multisynth.MS_P1 = 128 * multisynth.MS_a + temp - 512;
  multisynth.MS_P2 = 128 * multisynth.MS_b - multisynth.MS_c * temp;
  multisynth.MS_P3 = multisynth.MS_c;

  // Start with the prior ctl register then update the clk register for mA drive
  clkreg = clk0ctl.reg;               // Get the old register value
  clk0ctl.mAdrive = SI_CLK_8MA;       // save the current drive value
  clkreg &= SI_CLK_CLR_DRIVE;         // Clear the old mA drive bits in the register
  clkreg |= SI_CLK_8MA;               // set the bits for the new mA drive in the register
  clk0ctl.reg = clkreg;               // save the new register for future use

}


void InvertClk (unsigned char invert)
// This routine inverts the CLK0_INV bit in the clk control register.  
// When a sqaure wave is inverted, its the same as a 180 deg phase shift.
// This routine does not enable the clock.  Its assumed that its been enabled elsewhere
{

  unsigned char clkreg;

  // Start with the prior ctl register then update the clk register to invert the output
  clkreg = clk0ctl.reg;                          // Retrieve the prior register value
  if (invert) clkreg |= SI_CLK_INVERT;           // Set the invert bit in register to be written 
  else clkreg &= ~SI_CLK_INVERT;                 // clear the invert bit
  clk0ctl.reg = clkreg;                          // save new register for future use

  // Update clk control
  UpdateClkControlRegister ();
}

void UpdatePhase (unsigned int phase)
// This routing changes the phase of the output frequency
// Phase shift is based on period of PLL that generates the output frequency.
// The phase shift register can be 0 to 127 (128 values). Each increment of 1 to the register delays the output by
// 1/4 of the period of the PLL Clock. For example, is PLL is 900 Mhz, each increment of 1 in register delays 
// output by 0.25/900Mhz or 1/900/4 or 0.28 ns.  So max phase delay for 900 Mhz is 128*0.28=35ns.  For a 7 Mhz output,
// this is same as a 88deg phase shift.  i.e. Period of 7Mhz is 142 ns and 35/142*360=88
// Note: In order for the phase to be applied, the PLL must be reset!!
{
  
  // First need to convert the provided phase angle to bit value that will configure the phase control register
  // If an angle if provide that is greater that the max angle supported (see note above) need to indicate and error or correct
  // In this case, just set to the maximum
  if ( phase < clk0ctl.maxangle) {
    // Its below max angle then calculate value that can fit into phase control register (i.e. between 0 to 127) 
    clk0ctl.phase =  (4 * phase * (clk0ctl.PLLFreq / clk0ctl.freq)) / 360;
  } else clk0ctl.phase = clk0ctl.maxangle;

// Update phase registers based on the defined phase value in clk?ctl.phase 
  UpdatePhaseControlRegister ();
  
// Reset PLLA (bit 5 set) & PLLB (bit 7 set)
// If PLL not reset, phase is not applied.
  Si5351WriteRegister (SIREG_177_PLL_RESET, SI_PLLA_RESET | SI_PLLB_RESET );
}



void UpdateClkControlRegister (void)
{
// This routine write the clock control register variable stored in the clock control strucutre to the
// Si5351 clock control register.  The register must be defined elsewhere.
// This routine does not enable the clock.  Its assumed that its been enabled elsewhere
  Si5351WriteRegister (SIREG_16_CLK0_CTL, clk0ctl.reg);

}

void UpdatePhaseControlRegister (void)
// This routine write the phase control data to the Si5351 phase register 
// This routine does not
//   1) enable the clock.  Its assumed that its been enabled elsewhere
//   2) define the phase.  The phase should be calculated elsewhere and stored in the clock contol structure
// This routing only writes the phase defined in the clock control structure to the phase control register
{
  Si5351WriteRegister (SIREG_165_CLK0_PHASE_OFFSET, clk0ctl.phase);

}


/*
 * Calculate best rational approximation for a given fraction
 * taking into account restricted register size, e.g. to find
 * appropriate values for a pll with 5 bit denominator and
 * 8 bit numerator register fields, trying to set up with a
 * frequency ratio of 3.1415, one would say:
 *
 * rational_best_approximation(31415, 10000,
 *              (1 << 8) - 1, (1 << 5) - 1, &n, &d);
 *
 * you may look at given_numerator as a fixed point number,
 * with the fractional part size described in given_denominator.
 *
 * for theoretical background, see:
 * http://en.wikipedia.org/wiki/Continued_fraction
 */

void RationalNumberApproximation
        (
        unsigned long given_numerator, unsigned long given_denominator,
        unsigned long max_numerator, unsigned long max_denominator,
        unsigned long *best_numerator, unsigned long *best_denominator
        )
{
  unsigned long n, d, n0, d0, n1, d1;
  n = given_numerator;
  d = given_denominator;
  n0 = d1 = 0;
  n1 = d0 = 1;
  for (;;) {
    unsigned long t, a;
    if ((n1 > max_numerator) || (d1 > max_denominator)) {
      n1 = n0;
      d1 = d0;
      break;
    }
    if (d == 0)
      break;
    t = d;
    a = n / d;
    d = n % d;
    n = t;
    t = n0 + a * n1;
    n0 = n1;
    n1 = t;
    t = d0 + a * d1;
    d0 = d1;
    d1 = t;
  }
  *best_numerator = n1;
  *best_denominator = d1;
}


/*
Cannot use Wire Library because its Interupt based and need to change fequency from a timer (which is an interupt)
Interupts cannot be layered.  Its one at a time.

For I2C communication must be manually done
*/


void Si5351RepeatedWriteRegister(unsigned char  addr, unsigned char  bytes, unsigned char *data)
{

// I2C communication must be manually done. Wire library is Interupt based and cannot be used!!

//  Wire.beginTransmission(SI5351_ADDRESS);
//  Wire.write(addr);
//  for(int i = 0; i < bytes; i++)
//  {
//    Wire.write(data[i]);
//  }
//  Wire.endTransmission();

  i2cSendRepeatedRegister(addr, bytes, data);
}


void Si5351WriteRegister (unsigned char reg, unsigned char value)
// Routine uses the I2C protcol to write data to the Si5351 register.
{
  unsigned char err;
  
// I2C communication must be manually done. Wire library is Interupt based and cannot be used!!

//  Wire.begin();
//  Wire.beginTransmission(SI5351_ADDRESS);
//  Wire.write(reg);
//  Wire.write(value);
//  Wire.endTransmission();

  err = i2cSendRegister(reg, value);
  if (err) {
    Serial.print ("I2C W Err ");
    Serial.println (err);
  }
}

unsigned char Si5351ReadRegister (unsigned char reg)
// This function uses I2C protocol to read data from Si5351 register. The result read is returned
{
  unsigned char value, err;;

// I2C communication must be manually done. Wire library is Interupt based and cannot be used!!

//  Wire.begin();
//  Wire.beginTransmission(SI5351_ADDRESS);
//  Wire.write(reg);
//  Wire.endTransmission();
//  Wire.requestFrom(SI5351_ADDRESS, 1);
//  return Wire.read();

  err=i2cReadRegister(reg, &value);  
  if (err) {
    Serial.print ("I2C R Err ");
    Serial.println (err);
    value = 0;
  }

  return value;
}



