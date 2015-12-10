/* Interface.h
 *
 * Various interfaces between the desktop code ant the core of the emulator
 */

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <sys/types.h>
#include <stdint.h>

/* Interface.c
 *
 * All of the emulator interface routines, but decoupled from the user interface
 * to make this more easily portable to other systems in the future
 */

#include "../KimUnoRemix/config.h"

/* ************************************************************************** */
/* over in cpu.c and memory.c */
void reset6502();
void nmi6502();
void loadProgramsToRam();
void exec6502( int nCycles );
void write6502(uint16_t address, uint8_t value);
extern uint8_t serialEnable;
extern uint8_t SSTmode;



/* ************************************************************************** */
/* serial fakeo for KimUno code (stubs for now) */
void serout( uint8_t ch );
void serouthex( uint8_t ch );

/* Kim Serial - we implement these to communicate with the KIM */
/* output to terminal */
void KimSerialOut( uint8_t ch ); /* KIM outputs bytes via this */

/* input from terminal */
uint8_t KimSerialIn();    // get serial key or 0




/* ****************************************************************************************** */
/* keypad interface */
void KIMKeyPress( uint8_t keycode );


/* ****************************************************************************************** */
/* display fakeo */
extern unsigned char kimHex[6];        // LED display

void driveLEDs( void );


/* our interface to the builtin Serial console buffers */
#define kSerBufSize  (512)
extern uint8_t serOutBuf[ kSerBufSize ];
extern uint16_t serOutBufPos;
extern uint8_t serInBuf[ kSerBufSize ];
extern uint16_t serInBufPos;

void KimSerialReset( void );

void KimSerialOut( uint8_t ch );

uint8_t KimSerialIn();
void KimSerialInPush( uint8_t ch );
void KimSerialClearIn();

#endif /* INTERFACE_H */
