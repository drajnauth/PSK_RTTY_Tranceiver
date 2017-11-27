/*

Program Written by Dave Rajnauth, VE3OOI to control and service timer interupts 

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"


//////////////////////////////////
// Timer1 ISR - used for encoder and pushbutton polling. It runs at 3ms
//////////////////////////////////
ISR(TIMER1_COMPA_vect)
{
  CheckEncoder();
  CheckPushButtons ();
}


//////////////////////////////////
// Timer3 ISR - used for PSK decode. It runs at 31.25 baud or 32 ms duty cycle
//////////////////////////////////
ISR(TIMER3_COMPA_vect)
{

  // For decode send signal to check signal for phase change.  
  if (flags & DECODEPSK) {
    flags |= CHECKPSKVALUE;
    
  // For Tx, transmit a bit  
  } else if (flags & TRANSMITPSK) {
    if (! (flags & TRANSMIT_CHAR_DONE) ) {            // Check if all bits transmitted
      if (  !(pskVcode & (1 << bitpos++)) ) {         // Check bit value and invert clock for 0 bit.  Clock inversion is same as 180 degree phase shift)
        digitalWrite(TxEnable, LOW);                  // Turn off transmitter, power output decreases. helps reduce harmonics when carrier phase changed.  Phase changed for "0" bit
        InvertClk (pskSwap);                          // Invert carrier 
        digitalWrite(TxEnable, HIGH);                 // Enable transmitter, power ouput increases
        if (!pskSwap) pskSwap = 1;                    // Set phase change variable
        else pskSwap = 0;
      }
      
      // If all bits transmitted, then signal character done
      // pskVcodeLen is increased by 2 to allow 2 "0" bit to be transmitted (stop condition)
      if (bitpos >= pskVcodeLen) {                    
        flags |= TRANSMIT_CHAR_DONE;
      }
     
    }
    
  }

}

//////////////////////////////////
// Timer4 ISR - used for RTTY decode. It run at 45.45 baud or 22 ms
//////////////////////////////////
ISR(TIMER4_COMPA_vect)
{
  // For RTTY Rx, disable realtime acquisition (i.e. ADC free running mode) and sample every 22ms
  if (flags & DECODERTTY) {
    flags &= ~REALTIME;
    StartSampling ();

  // Transmitt RTTY bit
  } else if (flags & TRANSMITRTTY) {
    if (! (flags & TRANSMIT_CHAR_DONE) ) {      // Check if all bits transmitted.    
      if (rttyChar & (1 << bitpos++)) {         // if a "1" bit send MARK frequency and if 0 bit send space bit
        TxRTTYbit(1);                           // Transmit a 1 bit
      } else {    
        TxRTTYbit(0);                           // Transmit a 0 bit
      }

      // Check is all 8 bits sent.
      // Note that start bit and 2 stop bits padded to 5 bit baudot code
      if (bitpos > 7) {     
        flags |= TRANSMIT_CHAR_DONE;
      }
      
    }
  }
}


//////////////////////////////////
// Save Timer Registers - Save all timers registers. Used to save power-on default registeres 
//////////////////////////////////
void SaveTimerRegisters (void)
{
  cli();                              // Disable interupts

  // Store timer registers
  adcsraReset = ADCSRA;
  timsk1Reset = TIMSK1;
  tccr1aReset = TCCR1A;
  timsk3Reset = TIMSK3;
  tccr3aReset = TCCR3A;
  timsk4Reset = TIMSK4;
  tccr4aReset = TCCR4A;

  tcc0areset = TCCR0A;
  tccr0bReset = TCCR0B;
  timsk0Reset = TIMSK0;
  sei();                              // Enable interupts
}

//////////////////////////////////
//  Disable Timer 0
//////////////////////////////////
void DisableTimer0 (void)
{
// Carefull here. Timer 0 used for background Arduino functions such as
// Serial.print, I2C, SPI Delay and other interfaces
  
  cli();
  TCCR0A = 0;
  TCCR0B = 0;
  TIMSK0 = 0;
  sei();
}

//////////////////////////////////
// Enable Timer 1
//////////////////////////////////
void EnableTimer0 (void)
{
  cli();
  TCCR0A = tcc0areset;
  TCCR0B = tccr0bReset;
  TIMSK0 = timsk0Reset;
  sei();
}

//////////////////////////////////
// Restore Saved Timer Registers, Used to restore timers to default
//////////////////////////////////
void RestoreTimerRegisters (void)
{
  cli();
  ADCSRA = adcsraReset;
  TIMSK1 = timsk1Reset;
  TCCR1A = tccr1aReset;
  TIMSK3 = timsk3Reset;
  TCCR3A = tccr3aReset;
  TIMSK4 = timsk4Reset;
  TCCR4A = tccr4aReset;

  TCCR0A = tcc0areset;
  TCCR0B = tccr0bReset;
  TIMSK0 = timsk0Reset;
  sei();
}



//////////////////////////////////
//  Configure and enable a timer.
//////////////////////////////////
void EnableTimers (unsigned char timer, unsigned int count)
{
// Note timers enabled via TCCRnB register, must set a non-zero prescalar to enable
// Arduino uses Timer 0 for serial printing and for delay and other timing functions. Be carefull

  cli();          // disable global interrupts
  switch (timer) {
    case 0:
      break;

    case 1:   // Timer 1 used for input devices (rotary, push buttons, etc)
      TCCR1A = 0;     // reset Timer 1
      TCCR1B = 0;     // TCCRxB turns off timer
      TCNT1 = 0;      // Zero out counter

      // Set match register to 1795  for 113us (or 8850 Hz) with no prescalar for desired interval
      // for Prescalar 1/64, 27 for 113us, 5500 for 22 ms, 3750 for 15ms, 250 for 1ms
      // Set match register to 1000  for 64ms (or 15.6 Hz) with /1024 prescaller
      OCR1A = count;                            // set compare match register for interval
      TCCR1B |= (1 << WGM12);                   // turn on CTC mode
      TCCR1B |= (1 << CS10) | (1 << CS11);      // Set CSx0/CSx1 for /64 prescaler
//      TCCR1B |= (1 << CS10) | (1 << CS12);      // Set CSx0/CSx2 for /1024 prescaler
      TIMSK1 |= (1 << OCIE1A);                  // enable timer compare interrupt:
      break;

    case 2:           // Not available on some Arduinos
      break;

    case 3: // Timer 3.  Used for PSK decode 31.25 baud (32ms)
      TCCR3A = 0;     // reset Timerx
      TCCR3B = 0;     // TCCRxB turns off timer
      TCNT3 = 0;      // Zero out counter

      // Set match register to 500  for 32ms (or 31.25 Hz or Baud) with /1024 prescaller
      OCR3A = count;                            // set compare match register for interval
      TCCR3B |= (1 << WGM32);                   // turn on CTC mode
      TCCR3B |= (1 << CS30) | (1 << CS32);      // Set CSx0/CSx2 for /1024 prescaler
      TIFR3 |= (1 << ICF3) | (1 << OCF3A) | (1 << OCF3B) | (1 << OCF3C);    // Clear Interrupt Flags (write 1)
      TIMSK3 |= (1 << OCIE3A);                  // enable timer compare interrupt:
      break;

    case 4: // Timer 4.  Used for RTTY decode. Runs at 22ms
      TCCR4A = 0;     // reset Timerx
      TCCR4B = 0;     // TCCRxB turns off timer
      TCNT4 = 0;      // Zero out counter

      // Set match register to 344  for 22ms (or 45.45 Hz or Baud) with /1024 prescaller
      OCR4A = count;                            // set compare match register for interval
      TCCR4B |= (1 << WGM42);                   // turn on CTC mode
      TCCR4B |= (1 << CS40) | (1 << CS42);      // Set CSx0/CSx2 for /1024 prescaler
      TIMSK4 |= (1 << OCIE4A);                  // enable timer compare interrupt:
      break;

  }
  sei();          // enable global interrupts

}

//////////////////////////////////
// Disable a timer.
//////////////////////////////////
void DisableTimers (unsigned char timer)
{
// Note timers enabled via TCCRnB register, must set a non-zero prescalar to enable
// Arduino uses Timer 0 for serial printing and for delay and other timing functions

  cli();          // disable global interrupts
  switch (timer) {
    case 0:
      break;

    case 1:
      TCCR1A = 0;     // reset Timer 1
      TCCR1B = 0;     // TCCRxB turns off timer
      TCNT1 = 0;      // Zero out counter
      break;

    case 2:           // Not available on some arduinos
      break;

    case 3:
      TCCR3A = 0;     // reset Timer3
      TCCR3B = 0;     // TCCRxB turns off timer
      TCNT3 = 0;      // Zero out counter
      TIMSK3 = 0;     // Disable timer compare interrupts
      TIFR3 |= (1 << ICF3) | (1 << OCF3A) | (1 << OCF3B) | (1 << OCF3C);    // Reset Interrupt Flags
      break;

    case 4:
      TCCR4A = 0;     // reset Timer4
      TCCR4B = 0;     // TCCRxB turns off timer
      TCNT4 = 0;      // Zero out counter
      break;

  }
  sei();          // enable global interrupts

}



