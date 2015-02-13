/*
 * keys
 *
 *   support for scanning the keypad(s)
 */
 
#ifndef __KEYS_H__
#define __KEYS_H__

extern "C" {
extern uint8_t serialEnable;
extern uint8_t eepromProtect;  // default is to write-protect EEPROM

extern uint8_t SSTmode;
extern uint8_t useKeyboardLed; // 0 to use Serial port or 1 for HEX digits.
extern uint8_t curkey;
extern uint8_t keyboardMode;  // start with keyboard in 0: KIM-1 mode. 2: luxury mode

  // ---------- in cpu.c ------------------------------
  
  void exec6502(int32_t tickcount);
  void reset6502();
  void nmi6502();


uint8_t getAkey();
void clearkey();

  // getKIMkey() translates ASCII keypresses to codes the KIM ROM expects.
  // note that, inefficiently, the KIM Uno board's key codes are first translated to ASCII, then routed through
  // here just like key presses from the ASCII serial port are. That's inefficient but left like this
  // for hysterical raisins.
  
  
uint8_t getKIMkey();

// translate keyboard commands to KIM-I keycodes or emulator actions
void interpretkeys();
uint8_t parseChar(uint8_t n); //  parse keycode to return its ASCII code
uint8_t xkeyPressed();    // just see if there's any keypress waiting

////////

void initKeypad();
char scanKeypad();

// wrap all of that with an event thingy
#define kEventIdle     (0)
#define kEventPressed  (1)
#define kEventPressing (2)
#define kEventRelease  (3)

char scanKeypadEvents();
void scanKeys();

}

#endif

