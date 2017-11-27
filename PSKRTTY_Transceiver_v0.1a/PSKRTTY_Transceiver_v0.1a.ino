/*

  Program Written by Dave Rajnauth, VE3OOI to decode and transmit RTTY and PSK
  using the Kanga M0XPD 40m Rx/Tx/Si5351 shields.  The program assumes that RTTY and PSK are
  both received on a fixed frequency (i.e. turned frequency + audio offset).

  The program samples ADC 0 and uses correlation to decode FSK and PSK.  External filtering
  is expected to clean up the signals and avoid high frequency aliasing by the.

  The program also provide output to TTL TTY com ports (serial1 and serial2) which can directly be connected
  to a terminal (e.g. Bluetooth terminal or directly connected PC with terminal emulator).  The TTY
  interface is only used for keyboard.

  Alternate input/output is provided by push buttons and LCD for standalone operation.

  The program is broken into several components. Each component faciliates specific functionality and
  must be compiled and linked together.

  Aug 29, 2016
  - RTTY Decode working
  - PSK Decode working
  - Rotary Encoder working

  Aug 31
  - Rotary encoder changes frequency dynamically
  - Push button changes frequency increment
  - Automatic frequency clk enabled on decode
  - Started to add LCD shell functions

  Sept 1
  - FFT working to get Txt Waterfall or show peaks

  Sept 5
  - Display Peak signal level and decoded signal level
  - PSK may need some more work but seems to be working

  Sept 25
  - Changed Si5351 for faster operation
  - RTTY transmitter working

  Sept 27
  - PSK transmitter working

  Oct 24
  - 2.2inch 9340 LCD Working via SPI (very slow)
  - Waterfall working
  - Signal level display working
  - All functions on LCD working. Need to port to 9481 (via 16 bit port)
  - Calibrate Tx offset to match Rx frequency and Frequency displayed (PSK at 1000 Hz, RTTY at 870/1000 Hz)

  Dec 1
  - Added BlueTooth support (need to customize terminal application on android)
  - Added Seria11 and Serial2 input support
  - Added basic support LEDs and Pushbuttons

  Dec 9
  - Simplify Uart routines for automatic entry processing of terminal control codes
  - Remove Seria11 commands outside of terminal mode.
  - Defined push button and LED functions
  - Define menu when in TX mode in Waterfall Window

  Dec 13
  - Pushbutton Menu working -  3 Levels (root, rx and tx menus)
  - Tx fixed messages working
  - Insert callsign working but need to complete Callsign capture

  Dec 19
  - Updated Si5351 Routines to be fast (under 4ms) and accurate (+/- 1Hz). PSK performs better
  - update i2c routines for repeated writes (note can't use Wire library because its interupt based)
  - added INFO message

  Jan 2, 2017
  - combined all includes into one include file
  - split up variables into Allvariables and allexternvariables
  - Added comments to code
  - Added Clip detecton with LED signal
  - Split code stream. Created "PSKRTTY_Transceiver_v0.1" 

  Jan x - Future work
  - ???? Reset ADC when PSK start found ????
  - ???? Try recovering carrier by capturing a static sample 
	process it and autocorrelate with one sample ????
  - ???? complete callsign capture ?????


*/
#include "Arduino.h"

#include "AllIncludes.h"

#include "AllVariables.h"

extern Si5351_def multisynth;
extern Si5351_CLK_def clk0ctl;
extern Si5351_CLK_def clk1ctl;
extern Si5351_CLK_def clk2ctl;


//////////////////////////////////
// The setup function runs once when you press reset or power the board
//////////////////////////////////
void setup() {

  SaveTimerRegisters();

  // Define the baud rate for TTY communications. Note CR and LF must be sent by  terminal program
  // Serial2 (sport 1) is for Bluetooth Interface
  Serial2.begin(115200);

  // Serial1 (sport 0) is for directly connected TTL devices (e.g. PC running terminal emulator)
  Serial1.begin(115200);

  //  initialize the Si5351
  ResetSi5351 (SI_CRY_LOAD_8PF);

  // Read XTAl correction value from Arduino eeprom memory
  EEPROMReadCorrection();

  // Define Tx/Rx Module Functions controlled by pins
  pinMode(SideTone, OUTPUT);
  pinMode(TxEnable, OUTPUT);
  pinMode(RxMute, OUTPUT);

  digitalWrite(RxMute, HIGH);    // Enable receive by default

  // Setup Encoder Pins
  // Setup encoder pins as inputs with pullups
  pinMode(ENC_A, INPUT);            // Rotary A: set at input
  digitalWrite(ENC_A, HIGH);        // Enable weak pullups (i.e. grounded when engaged)
  pinMode(ENC_B, INPUT);            // Rotary A: set at input
  digitalWrite(ENC_B, HIGH);        // Enable weak pullups (i.e. grounded when engaged)
  pinMode(ENC_PB, INPUT);           // Rotary Push button
  digitalWrite(ENC_PB, HIGH);       // Enable weak pullups (i.e. grounded when engaged)

  pinMode (AUDIO_PIN, INPUT);       // Used to send sidetone to received when in Tx. Not used.

  //LED Pins
  pinMode(LED1, OUTPUT);          //Status LEDs are inputs
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);


  // Push Buttons
  pinMode(PBUTTON1, INPUT);       // Push buttons are input
  digitalWrite(PBUTTON1, HIGH);   // Enable weak pullups (i.e. grounded when engaged)
  pinMode(PBUTTON2, INPUT);
  digitalWrite(PBUTTON2, HIGH);
  pinMode(PBUTTON3, INPUT);
  digitalWrite(PBUTTON3, HIGH);

  // Peform software initialization
  ResetFrequencies ();
  Reset();
  TestLEDS();

  // This is used for testing.  Can toggle this pins to measure timing using scope
  // Pin 2 is DDE4, DDE5 is pin 3 and DDG5 is pin 4
  DDRE = (1 << DDE4) | (1 << DDE5);
  DDRG = (1 << DDG5);

}


//////////////////////////////////
// Main program loop
//////////////////////////////////
void loop()
{

  StatusLED();

  ProcessSerialTerminal ();
}


void StatusLED (void)
{
  if (statusLEDctr++ > BLINKCOUNT) {
    statusLEDctr = 0;
    if (digitalRead(OKLED)) {
      digitalWrite(OKLED, LOW);         // LED Off
    } else {
      digitalWrite(OKLED, HIGH);        // LED On
    }
  }

//  if ( !(TermFlags & DISP_WATERFALL) &&  !(TermFlags & DISP_NARROW_WATERFALL) ) {
    if (pskLocked || rttyLocked) {
      digitalWrite (LOCKLED, HIGH);
    } else {
      digitalWrite (LOCKLED, LOW);
    }
//  }

  if ( (flags & TRANSMITRTTY) || (flags & TRANSMITPSK) ) {
    digitalWrite (TXLED, HIGH);
  } else {
    digitalWrite (TXLED, LOW);
  }

  if (flags & CLIPPING) {
    digitalWrite(OKLED, HIGH);          // LED On continiousl to indicate clipping
    digitalWrite (LOCKLED, HIGH);       // LED Onm continiously to indicate clipping
    digitalWrite (TXLED, HIGH);
  }

}



//////////////////////////////////
//
//////////////////////////////////
void Reset (void)
{

  ADCSRA = 0;
  TIMSK1 = 0;
  TCCR1A = 0;
  TCCR1B = 0;

  TIMSK3 = 0;
  TCCR3A = 0;
  TCCR3B = 0;

  TIMSK4 = 0;
  TCCR4A = 0;
  TCCR4B = 0;

  TCCR0A = 0;
  TCCR0B = 0;
  TIMSK0 = 0;

  digitalWrite(TxEnable, LOW);          // Disable transmit
  digitalWrite(RxMute, HIGH);           // Unmute receiver
  noTone(SideTone);                     // turn off SideTone

  memset ((char *)adcbuff, 0, sizeof(adcbuff));
  memset ((char *)corrbuff, 0, sizeof(corrbuff));

  flags = 0;
  errorCode = 0;
  encoderVal = 0xFF;
  encoderState = 0;
  pbstate = 0;
  pbreset = 0;

  pb1ctr = pb2ctr = pb3ctr = pbenable = 0;
  pbreusectr = 0;

  ResetSi5351 (SI_CRY_LOAD_8PF);
  EEPROMReadCorrection();

  RestoreTimerRegisters();

  LCDDisplaySetup();
  ResetPButtonMenu ();

  Serial1.println (HEADER_MESSAGE);   // Banner defined in main.h
  Serial2.println (HEADER_MESSAGE);

  ResetSerialTerminal();
}

void TestLEDS (void)
{
  digitalWrite(LED1, HIGH);         // LED On
  digitalWrite(LED2, HIGH);         // LED On
  digitalWrite(LED3, HIGH);         // LED On
  delay(100);
  digitalWrite(LED1, LOW);          // LED Off
  delay(100);
  digitalWrite(LED2, LOW);          // LED Off
  delay(100);
  digitalWrite(LED3, LOW);          // LED Off
  delay(100);
  digitalWrite(LED1, HIGH);         // LED Off
  delay(100);
  digitalWrite(LED2, HIGH);         // LED Off
  delay(100);
  digitalWrite(LED3, HIGH);         // LED Off
  delay(100);
  digitalWrite(LED1, HIGH);         // LED On
  digitalWrite(LED2, LOW);          // LED Off
  digitalWrite(LED3, LOW);          // LED Ooff

}

//////////////////////////////////
//
//////////////////////////////////
void ResetFrequencies (void)
{
  frequency_clk0 = DEFAULT_FREQUENCY;
  frequency_clk0_tx = DEFAULT_FREQUENCY + TX_FREQUENCY_OFFSET;
  frequency_inc = DEFAULT_FREQUENCY_INCREMENT;
  frequency_mult = DEFAULT_FREQUENCY_MULTIPLIER;
  frequency_mult_old = frequency_mult;
}




//////////////////////////////////
// This routines are NOT part of the Si5351 and should not be included as part of the Si5351 routines.
// Note that some arduino do not have eeprom and would generate an error during compile.
// If you plan to use a Arduino without eeprom then you need to hard code a calibration value.
//////////////////////////////////
void EEPROMWriteCorrection(void)
// write the calibration value to Arduino eeprom
{
  eeprom_write_dword((uint32_t*)0, multisynth.correction);
}

void EEPROMReadCorrection(void)
// read the calibratio value from Arduino eeprom
{
  multisynth.correction = eeprom_read_dword((const uint32_t*)0);
}






