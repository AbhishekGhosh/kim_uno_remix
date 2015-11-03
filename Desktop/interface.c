/* Interface.c
 *
 * All of the emulator interface routines, but decoupled from the user interface
 * to make this more easily portable to other systems in the future
 */

#include "interface.h"

/* ************************************************** */
/* over in cpu.c and memory.c */
void reset6502();
void nmi6502();
void loadProgramsToRam();
void exec6502( int nCycles );


/* ************************************************** */
/* generic serial fakeo */
void serout( uint8_t ch )
{
    ch = ch;
    //printf( "%c", ch );
}

void serouthex( uint8_t ch )
{
    ch = ch;
    //printf( "%01x", ch );
}


/* ************************************************** */
/* Keypad support */

/* as the OS dependant part, we need to implement a few functions: */
uint8_t KIMKeyPressing(); /* returns 0 if nothing pressed, 1 if pressing */
uint8_t KIMKeyGet();      /* returns a kKimScancode for the currently being pressed key */
void    KIMKeyUsed();     /* tells us that key has been used */

static uint8_t pressingKey = kKimScancode_none;

/* we're notifying ourselves here through this valve that something was pressed */
void KIMKeyPress( uint8_t ch )
{
    /* initialize with an empty scancode */
    pressingKey = kKimScancode_none;

    /* check for a special function */
    switch( ch ) {
    case( kKimScancode_STOP ):   pressed_STop(); break;
    case( kKimScancode_RESET ):  pressed_ReSet(); break;
    case( kKimScancode_SSTON ):  pressed_SSTOn(); break;
    case( kKimScancode_SSTOFF ): pressed_SSTOff(); break;
    default:        
        /* If the code was in the lower range, store it! */
        if( ch < kKimScancode_none ) {
            pressingKey = ch;
        }
    }
}

/* return 1 if there was a press, otherwise 0 */
uint8_t KIMKeyPressing()
{
    if( pressingKey == kKimScancode_none) return 0;
    return 1;
}

/* get the currently pressed key */
uint8_t KIMKeyGet()
{
    return pressingKey;
}

/* notifying us that the key has been "used" */
void KIMKeyUsed()
{
    pressingKey = kKimScancode_none;
}


/* ************************************************** */
/* serial interface */

uint8_t serOutBuf[ kSerBufSize ];
uint16_t serOutBufPos;
uint8_t serInBuf[ kSerBufSize ];
uint16_t serInBufPos;

void KimSerialReset( void )
{
    serOutBufPos = 0;
    serInBufPos = 0;
}

void KimSerialOut( uint8_t ch )
{
    if( serOutBufPos >= kSerBufSize ) return; // lose it.
    serOutBuf[serOutBufPos] = ch;
    serOutBufPos++;
}

// get serial key or 0
uint8_t KimSerialIn()
{
    //printf( "KimSerialIn\n" ); fflush( 0 );
    if( serInBufPos == 0 ) return 0x00;
    return serInBuf[ 0 ];
}

void KimSerialInPush( uint8_t ch )
{
    if( serInBufPos >= kSerBufSize ) return;
    serInBuf[ serInBufPos ] = ch;
    serInBufPos++;
    serInBuf[ serInBufPos ] = '\0';
    //printf( "Kim input serial buffer is [%d](%s)", serInBufPos, serInBuf );
    //fflush( 0 );
}

void KimSerialClearIn() // clearkey()
{
    int j;
    //printf( "KimSerialClearIn\n" ); fflush( 0 );
    if( serInBufPos == 0 ) return;
    for( j=0 ; j < kSerBufSize-1 ; j++ ){
        serInBuf[j] = serInBuf[j+1];
    }
}


/* display interface */
char kimHex[6];        // seLED display

void driveLEDs( void )
{
    /* trigger the LED update */
}
