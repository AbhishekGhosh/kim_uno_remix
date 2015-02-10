/* Kim Uno Remix
 *
 *  A reworking a bit of Oscar Vermeulen's KimUno project
 *
 *  Scott Lawrence  yorgle@gmail.com
 */

#include "Arduino.h"
#include "config.h"
#include <EEPROM.h>
#ifndef AVRX
  #include <stdint.h>
#endif
#include <util/delay.h>
#ifdef CUseKeypadLibrary
  #include <Keypad.h>
#endif

extern "C" {
#include "memory.h"
}

extern uint8_t SSTmode;
extern uint8_t useKeyboardLed; // 0 to use Serial port or 1 for HEX digits.
uint8_t curkey = 0;
uint8_t eepromProtect=1;  // default is to write-protect EEPROM
int blitzMode=1;  // microchess status variable. 1 speeds up chess moves (and dumbs down play)
uint8_t keyboardMode=0;  // start with keyboard in 0: KIM-1 mode. 2: luxury mode
char shiftKey = 0;  // is the keypad shift key in effect?

char threeHex[3][2];        // seLED display

byte aCols[8] = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP
// aCols corrolates with the bits in "dig" below ( dp, a, b, c, d, e, f, g)

byte aRows[3] = { 9,10,11 };
byte ledSelect[8] =  { 12, 13, A0, A1,  A2, A3, A4, A7 }; // note that A6 and A7 are not used at present. Can delete them.

byte dig[30] = { /* NOTE: this mirrors the values in the end of the ROM "TABLE" */ 
// bits     _6543210
// digits   abcdefg
          B01111110,//0  
          B00110000,//1
          B01101101,//2
          B01111001,//3
          B00110011,//4
          B01011011,//5
          B01011111,//6
          B01110000,//7
          B01111111,//8
          B01111011,//9
          B01110111,//a
          B00011111,//b
          B01001110,//c
          B00111101,//d
          B01001111,//e
          B01000111,//f
          /* all of the above are confirmed identical to the ROM  */
          
          /* additional characters */
          B00000001, //g printed as -
          B00001000, //h printed as _
          B00000000, //i printed as <space>
          
          /* some letters we need for text display */
          B00010101, // 19 'n'  "s on"
          B00011101, // 20 'o'  "s on"/"s off"
          B01100111, // 21 'p'  "EEP"
          B00000101, // 22 'r'  "E ro"/"E rw"
          B00001111, // 23 't'  "sst"
          B00111110, // 24 'U'  "Uno"  
          B00101010, // 25 'w'  "rw" (common version for 7 seg)
          B00001101, // 26 'c'
          B00010111, // 27 'h'
          B00100000, // 28 'i'
          B01111101, // 29 'a'
};


void setupUno();


////////////////////////////////////////
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
#define kDt_Uno     (5)
#define kDt_Scott   (6)
#define kDt_Oscar   (7)

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
    textHex[2][1] = 25; // W
  } else if( which ==  kDt_EE_RO ) {
    textHex[0][0] = 14; // E
    textHex[0][1] = 14; // E
    textHex[1][0] = 21; // P
    textHex[2][0] = 22; // R
    textHex[2][1] = 20; // o
  } else if( which == kDt_Uno ) {
    textHex[0][0] = 24; // U
    textHex[0][1] = 19; // n
    textHex[1][0] = 20; // o
    textHex[2][0] = 0;  // v 0
    textHex[2][1] = 6;  //    6
  } else if( which == kDt_Scott ) {
    textHex[0][0] = 5; // s
    textHex[0][1] = 26; // c
    textHex[1][0] = 20; // o
    textHex[1][1] = 23; // t
    textHex[2][0] = 23; // t
    textHex[2][1] = 18;
  } else if( which == kDt_Oscar ) {
    textHex[0][0] = 20; // o
    textHex[0][1] = 5; // s
    textHex[1][0] = 26; // c
    textHex[1][1] = 29; // a
    textHex[2][0] = 22; // r
    textHex[2][1] = 18;
  }

  textTimeout = millis() + timeMillis; // 1/2 second
}


////////////////////////////////////////
// just see if there's any keypress waiting
uint8_t xkeyPressed();

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

  
extern "C" {
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
  
  displayText( kDt_Uno, 1000 ); // display version

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
  { 
    pinMode(ledSelect[led], OUTPUT);  // set led pins to output
    digitalWrite(ledSelect[led], LOW); // LOW = not lit
  }
  
  for(col=0 ; col<8 ; i++ )
  {
    pinMode( aCols[col], OUTPUT );
    digitalWrite( aCols[col], LOW );
  }
  
  // 2. switch column pins to output mode
  // column pins are the cathode for the LED segments
  // lame code to cycle through the 3 bytes of 2 digits each = 6 leds
  
  for (byt=0;byt<3;byt++)
  {
    for (i=0;i<2;i++)
    {
      ledNo=byt*2+i;
      for (col=0;col<8;col++)  
      {
        pinMode(aCols[col], OUTPUT);           // set pin to output
        //currentBit = (1<<(6-col));             // isolate the current bit in loop
        currentBit = (1<<(7-col));             // isolate the current bit in loop
        bitOn = (currentBit&dig[threeHex[byt][i]])==0;  
        digitalWrite(aCols[col], bitOn);       // set the bit
      }
      digitalWrite(ledSelect[ledNo], HIGH); // Light this LED 
      
      digitalWrite( ledSelect[1], HIGH );
      digitalWrite( ledSelect[2], LOW );
      digitalWrite( aCols[2], LOW );
      digitalWrite( aCols[3], HIGH );


      delay(2);
      digitalWrite(ledSelect[ledNo], LOW); // unLight this LED
    }
  }
#ifdef NEVER
  
  byte b;
  byte digit=0;
  for( byte i = 0 ; i<3 ; i++ )
  {
    for( byte j = 0 ; j<2 ; j++ )
    {
      // get the current byte to display (and display text if applicable)
      if( millis() < (long)textTimeout ) {
        b = textHex[i][j];
      } else {
        b = threeHex[i][j];
      }

      // get the segment mask
      byte s = dig[b];

      // select just this digit
      for( int d=0 ; d<8 ; d++ ) {
        pinMode( ledSelect[d], OUTPUT ); // make sure we're output
        digitalWrite( ledSelect[d], (d==digit)?HIGH:LOW ); /* difference here for cathode/anode */
      }

      // now go through and turn on/off the right segments
      for( byte col =0 ; col <8 ; col++ )
      { 
        pinMode( aCols[col], OUTPUT );
        digitalWrite( aCols[col], (s & (0x80>>col))? LOW : HIGH ); /* difference here for cathode/anode */
      }
      
      // wait a moment...
      delay( 3 );

      // go to the next display digit
      digit++;
    }
  }
  #endif

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
      if( millis() < (long)textTimeout ) {
        b = textHex[i][j];
      } else {
        b = threeHex[i][j];
      }

      // get the segment mask
      byte s = dig[b];

      // select just this digit
      for( int d=0 ; d<8 ; d++ ) {
        pinMode( ledSelect[d], OUTPUT ); // make sure we're output
        digitalWrite( ledSelect[d], ((d)==digit)?LOW:HIGH );
      }

      // now go through and turn on/off the right segments
      for( byte col =0 ; col <8 ; col++ )
      { 
        pinMode( aCols[col], OUTPUT );
        digitalWrite( aCols[col], (s & (0x80>>col))? HIGH : LOW );
      }

      // wait a moment...
      delay( 2 );

      // go to the next display digit
      digit++;
    }
  }
}
#endif

#ifdef CLedNovus

// display a bitpattern, map it to segments
void displayPattern( int digit, int pattern )
{
  // clear all segments
  for( int i=0 ; i<=8 ; i++ ) /* segment a .. segment f, dp */
    digitalWrite( aCols[i], LOW );

  // clear all digits
  for( int j=0 ; j <=7 ; j++ ) /* digit 0 .. digit 6 */
    digitalWrite( ledSelect[j], HIGH );

  // enable the right segments
  if( pattern & 0x80 ) digitalWrite( aCols[0], HIGH ); // dp
  if( pattern & 0x40 ) digitalWrite( aCols[1], HIGH ); // a
  if( pattern & 0x20 ) digitalWrite( aCols[2], HIGH ); // b
  if( pattern & 0x10 ) digitalWrite( aCols[3], HIGH ); // c
  if( pattern & 0x08 ) digitalWrite( aCols[4], HIGH ); // d
  if( pattern & 0x04 ) digitalWrite( aCols[5], HIGH ); // e
  if( pattern & 0x02 ) digitalWrite( aCols[6], HIGH ); // f
  if( pattern & 0x01 ) digitalWrite( aCols[7], HIGH ); // g

  // enable this digit
  digitalWrite( ledSelect[digit], LOW );
  delay( 2 );
  digitalWrite( ledSelect[digit], HIGH );
}

void driveLEDs()
{
  for( int i=0 ; i<=8 ; i++ ) /* segment a .. segment f, dp */
  {
    pinMode( aCols[i], OUTPUT );
    digitalWrite( aCols[i], LOW );
  }

  for( int j=0 ; j <=7 ; j++ ) /* digit 0 .. digit 6 */
  {
    pinMode( ledSelect[j], OUTPUT );
    digitalWrite( ledSelect[j], LOW );
  }

  byte displayText = 0;
  byte pattern = 0;
  
  for( int digit =0 ; digit < 7 ; digit++ )
  {
    if( digit == 0 ) pattern = 0x00; // nothing  -- half dig
    if( millis() >= (long)textTimeout ) { // not elegant, but good for now
      if( digit == 1 ) pattern = dig[threeHex[0][0]];
      if( digit == 2 ) pattern = dig[threeHex[0][1]];
      if( digit == 3 ) pattern = dig[threeHex[1][0]];
      if( digit == 4 ) pattern = dig[threeHex[1][1]];
      if( digit == 5 ) pattern = dig[threeHex[2][0]];
      if( digit == 6 ) pattern = dig[threeHex[2][1]];
    } else {
      if( digit == 1 ) pattern = dig[textHex[0][0]];
      if( digit == 2 ) pattern = dig[textHex[0][1]];
      if( digit == 3 ) pattern = dig[textHex[1][0]];
      if( digit == 4 ) pattern = dig[textHex[1][1]];
      if( digit == 5 ) pattern = dig[textHex[2][0]];
      if( digit == 6 ) pattern = dig[textHex[2][1]];
    }
    
    // patch the digits
    if( digit == 0 && shiftKey ) pattern |= 0x02;  // shift indicator
    if( digit == 4 ) pattern |= 0x80;  // decimal point separator
    
    displayPattern( digit, pattern );
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
    default             : c = n; // pass-through
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
  return;
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
} // end C segment

