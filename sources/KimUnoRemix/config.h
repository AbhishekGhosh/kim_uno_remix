/* Config.h
 *
 * lets you pick features to use and such for your target platform
 */

#define kVersionString "v23.20 15/11/18"
#define kVersionMajor  (23)
#define kVersionMinorA (2)
#define kVersionMinorB (0)
/* 
 * Version history
 *
 * v23.20 - kDisplayWide, Text strings tweaked for better readability
 * v23.19 - Added Briel Computers' clock program, more memory for Desktop
 * v23.18 - Write to protected EEProm displays an error
 * v23.17 - Shifted key ping-pong repeat bug fixed
 * v23.16 - Improved shifted key handling (press-hold-trigger) Text display restored
 * v23.15 - Textual indication of SST and EEProm added back in
 * v23.14 - Keypad code converted to the new method, Runtime bug squashed
 * v23.13 - PROGMEM for Keypad, LED Segments, ROM/RAM tables to minimize use. Text splashes broken
 * v23.12 - Desktop version via QT Creator, and minor changes to the main program
 * v23.11 - Changed loading of programs to RAM, programs moved to memory.c, "Mini Program B-D"
 * v23.10 - moved FONT to PROGMEM, removed ram/rom table code (low memory instability)
 * v23.9  - Updated for Arduino 1.6.0 (prog_char -> const)
 * v23.8  - Platform 2411 added (handheld), EEPROM toggle bug fixed
 * v23.7  - extracting out keys and display
 * v23.6  - popping +, popping fix
 * v23.5  - Better LED and Keypad support
 * v23.4  - Additional warnings removed from Arduino build
 * v23.3  - memory.[ch] added, initial integration (phase 1)
 * v23.2  - config.h, updated version string
 *         Baud Rate (below)
 *         Serial mode 0 keys to replace ctrl-keys for usability. In summary:
 *           'g' - GO
 *           's' - ST
 *           'r' - RS
 *           '[' ']' - SST on/off (pre-existing)
 *           'l' - AD (*L*ocation)
 *           'v' - DA (*V*alue)
 *           'p' - PC
 *           '+' - +  (pre-existing)
 *          ** note: this was temporarily removed as of 23.14 **
 *         Smart Serial added (won't stream out the display until it gets traffic)
 *         Upload warning removed with CAFE BABE
 *         Calculator and floading point library removed
 * v23.1  - CC Led, 4x6 keypad, Text display on LEDs
 * v23.0  - initial starting code from Oscar
 */

/* define on for Arduino, off for normal PC C compilers. */
/* in kimuno.ino, this only relates to some code segments that are shared with PC version */
#ifndef DESKTOP_KIM
#define AVRX
#endif

/* ************************************************************************** */
/* Platform */

/* this is for Oscar's Kim Uno "Reference" platform */
#define kPlatformKIMUno

/* this is for the Novus 750 hacked calculator */
#undef kPlatformNovus750

/* This is for the 24 key, 11 digit display handheld */
#undef kPlatform2411


/* ************************************************************************** */
/* Generic Platform Template */

#ifdef kPlatformExample
/* #define this if it's a common anode display (eg KIM Uno board) */
#define kDisplayIsCommonAnode
/* OR */
/* #define this if it's a common cathode display (eg Novus Calculator) */
#undef kDisplayIsCommonCathode 

#define kDisplayAddrOffset  (0)  /* starting segment for address  0 indexed, 4 digits, so this shows [0,1,2,3] */
#define kDisplayDataOffset  (5)  /* starting segment for data 0 indexed, so this shows [5,6] */

/* these are for optional display, eg shift indicator, separator dot */
#define kDisplayShift       (-1)  /* which digit to display the shift indicator (segment f) */
#define kDisplayDot         (-1)  /* which digit to display the dot */

/* for text display -- if defined, it uses a wider version of the display strings */
#define kDisplayWide
#endif

/* ************************************************************************** */
#ifdef kPlatformKIMUno
  /* this is for Oscar's reference KIM Uno platform */
  #define kDisplayIsCommonAnode
  #define kDisplayAddrOffset   (0) /* [0,1,2,3] */
  #define kDisplayDataOffset   (5) /* [5,6] */
  #define kDisplayShift        (4) /* shift indicator goes on [4] */
  #define kDisplayDot          (-1) /* no dot indicator */
  #define kDisplayWide

  #define CKeyThreeEight  /* original to KIM-UNO hardware */
#endif

/* ************************************************************************** */
#ifdef kPlatformNovus750
  /* this is for Scott's Novus 750 calculator hack */
  #define kDisplayIsCommonCathode
  #define kDisplayAddrOffset   (1) /* digit 0 is half digit, shift over by one [1,2,3,4] */
  #define kDisplayDataOffset   (5) /* data directly adjoins it [5,6] */
  #define kDisplayShift        (0) /* shift indicator goes on half digit */
  #define kDisplayDot          (4) /* dot separates addr from data */
  #undef kDisplayWide

  #define CKeyNovus /* Novus calculator */
#endif

/* ************************************************************************** */
#ifdef kPlatform2411
  /* this is for Scott's 24 key 11 digit calculator hack */
  #define kDisplayIsCommonCathode
  #define kDisplayAddrOffset   (0) /* digit 0 is half digit, shift over by one [1,2,3,4] */
  #define kDisplayDataOffset   (5) /* data directly adjoins it [5,6] */
  #define kDisplayShift        (4) /* shift indicator goes on [4] */
  #define kDisplayDot          (-1) /* dot separates addr from data */
  #define kDisplayWide

  #define CKeyFourSix
#endif


/* ************************************************************************** */
/* Serial */

/* Serial interface baud rate */
#define kBaudRate (9600)

/* additional debug config */
#undef DEBUGUNO


/* ************************************************************************** */
/* Keypad interface */

#define kShiftDelay (400) /* how many ms to wait until shift becomes active */

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
#define kKimScancode_STOP      (0x80) /* terminates execution, return to KIM */
#define kKimScancode_RESET     (0x81) /* total system reset, return to KIM */
#define kKimScancode_SSTON     (0x82) /* Single STep execution is ON */
#define kKimScancode_SSTOFF    (0x83) /* Single STep execution is OFF */
#define kKimScancode_SSTTOGGLE (0x84) /* Single STep Toggle */
#define kKimScancode_EEPTOGGLE (0x85) /* EEProm RW Toggle */


