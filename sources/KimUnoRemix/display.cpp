/*
 * display
 *
 *   This drives the LEDs for common anode, cathode, and size configuration
 *   It also handles text rendering
 */

#include "Arduino.h"
#include "config.h"
#include <avr/pgmspace.h>

extern "C" {
#ifdef kShiftKeypad
char shiftKey = 0;  // is the keypad shift key in effect?
#endif
extern unsigned char kimHex[];        // seLED display
unsigned char textHex[7];

// For all of this, we'll assume that they're wired correctly
// ledSegments corrolates with the bits in "segmentLookup" below ( dp, a, b, c, d, e, f, g)
const byte ledSegments[8]  = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP
#define kDisplayDP   (0x80) /* which bit is the decimal point -- needs to be 0x80 */


/* some of the displays may be wired incorrectly... */
#ifdef kPlatformKIMUno
const byte ledDigits[8]  =  { 12, 13, A0, A1,  A4, A2, A3, A7 };
// notice that the displays are ordred on the KIM UNO board as:
// led1 led2 led3 led4   led7 led5 led6 (led8)    (led8 is not connected)
//  D12  D13  A0   A1     A4   A2   A3    (A7)
#endif

#ifdef kPlatformNovus750
const byte ledDigits[8] =  { 12, 13, A0, A1,  A2, A3, A4, A7 };
#endif

#ifdef kPlatform2411
const byte ledDigits[8] =  { 12, 13, A0, A1,  A4, A2, A3, A7 };
#endif

#define kCharMax (31)

const unsigned char segmentLookup[kCharMax] PROGMEM = { 
  /* NOTE: this is copied from the rom at 0x1FE7 */ 
  /*
   The builtin 7-segment LED "font"
   
     aaa
    f   b
    f   b
     ggg
    e   c
    e   c
     ddd  x
  */
  /*           x g f e  d c b a */
  0x3F, /* 0   1 0 1 1  1 1 1 1 */
  0x06, /* 1   1 0 0 0  0 1 1 0 */
  0x5B, /* 2   1 1 0 1  1 0 1 1 */
  0x4F, /* 3   1 1 0 0  1 1 1 1 */
  0x66, /* 4   1 1 1 0  0 1 1 0 */
  0x6D, /* 5   1 1 1 0  1 1 0 1 */
  0x7D, /* 6   1 1 1 1  1 1 0 1 */
  0x07, /* 7   1 0 0 0  0 1 1 1 */
  0x7F, /* 8   1 1 1 1  1 1 1 1 */
  0x6F, /* 9   1 1 1 0  1 1 1 1 */
  0x77, /* A   1 1 1 1  0 1 1 1 */
  0x7C, /* B   1 1 1 1  1 1 0 0 */
  0x39, /* C   1 0 1 1  1 0 0 1 */
  0x5E, /* D   1 1 0 1  1 1 1 0 */
  0x79, /* E   1 1 1 1  1 0 0 1 */
  0x71, /* F   1 1 1 1  0 0 0 1 */
          
  /* additional characters */
  // xgfedcba
    B01000000, // 16 printed as -
    B00001000, // 17 printed as _
    B00000000, // 18 printed as <space>
    
    /* some letters we need for text display */
    B01010100, // 19 'n'  "s on"
    B01011100, // 20 'o'  "s on"/"s off"
    B01110011, // 21 'p'  "EEP"
    B01010000, // 22 'r'  "E ro"/"E rw"
    B01111000, // 23 't'  "sst"
    B00111110, // 24 'U'  "Uno"  
    B00011101, // 25 'w'  "rw" (common version for 7 seg)
    B01011000, // 26 'c'
    B01010100, // 27 'h'
    B00000010, // 28 'i'
    B01011111, // 29 'a'
    B10000000  // 30 '.'
};



////////////////////////////////////////
// for text display
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
  return;
  for( int x = 0 ; x<8 ; x++ ) 
    textHex[x] = 18; // ' ' space
  if( which == kDt_SST_ON ) {
    textHex[1] = 5;  // 5
    textHex[2] = 5;  // 5
    textHex[3] = 23; // t
    textHex[5] = 20; // o
    textHex[6] = 19; // n
  } else if( which == kDt_SST_OFF ) {
    textHex[1] = 5;  // 5
    textHex[2] = 5;  // 5
    textHex[3] = 23; // t
    textHex[4] = 20; // o
    textHex[5] = 15; // f
    textHex[6] = 15; // f
  } else if( which ==  kDt_EE_RW ) {
    textHex[1] = 14; // E
    textHex[2] = 14; // E
    textHex[3] = 21; // P
    textHex[5] = 22; // R
    textHex[6] = 25; // W
  } else if( which ==  kDt_EE_RO ) {
    textHex[1] = 14; // E
    textHex[2] = 14; // E
    textHex[3] = 21; // P
    textHex[5] = 22; // R
    textHex[6] = 20; // o
  } else if( which == kDt_Uno ) {
    textHex[1] = 24; // U
    textHex[2] = 19; // n
    textHex[3] = 20; // o
    textHex[4] = 18 | kDisplayDP;
    textHex[5] = kVersionMinorA;  // 0
    textHex[6] = kVersionMinorB;  //   7
  } else if( which == kDt_Scott ) {
    textHex[1] = 5;  // s
    textHex[2] = 26; // c
    textHex[3] = 20; // o
    textHex[4] = 23; // t
    textHex[5] = 23; // t
  } else if( which == kDt_Oscar ) {
    textHex[1] = 20; // o
    textHex[2] = 5;  // s
    textHex[3] = 26; // c
    textHex[4] = 29; // a
    textHex[5] = 22; // r
  }
  textTimeout = millis() + timeMillis; // when to switch back
}


// display a bitpattern, map it to segments
void displayPattern( int digit, int pattern )
{
#ifdef kDisplayIsCommonAnode
  #define kClearSeg    HIGH
  #define kClearDigit  LOW
  #define kEnableSeg   LOW
  #define kEnableDigit HIGH
#else
  #define kClearSeg    LOW
  #define kClearDigit  HIGH
  #define kEnableSeg   HIGH
  #define kEnableDigit LOW
#endif
  
  // clear all segments
  for( int i=0 ; i<=8 ; i++ ) /* segment a .. segment f, dp */
    digitalWrite( ledSegments[i], kClearSeg );

  // clear all digits
  for( int j=0 ; j <=7 ; j++ ) /* digit 0 .. digit 6 */
    digitalWrite( ledDigits[j], kClearDigit );

  // enable the right segments
  if( pattern & 0x80 ) digitalWrite( ledSegments[0], kEnableSeg ); // dp
  if( pattern & 0x01 ) digitalWrite( ledSegments[1], kEnableSeg ); // a
  if( pattern & 0x02 ) digitalWrite( ledSegments[2], kEnableSeg ); // b
  if( pattern & 0x04 ) digitalWrite( ledSegments[3], kEnableSeg ); // c
  if( pattern & 0x08 ) digitalWrite( ledSegments[4], kEnableSeg ); // d
  if( pattern & 0x10 ) digitalWrite( ledSegments[5], kEnableSeg ); // e
  if( pattern & 0x20 ) digitalWrite( ledSegments[6], kEnableSeg ); // f
  if( pattern & 0x40 ) digitalWrite( ledSegments[7], kEnableSeg ); // g

  // enable this digit
  digitalWrite( ledDigits[digit], kEnableDigit );
  delay( 1 );
  digitalWrite( ledDigits[digit], kClearDigit );
}

void driveLEDs()
{
  int i,j;

  for( i=0 ; i<=8 ; i++ ) /* segment a .. segment f, dp */
  {
    pinMode( ledSegments[i], OUTPUT );
    digitalWrite( ledSegments[i], LOW );
  }

  for( j=0 ; j <=6 ; j++ ) /* digit 0 .. digit 6 */
  {
    pinMode( ledDigits[j], OUTPUT );
    digitalWrite( ledDigits[j], LOW );
  }

  //for( int x = 0 ; x < 6 ; x++ )
  //{
  //    if( x == 4 ) Serial.print( " " );
  //    Serial.print( kimHex[x], HEX );
  //}
  //Serial.println( "" );
  
  for( int displayDigit = 0 ; displayDigit < 8 ; displayDigit++ )
  {
    byte pattern = 18; // space
    if( true ) { //millis() >= (long)textTimeout ) { // not elegant, but good for now
      // display the internal digits
      if( displayDigit == kDisplayAddrOffset   ) pattern = kimHex[0];
      if( displayDigit == kDisplayAddrOffset+1 ) pattern = kimHex[1];
      if( displayDigit == kDisplayAddrOffset+2 ) pattern = kimHex[2];
      if( displayDigit == kDisplayAddrOffset+3 ) pattern = kimHex[3];
      // 4
      if( displayDigit == kDisplayDataOffset   ) pattern = kimHex[4];
      if( displayDigit == kDisplayDataOffset+1 ) pattern = kimHex[5];

      pattern = pgm_read_byte_near( segmentLookup + pattern ); // remove decimal point here.
#ifdef kShiftKeypad
      if( (displayDigit == kDisplayShift)  &&  shiftKey ) pattern |= 0xff;//B01001001;  // shift indicator
#endif
      if( displayDigit == kDisplayDot ) pattern |= 0x80;  // decimal point separator
     } 
    else 
    {
      // display the temporary text
      i = textHex[displayDigit]; // digit to display, mask off dpoint bit (0x80)
      
      pattern = pgm_read_byte_near( segmentLookup + i) | (textHex[displayDigit] & 0x80); // add the dp bit back on
    }
   
    displayPattern( displayDigit,  pattern );
  }
}

}
