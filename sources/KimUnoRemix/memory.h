/* memory.h
 *
 *  encapsulate memory access, ROM and RAM code. (ROM read directly from PROGMEM on AVR)
 */

#include <Arduino.h>
#include "config.h"
#include <stdint.h>
#ifndef DESKTOP_KIM
#include <avr/pgmspace.h>
#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__

/*********************************************************************************/
/* --- OVERVIEW OF KIM-1 MEMORY MAP ----------------------------------------------
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

extern uint8_t RAM[];
extern uint8_t RAM003[];
extern uint8_t RAM002[];

extern const unsigned char rom002[1024] PROGMEM;  // 0x1c00
extern const unsigned char rom002Vectors[6] PROGMEM; // 0xfffA
extern const unsigned char rom003[1024] PROGMEM;  // 0x1800
extern const unsigned char mchess[1393] PROGMEM;  // 0xC000
extern const unsigned char disasm[505] PROGMEM;   // 0x2000

/*********************************************************************************/

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

