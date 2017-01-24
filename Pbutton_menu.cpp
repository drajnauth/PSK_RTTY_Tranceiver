/*

Routines Written by Dave Rajnauth, VE3OOI to contol pushbuttoms and associated menus

*/

#include "Arduino.h"

#include "AllIncludes.h"

#include "AllExternVariables.h"


void ResetPButtonMenu (void)
{
// Routine to reset LCD menu to root level (i.e. top level menu)
  
  MenuSelection = 0;
  MenuLevel = ROOTMENU;
  LCDDisplayMenuLevel(RootMenuOptions);
  LCDHighlightMenu (RootMenuOptions, MenuSelection, HIGHLIGHT_MENU);
}


void ClearCallSign (void)
{
// Routine to clear captured call sign.  Work in progress....

  // Reset callsign buffer and callsign detection variables and signal 
  memset (callsign, 0, sizeof(callsign));
  callsignsize = loadcharctr = 0;  
  TermFlags &= ~CALLSIGN_LOADED;
  
}

void LoadCallSign (char loadchar)
{
// Routine to capture call sign.  Work in progress....
// Bascially load characters decoded (up to a max) until a space if found
// Once a space is found load set signal that call sign loaded 

 if (TermFlags & CALLSIGN_LOADED) return;       // currently used for debugging
 
 if (loadchar == 0x20) {      // Space detected to set signal and define size of callsign
  callsignsize = loadcharctr;
  loadcharctr = 0;
  TermFlags |= CALLSIGN_LOADED;

 } else {                     // Load the character into the callsign bufffer
  callsign[loadcharctr++] = loadchar;

 }
 
 if (loadcharctr >= MAXCALLSIGN) {      // Size overflow so reset callsign
   ClearCallSign ();
 }
 
}


void LCDDisplayMenu (unsigned char menulevel)
{
// routine to switch to a menu based on the level 
// 0 is for root (top) menu, 1 for level 1, etc  
  
  MenuLevel = menulevel;        // Save the current menu level. Used to return back to the menu level when "return" menu option selected

  // Act of menu level
  // The LCDDisplayMenuLevel actually display the menu
  // 
  switch (menulevel) {
    case ROOTMENU:              // Display top level menu
      MenuSelection = 0;
      LCDDisplayMenuLevel(RootMenuOptions);                               // Display the actual menu
      LCDHighlightMenu (RootMenuOptions, MenuSelection, HIGHLIGHT_MENU);  // Highlight the menu line item in MenuSelection. MenuSelection is set by push button service routine
      break;

    case RXMENU:                // Display Rx menu
      MenuSelection = 0;
      LCDDisplayMenuLevel(RxMenuOptions);
      LCDHighlightMenu (RxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
      break;
        
    case TXMENU:                // Diplay Tx Menu
      MenuSelection = 0;
      LCDDisplayMenuLevel(TxMenuOptions);
      LCDHighlightMenu (TxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
      break;
  } 
    
}

void DisplayCurrentPButtonMenu (void)
{
// Routine to re-display the currently defined menu option
// This is used if the screen is temporarly altered and this routine display the last menu item selected

  switch (MenuLevel) {
    case ROOTMENU:
      LCDDisplayMenuLevel(RootMenuOptions);                               // Display the actual menu      
      LCDHighlightMenu (RootMenuOptions, MenuSelection, HIGHLIGHT_MENU);  // Highlight the menu line item in MenuSelection. MenuSelection is set by push button service routine
      break;

    case RXMENU:
      LCDDisplayMenuLevel(RxMenuOptions);
      LCDHighlightMenu (RxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
      break;
        
    case TXMENU:
      LCDDisplayMenuLevel(TxMenuOptions);
      LCDHighlightMenu (TxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
      break;
  } 
  
}


void LCDDisplayMenuLevel ( char menutxt[][MAXMENU_LEN] )
{
// Routine used to display the actual menu option identified in the menutxt[][] array
// Note that a two dimentional array is passed which contains the menu item in menutxt[m][n].  Index m is the column (i.e. menu line) and index n is the actual test
// i.e menutxt [0] is the text (i.e. null terminated string pointer) for line 0, menutxt[1] is text for next line of menu, etc 
// The menu is displayed in the waterfall window

  // Clear the waterfall window and define colour and size for text
  tft.fillRect(MIN_X, WF_START_Y, MAX_X, WF_WIN_SIZE, ILI9340_RED);
  tft.setTextColor(ILI9340_BLACK); 
  tft.setTextSize(2);

  // Check if text defined (if line is null don't print menu item)
  // The menu is split between two columns each with 4 text lines
  // Display column 1 menu items
  if (strlen(menutxt[0])) {                       // Line 0
    tft.setCursor(MENU_COL1_X, MENU_ROW0_Y);
    tft.print (menutxt[0]);
  }
  
  if (strlen(menutxt[1])) {                       // Line 1
    tft.setCursor(MENU_COL1_X, MENU_ROW1_Y);
    tft.print (menutxt[1]);
  }
  
  if (strlen(menutxt[2])) {                       // Line 2
    tft.setCursor(MENU_COL1_X, MENU_ROW2_Y);
    tft.print (menutxt[2]);
  }
  
  if (strlen(menutxt[3])) {                       // Line 3
    tft.setCursor(MENU_COL1_X, MENU_ROW3_Y);
    tft.print (menutxt[3]);
  }

  // Display these items in column 2
  if (strlen(menutxt[4])) {                       // Line 4
    tft.setCursor(MENU_COL2_X, MENU_ROW0_Y);
    tft.print (menutxt[4]);
  }
  
  if (strlen(menutxt[5])) {                       // Line 5
    tft.setCursor(MENU_COL2_X, MENU_ROW1_Y);
    tft.print (menutxt[5]);
  }
  
  if (strlen(menutxt[6])) {                       // Line 6
    tft.setCursor(MENU_COL2_X, MENU_ROW2_Y);
    tft.print (menutxt[6]);
  }
  
  if (strlen(menutxt[7])) {                       // Line 7
    tft.setCursor(MENU_COL2_X, MENU_ROW3_Y);
    tft.print (menutxt[7]);
  }
  
}

void LCDHighlightMenu (char menutxt[][MAXMENU_LEN], unsigned char mnum, unsigned char highlight)
{ 
// Routine to highlight selected menu item defined by "mnum" variable.  If "highlight" variable set, then highlight othewise remove highlight
// The menu text is passed as a two dimentional array menutxt[][]

  // Set highligt colour based on "highlight" variable
  if (highlight == HIGHLIGHT_MENU) tft.setTextColor(ILI9340_GREEN); 
  else if (highlight == UNHIGHLIGHT_MENU) tft.setTextColor(ILI9340_BLACK);
  
  tft.setTextSize(2);

  // Only need to overwrite the menu line. Assume the menutxt[][] matches the current menu displayed
  switch (mnum) {
    
    // For Column 1
    case 0:                         // Menu line 
      if (strlen(menutxt[0])) {
        tft.setCursor(MENU_COL1_X, MENU_ROW0_Y);
        tft.print (menutxt[0]);     
      }
      break;
      
    case 1:                         // Menu line 1
      if (strlen(menutxt[1])) {
        tft.setCursor(MENU_COL1_X, MENU_ROW1_Y);
        tft.print (menutxt[1]);
      }
      break;
      
    case 2:                         // Menu line 2
      if (strlen(menutxt[2])) {
        tft.setCursor(MENU_COL1_X, MENU_ROW2_Y);
        tft.print (menutxt[2]);
      }
      break;
      
    case 3:                         // Menu line 3
      if (strlen(menutxt[3])) {
        tft.setCursor(MENU_COL1_X, MENU_ROW3_Y);
        tft.print (menutxt[3]);
      }
      break;

    // Items below are for Column 2 
    case 4:                         // Menu line 4
      if (strlen(menutxt[4])) {
        tft.setCursor(MENU_COL2_X, MENU_ROW0_Y);
        tft.print (menutxt[4]);
      }
      break;
      
    case 5:                         // Menu line 5
      if (strlen(menutxt[5])) {
        tft.setCursor(MENU_COL2_X, MENU_ROW1_Y);
        tft.print (menutxt[5]);
      }
      break;
      
    case 6:                         // Menu line 6
      if (strlen(menutxt[6])) {
        tft.setCursor(MENU_COL2_X, MENU_ROW2_Y);
        tft.print (menutxt[6]);
      }
      break;
      
    case 7:                         // Menu line 7
      if (strlen(menutxt[7])) {
        tft.setCursor(MENU_COL2_X, MENU_ROW3_Y);
        tft.print (menutxt[7]);
      }
      break;
      
  }
  
}

char PButtonMenu (void) 
{ 
// This routine is used to process push buttons actions to define menu options
// Once a button is pressed the DisableButton() routine disables the button so it cannot 
// be pushed again. Once the action has been completed, the button will be re-enabled
// Its assumed that pushbutton will be re-enabled elsewhere (e.g. calling routine)

  // First button (RED) pushed
  // Used to diable waterfall and execute a menu item (similar to return key)
  if (IsPushed (PBUTTON1)) {
     DiableButton(PBUTTON1); 

     // This is to disable waterfall display if its active       
     if (TermFlags & DISP_WATERFALL ||  TermFlags & DISP_NARROW_WATERFALL) {
      StopSampling();
      ResetPSK();
      ResetRTTY();
      TermFlags &= ~DISP_WATERFALL;
      TermFlags &= ~DISP_NARROW_WATERFALL;
      DisplayCurrentPButtonMenu ();       // Redisplay current menu because waterfall and menu uses the same part of the LCD screen
      return 0;                           // No need to continue any other processing
    }

    // Execute the menu option highlighted.  
    // The "MenuSelection" variable is the actual highlighed menu line option
    // The xxMenuValues[] array contains control codes which are returned and pushed into the serial buffer and executed (see Uart.cpp)
    // i.e. RxMenuValues[6] return control code associated with option 6 in the Rx Menu
    switch (MenuLevel) {              
      case ROOTMENU:
        return RootMenuValues[MenuSelection];
        break;

      case RXMENU:
        return RxMenuValues[MenuSelection];
        break;
        
      case TXMENU:
        return TxMenuValues[MenuSelection];
        break;
    }
  }  

  // Second push button (middle button)
  // This button switches between waterfall modes or moves the menu selection down
  if (IsPushed (PBUTTON2)) {
    DiableButton(PBUTTON2);

    // Return code to switch waterfall mode 
    if (TermFlags & DISP_WATERFALL) return (CTL_N);
    if (TermFlags & DISP_NARROW_WATERFALL) return (CTL_W);

    // This is processed only is waterfall is not enabled
    // Remove highligh on current selections
    switch (MenuLevel) {
      case ROOTMENU:
        LCDHighlightMenu (RootMenuOptions, MenuSelection, UNHIGHLIGHT_MENU);
        break;

      case RXMENU:
        LCDHighlightMenu (RxMenuOptions, MenuSelection, UNHIGHLIGHT_MENU);
        break;
        
      case TXMENU:
        LCDHighlightMenu (TxMenuOptions, MenuSelection, UNHIGHLIGHT_MENU);
        break;
    } 

    // Decrement menuselection only if its non-zero
    if (MenuSelection) MenuSelection--;
    else MenuSelection = MAXMENU_ITEMS - 1;     // menuselection is zero so wrap around to top menu item

    // Highlight new menu selection
    switch (MenuLevel) {
      case ROOTMENU:
        LCDHighlightMenu (RootMenuOptions, MenuSelection, HIGHLIGHT_MENU);
        break;

      case RXMENU:
        LCDHighlightMenu (RxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
        break;
        
      case TXMENU:
        LCDHighlightMenu (TxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
        break;
    }    
  }

  // Button 3 (bottom buttom) activated
  // This button switches between waterfall modes or moves the menu selection up
  if (IsPushed (PBUTTON3)) {
    DiableButton(PBUTTON3);

    // Return code to switch waterfall mode 
    if (TermFlags & DISP_WATERFALL) return (CTL_N);
    if (TermFlags & DISP_NARROW_WATERFALL) return (CTL_W);

    // This is processed only is waterfall is not enabled
    // Remove highligh on current selections
    switch (MenuLevel) {
      case ROOTMENU:
        LCDHighlightMenu (RootMenuOptions, MenuSelection, UNHIGHLIGHT_MENU);
        break;

      case RXMENU:
        LCDHighlightMenu (RxMenuOptions, MenuSelection, UNHIGHLIGHT_MENU);
        break;
        
      case TXMENU:
        LCDHighlightMenu (TxMenuOptions, MenuSelection, UNHIGHLIGHT_MENU);
        break;
    } 

    // Decrement menuselection only if its non-zero
    MenuSelection++;
    if (MenuSelection >= MAXMENU_ITEMS) MenuSelection = 0;     // menuselection is zero so wrap around to top menu item

    // Highlight new menu selection
    switch (MenuLevel) {
      case ROOTMENU:
        LCDHighlightMenu (RootMenuOptions, MenuSelection, HIGHLIGHT_MENU);
        break;

      case RXMENU:
        LCDHighlightMenu (RxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
        break;
        
      case TXMENU:
        LCDHighlightMenu (TxMenuOptions, MenuSelection, HIGHLIGHT_MENU);
        break;
    }    
  }

  
  return 0;

}


unsigned char IsPushed (unsigned char button) 
{
// Routine to check if a button has been pressed.  If the button is disabled ignore. 
// Buttom must be enabled in order for it to be returned
// pbenable is set by CheckPushButtons ()
  
  if (button == PBUTTON1 && pbenable & PB1ENABLED && flags & PBUTTON1_PUSHED) {
    return PB1ENABLED;
    
  } else if (button == PBUTTON2 && pbenable & PB2ENABLED && flags & PBUTTON2_PUSHED) {
    return PB2ENABLED;
    
  } else if (button == PBUTTON3 && pbenable & PB3ENABLED && flags & PBUTTON3_PUSHED) {
    return PB3ENABLED;
    
  } else {
    return 0;
  }

  Serial1.println ("Bad Button");
  return 0;
}


void DiableButton (unsigned char button) 
{
// Routine to disable a button. i.e clear the enable flag
  
  if (button == PBUTTON1) {
    pbenable &= ~PB1ENABLED;
    
  } else if (button == PBUTTON2) {
    pbenable &= ~PB2ENABLED;
    
  } else if (button == PBUTTON3) {
    pbenable &= ~PB3ENABLED;
  }
}

void CheckPushButtons (void)
{
// This routine is called by Timer 1 (3 ms duty cycle) to detect pushbutton activity and debounce
// Pushbutton pins have weakpullup so default state is high (i.e. 1) and when pushed its grounded and state is 0 (Low)

  // This is used if a button can be push 2 times in a row
  // Not currently used.
  if (flags & PBUTTON_REUSED) {
    if (pbreusectr++ > BUTTON_REUSE) {
      pbreusectr = 0;
      flags &= ~PBUTTON_REUSED;
    }
  }

  // If pushbutton is disabled, then ignore detection and debounce
  if (! (pbenable & PB1ENABLED)) {

    // Check button state
    if (digitalRead(PBUTTON1)) {

      // Debounce button (if pushed) only if button was not previously pushed
      if ( !(flags & PBUTTON1_PUSHED) ) {
        if (pb1ctr >= PBDEBOUNCE) {           // Check if debounce counter exceed debounce threshold
          flags |= PBUTTON1_PUSHED;           // Set push flag
          pbenable |= PB1ENABLED;             // Disable push button. This is reset after button has been processed
        } else {
          pb1ctr++;
        } 
      }   
      
    // Button already pushed so allow a relaxation period (i.e. idle period where button ignored)
    // After relaxation period enable button (i.e. clear pushed flag)
    } else {
      if (pb1ctr) pb1ctr--;               // Decrement relaxation coutter
      else flags &= ~PBUTTON1_PUSHED;     // if counter is zero, then reset push flag
    }
  }

  // Repeate the above for button 2
  if (! (pbenable & PB2ENABLED)) {
    if (digitalRead(PBUTTON2)) {
      if ( !(flags & PBUTTON2_PUSHED) ) {
        if (pb2ctr >= PBDEBOUNCE) {
          flags |= PBUTTON2_PUSHED;
          pbenable |= PB2ENABLED;
        } else {
          pb2ctr++;
        }    
      }   
    } else {
      if (pb2ctr) pb2ctr--;
      else flags &= ~PBUTTON2_PUSHED;
    }
  }

  // Repeate the above for button 3
  if (! (pbenable & PB3ENABLED)) {
    if (digitalRead(PBUTTON3)) {
      if ( !(flags & PBUTTON3_PUSHED) ) {
        if (pb3ctr >= PBDEBOUNCE) {
          flags |= PBUTTON3_PUSHED;
          pbenable |= PB3ENABLED;
        } else {
          pb3ctr++;
        }    
      }   
    } else {
      if (pb3ctr) pb3ctr--;
      else flags &= ~PBUTTON3_PUSHED;
    }
  }
}


