/* memory.h
 *
 *  encapsulate memory access, ROM and RAM code. (ROM read directly from PROGMEM on AVR)
 */

#include <Arduino.h>
#include <stdint.h>

#ifndef AVRX
#include <avr/pgmspace.h>
#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__

/* the basic idea is:
  ROM - data contained in PROGMEM
  RAM - data contained in RAM array
 */

// NOTE: This doesn't seem to work in practice due to the added calls requiring RAM we don't have
// This needs to be reworked/removed.
 
/* our Memory Map structure */
typedef struct MMAP {
  uint16_t aStart;
  uint16_t aSize;
  unsigned char * buffer;
} MMAP;

#define kEndOfSegments (0xFFFF)

////////////////////////
// RAM and ROM space

#define kNotFound  (0xff)

uint8_t xfindAddressSegment( uint16_t address, MMAP * segents ); /* return the segment containing the requested address, 0xff if not found */
uint8_t xvalidWrite( uint16_t address, MMAP * segments ); /* returns 1 if the segment is valid to write to */
uint8_t xwriteMemory( uint16_t address, uint8_t data, MMAP * segments ); /* write to RAM data byte at the specified address */
uint8_t xreadMemory( uint16_t address, MMAP * segments ); /* read from RAM or PROGMEM ROM */

void copyRamSegments( MMAP * segs ); /* Copy the list of segments from PROGMEM into RAM at the specified locations */


#endif /* __MEMORY_H__*/

