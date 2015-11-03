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

#include "../sources/KimUnoRemix/config.h"

/* ************************************************************************** */
/* over in cpu.c and memory.c */
void reset6502();
void nmi6502();
void loadProgramsToRam();
void exec6502( int nCycles );
extern uint8_t serialEnable;
extern uint8_t SSTmode;


/* ************************************************************************** */
/* Keypad interface */

/* these are the codes that the keypad interface will use to pass
 * back through the emulation */

/* these are just handed back */
#define kKimScancode_0      (0x00)
#define kKimScancode_1      (0x01)
#define kKimScancode_2      (0x02)
#define kKimScancode_3      (0x03)
#define kKimScancode_4      (0x04)
#define kKimScancode_5      (0x05)
#define kKimScancode_6      (0x06)
#define kKimScancode_7      (0x07)
#define kKimScancode_8      (0x08)
#define kKimScancode_9      (0x09)
#define kKimScancode_A      (0x0A)
#define kKimScancode_B      (0x0B)
#define kKimScancode_C      (0x0C)
#define kKimScancode_D      (0x0D)
#define kKimScancode_E      (0x0E)
#define kKimScancode_F      (0x0F)

#define kKimScancode_ADDR   (0x10) /* selects the address entry mode */
#define kKimScancode_DATA   (0x11) /* selects the data entry mode */
#define kKimScancode_PLUS   (0x12) /* increments address by 1 */
#define kKimScancode_GO     (0x13) /* start program execution on the current addr */
#define kKimScancode_PC     (0x14) /* recalls the address in the Program Counter */
#define kKimScancode_none   (0x15) /* illegal/no press */

/* these perform special tasks, functions below */
#define kKimScancode_STOP   (0x80) /* terminates execution, return to KIM */
#define kKimScancode_RESET  (0x81) /* total system reset, return to KIM */
#define kKimScancode_SSTON  (0x82) /* Single STep execution is ON */
#define kKimScancode_SSTOFF (0x83) /* Single STep execution is OFF */


/* move these somewhere else later */
/* ST - throw NMI to stop execution */
#define pressed_STop()   nmi6502()

/* RS - hardware reset */
#define pressed_ReSet()  reset6502()

/* SSTOn - turn on Single STep */
#define pressed_SSTOn()  (SSTmode = 1)

/* SSTOff - turn off Single STep */
#define pressed_SSTOff() (SSTmode = 0)

/* as the OS dependant part, we need to implement a few functions: */
uint8_t KIMKeyPressing(); /* returns 0 if nothing pressed, 1 if pressing */
uint8_t KIMKeyGet();      /* returns a kKimScancode for the currently being pressed key */
void    KIMKeyUsed();     /* tells us that key has been used */
void KIMKeyPress( uint8_t ch ); /* our code calls this to set the keycode */



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
/* display fakeo */
extern char kimHex[6];        // seLED display

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
