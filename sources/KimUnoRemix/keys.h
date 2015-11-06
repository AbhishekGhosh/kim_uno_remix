/*
 * keys
 *
 *   support for scanning the keypad(s)
 */
 
#ifndef __KEYS_H__
#define __KEYS_H__

extern "C" {
#include <stdint.h>

extern uint8_t serialEnable;  /* 0 (disabled) by default */
extern uint8_t SSTmode;       /* 0 (SST OFF) by default */
extern uint8_t eepromProtect; /* 1 (read-only) by default */

extern uint8_t useKeyboardLed; // 0 to use Serial port or 1 for HEX digits.
extern uint8_t keyboardMode;  // start with keyboard in 0: KIM-1 mode. 2: luxury mode

////////

void initKeypad();
void keypadScan();
}

#endif

