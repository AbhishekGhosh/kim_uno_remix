/* Interface.c
 *
 * All of the emulator interface routines, but decoupled from the user interface
 * to make this more easily portable to other systems in the future
 */

#include "interface.h"

/* ************************************************** */
/* generic serial fakeo (we implement these) */
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
void driveLEDs( void )
{
    /* trigger the LED update */
}
