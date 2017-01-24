/*

Program Written by Dave Rajnauth, VE3OOI to perform the main processing when in terminal mode. 
This includes accepting inputs from Serial TTY, Bluetooth and Push Buttons and executing associated
processing 

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"


void ResetSerial (void)
// This routine is used to reset serial communications.
// It zero out all serial buffers and counters
{
  memset(rbuff, 0, sizeof(rbuff));
  memset(numbers, 0, sizeof(numbers));
  memset(commands, 0, sizeof(commands));
  ctr = 0;
}


void ResetSerialTerminal (void)
{
// This routine resets terminal processing specific variables
  idle = 0;
  TermFlags = 0;
  endBuff = 0;

  memset(rbuff, 0, sizeof(rbuff));

  StopTransmitter();
  
  ResetRTTY();
  ResetPSK();
  ToggleRTTY ();

  ResetPButtonMenu ();

  Serial2.println ("Terminal Mode");
  DisplayHelp (0);

  Serial1.println ("Terminal Mode"); 
  DisplayHelp (1);

  FlushSerialPorts();

}


void FlushSerialPorts (void)
{
// Function to clear garbage from serial buffers (e.g. on power up there could be garbage in Arduino Serial buffer)
  Serial1.flush();      // Wait for all Tx character to be transmitted
  Serial2.flush();
  
  // Flush serial buffer for noise
  // Don't disable timers and run this. Serial needs timer0
  for (unsigned char i=0; i<20; i++) {
    while (Serial1.available()) Serial1.read();
    while (Serial2.available()) Serial2.read();
    delay (10);
  }
  
}


void DisplayHelp (unsigned char serialport)
{
// This routine display help on the specific serial port
// Currently Serial1 (port 0) is for directly connected TTL devices (e.g. PC running terminal emulator)
// Serial2 (port 1) is for Bluetooth connected devices

  if (serialport) {
    Serial1.println ("\r\n");
    Serial1.println ("^B - Toggle HEX Display");
    Serial1.println ("^C - Clear LCD");
    Serial1.println ("^D - Capture Call Sign");
    Serial1.println ("^E - Clear Error Code");
    Serial1.println ("^F - Stop Tranmitting");
    Serial1.println ("^G - Tx Test Messages");
    Serial1.println ("^I - Root Menu");
    Serial1.println ("^K - Rx Meun");
    Serial1.println ("^L - Tx Menu");
    Serial1.println ("^N - Narrow Waterfall");
    Serial1.println ("^O - LCD Test");
    Serial1.println ("^P - Toggle PSK Tx/Rx");
    Serial1.println ("^Q - System Info");
    Serial1.println ("^R - Toggle RTTY Tx/Rx");
    Serial1.println ("^S - Toggle Signal Levels");
    Serial1.println ("^T - Set Threshold");
    Serial1.println ("^U - Mute Receiver");
    Serial1.println ("^V - Calibrate Si5351");
    Serial1.println ("^W - Enable Waterfall");
    Serial1.println ("^X - Dump ADC Samples");
    Serial1.println ("^Z - Reset");
  } else {
    Serial2.println ("\r\n");
    Serial2.println ("^B - Toggle HEX Display");
    Serial2.println ("^C - Clear LCD");
    Serial2.println ("^D - Capture Call Sign");
    Serial2.println ("^E - Clear Error Code");
    Serial2.println ("^F - Stop Tranmitting");
    Serial2.println ("^G - Tx Test Messages");
    Serial2.println ("^I - Root Menu");
    Serial2.println ("^K - Rx Meun");
    Serial2.println ("^L - Tx Menu");
    Serial2.println ("^N - Narrow Waterfall");
    Serial2.println ("^O - LCD Test");
    Serial2.println ("^P - Toggle PSK Tx/Rx");
    Serial2.println ("^Q - System Info");
    Serial2.println ("^R - Toggle RTTY Tx/Rx");
    Serial2.println ("^S - Toggle Signal Levels");
    Serial2.println ("^T - Set Threshold");
    Serial2.println ("^U - Mute Receiver");
    Serial2.println ("^V - Calibrate Si5351");
    Serial2.println ("^W - Enable Waterfall");
    Serial2.println ("^X - Dump ADC Samples");
    Serial2.println ("^Z - Reset");
  }
  
}


void ProcessSerialTerminal ( void )
// This routing is called to check is there is serial input and store the input into the serial buffer
// If a specific control code (binary 0x01 to 0x1A) is found then process the code otherwise 
// complete process appropriate to the current mode (e.g. receiver, transmit, waterfall spectrum, etc

// When in transmit mode any character entered will be stored in the serial buffer then 
// popped out subsequently for transmission

// When in receive mode, only control codes are processed
{
  char temp;
  unsigned char rbuff_offset;

// Check it data in Serial 1, if so then store it in the buffer
  if (Serial1.available()) {
    temp = Serial1.read();       // Read a character

    // If printable its for Tx
    if (isPrintable (temp) || temp == 0xD || temp == 0xA) {
      rbuff[endBuff++] = temp;  // endbuff designates the location of the last character - used to pop characters 
      Serial1.write (temp);
        
      if (temp == 0xD) {
        rbuff[endBuff++] = 0xA;
        Serial1.write (0xA);
      } 

      // Check for buffer overflow
      if (endBuff >= sizeof(rbuff)) {             // Need >= because need to make sure 2 spots are available for CR/LF
        LCDSignalError(SERIAL_BUFFER_OVERFLOW);
        SignalError(SERIAL_BUFFER_OVERFLOW);
        endBuff = 0;
        rbuff[endBuff + 1] = 0;
      }
      rbuff[endBuff] = 0;


    // Control code so process it
    } else {
      if (TermFlags & DISPHEX) {        // Print out the code if necessary
        Serial1.print ("S1: ");
        Serial1.println (temp, HEX);
      }
      ProcessControlCharacter ( temp );        
    }

  // Repeat above for serial 2.  
  // Need to keep both Serial1 and Serial2 seperate (i.e. processed seperately) because could get nasty things happing
  // if characters received in both 
  } else if (Serial2.available()) {
    temp = Serial2.read();       
    if (isPrintable (temp) || temp == 0xD || temp == 0xA) {
      rbuff[endBuff++] = temp;
      Serial2.write (temp);
      
      if (temp == 0xD) {
        rbuff[endBuff++] = 0xA;
        Serial2.write (0xA);
      } 

      if (endBuff >= sizeof(rbuff)) {             
        LCDSignalError(SERIAL_BUFFER_OVERFLOW);
        SignalError(SERIAL_BUFFER_OVERFLOW);
        FlushSerialPorts();
        ResetSerial ();
        endBuff = 0;
        rbuff[endBuff + 1] = 0;
      }
      rbuff[endBuff] = 0;
        
    } else {
      if (TermFlags & DISPHEX) {
        Serial2.print ("S2: ");
        Serial2.println (temp, HEX);
      }
      ProcessControlCharacter ( temp );        
    }

  // This if for test Tx mode. In this case if the buffer is empty fill it with a test message
  // The text message will be transmitted subsequently
  } else if (TermFlags & TESTMSG) {
    if (!rbuff[0]) {
      memcpy (rbuff, "ABCD EFGH 01234567890\r\n",23); 
      endBuff = 23;
    }
  } 

  // Push buttom menu code below will copy control codes and/or messages to rbuff  which is processed downstream
  if (IsPushed (PBUTTON1) || IsPushed (PBUTTON2) || IsPushed (PBUTTON3)) {
    temp = PButtonMenu ();

    // If there is a canned Tx message to be transmitted (i.e. selected from the LCD Tx menu)
    if (temp == TXMESSAGEID || temp == TXMESSAGEID_INSERT_CALLSIGN) {
      memset (rbuff, 0, sizeof(rbuff));

      // if there is a Calsign defined then insert it into rbuff ahead of any "canned" Tx message 
      if (temp == TXMESSAGEID_INSERT_CALLSIGN) {
        rbuff_offset = callsignsize;
        memcpy (rbuff, callsign, rbuff_offset);
      } else rbuff_offset = 0;


      // Insert canned Tx message 
      switch ( (MenuSelection-1) ) {
        case 0:
          memcpy (rbuff+rbuff_offset, txmsg0, sizeof(txmsg0));
          endBuff = sizeof(txmsg0) + rbuff_offset;
          break;

        case 1:
          memcpy (rbuff+rbuff_offset, txmsg1, sizeof(txmsg1));
          endBuff = sizeof(txmsg1) + rbuff_offset;
          break;
          
        case 2:
          memcpy (rbuff+rbuff_offset, txmsg2, sizeof(txmsg2));
          endBuff = sizeof(txmsg2) + rbuff_offset;
          break;
          
        case 3:
          memcpy (rbuff+rbuff_offset, txmsg3, sizeof(txmsg3));
          endBuff = sizeof(txmsg3) + rbuff_offset;
          break;
          
        case 4:
          memcpy (rbuff+rbuff_offset, txmsg4, sizeof(txmsg4));
          endBuff = sizeof(txmsg4) + rbuff_offset;
          break;
          
        case 5:
          memcpy (rbuff+rbuff_offset, txmsg5, sizeof(txmsg5));
          endBuff = sizeof(txmsg5) + rbuff_offset;
          break;
      }
      temp = 0;
      MenuSelection = 0;
      DisplayCurrentPButtonMenu ();

    // Its not a Tx canned message so its must be a control code that was inserted into rbuff so process it
    } else {
      if (TermFlags & DISPHEX) {
        Serial1.print ("PB: ");
        Serial1.println (temp, HEX);
      }
      ProcessControlCharacter ( temp );
    }
  }

  // Transmitting RTTY. Transmit next character in rbuff. 
  if ( flags & TRANSMITRTTY ) {

    // Character transmission done (flag set by Tx Timer) so send next character or continue to send idle code (for RTTY this is Mark frequency)
    if (flags & TRANSMIT_CHAR_DONE) {
      // Frequency display not updated when rotary switch turned.  Updating frequency data on LCD is slow and will result in syncronization error.
      UpdateFrequencyData (0);           // Update frequency display and DON'T change frequency of OSC.  Tx timer changes Mark/Space frequency based on bit value

      // Transmitter turned on for the first time, so send 2 idle code to synchronize the receiver
      if (idle < 3) {                   // Send idle (i.e. constant mark) for 3 characters
        rttyChar = 0xFF;
        bitpos = 0;
        flags &= ~TRANSMIT_CHAR_DONE;
        idle++;

      // Check for character and transmit it. If no character then send idle code
      } else {

        // Need to switch between Letters and Figures depending of character to be transmitted. rttyLTRSSwitch signal when we need to do a switch
        // This is used to define where a Letters/Figures switch is needed (i.e. transmit switch characters and not transmit character)
        if (!rttyLTRSSwitch) {
          temp = SerialTerminalPop();       // Pop a character from rbuff 
          if (temp) {                       // Echo character on LCD
            LCDDisplayCharacter(temp);              
          }
          rttyChar = setupRTTYChar (temp);  // Need to convert ASCII char to Baudot using Letters/Figures tables. 
          
          if (rttyLTRSSwitch) {             // If a switch is needed (setupRTTYchar signals switch), then setup to transmit it on the next loop
            rttyLTRSSwitch = rttyChar;
            rttyChar = rttyFigures;
          }
          bitpos = 0;                       // Bitpos is used by the Rx Tx Timer to know what bit to transmit in to character to be transmitted
          flags &= ~TRANSMIT_CHAR_DONE;     // Signal Tx time to start sending bits.  After all bits transmitted, timer sets character done flag

        // Transmit Letters/Figures switch character
        } else {
          rttyChar = rttyLTRSSwitch;
          rttyLTRSSwitch = 0;
          bitpos = 0;
          flags &= ~TRANSMIT_CHAR_DONE;
        }
      }        
    }

  // Transmitting PSK. Transmit next character in rbuff.  
  } else if (flags & TRANSMITPSK) {

    // Character transmission done (flag set by Tx Timer) so send next character or continue to send idle code (for PSK this is consecutive phase shifts)
    if (flags & TRANSMIT_CHAR_DONE) {
      UpdateFrequencyData (1);        // Update frequency display and change frequency of OSC. PSK only changes phase of alreay running carrier

      // Transmitter turned on for the first time, so send 10 idle codes to synchronize the receiver (for PSK this is consecutive phase shifts)
      if (!idle) {                    // Send idle (i.e. constant mark) for 3 characters
        pskVcode = 0;
        pskVcodeLen = 10;
        bitpos = 0;
        flags &= ~TRANSMIT_CHAR_DONE;
        idle++;

      // Check for character and transmit it. If no character then send idle code
       } else if (rbuff[0]) {
        temp = SerialTerminalPop();
        if (temp) {
          LCDDisplayCharacter(temp);         
        }
        pskVcode = LookupVaricode(temp);         // Convert ASCI to PSK Varicode
        pskVcodeLen = numbits (pskVcode);        // Varicode is variable length and need to define number of bits to Tx
        pskVcodeLen += PSK_CHAR_GAP_COUNT;       // Add consecutive 0 bits for intercharacter gap (i.e. send predefined consecutive phase shifts for Synchronization)
        bitpos = 0;                              // Start sending first bit. This counter is used by Tx Timer
        flags &= ~TRANSMIT_CHAR_DONE;            // Signal Tx Timer to start sending bits

       // Nothing to be transmitted so send idle code (i.e. at least 2 consecutive phase shifts)
       } else {
        pskVcode = 0;
        pskVcodeLen = 2;
        bitpos = 0;
        flags &= ~TRANSMIT_CHAR_DONE;
       }
    }

  // Not Transmitting so must be receiving so run decode engine.
  } else {
    DecodeLoop();
  }

}

// Function to process a control code (i.e. code below ACII space character) and perform a function/service
void ProcessControlCharacter ( char code )
{
   
    switch (code) {
      case CTL_A:                       // TBD - Undefined
        break;
      
      case CTL_B:                       // Enable Hex Display of Control Characters
        if (TermFlags & DISPHEX) {      // Toggle on/off
          TermFlags &= ~DISPHEX;
        } else {
          TermFlags |= DISPHEX;
        }
        break;

      case CTL_C:                       // Clear LCD Display
        LCDDisplaySetup();
        LCDDisplayFrequencyIncrement ();
        LCDDisplayFrequency ();
        DisplayCurrentPButtonMenu ();
        break;

     // This is still under development....
     case CTL_D:                        // Capture Call sign 
        ClearCallSign ();
        LoadCallSign ('A');             // This is hardcoded for testing. Need to add code to do this....
        LoadCallSign ('B');
        LoadCallSign ('1');
        LoadCallSign ('C');
        LoadCallSign ('D');
        LoadCallSign ('E');
        LoadCallSign (' ');
        break;

      case CTL_E:                             // Clear Error Code
        LCDSignalError (0);
        errorCode = 0;
        flags &= ~DISPLAY_ERROR;
        break;

      case CTL_F:                             // Stop Transmitting
        TermFlags &= ~TESTMSG;
        endBuff = 0;
        ResetSerial ();
        
        // Check transmission mode and disable appropriately
        if (flags & TRANSMITRTTY) {
          ToggleRTTY();
          LCDDisplayMenu(RXMENU);
        }
        if (flags & TRANSMITPSK) {
          TogglePSK();
          LCDDisplayMenu(RXMENU);
        }
        break;

      case CTL_G:                             // Display a continious test message 
        // Check which receive mode and enable as appropriate
        // if not in receive mode, the just enable filling serial buffer with text messages
        if (flags & DECODERTTY) {
          ToggleRTTY();
          LCDDisplayMenu(TXMENU);
        } else if (flags & DECODEPSK) {
          TogglePSK();
          LCDDisplayMenu(TXMENU);
        }
        // Toggle tranmission of test messages
        if (TermFlags & TESTMSG) {          
          TermFlags &= ~TESTMSG;
          endBuff = 0;
          ResetSerial ();
        } else {                              // Enable filling serial buffer with test messages
          TermFlags |= TESTMSG;
          Serial1.println ("Sending...");
        }
        break;

      case CTL_H:                           // Display Help message on terminal
        // Don't allow this if transmitting.
        if (flags & TRANSMITPSK || flags & TRANSMITRTTY) {
          LCDSignalError(CANNOT_COMPLETE_DECODE_ENABLED);
          break;

        // Disable tempoary reception (i.e. will lose a character or two).
        } else if (flags & DECODEPSK || flags & DECODERTTY) {
          ToggleSampling (0);
        }
          
        DisplayHelp (0);                    // Display help on both channels (i.e. Serial1 and Serial2)
        DisplayHelp (1);

        if (flags & DECODEPSK || flags & DECODERTTY) {
          ToggleSampling (1);
        }
        break;

      case CTL_I:                             // Switch to Root Menu
        LCDDisplayMenu(ROOTMENU);
        break;

      case CTL_K:                             // Switch to Rx Menu (this does not enable Rx)
        LCDDisplayMenu(RXMENU);
        break;

      case CTL_L:                             // Switch to Tx Menu (this does not enable Tx)
        LCDDisplayMenu(TXMENU);
        break;
        
      case CTL_N:                             // Display narrow waterfall spectrum (i.e. show Mark/Space bin magnitude)
        ExecuteNarrowWaterfall ();
        break;

      case CTL_O:                             // Display LCD test pattern
        LCDDisplaySetup(); 
        LCDDisplayTest();
        delay(3000);                          // Pause and then reset display back to default
        ResetFrequencies ();
        LCDDisplaySetup();
        LCDDisplayFrequencyIncrement ();
        LCDDisplayFrequency ();
        DisplayCurrentPButtonMenu ();
        break;

      case CTL_P:                             // Toggle PSK mode. If in Rx mode switch to Tx and if in Tx switch back to Rx
        TogglePSK ();
        break;

      case CTL_Q:                             // Display technical info on LCD as well on serial ports
        LCDDisplayInfo();
        DisplayInfo(0);
        DisplayInfo(1);
        break;

      case CTL_R:                             // Toggle RTTY mode. If in Rx mode switch to Tx and if in Tx switch back to Rx
        ToggleRTTY ();
        break;

      // With a 16 Mhz arduino, there is not enough horsepower to decode and display live signal level
      // This mode disables, reception and display signal level live.
      case CTL_S:                             // Toggole continious signal level display
        if (flags & TRANSMITPSK || flags & TRANSMITRTTY) {
          LCDSignalError(CANNOT_COMPLETE_DECODE_ENABLED);
          break;
        }
        if (flags & DISPLAY_SIGNAL_LEVEL) {
          flags &= ~DISPLAY_SIGNAL_LEVEL;
        } else {
          flags |= DISPLAY_SIGNAL_LEVEL;
        }
        break;

      // This mode will update signal level display once and calculate PSK and RTTY thresholds for decode
      case CTL_T:                             // Update slevels and thresholds
        flags |= MEASURETHRESHOLD;
        break;

      // This is for future. Currently the audio is diverted in the received and does not go through 
      // the audio amp for the receiver shield.  
      case CTL_U:                             // Mute/Unmute Receiver. 
        if (TermFlags & MUTERX) {
          TermFlags &= ~MUTERX;
          Serial1.println ("UnMute Rx");
          Serial2.println ("UnMute Rx");
          digitalWrite(RxMute, HIGH);      // Unmute receiver
        } else {
          TermFlags |= MUTERX;
          Serial1.println ("Mute Rx");
          Serial2.println ("Mute Rx");
          digitalWrite(RxMute, LOW);      // Mute receiver
        }
        break;

      // This executes the calibrate function.  It bascially parses characters and numbers
      // and updates the Si5351 calibration values stored in EEPROM
      case CTL_V:                             // Calibrate Si5351
        CalibrateSi5351 ();
        break;

      // I call this waterfall but its actually a spectrum display. A DFT is done and the 
      // result is displayed in the waterfall window on the LCD
      // 16 Mhz 8 bit Arduino does not have the horsepower to do a live spectum or waterfall 
      case CTL_W:                         // Display spectrum (waterfall replacement)
        ExecuteWaterfall ();
        break;
        
      case CTL_X:                         // Toggle Dumping ADC values to console
        if (flags & ADCMONITOR) {
          flags &= ~ADCMONITOR;
          ToggleRTTY ();                  // Reset back to RTTY Rx
        } else {                          
          // Disable Tx if enables
          if (flags & TRANSMITPSK || flags & TRANSMITRTTY) {  // Diable Tx
            StopTransmitter();
          } 
          StopSampling();
          ResetPSK();
          ResetRTTY();
          // Disable waterfall if enabled
          if (TermFlags & DISP_WATERFALL || TermFlags & DISP_NARROW_WATERFALL) {
            TermFlags &= ~DISP_WATERFALL;
            TermFlags &= ~DISP_NARROW_WATERFALL;
          }
          flags |= ADCMONITOR;
          StartSampling();
        }
        break;

      case CTL_Y:                         // Test LCD menu display
        LCDDisplayMenuLevel(RootMenuOptions);
        delay (500);
        for (int i=0; i<8; i++) {
          LCDHighlightMenu (RootMenuOptions, i, 1);
          delay (100);
          LCDHighlightMenu (RootMenuOptions, i, 0);
          delay (100);
        }
        break;

      case CTL_Z:                       // Reset System
        StopSampling();
        StopTransmitter();
        DisableTimers (4);              // Timer 4 is for 22ms for RTTY
        DisableTimers (3);              // Timer 3 is for 32ms for PSK
        Reset();
        break;

    }

}

void DisplayInfo (unsigned char serialport)
{
// This routing display various technical info about the mode
  
  if (serialport) {  
    Serial1.print ("RTTY: ");
    Serial1.println (frequency_clk0);     // Currently tuned frequency
    Serial1.print ("Space Freq: ");
    Serial1.print (rttySpaceFreq);        // Frequency used to detect space
    Serial1.print (" Bin: ");
    Serial1.println (rttySpaceBin);       // Correlation delay value for space detection
    Serial1.print ("Mark Freq: ");
    Serial1.print (rttyMarkFreq);         // Frequency to detect mark
    Serial1.print (" Bin: ");
    Serial1.println (rttyMarkBin);        // Correlation delay value for mark detection
    Serial1.print ("Thresh: ");
    Serial1.println (magThresh);          // Decode threshold. Mark/Space bin must be at least this value

    Serial1.print ("\r\nPSK: ");
    Serial1.print (frequency_clk0);       // Currently tuned frequency
    Serial1.print (" Bin Thresh: ");
    Serial1.print (pskbinthresh);         // When signals in phase this is the delay, used to detect phase shift
    Serial1.print (" Thresh: ");
    Serial1.println (magThresh);          // Delay 0 threshold.  If below this then its a phase shift
    
  } else {
    Serial2.print ("RTTY: ");             // See comments above
    Serial2.println (frequency_clk0);
    Serial2.print ("Space Freq: ");
    Serial2.print (rttySpaceFreq);
    Serial2.print (" Bin: ");
    Serial2.println (rttySpaceBin);
    Serial2.print ("Mark Freq: ");
    Serial2.print (rttyMarkFreq);
    Serial2.print (" Bin: ");
    Serial2.println (rttyMarkBin);
    Serial2.print ("Thresh: ");
    Serial2.println (magThresh);
    
    Serial2.print ("\r\nPSK: ");
    Serial2.print (frequency_clk0);
    Serial2.print (" Bin Thresh: ");
    Serial2.print (pskbinthresh);
    Serial2.print (" Thresh: ");
    Serial2.println (magThresh);
  
  }  
}

void ExecuteNarrowWaterfall (void)
{
// This routine enable narrow spectrum display

// First reset everthing.
    StopSampling();
    DisableTimers (1);              // Timer 1 is for 3ms for Rotary
    ResetPSK();
    ResetRTTY();

// Tune frequency and enable flags for narrow display          
    SetFrequency (frequency_clk0);
    EnableTimers (1, TIMER3MS);     // Timer 1 is for Rotary 
    flags |= DOFHT;
    flags |= NARROW_WATERFALL;
    TermFlags |= DISP_NARROW_WATERFALL;      
    TermFlags &= ~DISP_WATERFALL;   // Disable wide (normal) spectrum   
    setupFFT();                     
    StartSampling(); 
 
}

void ExecuteWaterfall (void)
{
// This routine enables the wide (normal) spectrum display

// First reset everthing
    StopSampling();
    DisableTimers (1);                  // Timer 1 is for Rotary
    ResetPSK();
    ResetRTTY();

// Enable various flags for spectrum display
    SetFrequency (frequency_clk0);
    EnableTimers (1, TIMER3MS);         // Timer 1 is for Rotary 
    flags |= DOFHT;
    TermFlags |= DISP_WATERFALL;      
    TermFlags &= ~DISP_NARROW_WATERFALL;   // Disable narrow display   
    flags &= ~NARROW_WATERFALL;
    setupFFT();
    StartSampling();
}

void ToggleRTTY (void)
{
// This routine switches between Rx and Tx for RTTY

// Stop all timers otherwise bad things may happen 
    StopSampling();
    DisableTimers (4);              // Timer 4 is for 22ms for RTTY
    DisableTimers (3);              // Timer 3 is for 32ms for PSK
    DisableTimers (1);              // Timer 1 is for 5ms for Rotary
    FlushSerialPorts();             // This needs timer 0 running!

    if ( !(flags & DECODERTTY) ) {        // Currently in Tx and need to switch to Rx
      if (flags & TRANSMITPSK || flags & TRANSMITRTTY) {    // Start decode on a seperate line
        LCDDisplayCharacter(0xD);
        LCDDisplayCharacter(0xA);
      }
      StopTransmitter();                  // Disable Tx and reset
      ResetRTTY();
      ResetPSK();
      LCDDisplayMode ((char *)"RTTY Rx");       // Update mode on LCD
      LCDDisplayCharacter ('R');
      LCDDisplayCharacter ('x');
      LCDDisplayCharacter(0xD);
      LCDDisplayCharacter(0xA);
      LCDDisplayFrequency ();
      LCDDisplayMenu(RXMENU);                   // Display Rx menu
      SetFrequency (frequency_clk0);
      RTTYControl (0);                          // Turn on sampling and RTTY Rx
      
    } else if (flags & DECODERTTY) {            // Current in Rx mode and switch to Tx mode
      ResetRTTY();
      ResetPSK();
      SetFrequency (rttyTransmitMarkFreq);      // Enable to carrier to be mark frequency for idle condition
                                                // Subsequent code will manipulate carrier
      flags |= TRANSMITRTTY;                    // This is all that's needed to enable Tx
      flags |= TRANSMIT_CHAR_DONE;
      idle = 0;
      LCDDisplayMode ((char *)"RTTY Tx");       // Update mode on LCD
      LCDDisplayCharacter ('T');
      LCDDisplayCharacter ('x');
      LCDDisplayCharacter(0xD);
      LCDDisplayCharacter(0xA);          
      LCDDisplayFrequency ();
      
      LCDDisplayMenu(TXMENU);                   // Display Tx Menu

//      digitalWrite(RxMute, LOW);          // Mute receiver.  Not needed
      EnableTimers (1, TIMER3MS);         // Timer 1 is for Rotary
      EnableTimers (4, TIMER22MS);        // Timer 4 is for 22ms for RTTY
    }
  
}

void TogglePSK (void)        
{
// This routine switches between Rx and Tx for PSK

// Stop all timers otherwise bad things may happen 
    StopSampling();
    DisableTimers (4);              // Timer 4 is for 22ms for RTTY
    DisableTimers (3);              // Timer 3 is for 32ms for PSK
    DisableTimers (1);              // Timer 1 is for 5ms for Rotary
    FlushSerialPorts();
        
    if ( !(flags & DECODEPSK) ) {   // Currently in Tx so switch to Rx
      if (flags & TRANSMITPSK || flags & TRANSMITRTTY) {  // Start decode on a new line
        LCDDisplayCharacter(0xD);
        LCDDisplayCharacter(0xA);
      }
      StopTransmitter();            // Disable Tx and reset
      ResetRTTY();
      ResetPSK();
      LCDDisplayMode ((char *)"PSK Rx");      // Update LCD mode
      LCDDisplayCharacter ('R');
      LCDDisplayCharacter ('x');
      LCDDisplayCharacter(0xD);
      LCDDisplayCharacter(0xA);
      SetFrequency (frequency_clk0);      
      LCDDisplayFrequency ();
      PSKControl (0);
      
    } else if (flags & DECODEPSK)  {
      ResetRTTY();
      ResetPSK();              
      SetFrequency (frequency_clk0_tx);   // Turn on carrier with offset for Tx so that its received at 1Khz
                                          // Subsequent code will manipulate carrier
      flags |= TRANSMIT_CHAR_DONE;
      flags |= TRANSMITPSK;               // This is all that's needed to enable Tx
      idle = 0;
      pskSwap = 1;
      LCDDisplayMode ((char *)"PSK TX");  // Update mode on LCD
      LCDDisplayCharacter ('T');
      LCDDisplayCharacter ('x');
      LCDDisplayCharacter(0xD);
      LCDDisplayCharacter(0xA);         
      LCDDisplayFrequency ();
      LCDDisplayMenu(TXMENU);             // Display Tx Menue

 //      digitalWrite(RxMute, LOW);          // Mute receiver. Not needed
       
      EnableTimers (1, TIMER3MS);         // Timer 1 is for Rotary 
      EnableTimers (3, TIMER32MS);        // Timer 3 is for 32ms for PSK
    }
}




char SerialTerminalPop (void)
{
// This routine removes a character from the serial buffer and shift the buffer so that
// additional chacters can be added.

  char cpopped;        // The popped chacter
  unsigned char i;     // general counter

  if (rbuff[0]) {      // if nothing in buffer not need to do any process, return null
    cpopped = rbuff[0];
  } else {
    return 0;
  }

  // Shift the buffer to the left by 1 byte
  for (i = 1; i < endBuff; i++) {
    rbuff[i - 1] = rbuff[i];
  }
  rbuff[endBuff--] = 0;     // Make sure the last two characters are null (null terminated strings)
  rbuff[endBuff] = 0;

  return cpopped;
}


void StopTransmitter (void)
{
// This routine disables transmitter

  flags &= ~TRANSMITRTTY;
  flags &= ~TRANSMITPSK;
  flags &= ~TRANSMIT_CHAR_DONE;
  digitalWrite(TxEnable, LOW);      // Disable Tranmitter
//  digitalWrite(RxMute, HIGH);       // Unute receiver. Not used
  DisableSi5351Clocks();            // This is rather harsh but it may save finals if TxEnable is not low.
                                    //Need to do a SetFrequency() after this is called

}

unsigned char ParseSerial ( char *str )
// This routine is used to parse all character and numbers in the serial buffer (str pointer)
// Characters are entered into the ccmmands[] array and numbers are entered into the numbers[] array.
// Each element of commands[] represents a atomic command character and each element of numbers[]
// represents a seperate atomic number within the serial buffer
{
  static unsigned char i, j, k;        // Misc counters used, i is for serial buffer, j is for commands[], k is for numbers[]

  memset(numbers, 0, sizeof(numbers)); // Flush out the arrays
  memset(commands, 0, sizeof(commands));

  for (i = 0, j = 0, k = 0; i < strlen(str); i++) { // Step through the serial buffer - str is a pointer to the serial buffer
    if ( isalpha( str[i] ) ) {               // Its alphabetic so store it in commands[]
      commands[j++] = toupper(str[i]);
      
    } else if ( isdigit( str[i] ) ) {       // its numeris so store in numbers[]
      numbers[k++] = strtol ( (char *)&str[i], NULL, 10 );
      while ( isdigit( str[i + 1] ) ) {
        if (i >= strlen(str)) break;
        i++;
      }
    } else if ( isgraph( str[i] ) ) {
      commands[j++] = str[i];
    }
  }
  return j;

}

void CalibrateSi5351 (void)
{
// This routine is used to calibrate the Si5351.  It this is not done the actual frequency
// tunes may be off by several hundred Hz

// Bacially, the you enter a calibration factor and the freqency that should be output.  
// You would measure the frequency output from the Si5351 (not the transmitter) and adjust calibration 
// You will need an accurate frequency counter and zero beat agains WWV

// The Calibration value is a signed number. Numbers always entered as positive numbers.  If 'N' is found 
// then the number is considered negative

// Only works on serial1.  Does not use serial2 (bluetooth)

  unsigned long freq;       // Frequency entered in serial buffer
  int cal;                  // calibration value enters in serial buffer

  // Display current calibration value
  EEPROMReadCorrection();
  Serial1.print ("Current Correction: ");
  Serial1.println (multisynth.correction);
  
  // Show usage information
  Serial1.println ("At the prompt below enter frequency and calibration include N if calibration is negative");
  Serial1.println ("Examples:"); 
  Serial1.println ("\tFor cal of 150 enter: 7150000 150");
  Serial1.println ("\tFor cal of -75 enter: 7100000 N 75");
  Serial1.println ("Enter Calibration Frequency and new correction: ");

  FlushSerialPorts ();  // Ensure all serial arduino maintained tx and rx buffers are clean
  ResetSerial();        // Ensure that we are starting with clean buffer and counters

  // Retrieve information from serial1.  this is not available for serial2 (bluetooth)
  if (ProcessSerial()) {
    Serial1.println ("Exiting");
    return;
  }

  // Validate inputs
  if (numbers[0] < LOW_FREQUENCY_LIMIT || numbers[0] > HIGH_FREQUENCY_LIMIT) {
    Serial1.println ("Freq out of band");
    return;
  }
  if (numbers[1] > 500) {                   // Arbituarly choose 500 as limit. May need adjustment
    Serial1.println ("Bad Calibration");
    return;
  }
  // Update number to be positive or negative
  if (commands[0] == 'N') cal = - (int)numbers[1];
  else cal = (int)numbers[1];
  
  freq = numbers[0];

  // Display action to be taken to verify
  Serial1.print ("Output Freq: ");
  Serial1.println (freq);
  Serial1.print ("Old Calibration: ");
  Serial1.print (multisynth.correction);
  Serial1.print (" New Calibration: ");
  Serial1.println (cal);

  // Store the new value entered, reset the Si5351 and then enable frequency based on new setting     
  multisynth.correction = cal;
  EEPROMWriteCorrection();
  
  ResetSi5351 (SI_CRY_LOAD_8PF);
  
  EEPROMReadCorrection();
  SetFrequency (freq);
}

unsigned char ProcessSerial ( void ) 
// This routing is called to check is there is serial input and store the input into the serial buffer
// if a CR/LF (Enter pressed) is received, then process the command and flush the buffer.
// If CTL_Z is received then abort and return abort

{
    char temp;

    // Serial.available() returns the number of character that have been entered at the keyboard.
    // The idea here is that you keep processing characters until none are left.
    // This routine is faster that a user's typing and it needs to check for CR/LF 
    while (temp != CTL_Z) {
        if (Serial1.available() && ctr < sizeof(rbuff)) {
            temp = Serial1.read();       // Read a character
            Serial1.write(temp);         // Echo the character back to the user
            if (isPrintable (temp)) {    // If the character is alphanumeric than store it in the buffer
              rbuff[ctr++] = temp;
            } else if (temp == 0xD || temp == 0xA) {    // If the character is not printable and its a CR/LF then process the buffer
              if (ctr) {
                ParseSerial(rbuff);
                return 0;
              }
            } else if (temp == CTL_Z) {
              ResetSerial ();
            }
        }
        // This checks to see if the users has entered too much data which would overflew the serial buffer
        if (ctr >= sizeof(rbuff)) {           // >= means its one less that the actual size. This is intentional
            Serial1.println ("Overflow");
            ResetSerial ();
            return 1;
        } 
    }
    return 1;
}




