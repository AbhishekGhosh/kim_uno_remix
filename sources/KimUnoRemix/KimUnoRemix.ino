/* Kim Uno Remix
 *
 *  A reworking a bit of Oscar Vermeulen's KimUno project
 *
 *  Scott Lawrence  yorgle@gmail.com
 */

#include "Arduino.h"
#include "config.h"
#include "display.h"
#include "keys.h"
#include <EEPROM.h>
#ifndef AVRX
  #include <stdint.h>
#endif


/**************************/
/* Emulated CPU Functions */

extern "C" {
  #include "memory.h"
  // ---------- in cpu.c ------------------------------
  void exec6502(int32_t tickcount);
  void reset6502();
}


/******************/
/* Serial functions */

extern "C" {
  // ---------- called from cpu.c ----------------------
  uint8_t serialEnable = 0;
  void serout(uint8_t val)    { if( serialEnable ) Serial.write(val);  }
  void serouthex(uint8_t val) { if( serialEnable ) Serial.print(val, HEX); }
  void printhex(uint16_t val) { if( serialEnable ) { Serial.print(val, HEX); Serial.println(); } }
} /* C */


/***********/
/* Utility */

// check for out of RAM
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


/********************/
/* EEPROM FUNCTIONS */
extern "C" {
  uint8_t eepromProtect = 1;

  uint8_t eepromread(uint16_t eepromaddress) {
    return EEPROM.read(eepromaddress);
  }
  
  void eepromwrite(uint16_t eepromaddress, uint8_t bytevalue) {
    if (eepromProtect==0) {
      EEPROM.write(eepromaddress, bytevalue);
    } else {
      Serial.println(F("ERROR: EEPROM STATE IS WRITE-PROTECTED. HIT '>' TO TOGGLE WRITE PROTECT"));
      //Serial.println(freeRam());
    }
  }

} /* C */


/******************/
/* Arduino Engine */

void setup () {
  /* set up serial */
  Serial.begin ( kBaudRate );
  Serial.println ();

  /* clear the display buffer */
  for( int i=0 ; i<6 ; i++) kimHex[i] = '0';

  /* reset the CPU */
  reset6502();

  /* set up vectors and load programs to RAM */
  loadProgramsToRam();
  
  /* splash display */
  displayText( kDt_Uno, 1000 );

  /* display user prompt */
  Serial.print(F(kVersionString " Free:")); // just a little check, to avoid running out of RAM!
  Serial.println(freeRam());
}

/************************/
/* Run the emulation... */

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
