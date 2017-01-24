#ifndef _PBUTTON_H_
#define _PBUTTON_H_


#define MAXCALLSIGN 10

#define MENU_COL1_X 0
#define MENU_COL2_X 120
#define MENU_ROW0_Y 20
#define MENU_ROW1_Y 40
#define MENU_ROW2_Y 60
#define MENU_ROW3_Y 80

#define PBDEBOUNCE 30
#define PB1ENABLED 0x1
#define PB2ENABLED 0x2
#define PB3ENABLED 0x4

#define BUTTON_REUSE 500

#define MAXMENU_ITEMS 8
#define MAXMENU_LEN 11

#define TXMESSAGEID_INSERT_CALLSIGN 127
#define TXMESSAGEID 126

#define HIGHLIGHT_MENU 1
#define UNHIGHLIGHT_MENU 0

#define ROOTMENU 0
#define RXMENU 1
#define TXMENU 2

// Push Button Routines
void CheckPushButtons (void);
unsigned char IsPushed (unsigned char button); 
void DiableButton (unsigned char button);
char PButtonMenu (void);
void ResetPButtonMenu (void);

void LoadCallSign (char loadchar);
void ClearCallSign (void);

void LCDDisplayMenu (unsigned char menulevel);
void DisplayCurrentPButtonMenu (void);
void LCDDisplayMenuLevel ( char menutxt[][MAXMENU_LEN] );
void LCDHighlightMenu (char menutxt[][MAXMENU_LEN], unsigned char mnum, unsigned char highlight);
void LCDClearWaterfallWindow (void);


#endif // _PBUTTON_H_
