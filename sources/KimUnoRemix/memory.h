/* memory.h
 *
 *  encapsulate memory access, ROM and RAM code. (ROM read directly from PROGMEM on AVR)
 */

#include <stdint.h>
#include <avr/pgmspace.h>

/* the basic idea is:
  ROM - data contained in PROGMEM
  RAM - data contained in RAM array
 */


typedef struct MMAP {
  uint16_t aStart;
  uint16_t aSize;
  prog_uchar * buffer;
} MMAP;

#define kEndOfSegments (0xFFFF)

////////////////////////
// RAM and ROM space

#define kNotFound  (0xff)

uint8_t findAddressSegment( uint16_t address, MMAP * segents ); /* return the segment containing the requested address, 0xff if not found */
uint8_t validWrite( uint16_t address, MMAP * segments ); /* returns 1 if the segment is valid to write to */
uint8_t writeMemory( uint16_t address, uint8_t data, MMAP * segments ); /* write to RAM data byte at the specified address */
uint8_t readMemory( uint16_t address, MMAP * segments ); /* read from RAM or PROGMEM ROM */

uint8_t copyRamSegments( MMAP * segs ); /* Copy the list of segments from PROGMEM into RAM at the specified locations */
