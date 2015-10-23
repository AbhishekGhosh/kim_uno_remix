/*
 * keys
 *
 *   support for scanning the keypad(s)
 */
 
 
#include "Arduino.h"
#include "config.h"
#include "keys.h"
#include "display.h"

extern "C" {
  
uint8_t curkey = 0;

extern uint8_t serialEnable;
extern uint8_t SSTmode;
extern uint8_t eepromProtect;  // default is to write-protect EEPROM

uint8_t getAkey()   { return(curkey);  }
void clearkey()     { curkey = 0; }


  // getKIMkey() translates ASCII keypresses to codes the KIM ROM expects.
  // note that, inefficiently, the KIM Uno board's key codes are first translated to ASCII, then routed through
  // here just like key presses from the ASCII serial port are. That's inefficient but left like this
  // for hysterical raisins.
  
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
      reset6502(); clearkey(); break;
    
    case 20: // CtrlT = ST key = throw an NMI to stop execution of user program
      nmi6502(); clearkey(); break;
      
    case '[': // SST off
      SSTmode = 0; clearkey();
      std::cout << "SST OFF" << std::endl;
      break;

    case ']': // SST on
      SSTmode = 1; clearkey();
      std::cout << "SST ON" << std::endl;
      break;
      
    case 9: // TAB pressed, toggle between serial port and onboard keyboard/display
      if (useKeyboardLed==0) 
      {
          /* keyboard hex digits */
        useKeyboardLed=1;
      } else {
          /* keyboard terminal */
        useKeyboardLed=0;
      }
      reset6502();  clearkey();  break;
      
    case '>': // Toggle write protect on eeprom
      if (eepromProtect==0) {
        eepromProtect = 1;
        std::cout << "EEPROM RO" << std::endl;
      } else {
        eepromProtect = 0;
        std::cout << "EEPROM RW" << std::endl;
        //delay(20);
      }
      clearkey(); break;
  }
}


uint8_t parseChar(uint8_t n) //  parse keycode to return its ASCII code
{
  uint8_t c = 0;
    
  // KIM-I keys
  switch (n-1) {              //KIM Uno keyscan codes to ASCII codes used by emulator
    case	7	: c = 	'0' ;  break;	//        note: these are n-1 numbers!
    case	6	: c = 	'1';  break;	// 
    case	5	: c = 	'2';  break;	// 
    case	4	: c = 	'3';  break;	//
    case	3	: c = 	'4';  break;	//
    case	2	: c = 	'5';  break;	//
    case	1	: c = 	'6';  break;	//
    case	0	: c = 	20;  break;	// ST
    
    case	15	: c = 	'7' ;  break;	//
    case	14	: c = 	'8';  break;	//
    case	13	: c = 	'9';  break;	//
    case	12	: c = 	'A';  break;	//
    case	11	: c =   'B';  break;	//
    case	10	: c =   'C';  break;	//
    case	9	: c =   'D';  break;	//
    case	8	: c =   18;  break;	// RS
    
    case	23	: c =  'E';  break;     //
    case	22	: c =  'F';  break;     //
    case	21	: c = 	1;   break;     // AD
    case	20	: c = 	4;   break;     // DA
    case	19	: c = 	'+'; break;     // + 
    case	18	: c = 	7;   break;	// GO
    case	17	: c =   16;  break;	// PC
    case	16	: c = (SSTmode==0?']':'[');  break; // 	SST toggle
    default             : c = n; // pass-through
  }
  return c;
}

uint8_t xkeyPressed()    // just see if there's any keypress waiting
{
  return (curkey==0?0:1);
}


/* ******************************* */

/*
   key definitions we can use:
       '0'-'9', 'A'-'F' for digits
        7: GO   20: STEP  18: RESET  't': SST TOGGLE
        1: AD    4: DA    16: PC     '+': PLUS
        '$': shift toggle
        '>': EEPROM WRITE MODE TOGGLE
        
*/

#ifdef kPlatformKIMUno
#define kROWS (3)
#define kCOLS (8)

byte colPins[kCOLS] = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP
byte rowPins[kROWS] = { 9,10,11 };

char lookup[kCOLS][kROWS] =
{
  {  20,  18, 't' },
  { '6', 'D', 16  },
  { '5', 'C',  7  },
  { '4', 'B', '+' },
  { '3', 'A',  4  },
  { '2', '9',  1  },
  { '1', '8', 'F' },
  { '0', '7', 'E' }
};

char lookup_shifted[kCOLS][kROWS] = {};

#endif

#ifdef kPlatformNovus750
// keypad definition
#define kROWS (3)
#define kCOLS (7)
byte rowPins[kROWS] = { 9, 10, 11 }; //connect to the row pinouts of the keypad
byte colPins[kCOLS] = { 2, 3, 4, 5, 6, 7, 8 }; //connect to the column pinouts of the keypad

/* NOVUS 750 keypad is layed out like this:
   (sw)        /
   7   8   9   x
   4   5   6   -
   1   2   3   +
   CE  0   .   =
   
   with the "default" keymap being defined as such:
        { 'c', ' ', '0' },
        { '=', '7', '1' },
        { '-', '8', '2' },
        { '+', '9', '3' },
        { ' ', '.', '4' },
        { '$', ' ', '5' },
        { 'x', ' ', '6' }
        
   So, we'll define our layouts:
       [regular]          [shifted]
     (sw)        $      (sw)        $
     C   D   E   F      GO  ST  RS  SST
     8   9   A   B      AD  DA  PC  +
     4   5   6   7      EE
     0   1   2   3      
*/

char lookup[kCOLS][kROWS] =
{
  { '0', ' ', '1' },
  { '3', 'C', '4' },
  { 'B', 'D', '5' },
  { '7', 'E', '6' },
  { ' ', '2', '8' },
  { '$', ' ', '9' },
  { 'F', ' ', 'A' }
};

char lookup_shifted[kCOLS][kROWS] = 
{
  { 's', ' ', 'o' }, // s = scott, o = oscar
  { ' ',  7 , '>' },
  { '#',  20, ' ' }, // # is the popping version of +
  { '+',  18, ' ' },
  { ' ', ' ',  1  },
  { '$', ' ',  4  },
  { 't', ' ',  16 }
};
#endif

#ifdef kPlatform2411
#define kROWS (4)
#define kCOLS (6)

byte colPins[kCOLS] = { 7,6,5,4,3,2 }; // note col A5 is the extra one linked to DP
byte rowPins[kROWS] = { 10,11,A5,9 };

char lookup[kCOLS][kROWS] =
{
  {   7,  20,  18,  't' },
  {   1,   4,  16,  '+' },
  { 'C', 'D',  'E', 'F' },
  { '8', '9',  'A', 'B' },
  { '4', '5',  '6', '7' },
  { '0', '1',  '2', '3' }
};


        
char lookup_shifted[kCOLS][kROWS] = {};

#endif



#define kNoPress ('Z')


#ifdef kDisplayIsCommonAnode
#define kHL  (HIGH)
#define kLH  (LOW)
#else
#define kHL  (LOW)
#define kLH  (HIGH)
#endif

void initKeypad()
{
  for( int x=0 ; x<kROWS ; x++ )
  {
    pinMode( rowPins[x], OUTPUT );
    digitalWrite( rowPins[x], kHL );
  }
  for( int x=0 ; x<kCOLS ; x++ )
  {
    pinMode( colPins[x], INPUT );
    digitalWrite( colPins[x], kHL );
  }
}


char scanKeypad()
{
  initKeypad();
  for( int r=0 ; r<kROWS ; r++ )
  {
    digitalWrite( rowPins[r], kLH );
    for( int c=0 ; c<kCOLS ; c++ )
    {
      if( digitalRead( colPins[c] ) == kLH )
      {
        if( shiftKey ) {
          return lookup_shifted[c][r];
        } else {
          //Serial.write( lookup[c][r] );
          //Serial.println ("" );
          return lookup[c][r];
        }
      }
    }
    digitalWrite( rowPins[r], kHL );
  }

  return kNoPress;
}

// wrap all of that with an event thingy
#define kEventIdle     (0)
#define kEventPressed  (1)
#define kEventPressing (2)
#define kEventRelease  (3)

char currentKey = kNoPress;
char previousKey = kNoPress;
char keyEvent = kEventIdle;

char scanKeypadEvents()
{
  char ret = 0;
  currentKey = scanKeypad();
  keyEvent = kEventIdle;

  if( currentKey != kNoPress ) {
    keyEvent = kEventPressing; // Pressing still
    ret = currentKey;
  }

  if( currentKey != previousKey ) {
    if( currentKey == kNoPress ) {
      keyEvent = kEventRelease; // key just released
      ret = previousKey;
    } else {
      keyEvent = kEventPressed; // key just pressed
      ret = currentKey;
    }
  }
  previousKey = currentKey;
  return ret;
}

void scanKeys()
{
  static long startPressTime = 0;
  char ke = scanKeypadEvents();
  static char popShift = 0;
  static int holdTicks = 0;
  
  curkey = 0;
  switch( keyEvent ) {
    case( kEventPressed ):
      startPressTime = millis();
      holdTicks = 0;
      switch( ke ) {
        case( '$' ): shiftKey ^= 1; break;
        /*
        case( 's' ): displayText( kDt_Scott, 1000 ); break;
        case( 'o' ): displayText( kDt_Oscar, 1000 ); break;
        */
        
        /* the following "pop" the shift key out */
        case( '#' ):
          ke = '+';  // and adjust it
        case( 1 ): // AD
        case( 4 ): // DA
        case( 18 ): // RS
        case( 't' ): // SST toggle
        case( 16 ):  // PC
        case( '>' ): // EEPROM TOGGLE
          popShift= 1;
          /* fall through */
        default:
          curkey = ke;
      }
      break;

    case( kEventRelease ):
      // we need to do it this way, otherwise it gets confused and re-scans
      // the single press, giving a faulty ghost repress
      if( popShift ) {
        popShift = 0;
        shiftKey = 0;
      }
      startPressTime = 0;
      break;
    
    case( kEventPressing ):
      if( (millis() - startPressTime) > 1000 ) {
        if( holdTicks == 0 ) {
          if( ke == 18 ) {
            curkey = currentKey = ke ='>';
          }
        }
        holdTicks++;
      }
      break;
      
    case( kEventIdle ):
    default:
      break;
  }
  // check for text display
  switch( curkey ) {
    case( 't' ):
      curkey = SSTmode==0?']':'[';
      if( SSTmode == 0 ) displayText( kDt_SST_ON, 500 );
      else               displayText( kDt_SST_OFF, 500 );
      break;
    case( '>' ):
      if( eepromProtect == 0 ) displayText( kDt_EE_RO, 500 );
      else                     displayText( kDt_EE_RW, 500 );
      break;
    default:
      break;
  }      
}


}
