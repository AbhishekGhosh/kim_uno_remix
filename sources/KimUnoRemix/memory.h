/* memory.h
 *
 *  encapsulate memory access, ROM and RAM code.
 * (ROM read directly from PROGMEM on AVR)
 */

#include <Arduino.h>
#include "config.h"
#include <stdint.h>
#ifndef DESKTOP_KIM
#include <avr/pgmspace.h>
#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__

/******************************************************************************/
/* --- OVERVIEW OF KIM-1 MEMORY MAP --------------------------------------------
 *
 * $0000 - $03FF    1024  RAM[] (AVR)
 * $0400 - $07FF    1024  EEPROM (AVR)
 *  or:
 * $0000 - $0FFF    4096  RAM[] (Desktop)
 *
 * $1700 - $173F          6530-003 - IO and timer, available for user
 * $1740 - $177F          6530-002 - IO and timer, used for KIM, LED, Keyboard
 * $1780 - $17BF    64    6530-003 - RAM003[] RAM available for user
 * $17C0 - $17FF    64    6530-002 - RAM002[] RAM available for user ($17E7-$17FF used by KIM)
 * $1800 - $1BFF    1024  6530-003 - KIM ROM
 * $1C00 - $1FFF    1024  6530-002 - KIM ROM
 *
 * $FFFA - $FFFF          ROM 002 Vectors (copied)
 *
 * "Loaded in" ROMs:
 * $C000 -                MicroChess (www.benlo.com/microchess)
 * $D000 -                (FP/Calculator (www.crbond.com/calc65.htm))
 * $2000 -                Baum & Wozniak disassember (6502.org/documents/publications/6502notes/6502_user_notes_14.pdf)
 *
 */

/* possible video/color buffer
	based on http://www.6502asm.com

	Their frame buffer sits at $200-$5FF
	Ours will be at $3000 - $33FF 

	It generates a 32x32 framebuffer, each pixel is one byte
	starting in the upper right going down to the lower left

	color table: (from the C64)
	$0 black	$8 orange
	$1 white	$9 brown
	$2 red		$A light red
	$3 cyan		$B dark gray
	$4 purple	$C gray
	$5 green	$D light green
	$6 blue		$E light blue
	$7 yellow	$F light gray

	(this can be expanded later to be a 32x32 tile buffer, with
	 a secondary character buffer.  then the upper nibble can define
	 the character color.)
*/

/* possible Interface "chip" API

	One other thing that may be useful is a utility API with 
	various interfaces to the real world.  Let's put this at $F000
	This is something that could be created in real hardware too

	; interface versioning
  $F000   (read) Version Major (eg 0x23)
  $F001   (read) Version Minor (eg 0x17)
  $F002 (read) system
    0x00 Desktop Emulation
    0x01 Arduino AVR
	$F00F	capabilities
		0x01 random values
		0x02 realtime clock
		0x04 ms clock
		0x08 7 segment addressing
		0x10 analog input
    0x40 color buffer
    0x80 character buffer

	; random value
	$F010 	random value (write for seed/reset), read for value

	; realtime clock
	$F021 	time: 256th second (read for value, write for offset)
	$F022	time: seconds
	$F023	time: minutes
	$F024	time: hours
	$F025	time: day
	$F026	time: month
	$F027	time: year  (2000+ $F007)

	; ms clock
	$F0F0	ms clock: byte xxxF
	$F0F1	ms clock: byte xxFx
	$F0F2	ms clock: byte xFxx
	$F0F3	ms clock: byte Fxxx

	; direct segment rendering
	$F030	7 segment display digit 0xxx xx, pattern (r/w)
	 ....
	$F035	7 segment display digit xxxx x0, pattern (r/w)
	$F03F	call here to draw the 7 segment display

	; AVR analog inputs
	$F040	0..255 - value in A0
	....
	$F045	0..255 - value in A5
 */

extern uint8_t RAM[];
extern uint8_t RAM003[];
extern uint8_t RAM002[];

extern const unsigned char rom002[1024] PROGMEM;  // 0x1c00
extern const unsigned char rom002Vectors[6] PROGMEM; // 0xfffA
extern const unsigned char rom003[1024] PROGMEM;  // 0x1800
extern const unsigned char mchess[1393] PROGMEM;  // 0xC000
extern const unsigned char disasm[505] PROGMEM;   // 0x2000

/******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
typedef struct MMAP {
    uint16_t addr;
    uint16_t len; /* the final element in the list has len = 0 */
    uint8_t flags;
    const uint8_t * data;
} MMAP;

#define kMMAP_PROGMEM  (0x01) /* data is a PROGMEM pointer */
#define kMMAP_RAM      (0x02) /* data is a RAM pointer */
#define kMMAP_EEPROM   (0x04) /* data is in EEPROM */
#define kMMAP_END      (0x80) /* end of structure/array  */

extern const struct MMAP PROGMEM MemoryReadSegments[];  /* RAM and ROM bits */
extern const struct MMAP PROGMEM MemoryWriteSegments[]; /* RAM bits */
extern const struct MMAP PROGMEM CopyToRamSegments[]; /* RAM bits */

void loadProgramsToRam( void );

#endif /* __MEMORY_H__*/

