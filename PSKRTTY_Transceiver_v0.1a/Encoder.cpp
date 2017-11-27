/*

Routines Written by Dave Rajnauth, VE3OOI to control perform decoding of a rotary encoder and push buttons 

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"

void CheckEncoder (void)
{
// This routine is used to poll the encoder for rotation or rotary button pushed.
// Rotation with cause frequency to increase or decrease by the current increment.
// Rotary pushbutton will cycle through increment values
  
  // May overrun so don't process if encoderState has not been cleared (i.e. last update processed)
  if (encoderState) return;

  // Check for rotation or push
  frequency_inc = ReadEncoder();        // Returns 0, 1 or -1
  frequency_mult = ReadPBEncoder();

  // Set encoderState (i.e. send signal) if button push for downstream procesing
  if (frequency_mult_old != frequency_mult) {
    encoderState |= 1;
  }
  frequency_mult_old = frequency_mult;

  // Update frequency based on frequency increment (either +1 or -1)
  if (frequency_inc) {
    frequency_clk0 += (frequency_inc * frequency_mult);

    if (frequency_clk0 < LOW_FREQUENCY_LIMIT) frequency_clk0 = LOW_FREQUENCY_LIMIT;
    if (frequency_clk0 > HIGH_FREQUENCY_LIMIT) frequency_clk0 = HIGH_FREQUENCY_LIMIT;

    
    encoderVal = 0xFF;        // Reset for next ReadEncoder()
    encoderState |= 2;        // Set encoderState (i.e. send signal) if rotation detected for downstream procesing
  }
 
}


int ReadEncoder(void)
{
// Routine to Read Encoder Rotation - used increment/decrement frequency
// Based on routine on hihiduino.
// see https://hifiduino.wordpress.com/2010/10/20/rotaryencoder-hw-sw-no-debounce/
// Returns 0 for no rotation, 1 for CW rotation and -1 for CCW rotation

  int state;
  old_AB <<= 2;                             //remember previous state
  old_AB |= (( ENC_PORT & 0x18 ) >> 3);     //add current state
  state = enc_states[( old_AB & 0x0f )];
  if (state > 0) encoderVal = CCW;
  else if (state < 0) encoderVal = CW;
  else encoderVal = CW;
  return ( state );
}

//////////////////////////////////
//////////////////////////////////
unsigned long ReadPBEncoder(void)
{
// Routine to Read Encoder Push Button - used to select frequency increment multiplier each push increase by x10
// The button reads as 0 when pushed (pin gounded) and 1 when not pushed (weak pullup enabled on this pin)
// The button is software debounced for release.  There is hardware debounce for when its pushed (i.e. when pin grounded)
// Frequency increment increase by 10 by each push of the rotary encoder.

  unsigned char button;
  unsigned long mult;
  
  mult = frequency_mult;          // Current multiple

  button = (( ENC_PBPORT & 0x08 ) >> 3);     //Read PIN and shift so its 1 or 0

  // Software debounce button
  // In default state button state is 1, when pushed its grounded so its 0
  // pbstate is used to allow button to be release for a period of time - i.e. allow a relaxation period to expire
  // pbreset is used to reset frequency for a long rotaty button push
  if (!button && !pbstate) {        // Button pushed and relax condition met
    mult = frequency_mult * 10;
    pbstate = 100;                  // Reset counter for relaxation period
    pbreset = 0;                    

  // If button pushed for a long time, reset frequencies back to default
  } else if (!button && pbstate) {            // Button continually pushed 
    if (pbreset++ > PUSH_BUTTON_RESET) {
      ResetFrequencies ();
      pbreset = 0;                            // reset long press counter
    }

  // Relaxation period
  } else if (button) {
    if (pbstate) pbstate--;
  }

  // validate bounds for frequency increment
  if (mult > MAXIMUM_FREQUENCY_MULTIPLIER) {
    mult = MINIMUM_FREQUENCY_MULTIPLIER;
  }
  
  return ( mult );    // return freq increment
}





