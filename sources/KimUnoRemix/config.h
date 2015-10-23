
/* Config.h
 *
 * lets you pick features to use and such
 */

#define kVersionString "v23.12 15/10/22"
#define kVersionMajor  (23)
#define kVersionMinorA (1)
#define kVersionMinorB (2)
/* 
 * Version history
 *
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

/* ****************************************************************************************** */
/* Platform */

/* this is for Oscar's Kim Uno platform */
#undef kPlatformKIMUno
/* this is for the Novus 750 hacked calculator */
#undef kPlatformNovus750
/* This is for the 24 key, 11 digit display handheld */
#define kPlatform2411

/* ****************************************************************************************** */
/* Display */

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

/* for text display -- if defined, it uses a condensed version of the display strings */
#define kDisplayCompress
#endif

#ifdef kPlatformKIMUno
  /* this is for Oscar's reference KIM Uno platform */
  #define kDisplayIsCommonAnode
  #define kDisplayAddrOffset   (0) /* [0,1,2,3] */
  #define kDisplayDataOffset   (5) /* [5,6] */
  #define kDisplayShift        (-1) /* no shift indicator */
  #define kDisplayDot          (-1) /* no dot indicator */

  #undef CUseKeypadLibrary
  #define CKeyThreeEight  /* original to KIM-UNO hardware */
#endif

#ifdef kPlatformNovus750
  /* this is for Scott's Novus 750 calculator hack */
  #define kDisplayIsCommonCathode
  #define kDisplayAddrOffset   (1) /* digit 0 is half digit, shift over by one [1,2,3,4] */
  #define kDisplayDataOffset   (5) /* data directly adjoins it [5,6] */
  #define kDisplayShift        (0) /* shift indicator goes on half digit */
  #define kDisplayDot          (4) /* dot separates addr from data */

  #define CUseKeypadLibrary /* all of the below require this */
  #define CKeyNovus /* Novus calculator */
#endif

#ifdef kPlatform2411
  /* this is for Scott's Novus 750 calculator hack */
  #define kDisplayIsCommonCathode
  #define kDisplayAddrOffset   (0) /* digit 0 is half digit, shift over by one [1,2,3,4] */
  #define kDisplayDataOffset   (5) /* data directly adjoins it [5,6] */
  #define kDisplayShift        (-1) /* shift indicator goes on half digit */
  #define kDisplayDot          (-1) /* dot separates addr from data */

  #define CUseKeypadLibrary /* all of the below require this */
  #define CKeyFourSix
#endif



/* ****************************************************************************************** */
/* Serial */

/* Serial interface baud rate */
#define kBaudRate (9600)

/* additional debug config */
#undef DEBUGUNO
