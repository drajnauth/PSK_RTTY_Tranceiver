/*

Routines Written by Dave Rajnauth, VE3OOI to control and format the LCD display 

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"

void LCDBlank (void) 
{
// Routine to disable or turn off LCD Screen
  ToggleSampling (0);     // LCD functions are slow and need to disable data sampling.
  tft.begin();
  tft.fillScreen(ILI9340_BLACK);
  ToggleSampling (1);
}

void PrintCursorLocation (void)
{
// Diagnostic routine to display location of cursor.
// Not used for production code.  Compiler will optimize code out
  Serial1.print (tft.getCursorX());
  Serial1.print (" ");
  Serial1.println (tft.getCursorY());
}


void LCDDisplaySetup() 
{
// Routine used to configure and format the LCD screen

  
  ToggleSampling (0);       // LCD functions are slow and need to disable data sampling.

  // This was originally used to signal decode to LCD window or Serial TTY terminal
  // Currently not used. All decode goes to LCD
  flags |= USE_LCD;

  tft.begin();     
  tft.fillScreen(ILI9340_BLACK);

  // Box for title bar
  tft.fillRect(MIN_X, MIN_Y, MAX_X, TITLE_WIN_SIZE, ILI9340_BLUE);
  tft.setTextColor(ILI9340_WHITE); 
  tft.setTextSize(1);
  tft.setCursor(MAX_X/4, 3);
  tft.print((char *)LCD_HEADER_MESSAGE);        // Banner defined in main.h
  
  // Box for waterfall/Spectrum
  tft.fillRect(MIN_X, WF_START_Y, MAX_X, WF_WIN_SIZE, ILI9340_RED);
  
  // Box for Freq Data
  tft.fillRect(MIN_X, DATA_START_Y, MAX_X, DATA_WIN_SIZE, ILI9340_GREEN);
  
  // Box for decode 
  tft.fillRect(MIN_X, DECODE_START_Y, MAX_X, DECODE_WIN_SIZE, ILI9340_BLUE);

  // Populate the LCD with default values
  LCDDisplayFrequency ();
  LCDDisplayFrequencyIncrement ();
  if (flags & DECODERTTY) LCDDisplayMode ((char *)"RTTY Rx");
  else if (flags & DECODEPSK) LCDDisplayMode ((char *)"PSK Rx");
  else if (flags & TRANSMITPSK) LCDDisplayMode ((char *)"PSK Tx");
  else if (flags & TRANSMITRTTY) LCDDisplayMode ((char *)"RTTY Rx");
  else LCDDisplayMode ((char *)"-------");        // Default for no mode defined
  LCDSignalError (0);
  LCDDisplayLevel ();  

  // Set location for initial decode cursor location
  lcdchars = 0;
  tft.setCursor(MIN_X, DECODE_START_Y);
  currentx = tft.getCursorX(); 
  currenty = tft.getCursorY();

  // This counter is used to autoclear the error message on the LCD otherwise you can't tell if a new
  // error code is present
  LCDErrctr = 0;   

  ToggleSampling (1);           
}

void LCDClearWaterfallWindow (void)
{ 
// Routine to clear the waterfall/spectrum window
   
  tft.fillRect(MIN_X, WF_START_Y, MAX_X, WF_WIN_SIZE, ILI9340_RED);
  tft.setTextColor(ILI9340_WHITE); 
  tft.setTextSize(1);
}

void LCDDrawWaterfallWindowMarkers (unsigned char narrow)
{
// Routine to display passband markers on waterfall display
// There are passband markers for the wide display (FFT) and SPACE/MARK frequency markers for narrow display mode
  
  int marker;
  unsigned int x1, x2, y1, y2;
  
  if (narrow) {
    // Draw Window divider between FFT and correlation
    x1 = 120;
    x2 = WF_BIN_WIDTH-2;
    y1 = WF_START_Y;
    y2 = WF_WIN_SIZE;
    tft.fillRect(x1, y1, x2, y2, ILI9340_GREEN);   

    marker = MARK_FREQUENCY_CBIN - (MARK_FREQUENCY_CBIN-2);
    x1 = 120 + marker*PB_BIN_WIDTH + PB_BIN_WIDTH/2 - WF_BIN_WIDTH/2;
    x2 = WF_BIN_WIDTH/2;
    y1 = WF_START_Y;
    y2 = WF_WIN_SIZE;
    tft.fillRect(x1, y1, x2, y2, ILI9340_BLACK);

    marker = SPACE_FREQUENCY_CBIN - (MARK_FREQUENCY_CBIN-2);
    x1 = 120 + marker*PB_BIN_WIDTH + PB_BIN_WIDTH/2 - WF_BIN_WIDTH/2;
    x2 = WF_BIN_WIDTH/2;
    y1 = WF_START_Y;
    y2 = WF_WIN_SIZE;
    tft.fillRect(x1, y1, x2, y2, ILI9340_BLACK);

  // Wideband mode so display the SPACE/MARK passband
  } else {
    marker = MARK_FREQUENCY_BIN - (SPACE_FREQUENCY_BIN-2);
    x1 = marker*PB_BIN_WIDTH + PB_BIN_WIDTH/2 - WF_BIN_WIDTH/2;
    x2 = WF_BIN_WIDTH-3;
    y1 = WF_START_Y;
    y2 = WF_WIN_SIZE;
    tft.fillRect(x1, y1, x2, y2, ILI9340_BLACK);

    marker = SPACE_FREQUENCY_BIN - (SPACE_FREQUENCY_BIN-2);
    x1 = marker*PB_BIN_WIDTH + PB_BIN_WIDTH/2 - WF_BIN_WIDTH/2;
    x2 = WF_BIN_WIDTH-3;
    y1 = WF_START_Y;
    y2 = WF_WIN_SIZE;
    tft.fillRect(x1, y1, x2, y2, ILI9340_BLACK);    
  }

  
}


void LCDDisplayPassbandWaterfall (void)
{
// Display narrow spectum.
// The screen is divided horizontally into 2 parts. The right half show the FFT bins around the Space/Mark frequencies and the
// Left half should the same for the correlation frequency prediction
      
  unsigned char sucess;
  int i;
  volatile long height;
  unsigned int x1, x2, y1, y2;
  
  volatile int fftval, bin, old, peak, peakbin;
  volatile long k1, k2, k3;

  // Initialize values
  height = peakbin = old = bin = peak = 0;
  k1 = k2 = k3 = 0;
  sucess = 0;

  // First display FFT bins 2 bins before Space frequency to 2 bin past Mark frequency
  // Define initial value
  old = fht_log_out[ (SPACE_FREQUENCY_BIN-3)];
  for (i=(SPACE_FREQUENCY_BIN-2); i<=(MARK_FREQUENCY_BIN+2); i++) {
    fftval  = fht_log_out[i];         // FFT output value for bin i

    // Perform peak detection
    // Check for maximum (i.e. left side of peak)
    if (fftval > old && fftval > peak) {
      peak = fftval;
      peakbin = i;
      k1 = old;
      k2 = fftval;
    }

    // Check if this value is less than previous value (i.e. right side of peak)
    if (peakbin == (i-1)) {
      if (fftval < k2) {
        k3 = fftval;
        sucess = 1;
      } else if (k1) {    // Error here, not a real peak so reset peak detection
        peak = peakbin = k1 = k2 = k3 = 0;
        sucess = 0;
      }
    }
    old = fftval;
  }

  // Do peak fitting to identify peak bin as well as extrapolated peak height
  if (sucess && peak && (k1 < k2) && (k3 < k2) && peakbin) {
    //fractional bin = (k3-k1)/2/(k2*2 - k1 - k3), fraction from the max may be +/-
    //est peak = k2 - 0.24(k1-k3)*bin
    // Multiply by 10 to get fractional part accomodated
    bin = (k3 - k1)*5;
    bin /= (k2*2 - k1 - k3);
    peakbin =  peakbin*10 + (int)bin;
    height = k2*40 - (k1-k3)*peakbin;
    height >>= 2;       // need to divide by 40 which is divide by 4 then divide by 10, divide by 4 is same as shift 2 bits
    
    if (height < 0) height = peak;
    else height = fpRound(height, 10);
  } else height = peakbin = 0;

  // Divide by 10 and round variable
  peakbin = fpRound(peakbin, 10);
  corrDly = fpRound(corrDly, 10);

  // Clear entire waterfall display. Prepre to display new data.
  LCDClearWaterfallWindow ();
  
// Display FFT bin number in first half of display
  if (peakbin >= (SPACE_FREQUENCY_BIN-2) && peakbin <= (MARK_FREQUENCY_BIN+2)) {
    peakbin -= (SPACE_FREQUENCY_BIN-2);  // shift bins to start from 0

    // Height is measured top down (i.e. 0,0 is top left corner)
    height = height * MAX_WF_HEIGHT / MAX_WF_VALUE;         
    if (height > MAX_WF_HEIGHT) height = MAX_WF_HEIGHT;
    x1 = peakbin*PB_BIN_WIDTH;
    x2 = PB_BIN_WIDTH-1;
    y1 = WF_END_Y - height + 2;
    y2 = height;
    tft.fillRect(x1, y1, x2, y2, ILI9340_YELLOW);

    // Display passband markers for FFT
    LCDDrawWaterfallWindowMarkers (0);
  }


  // Display correlation bin number in second half of display
  // Same process as above except horizontal shift on LCD
  if (corrDly <= (SPACE_FREQUENCY_CBIN+2) && corrDly >= (MARK_FREQUENCY_CBIN-2)) {
    corrDly -= (MARK_FREQUENCY_CBIN-2);  // shift bins to start from 0

    // Height is measured top down (i.e. 0,0 is top left corner)
    height = height * MAX_WF_HEIGHT / MAX_WF_VALUE;
    if (height > MAX_WF_HEIGHT) height = MAX_WF_HEIGHT;
    x1 = 120 + corrDly*PB_BIN_WIDTH;
    x2 = PB_BIN_WIDTH-1;
    y1 = WF_END_Y - height + 2;
    y2 = height;
    tft.fillRect(x1, y1, x2, y2, ILI9340_BLUE);
    
    // Display passband markers for correlation
    LCDDrawWaterfallWindowMarkers (1);
  }

  
}

long fpRound (long value, int divisor)
{
// Routine complete a division and then round the number after divison
 
  long rounded, temp;

  if (!divisor || !value) return 0;

  // Basciall check remainder to see if its greater than 5
  rounded = value / divisor; 
  temp = value * 10 / divisor;

  if ((temp - rounded*10) > 5) rounded++;

  return rounded;
  
}


void LCDDisplayWaterFall (void)
{
// Routine to display FFT in wideband mode in Waterfall display window
// Take the FFT output and scale

  int i;
  unsigned int height;
  unsigned int x1, x2, y1, y2;

  // Clear waterfall area
  tft.fillRect(MIN_X, WF_START_Y, MAX_X, WF_WIN_SIZE, ILI9340_RED);
  tft.setTextColor(ILI9340_WHITE); 
  tft.setTextSize(1);

  // Display FFT output
  for (i=2; i<FHT_N2; i++) {

    // Height is top down.
    height = fht_log_out[i] * MAX_WF_HEIGHT / MAX_WF_VALUE;
    if (height > MAX_WF_HEIGHT) height = MAX_WF_HEIGHT;
    x1 = i*WF_BIN_WIDTH;
    x2 = WF_BIN_WIDTH-1;
    y1 = WF_END_Y - height + 2;
    y2 = height;
    tft.fillRect(x1, y1, x2, y2, ILI9340_YELLOW);

    // Display passband markers over spectrum for Space and Mark frequencies
    if (i == MARK_FREQUENCY_BIN) {
      x1 = i*WF_BIN_WIDTH+2;
      x2 = WF_BIN_WIDTH-2;
      y1 = WF_START_Y;
      y2 = WF_WIN_SIZE;
      tft.fillRect(x1, y1, x2, y2, ILI9340_BLACK);
    } else if (i == SPACE_FREQUENCY_BIN) {
      x1 = i*WF_BIN_WIDTH+2;
      x2 = WF_BIN_WIDTH-2;
      y1 = WF_START_Y;
      y2 = WF_WIN_SIZE;
      tft.fillRect(x1, y1, x2, y2, ILI9340_BLACK);
    }
  }  

  
}


void LCDDisplayInfo (void)
{  
// Routine to display technical system info on LCD
  
  // Clear Box for decode 
  LCDClearDisplayWindow ();
  tft.setCursor(MIN_X, DECODE_START_Y);
  tft.setTextColor(ILI9340_WHITE);    
  tft.setTextSize(1);

  tft.print (HEADER_MESSAGE);

  tft.print (" PSK Bin Thresh: ");
  tft.print (pskbinthresh);
  tft.print (" Thresh: ");
  tft.println (magThresh);

  tft.print ("RTTY Space Freq: ");
  tft.print (rttySpaceFreq);
  tft.print (" Bin: ");
  tft.println (rttySpaceBin);
  tft.print ("RTTY Mark Freq: ");
  tft.print (rttyMarkFreq);
  tft.print (" Bin: ");
  tft.println (rttyMarkBin);
  tft.print ("RTTY Thresh: ");
  tft.println (magThresh);

  tft.print ("Tx Freq Offset: ");
  tft.print (TX_FREQUENCY_OFFSET);
  
}



void LCDClearDisplayWindow (void)
{
// Routine to clear the decode window and reset cursor to the start of the decode window
  ToggleSampling (0);
  
  tft.fillRect(MIN_X, DECODE_START_Y, MAX_X, DECODE_WIN_SIZE, ILI9340_BLUE);
  tft.setCursor(MIN_X, DECODE_START_Y);
  lcdchars = 0;

  currentx = tft.getCursorX(); 
  currenty = tft.getCursorY();

  ToggleSampling (1);

}



void LCDDisplayCharacter (char value)
{
// Routine to display a single character in the LCD Decode window.

 
  int cx, cy; // Current location of cursor

  // Get current location of cursor
  cx = tft.getCursorX();
  cy = tft.getCursorY();

  // check it this is first time running this routine...if so set cursor to the start of the decode window
  if (!lcdchars) {
    tft.setCursor(MIN_X, DECODE_START_Y);

  // Check if at end of screen and return to start
  // Scrolling is difficult and extremely slow so don't bother scolling
  // Need to investigate if LCD support hardware scrolling
  } else if (lcdchars >= MAX_DECODE_CHARACTERS || (cy == MAX_DECODE_Y && cx >= MAX_DECODE_X) || cy > MAX_DECODE_Y ) {
    ToggleSampling (0);
    tft.fillRect(MIN_X, DECODE_START_Y, MAX_X, DECODE_WIN_SIZE, ILI9340_BLUE);
    tft.setCursor(MIN_X, DECODE_START_Y);
    lcdchars = 0;
    ToggleSampling (1);

  // set cursor to the last location of character printed to continue line 
  } else if (cy < DECODE_START_Y ) {
    tft.setCursor(currentx, currenty);
  }

  // Change colour based on Tx or Rx
  if (flags & TRANSMITRTTY || flags & TRANSMITPSK) {
    tft.setTextColor(ILI9340_CYAN);     
  } else {
    tft.setTextColor(ILI9340_WHITE);     
  }
  tft.setTextSize(2);

  // Print the character
  tft.print ((char) value);

  // Increment the total number of characters printed in window
  lcdchars++;

  // Save the current location for next print
  currentx = tft.getCursorX(); 
  currenty = tft.getCursorY();

}




void LCDDisplayFrequency (void)
{
// Routine to display frequency in the data window

  ToggleSampling (0);

  // Clear current frequency displayed
  tft.fillRect(MIN_X, DATA_START_Y+4, 123, DATA_WIN_SIZE/2, ILI9340_GREEN);

  // Use large numbers
  tft.setTextSize(3);

  // Set colour based on Tx, Rx
  if (flags & TRANSMITRTTY || flags & TRANSMITPSK) {
    tft.setTextColor(ILI9340_RED); 
  } else {
    tft.setTextColor(ILI9340_BLUE); 
  }

  // Print the frequency
  tft.setCursor(MIN_X, DATA_START_Y+4);
  tft.print((unsigned long)frequency_clk0);

  ToggleSampling (1);
}


void LCDDisplayFrequencyIncrement (void)
{
// Routine to display the increment that is used to when rotary turned
  
  ToggleSampling (0);

  // Clear current increment displayed
  tft.fillRect(2, DATA_START_Y+35, 73, DATA_END_Y-DATA_START_Y-35, ILI9340_GREEN);
  tft.setTextSize(2);

  // Print the current increment
  tft.setTextColor(ILI9340_BLACK); 
  tft.setCursor(2, DATA_START_Y+35);
  tft.print((unsigned long)frequency_mult);
  
  ToggleSampling (1);
}


void LCDDisplayMode (char *mode)
{
// Routine to display the Rx/Tx mode

  // Clear the mode currently displayed
  tft.fillRect(130, DATA_START_Y+4, 240-130, 14, ILI9340_GREEN);

  tft.setTextSize(2);
//  tft.setTextColor(ILI9340_MAGENTA); 

  // Set colour based on Tx Rx
  if (flags & TRANSMITRTTY || flags & TRANSMITPSK) {
    tft.setTextColor(ILI9340_RED); 
  } else {
    tft.setTextColor(ILI9340_BLUE); 
  }

  // Display the mode
  tft.setCursor(130, DATA_START_Y+4);
  tft.print((char *)mode);
}


void LCDSignalError (unsigned long errorcode)
{
// Routine to display any errors encoutered

  ToggleSampling (0);
  
  // Clear last error
  tft.fillRect(80, DATA_START_Y+35, 50, DATA_END_Y-DATA_START_Y-35, ILI9340_GREEN);

  // Set the colour and location for error msg
  tft.setTextColor(ILI9340_RED); 
  tft.setTextSize(2);
  tft.setCursor(80, DATA_START_Y+35);

  // Display Txt assocaited with error code
  switch (errorcode) {
    case NULL:
      tft.print((char *)"----");
      break;
      
    case SERIAL_BUFFER_OVERFLOW:
      tft.print((char *)"SOVF");
      break;

    case PSK_BUFFER_OVERFLOW:
      tft.print((char *)"OVF");
      break;

    case PSK_DATA_OVERRUN:
      tft.print((char *)"OVR");
      break;

    case FREQUENCY_BAD_CHANNEL:
      tft.print((char *)"BC");
      break;

    case FREQUENCY_BAD_RANGE:
      tft.print((char *)"BR");
      break;

    case CANNOT_COMPLETE_DECODE_ENABLED:
      tft.print((char *)"CC");
      break;

    case FREQUENCY_NOT_SET:
      tft.print((char *)"FNS");
      break;

  }

  ToggleSampling (1);
}



void LCDDisplayLevel (void)
{
// Routine to display the various signal level
  
  ToggleSampling (0);
  
  // clear all prior signal levels
  tft.fillRect(135, DATA_START_Y+20, MAX_X, DATA_END_Y-DATA_START_Y-20, ILI9340_GREEN);

  tft.setTextSize(2);
  tft.setTextColor(ILI9340_BLACK); 

  // print the signal level
  tft.setCursor(135, DATA_START_Y+20);
  tft.print((long)sLevel);  

  // print correlation level (correlation sum at delay 0)
  tft.setCursor(140, DATA_START_Y+38);
  tft.print((long)corrLevel);  

  // print digital signal level (work in progress...supposed to be indicator of max)
  if (digitalSignalLevel > 0) tft.setTextColor(ILI9340_RED);
  tft.setCursor(185, DATA_START_Y+20);
  tft.print((long)digitalSignalLevel); 

  ToggleSampling (1);

}


void LCDDisplayTest(void) 
{
// Routine to test out the LCD display.

  int i,j;

// ONE ---------------------------------------
  digitalSignalLevel = 999;
  sLevel = 999;
  corrLevel = -99999;
  corrAvg = -99999;
  frequency_mult = 999999;
  
  // Data
  flags |= DECODERTTY;
  LCDDisplayFrequency ();
  LCDDisplayFrequencyIncrement ();
  LCDDisplayMode ((char *)"RTTY TX");
  LCDSignalError (SERIAL_BUFFER_OVERFLOW);
  LCDDisplayLevel ();

  for (j=0; j<10; j++) {
    for (i=0; i<FHT_N2; i++){
      fht_log_out[i] = random (1,100);
    }
    LCDDisplayWaterFall ();
  }
  flags = 0;


  tft.setTextColor(ILI9340_WHITE);     
  tft.setCursor(MIN_X, DECODE_START_Y);
  tft.setTextSize(2);
  maxX = tft.width();
  maxY = tft.height();
  tft.print ("LCD: ");
  tft.print (maxX);
  tft.print (" x ");
  tft.println (maxY);

  // Decoded Characters
  tft.setCursor(MIN_X, DECODE_START_Y+20);
  tft.setTextSize(2);
  lcdchars = 0;

  tft.print ("VE3OOI de XYZ\r\nRPT: 599 599\r\nName: M. Mouse qth CALEDON loc fn03cw\r\nBTU VE3OOI de XYZ\r\n"); 

  lcdchars = 78;
//  for (i=0; i<187; i++) {
//    LCDDisplayCharacter('A');  
//  }

  delay(1000);
    LCDDisplayMenuLevel (RootMenuOptions);
    delay (500);
    for (i=0; i<8; i++) {
      LCDHighlightMenu (RootMenuOptions, i, 1);
      delay (200);
      LCDHighlightMenu (RootMenuOptions, i, 0);
      delay (100);
    }

    LCDDisplayMenuLevel (RxMenuOptions);
    delay (500);
    for (i=0; i<8; i++) {
      LCDHighlightMenu (RxMenuOptions, i, 1);
      delay (200);
      LCDHighlightMenu (RxMenuOptions, i, 0);
      delay (100);
    }

    LCDDisplayMenuLevel (TxMenuOptions);
    delay (500);
    for (i=0; i<8; i++) {
      LCDHighlightMenu (TxMenuOptions, i, 1);
      delay (200);
      LCDHighlightMenu (TxMenuOptions, i, 0);
      delay (100);
    }
    
  delay(700);

    
// TWO ---------------------------------------

  digitalSignalLevel = 50;
  sLevel = 30;
  corrLevel = -200;
  corrAvg = -200;
  frequency_mult = 100;
  frequency_clk0 = 7123456;
  
  flags |= TRANSMITPSK;
  LCDDisplayFrequency ();
  LCDDisplayFrequencyIncrement ();
  LCDDisplayMode ((char *)"PSK TX");
  LCDSignalError (PSK_BUFFER_OVERFLOW);
  LCDDisplayLevel ();

  tft.fillRect(MIN_X, DECODE_START_Y, MAX_X, DECODE_WIN_SIZE, ILI9340_BLUE);
  tft.setCursor(MIN_X, DECODE_START_Y);
  lcdchars = 0;
  
  LCDDisplayMenuLevel(TxMenuOptions);
  for (i=0; i<8; i++) {
    LCDHighlightMenu (TxMenuOptions, i, 1);
    delay (100);
    LCDHighlightMenu (TxMenuOptions, i, 0);
    delay (100);
    LCDDisplayCharacter('C');
    LCDDisplayCharacter('Q');
    LCDDisplayCharacter(' ');
    LCDDisplayCharacter('D');
    LCDDisplayCharacter('E');
    LCDDisplayCharacter(' ');
    LCDDisplayCharacter('V');
    LCDDisplayCharacter('E');
    LCDDisplayCharacter('3');
    LCDDisplayCharacter('O');
    LCDDisplayCharacter('O');
    LCDDisplayCharacter('I');
    LCDDisplayCharacter(' ');
    LCDDisplayCharacter('P');
    LCDDisplayCharacter('S');
    LCDDisplayCharacter('E');
    LCDDisplayCharacter(' ');
    LCDDisplayCharacter('K');
    LCDDisplayCharacter(0xD);
    LCDDisplayCharacter(0xA);
  }
  flags = 0;

}


/*  // The following routines are used for testing
void LCDLoopTest (void)
{
// This routine is used to test the timing of various LCD functions
      rttyTransmitSpaceFreq = frequency_clk0;
      rttyTransmitMarkFreq = frequency_clk0 + RTTY_SHIFT_FREQUENCY;
//      CalculateDividers (rttyTransmitSpaceFreq);
        SetFrequency (rttyTransmitMarkFreq, 0);
      unsigned char swap;
      swap = 1;
      
      while (1) {
//          ProgramSi5351(0);
//          SetFrequency (rttyTransmitMarkFreq, 0);
        delayMicroseconds (31500);
        digitalWrite(TxEnable, LOW);
        InvertClk (swap);
        digitalWrite(TxEnable, LOW);
        if (!swap) swap = 1;
        else swap = 0;
//        CalculateDividers (rttyTransmitSpaceFreq);
        PORTE ^= (1 << DDE5);      //PE5 is D3
      }


}

*/


