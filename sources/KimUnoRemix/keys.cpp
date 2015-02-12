/*
 * keys
 *
 *   support for scanning the keypad(s)
 */
 
 
#include "Arduino.h"
#include "config.h"
#include "display.h" 

extern "C" {

extern uint8_t serialEnable;

byte aCols[8] = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP

byte aRows[3] = { 9,10,11 };


extern uint8_t SSTmode;
extern uint8_t useKeyboardLed; // 0 to use Serial port or 1 for HEX digits.
uint8_t curkey = 0;
uint8_t eepromProtect=1;  // default is to write-protect EEPROM
int blitzMode=1;  // microchess status variable. 1 speeds up chess moves (and dumbs down play)
uint8_t keyboardMode=0;  // start with keyboard in 0: KIM-1 mode. 2: luxury mode

  // ---------- in cpu.c ------------------------------
  void exec6502(int32_t tickcount);
  void reset6502();
  void nmi6502();
  void initKIM(void);
  void loadTestProgram(void);
  extern void driveLEDs();
  void scanKeys(); 


  uint8_t getAkey()            { return(curkey);  }
  void clearkey()             { curkey = 0; }

  // getKIMkey() translates ASCII keypresses to codes the KIM ROM expects.
  // note that, inefficiently, the KIM Uno board's key codes are first translated to ASCII, then routed through
  // here just like key presses from the ASCII serial port are. That's inefficient but left like this
  // for hysterical raisins.
  
void initKeypad()
{
  int i;
  // --------- initialse for scanning keyboard matrix -----------------
  // set columns to input with pullups
  for (i=0;i<8;i++)
  {  pinMode(aCols[i], INPUT);           // set pin to input
     digitalWrite(aCols[i], HIGH);       // turn on pullup resistors  
  }
  // set rows to output, and set them High to be in Neutral position
  for (i=0;i<3;i++)
  {  pinMode(aRows[i], OUTPUT);           // set pin to output
     digitalWrite(aRows[i], HIGH);       // set to high
  }

}
  
uint8_t getKIMkey() {
  //Serial.print('#');  Serial.print(curkey);  Serial.print('#');

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
  if( keyboardMode == 0 ) {
    if( curkey == 'g' ) return( 0x07 ); /* GO */
    if( curkey == 'l' ) return( 0x10 ); /* AD - address Location */
    if( curkey == 'v' ) return( 0x11 ); /* DA - data Value */
    if( curkey == 'p' ) return( 0x14 ); /* PC */
  }
  
  return(curkey); // any other key, should not be hit but ignored by KIM
}


// translate keyboard commands to KIM-I keycodes or emulator actions
void interpretkeys()
{    
  // round 1: keys that always have the same meaning
  switch (curkey) {
    case 'r': // reset but only in mode 1.
      if( keyboardMode != 0 ) break;
    case 18:  // CtrlR = RS key = hardware reset (RST)
      reset6502(); clearkey(); Serial.print("RSet\n"); break;
    
    case 's':
      if( keyboardMode != 0 ) break;
    case 20: // CtrlT = ST key = throw an NMI to stop execution of user program
      nmi6502(); clearkey(); Serial.print("STop\n"); break;
      
    case '[': // SST off
      SSTmode = 0; clearkey();
      Serial.print(F("                                      SST OFF         "));
      displayText( kDt_SST_OFF, 500 );
      break;
    case ']': // SST on
      SSTmode = 1; clearkey();
      Serial.print(F("                                      SST ON          ")); 
      displayText( kDt_SST_ON, 500 );
      break;
      
    case 9: // TAB pressed, toggle between serial port and onboard keyboard/display
      if (useKeyboardLed==0) 
      { useKeyboardLed=1;    Serial.print(F("                    Keyboard/Hex Digits Mode ")); }
      else 
      { useKeyboardLed=0;    Serial.print(F("                        Serial Terminal Mode         "));}
      reset6502();  clearkey();  break;
      
    case '>': // Toggle write protect on eeprom
      if (eepromProtect==0) {
        eepromProtect = 1; 
        Serial.print(F("                                      Eeprom R/O     "));
        displayText( kDt_EE_RO, 500 );
      } else {
        eepromProtect = 0;
        Serial.print(F("                                      Eeprom R/W     "));
        displayText( kDt_EE_RW, 500 );
        delay(20);
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


#ifdef CKeyThreeEight
void scanKeys() 
{
  int led,row,col, noKeysScanned;
  static int keyCode = -1, prevKey = 0;
  static unsigned long timeFirstPressed = 0;
    
  // 0. disable driving the 7segment LEDs -----------------
  disableLEDs();
  // 1. initialise: set columns to input with pullups
  for (col=0;col<8;col++)
  {  pinMode(aCols[col], INPUT);           // set pin to input
     digitalWrite(aCols[col], HIGH);       // turn on pullup resistors  
  }
  // 2. perform scanning
  noKeysScanned=0;

  for (row=0; row<3; row++)
  { digitalWrite(aRows[row], LOW);       // activate this row     
    for (col=0;col<8;col++)
    { if (digitalRead(aCols[col])==LOW)  // key is pressed
      { keyCode = col+row*8+1;
        if (keyCode!=prevKey)
        {   //Serial.println();
            //Serial.print(" col: ");  Serial.print(col, DEC); 
            //Serial.print(" row: ");  Serial.print(row, DEC); 
            //Serial.print(" prevKey: ");  Serial.print(prevKey, DEC); 
            //Serial.print(" KeyCode: ");  Serial.println(keyCode, DEC); 
           prevKey = keyCode;
           curkey = parseChar(keyCode);
            //Serial.print(" curkey: ");  Serial.print(curkey, DEC); 
           timeFirstPressed=millis();  // 
        }
        else // if pressed for >1sec, it's a ModeShift key
        {
          if ((millis()-timeFirstPressed)>1000) // more than 1000 ms
          {
              if (keyCode==17) //it was the SST button
              {
//                keyboardMode=(keyboardMode==0?1:0); // toggle
//                Serial.print(F("                                      Eeprom R/O     ")); 
//                Serial.print(F("                                keyboardMode: "));  Serial.print(keyboardMode, DEC); 
//                SSTmode=0;
//                curkey=0;  // don't do anything else with this keypress
              }
              if (keyCode==9) // it was RS button
                curkey = '>';  // toggle eeprom write protect
              timeFirstPressed=millis(); // because otherwise you toggle right back!
          }          
        }
      }
      else
        noKeysScanned++;  // another row in which no keys were pressed
    }     
    digitalWrite(aRows[row], HIGH);       // de-activate this row
  }

  if (noKeysScanned==24)    // no keys detected in any row, 3 rows * 8 columns = 24. used to be 28.
    prevKey=0;        // allows you to enter same key twice 
} // end of function
#endif

#ifdef CKeyFourSix

const byte ROWS = 6; // six rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {
  {   7,  20,  18, 't' }, // go (7), step(0x12), reset (18), sst (20)
  {   1,   4,  16, '+' } , // ad (0x10), da (0x11), PC (16), plus (0x12)
  { 'C', 'D', 'E', 'F' }, // 0..15
  { '8', '9', 'A', 'B' },
  { '4', '5', '6', '7' },
  { '0', '1', '2', '3' }
};

// set up with the same as KIM UNO
byte rowPins[ROWS] = { 7, 6, 5, 4, 3, 2 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 10, 11, A5, 9 };    //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


// Four by six
void scanKeys()
{
  static int keyCode = -1, prevKey = 0;
  static unsigned long timeFirstPressed = 0;

  // scan the keys and then remap them
   int i;

  // first, wipe everything
  for( i = 0 ; i<8 ; i++ ) {
    pinMode( aCols[i], INPUT );
    digitalWrite( aCols[i], LOW );
    pinMode( ledSelect[i], INPUT );
    digitalWrite( ledSelect[i], LOW );
  }

  for( i=0 ; i<ROWS ; i++ ) {
    pinMode( rowPins[i], INPUT );
    digitalWrite( rowPins[i], HIGH );
  }

  for( i=0 ; i<COLS ; i++ ) {
    pinMode( colPins[i], OUTPUT );
    digitalWrite( colPins[i], HIGH );
  }

  char key = keypad.getKey();
    if( key == NO_KEY && ((keypad.getState() == IDLE) || ( keypad.getState() == RELEASED ))) {
    prevKey = 0;
    return;
  }

  // convert key to keycode (addressing in the other matrix )
  if( key == 't' ) {
    // sst toggle
    key = SSTmode==0?']':'[';
  }

  if( key != NO_KEY )
    keyCode = key; // fill this.

  if (keyCode!=prevKey)
  {
    prevKey = keyCode;
    curkey = key; // we're already decoded. parseChar(keyCode);
    timeFirstPressed=millis();  // 
  }
  else // if pressed for >1sec, it's a ModeShift key
  {
    if ((millis()-timeFirstPressed)>1000) // more than 1000 ms
    {   
        if (key == '[' || key == ']' ) //keyCode==17) //it was the SST button
        { 
//          keyboardMode=(keyboardMode==0?1:0); // toggle
//                Serial.print(F("                                      Eeprom R/O     ")); 
//          Serial.print(F("                                keyboardMode: "));  Serial.print(keyboardMode, DEC);
//          SSTmode=0;
//          curkey=0;  // don't do anything else with this keypress
        }
        if (keyCode==18) // it was RS button // fix
          curkey = '>';  // toggle eeprom write protect
        timeFirstPressed=millis(); // because otherwise you toggle right back!
    }
  }
}
#endif

#ifdef CKeyNovus

// keypad definition
#define kROWS (3)
#define kCOLS (7)
byte rowPins[kROWS] = { 9, 10, 11 }; //connect to the row pinouts of the keypad
byte colPins[kCOLS] = { 2, 3, 4, 5, 6, 7, 8 }; //connect to the column pinouts of the keypad
#define kNoPress ('Z')

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
        
   key definitions we can use:
       '0'-'9', 'A'-'F' for digits
        7: GO   20: STEP  18: RESET  't': SST TOGGLE
        1: AD    4: DA    16: PC     '+': PLUS
        '$': shift toggle
        '>': EEPROM WRITE MODE TOGGLE
        
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

#define kLEDs (7)
//                      p   a   b   c    d   e   f  g
char   segments[8] = { A5,  2,  3,  4,   5,  6,  7, 8 };
char digits[kLEDs] = { 12, 13, 14, 15, 16, 17, 18    };

void initKeypad()
{
    for( int x=0 ; x<kLEDs ; x++ )
  {
    // output, low - press causes all digits to light
    // input, low/high - press causes one dight to light
    // output, high - streaming things
    pinMode( digits[x], INPUT );
    digitalWrite( digits[x], LOW );
  }

  for( int x=0 ; x<kROWS ; x++ )
  {
    pinMode( rowPins[x], OUTPUT );
    digitalWrite( rowPins[x], HIGH );
  }
  for( int x=0 ; x<kCOLS ; x++ )
  {
    pinMode( colPins[x], INPUT );
    digitalWrite( colPins[x], HIGH );
  }
}


char scanKeypad()
{
  initKeypad();
  for( int r=0 ; r<kROWS ; r++ )
  {
    digitalWrite( rowPins[r], LOW );
    for( int c=0 ; c<kCOLS ; c++ )
    {
      if( digitalRead( colPins[c] ) == LOW ) {
        if( shiftKey ) {
          return lookup_shifted[c][r];
        } else {
          return lookup[c][r];
        }
      }
    }
    digitalWrite( rowPins[r], HIGH );
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
  char ke = scanKeypadEvents();
  static char popShift = 0;
  
  curkey = 0;
  switch( keyEvent ) {
    case( kEventPressed ):
      switch( ke ) {
        case( '$' ): shiftKey ^= 1; break;
        case( 's' ): displayText( kDt_Scott, 1000 ); break;
        case( 'o' ): displayText( kDt_Oscar, 1000 ); break;
        
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
    
    case( kEventPressing ):
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

#endif



}
