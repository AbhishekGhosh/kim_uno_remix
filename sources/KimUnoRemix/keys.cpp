/*
 * keys
 *
 *   support for scanning the keypad(s)
 */
 
 
#include "Arduino.h"
#include "config.h"
#include "keys.h"
extern "C" {
#include "display.h"

/* ******************************* */
/* KIM UNO PLATFORM
 *  Oscar's original reference device
*/
#ifdef kPlatformKIMUno
#define kROWS (3)
#define kCOLS (8)

const byte colPins[kCOLS] = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP
const byte rowPins[kROWS] = { 9,10,11 };

const char lookup[kCOLS * kROWS] PROGMEM =
{
  kKimScancode_STOP,  kKimScancode_RESET,  kKimScancode_SSTTOGGLE,
     kKimScancode_6,      kKimScancode_D,  kKimScancode_PC,
     kKimScancode_5,      kKimScancode_C,  kKimScancode_GO,
     kKimScancode_4,      kKimScancode_B,  kKimScancode_PLUS,
     kKimScancode_3,      kKimScancode_A,  kKimScancode_DATA,
     kKimScancode_2,      kKimScancode_9,  kKimScancode_ADDR,
     kKimScancode_1,      kKimScancode_8,  kKimScancode_F,
     kKimScancode_0,      kKimScancode_7,  kKimScancode_E
};

const char lookup_shifted[kCOLS * kROWS] PROGMEM =
{
  kKimScancode_STOP,  kKimScancode_EEPTOGGLE,  kKimScancode_SSTTOGGLE,
     kKimScancode_6,      kKimScancode_D,  kKimScancode_PC,
     kKimScancode_5,      kKimScancode_C,  kKimScancode_GO,
     kKimScancode_4,      kKimScancode_B,  kKimScancode_PLUS,
     kKimScancode_3,      kKimScancode_A,  kKimScancode_DATA,
     kKimScancode_2,      kKimScancode_9,  kKimScancode_ADDR,
     kKimScancode_1,      kKimScancode_8,  kKimScancode_F,
     kKimScancode_0,      kKimScancode_7,  kKimScancode_E
};

#endif


/* ******************************* */
/* NOVUS 750 PLATFORM 
 *  device hacked into the Novus 750 calculator
*/
#ifdef kPlatformNovus750
// keypad definition
#define kROWS (3)
#define kCOLS (7)

const byte rowPins[kROWS] = { 9, 10, 11 }; //connect to the row pinouts of the keypad
const byte colPins[kCOLS] = { 2, 3, 4, 5, 6, 7, 8 }; //connect to the column pinouts of the keypad

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
/* 'x' are unused positions in the array. */
const char lookup[kCOLS * kROWS] PROGMEM =
{
     kKimScancode_0,            'x', kKimScancode_1,
     kKimScancode_3, kKimScancode_C, kKimScancode_4,
     kKimScancode_B, kKimScancode_D, kKimScancode_5,
     kKimScancode_7, kKimScancode_E, kKimScancode_6,
                'x', kKimScancode_2, kKimScancode_8,
                
  kKimScancode_PLUS,            'x', kKimScancode_9,
     kKimScancode_F,            'x', kKimScancode_A
};

const char lookup_shifted[kCOLS * kROWS] PROGMEM = 
{
       kKimScancode_none,                 'x',       kKimScancode_none,
       kKimScancode_none,     kKimScancode_GO,  kKimScancode_EEPTOGGLE,
       kKimScancode_PLUS,   kKimScancode_STOP,       kKimScancode_none,
       kKimScancode_none,  kKimScancode_RESET,       kKimScancode_none,
                     'x',   kKimScancode_none,       kKimScancode_ADDR,
       kKimScancode_PLUS,                 'x',       kKimScancode_DATA,
  kKimScancode_SSTTOGGLE,                 'x',         kKimScancode_PC
};
#endif


/* ******************************* */
/* 2411 PLATFORM 
 *  device with 24 key keypad and 11 digit display
 */
#ifdef kPlatform2411
#define kROWS (4)
#define kCOLS (6)

const byte colPins[kCOLS] = { 7,6,5,4,3,2 }; // note col A5 is the extra one linked to DP
const byte rowPins[kROWS] = { 10,11,A5,9 };

const char lookup[kCOLS * kROWS] PROGMEM =
{
  kKimScancode_GO,   kKimScancode_STOP, kKimScancode_RESET, kKimScancode_SSTTOGGLE,
  kKimScancode_ADDR, kKimScancode_DATA, kKimScancode_PC,    kKimScancode_PLUS,
  kKimScancode_C,    kKimScancode_D,    kKimScancode_E,     kKimScancode_F,
  kKimScancode_8,    kKimScancode_9,    kKimScancode_A,     kKimScancode_B,
  kKimScancode_4,    kKimScancode_5,    kKimScancode_6,     kKimScancode_7,
  kKimScancode_0,    kKimScancode_1,    kKimScancode_2,     kKimScancode_3
};

const char lookup_shifted[kCOLS * kROWS] PROGMEM =
{ /* should unused shifted be ' '? */
  kKimScancode_GO,   kKimScancode_STOP, kKimScancode_EEPTOGGLE, kKimScancode_SSTTOGGLE,
  kKimScancode_ADDR, kKimScancode_DATA, kKimScancode_PC,    kKimScancode_PLUS,
  kKimScancode_C,    kKimScancode_D,    kKimScancode_E,     kKimScancode_F,
  kKimScancode_8,    kKimScancode_9,    kKimScancode_A,     kKimScancode_B,
  kKimScancode_4,    kKimScancode_5,    kKimScancode_6,     kKimScancode_7,
  kKimScancode_0,    kKimScancode_1,    kKimScancode_2,     kKimScancode_3
};

#endif


/* ******************************* */
/* Common handlers */

extern "C" {
  void serln( char * txt );
  void serout(uint8_t val);
  void printhex(uint16_t val);
  void KIMKeyPress( uint8_t ch );
}

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

void filterPress( uint8_t ch )
{
  /* handle the press */
  KIMKeyPress( ch );
  
  /* and update our display */
  if( ch == kKimScancode_SSTTOGGLE &&  SSTmode ) displayText( kDt_SST_ON,  400 );
  if( ch == kKimScancode_SSTTOGGLE && !SSTmode ) displayText( kDt_SST_OFF, 400 );
  if( ch == kKimScancode_EEPTOGGLE &&  eepromProtect ) displayText( kDt_EE_RO, 400 );
  if( ch == kKimScancode_EEPTOGGLE && !eepromProtect ) displayText( kDt_EE_RW, 400 );
}

void keypadScan()
{
  static uint8_t last_scancode = 0xFF; /* never used anywhere */
  static long timeoutMillis = 0;
  uint8_t scancode = 0xFF;

  // set up the ports..
  initKeypad();

  // 1. find the pressed-down key (if any)
  for( int r=0 ; r<kROWS ; r++ )
  {
    digitalWrite( rowPins[r], kLH );
    // we could use direct port writes here, but we may have to intrduce 
    // delays to be sure that line settle in time. Probably not though..
    // Anyway, for portability's sake, this is much easier to maintain 
    // and re-use/port. ;)
    
    for( int c=0 ; c<kCOLS ; c++ )
    {
      if( digitalRead( colPins[c] ) == kLH )
      {
        // we got a HIGH! save aside the index (scancode)
        scancode = r + (c * kROWS );
      }
    }
    digitalWrite( rowPins[r], kHL );
  }
  
  
  // 2. at this point, we have a scancode in 'scancode' or 0xFF
  if( scancode != last_scancode ) {
    // something changed, now to figure out what:
    
    if( scancode != 0xFF ) {
      // 3. it was a key press!
      // start our timeout for the shift-press-hold mechanism
      timeoutMillis = millis() + kShiftDelay;
      shiftKey = 0;
      
    } else {
      // 5. it was a key release!
      if( shiftKey == 0 ) {
        // 6. wasn't shifted, we can send it!
        filterPress( pgm_read_byte_near( lookup + last_scancode ));
      }
      shiftKey = 0;
      
    }
  } else {
    // 4. no change, see if it's being held..
    if( scancode != 0xff && shiftKey == 0) {
      // key is being held...       
      if( millis() > timeoutMillis ) {
        // held for the timeout period
        shiftKey = 1;
        filterPress( pgm_read_byte_near( lookup_shifted + scancode ));
      }
    }
  }
  last_scancode = scancode;
}


}
