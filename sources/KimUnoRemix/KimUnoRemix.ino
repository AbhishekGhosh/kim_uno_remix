/* Kim Uno Remix
 *
 *  A reworking a bit of Oscar Vermeulen's KimUno project
 *
 *  Scott Lawrence  yorgle@gmail.com
 *
 * Version History
 *  v1.0.1  - first KUR version
 */

#include "Arduino.h"
#include "config.h"
#include <EEPROM.h>
#ifndef AVRX
  #include <stdint.h>
#endif
#include <util/delay.h>
#ifdef CKeyFourSix
  #include <Keypad.h>
#endif
#include "memory.h"

extern uint8_t SSTmode;
extern uint8_t useKeyboardLed; // 0 to use Serial port or 1 for HEX digits.
uint8_t curkey = 0;
uint8_t eepromProtect=1;  // default is to write-protect EEPROM
int blitzMode=1;  // microchess status variable. 1 speeds up chess moves (and dumbs down play)
uint8_t keyboardMode=0;  // start with keyboard in 0: KIM-1 mode. 2: luxury mode

char threeHex[3][2];        // seLED display

byte aCols[8] = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP
byte aRows[3] = { 9,10,11 };
byte ledSelect[8] =  { 12, 13, A0, A1,  A2, A3, A7, A4 }; // note that A6 and A7 are not used at present. Can delete them.

byte dig[25] = { /* NOTE: this mirrors the values in the end of the ROM "TABLE" */ 
// bits     _6543210
// digits   _abcdefg
          0b01111110,//0  
          0b00110000,//1
          0b01101101,//2
          0b01111001,//3
          0b00110011,//4
          0b01011011,//5
          0b01011111,//6
          0b01110000,//7
          0b01111111,//8
          0b01111011,//9
          0b01110111,//a
          0b00011111,//b
          0b01001110,//c
          0b00111101,//d
          0b01001111,//e
          0b01000111,//f
          /* all of the above are confirmed identical to the ROM  */
          
          /* additional characters */
          0b00000001, //g printed as -
          0b00001000, //h printed as _
          0b00000000, //i printed as <space>
          
          /* some letters we need for text display */
          0b00010101, // 19 'n'  "s on"
          0b00011101, // 20 'o'  "s on"/"s off"
          0b01100111, // 21 'p'  "EEP"
          0b00000101, // 22 'r'  "E ro"/"E rw"
          0b00001111, // 23 't'  "sst"
          0b00101010, // 24 'w'  "rw" (common version for 7 seg)
};


// for text display
char textHex[3][2];         // for text indicator
unsigned long textTimeout;

// displayText
//  pass it in one of these values, and the number of ms to display (eg 500)
//  for text display

#define kDt_blank   (0)
#define kDt_SST_ON  (1)
#define kDt_SST_OFF (2)
#define kDt_EE_RW   (3)
#define kDt_EE_RO   (4)

void displayText( int which, long timeMillis )
{
  textHex[1][0] = 18; textHex[1][1] = 18; // "  "

  if( which == kDt_SST_ON ) {
    textHex[0][0] = 5;  // 5
    textHex[0][1] = 5;  // 5
    textHex[1][0] = 23; // t
    textHex[2][0] = 20; // o
    textHex[2][1] = 19; // n
  } else if( which == kDt_SST_OFF ) {
    textHex[0][0] = 5;  // 5
    textHex[0][1] = 5;  // 5
    textHex[1][0] = 22; // t
    textHex[2][0] = 20; // o
    textHex[2][1] = 15; // f
  } else if( which ==  kDt_EE_RW ) {
    textHex[0][0] = 14; // E
    textHex[0][1] = 14; // E
    textHex[1][0] = 21; // P
    textHex[2][0] = 22; // R
    textHex[2][1] = 24; // W
  } else if( which ==  kDt_EE_RO ) {
    textHex[0][0] = 14; // E
    textHex[0][1] = 14; // E
    textHex[1][0] = 21; // P
    textHex[2][0] = 22; // R
    textHex[2][1] = 20; // o
  }

  textTimeout = millis() + timeMillis; // 1/2 second
}


extern "C" {
  // ---------- in cpu.c ------------------------------
  void exec6502(int32_t tickcount);
  void reset6502();
  void nmi6502();
  void initKIM(void);
  void loadTestProgram(void);
  extern void driveLEDs();
  void scanKeys(); 
  
  // ---------- called from cpu.c ----------------------
  uint8_t serialEnable = 0;
  void serout(uint8_t val)    { if( serialEnable ) Serial.write(val);  }
  void serouthex(uint8_t val) { if( serialEnable ) Serial.print(val, HEX); }
  uint8_t getAkey()            { return(curkey);  }
  void clearkey()             { curkey = 0; }
  void printhex(uint16_t val) { if( serialEnable ) { Serial.print(val, HEX); Serial.println(); } }


  // getKIMkey() translates ASCII keypresses to codes the KIM ROM expects.
  // note that, inefficiently, the KIM Uno board's key codes are first translated to ASCII, then routed through
  // here just like key presses from the ASCII serial port are. That's inefficient but left like this
  // for hysterical raisins.
  
  
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
  
uint8_t eepromread(uint16_t eepromaddress) {
  return EEPROM.read(eepromaddress);
}

void eepromwrite(uint16_t eepromaddress, uint8_t bytevalue) {
  if (eepromProtect==0) {
    EEPROM.write(eepromaddress, bytevalue);
  } else {
    Serial.println(F("ERROR: EEPROM STATE IS WRITE-PROTECTED. HIT '>' TO TOGGLE WRITE PROTECT"));
    Serial.println(freeRam());
  }
}

}

extern "C" {
#include "memory.h"
}
extern MMAP ROMSegments[];

void setup () {
  Serial.begin ( kBaudRate );
  Serial.println ();
  
  /*
  serialEnable=true;
  for( int i=0x2000; i< 0x2010 ; i++ )
  {
    Serial.print( i, HEX );
    Serial.print( " " );
    Serial.println( readMemory( i, ROMSegments ), HEX );
  }
  
  prog_uchar * buf = ROMSegments[2].buffer;
  
  for( int j=0 ; j<0x10 ; j++ )
  {
    Serial.print( j+0x2000, HEX );
    Serial.print( " " );
    
    Serial.println( pgm_read_byte_near( buf+j ), HEX );
  }


while(1) { delay( 1000 ); }
*/
  setupUno();

  reset6502();
  initKIM(); // Enters 1c00 in KIM vectors 17FA and 17FE. Might consider doing 17FC as well????????
  loadTestProgram();
  
  Serial.print(F(kVersionString " Free:")); // just a little check, to avoid running out of RAM!
  Serial.println(freeRam());
}

void loop () {
  exec6502(100); //do 100 6502 instructions
  
  if (Serial.available())
  {
    serialEnable = 1;
    curkey = Serial.read() & 0x7F;
    interpretkeys();
  }
    
  scanKeys();  
  if (xkeyPressed()!=0) //KIM Uno board input?
    interpretkeys();

  //driveLEDs(); // doing that here would cause a massive slowdown but keeps display on at all times
}

// check for out of RAM
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
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


// =================================================================================================
// KIM Uno Board functions are bolted on from here
// =================================================================================================

void setupUno() {
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
  
  // --------- clear display buffer ------------------------------------
  for (i=0;i<3;i++)
  { threeHex[i][0]=threeHex[i][0]=0; }
  Serial.println(F("Ready"));
}


extern "C" {
/* drive LEDs, this will be common anode or common cathode indexing */

#ifdef CLedCommonAnode
void driveLEDs()
{ 
  int led, col, ledNo, currentBit, bitOn;
  int byt,i;

  // 1. initialse for driving the 6 (now 8) 7segment LEDs
  // ledSelect pins drive common anode for [all segments] in [one of 6 LEDs]
  for (led=0;led<7;led++)
  { pinMode(ledSelect[led], OUTPUT);  // set led pins to output
    digitalWrite(ledSelect[led], LOW); // LOW = not lit
  }
  // 2. switch column pins to output mode
  // column pins are the cathode for the LED segments
  // lame code to cycle through the 3 bytes of 2 digits each = 6 leds
  for (byt=0;byt<3;byt++)
    for (i=0;i<2;i++)
    {
      ledNo=byt*2+i;
      for (col=0;col<8;col++)  
      {  pinMode(aCols[col], OUTPUT);           // set pin to output
         //currentBit = (1<<(6-col));             // isolate the current bit in loop
         currentBit = (1<<(7-col));             // isolate the current bit in loop
         bitOn = (currentBit&dig[threeHex[byt][i]])==0;  
         digitalWrite(aCols[col], bitOn);       // set the bit
      }
      digitalWrite(ledSelect[ledNo], HIGH); // Light this LED 
      delay(2);
      digitalWrite(ledSelect[ledNo], LOW); // unLight this LED
    }
} // end of function
#endif

#ifdef CLedCommonCathode
// common cathode
void driveLEDs()
{
  char digit = 0;
  byte b;

  for( byte i = 0 ; i<3 ; i++ )
  {
    for( byte j = 0 ; j<2 ; j++ )
    {
      // get the current byte to display (and display text if applicable)
      if( millis() < textTimeout ) {
        b = textHex[i][j];
      } else {
        b = threeHex[i][j];
      }

      // get the segment mask
      byte s = dig[b];

      // select just this digit
      for( int d=0 ; d<8 ; d++ ) {
        pinMode( ledSelect[d], OUTPUT ); // make sure we're output
        digitalWrite( ledSelect[d], (d==digit)?LOW:HIGH );
      }

      // now go through and turn on/off the right segments
      for( byte col =0 ; col <8 ; col++ )
      { 
        pinMode( aCols[col], OUTPUT );
        digitalWrite( aCols[col], (s & (0x80>>col))? HIGH : LOW );
      }

      // wait a moment...
      delay( 3 );

      // go to the next display digit
      digit++;
    }
  }
}
#endif
} // end of C segment


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
  }
  return c;
}

uint8_t xkeyPressed()    // just see if there's any keypress waiting
{
  return (curkey==0?0:1);
}


extern "C" {  // the extern C is to make function accessible from within cpu.c

#ifdef CKeyThreeEight
void scanKeys() 
{
  int led,row,col, noKeysScanned;
  static int keyCode = -1, prevKey = 0;
  static unsigned long timeFirstPressed = 0;
    
  // 0. disable driving the 7segment LEDs -----------------
  for (led=0;led<8;led++)
  { pinMode(ledSelect[led], INPUT);  // set led pins to input
                                  // not really necessary, just to stop them
                                 // from driving either high or low.
    digitalWrite(ledSelect[led], HIGH); // Use builtin pullup resistors
  }
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
} // end C segment

