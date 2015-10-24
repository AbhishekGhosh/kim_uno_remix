/* Interface.c
 *
 * All of the emulator interface routines, but decoupled from the user interface
 * to make this more easily portable to other systems in the future
 */

#include "interface.h"

/* over in cpu.c and memory.c */
void reset6502();
void nmi6502();
void loadProgramsToRam();
void exec6502( int nCycles );


/* serial fakeo */
void serout( uint8_t ch )
{
    ch = ch;
    //printf( "%c", ch );
}

void serouthex( uint8_t ch )
{
    ch = ch;
    //printf( "%01x", ch );
}


/* keyboard interface stuff */
uint8_t curkey = 0;

extern uint8_t serialEnable;
extern uint8_t SSTmode;

uint8_t getAkey()   { return(curkey);  }
void clearkey()     { curkey = 0; }

uint8_t getKIMkey() {
    if (curkey==0)
      return (0xFF);	//0xFF: illegal keycode

    // numeric hex
    if ((curkey>='0') & (curkey <='9'))
      return (curkey-'0');
    if ((curkey>='A') & (curkey <='F'))
      return(curkey-'A'+10);
    if ((curkey>='a') & (curkey <='f'))
      return(curkey-'a'+10);

    // control presses
    if (curkey==1) // ctrlA
      return(0x10); // AD address mode
    if (curkey==4) // ctrlD
      return(0x11); // DA data mode
    if (curkey=='+') // +
      return(0x12); // step
    if (curkey==7) // ctrlG
      return(0x13); // GO
    if (curkey==16) // ctrlP
      return(0x14); // PC mode
    // curkey==ctrlR for hard reset (/RST) (KIM's RS key) is handled in main loop!
    // curkey==ctrlT for ST key (/NMI) is handled in main loop!

    // additional control press shortcuts
    if( curkey == 'g' ) return( 0x07 ); /* GO */
    if( curkey == 'l' ) return( 0x10 ); /* AD - address Location */
    if( curkey == 'v' ) return( 0x11 ); /* DA - data Value */
    if( curkey == 'p' ) return( 0x14 ); /* PC */

    return(curkey); // any other key, should not be hit but ignored by KIM
}

// translate keyboard commands to KIM-I keycodes or emulator actions
void interpretkeys()
{
    // round 1: keys that always have the same meaning
    switch (curkey) {
      case 18:  // CtrlR = RS key = hardware reset (RST)
        reset6502();
        clearkey();
        break;

      case 20: // CtrlT = ST key = throw an NMI to stop execution of user program
        nmi6502();
        clearkey();
        break;

      case '[': // SST off
        SSTmode = 0;
        clearkey();
        break;

      case ']': // SST on
        SSTmode = 1;
        clearkey();
        break;

      case 9: // TAB pressed, toggle between serial port and onboard keyboard/display
        /*
        if (useKeyboardLed==0)
        {
          useKeyboardLed=1;    Serial.print(F("                    Keyboard/Hex Digits Mode "));
        } else {
          useKeyboardLed=0;    Serial.print(F("                        Serial Terminal Mode         "));
        }
        */
        reset6502();  clearkey();  break;

    }
}

uint8_t xkeyPressed()    // just see if there's any keypress waiting
{
    return (curkey==0?0:1);
}


/* display fakeo */
char kimHex[6];        // seLED display

void driveLEDs( void )
{
    /* trigger the LED update */
}
