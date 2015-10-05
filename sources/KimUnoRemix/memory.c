/*
 * memory
 *
 *  interface for the ROM and RAM
 */
   
#include <avr/pgmspace.h>
#include "memory.h"
#include "config.h"

////////////////////////////////////////////////////////////////////////////////

////////////////////////
// programs to copy in.
/*
MMAP copyToRam[] = {
  { 0x0200, 0x0004, buf },
  { kEndOfSegments }
};
*/

////////////////////////
// ROM

/*MMAP segments[] = {
  { 0x0000, 0x0400, buf },
  { 0x1000, 0x03ff, buf },
  { 0x1400, 0x03ff, buf },
  { kEndOfSegments }
};*/


////////////////////

#ifdef NEVER
/* return the segment containing the requested address, 0xff if not found */
uint8_t findAddressSegment( uint16_t address, MMAP * segments )
{
  uint8_t segment = 0;
  
  /* scan through all of the segments to find the first one that contains the address */
  while( segments[segment].aStart && segments[segment].aSize )
  {
    /* check if we found it */
    if(    address >= segments[segment].aStart
        && address <= (segments[segment].aStart + segments[segment].aSize-1) )
    {
      /* yup! */
      return segment;
    }
    segment++;
  }
  
  /* we couldn't find it. :( */
  return kNotFound;  
}


/* returns 1 if the segment is valid to write to */
uint8_t xvalidWrite( uint16_t address, MMAP * segments )
{
  /* find the segment */
  uint8_t segment = findAddressSegment( address, segments );
  
  /* if it wasn't found, we can't write there */
  if( segment == kNotFound ) return 0;
  
  /* if the segment is RAM, we can write there */
  //if( (segments[segment].flags & kMM_ROM) == kMM_RAM ) {
  //  return 1;
  //}
  
  /* otherwise, we can't */
  return 0;
}

/* write to RAM data byte at the specified address */
uint8_t xwriteMemory( uint16_t address, uint8_t data, MMAP * segments )
{
  uint8_t segment = findAddressSegment( address, segments );
  if( segment == kNotFound ) {
    /* error */
    return 0;
  }
  segments[segment].buffer[ address - segments[segment].aStart] = data;
  return 1;
}

/* read from RAM or PROGMEM ROM */
uint8_t xreadMemory( uint16_t address, MMAP * segments )
{
  uint8_t segment = findAddressSegment( address, segments );
  if( segment == kNotFound ) {
    /* error */
    return 0; /* not found */
  }
  
  // okay, figure out the index into the buffer
  uint16_t inset = address - segments[segment].aStart;
  // and the buffer pointer itself (necessary for pgm_read_x_near())
  const unsigned char * buf = segments[segment].buffer;

#ifdef AVRX
  return pgm_read_byte_near( buf + inset );
#else
  return buf[ inset ];
#endif
}

#endif

////////////////////

/* Copy the list of segments from PROGMEM into RAM at the specified locations */
void copyRamSegments( MMAP * segments ) 
{
  uint8_t segment = 0;
  uint16_t offs;
  if( !segments ) return;
 
  while( segments[segment].aStart != kEndOfSegments )
  {
    /* copy the data from  */
    for( offs = 0 ; offs < segments[segment].aSize ; offs++ ) {
      writeMemory( segments[segment].aStart+offs, pgm_read_byte_near( segments[segment].buffer[offs] ), segments);
    }
    segment++;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

