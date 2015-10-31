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



// --- OVERVIEW OF KIM-1 MEMORY MAP -------------------------------------------------
extern uint8_t RAM[1024];    // main 1KB RAM         0x0000-0x03FF
#ifndef AVRX
extern uint8_t RAM2[1024];   // on PC, ram in        0x0400-0x07FF instead of Arduino EEPROM
#endif 

// I/O and timer of 6530-003, free for user          0x1700-0x173F, not used in KIM ROM
// I/O and timer of 6530-002, used by KIM            0x1740-0x177F, used by LED/Keyboard
extern uint8_t RAM003[64];    // RAM from 6530-003   0x1780-0x17BF, free for user applications
extern uint8_t RAM002[64];    // RAM from 6530-002   0x17C0-0x17FF, free for user except 0x17E7-0x17FF

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

