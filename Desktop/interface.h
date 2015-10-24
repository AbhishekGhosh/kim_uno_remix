
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <sys/types.h>
#include <stdint.h>

/* Interface.c
 *
 * All of the emulator interface routines, but decoupled from the user interface
 * to make this more easily portable to other systems in the future
 */

#include "../sources/KimUnoRemix/config.h"

/* over in cpu.c and memory.c */
void reset6502();
void nmi6502();
void loadProgramsToRam();
void exec6502( int nCycles );


/* serial fakeo */
void serout( uint8_t ch );
void serouthex( uint8_t ch );

/* keyboard interface stuff */
extern uint8_t curkey;

extern uint8_t serialEnable;
extern uint8_t SSTmode;

uint8_t getAkey();
void clearkey();

void interpretkeys();

uint8_t getKIMkey();
uint8_t xkeyPressed();    // just see if there's any keypress waiting

/* display fakeo */
extern char kimHex[6];        // seLED display

void driveLEDs( void );


#endif /* INTERFACE_H */
