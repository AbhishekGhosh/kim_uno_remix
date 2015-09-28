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

/* fix for arduino bug (spits out a warning when nothing is wrong) */
/* ref: http://www.avrfreaks.net/forum/solution-warnings-when-using-pstr-c */
#ifndef __PGMSPACE_FIX_H__
  #define __PGMSPACE_FIX_H__
  #define PROGMEM_SECTION __attribute__(( section(".progmem.data") ))
  #undef PSTR
  #define PSTR(s) (__extension__({static char __c[] PROGMEM_SECTION = (s); &__c[0];}))
#endif  // PGMSPACE_FIX_H



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

uint8_t findAddressSegment( uint16_t address, MMAP * segents ); /* return the segment containing the requested address, 0xff if not found */
uint8_t validWrite( uint16_t address, MMAP * segments ); /* returns 1 if the segment is valid to write to */
uint8_t writeMemory( uint16_t address, uint8_t data, MMAP * segments ); /* write to RAM data byte at the specified address */
uint8_t readMemory( uint16_t address, MMAP * segments ); /* read from RAM or PROGMEM ROM */

void copyRamSegments( MMAP * segs ); /* Copy the list of segments from PROGMEM into RAM at the specified locations */


#endif /* __MEMORY_H__*/

