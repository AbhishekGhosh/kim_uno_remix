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
#include <util/delay.h>
#ifdef CUseKeypadLibrary
  #include <Keypad.h>
#endif

extern "C" {
#include "memory.h"
}

void setupUno();


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
  void driveLEDs();
  extern uint8_t curkey;
  void scanKeys(); 
  
  // ---------- called from cpu.c ----------------------
  uint8_t serialEnable = 0;
  void serout(uint8_t val)    { if( serialEnable ) Serial.write(val);  }
  void serouthex(uint8_t val) { if( serialEnable ) Serial.print(val, HEX); }
  void printhex(uint16_t val) { if( serialEnable ) { Serial.print(val, HEX); Serial.println(); } }

}

// check for out of RAM
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

  
extern "C" {
  
extern uint8_t eepromProtect;
  // move this into memory.c?
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
  /* set up serial */
  Serial.begin ( kBaudRate );
  Serial.println ();
  
  /* initialize the hardware */
  setupUno();

  reset6502();
  initKIM(); // Enters 1c00 in KIM vectors 17FA and 17FE. Might consider doing 17FC as well????????
  loadTestProgram();
  
  /* splash display */
  displayText( kDt_Uno, 1000 );

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
  initKeypad();
  
  // --------- clear display buffer ------------------------------------
  for (i=0;i<3;i++)
  { threeHex[i][0]=threeHex[i][0]=0; }
  Serial.println(F("Ready"));
}
