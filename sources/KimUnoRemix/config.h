
/* Config.h
 *
 * lets you pick features to use and such
 */

#define kVersionString "v23.8 Feb 12" // be sure to change kDt_Uno also
#define kVersionMajor  (23)
#define kVersionMinorA (0)
#define kVersionMinorB (7)
/* 
 * Version history
 *
 * v23.8 - 
 * v23.7 - extracting out keys and display
 * v23.6 - popping +, popping fix
 * v23.5 - Better LED and Keypad support
 * v23.4 - Additional warnings removed from Arduino build
 * v23.3 - memory.[ch] added, initial integration (phase 1)
 * v23.2 - config.h, updated version string
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
 * v23.1 - CC Led, 4x6 keypad, Text display on LEDs
 * v23.0 - initial starting code from Oscar
 */

/* define on for Arduino, off for normal PC C compilers. */
/* in kimuno.ino, this only relates to some code segments that are shared with PC version */
#define AVRX

/* ****************************************************************************************** */
/* Platform */

/* this is for Oscar's Kim Uno platform */
#define kPlatformKIMUno
#undef kPlatformNovus750

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
#endif

#ifdef kPlatformNovus750
  /* this is for Scott's Novus 750 calculator hack */
  #define kDisplayIsCommonCathode
  #define kDisplayAddrOffset   (1) /* digit 0 is half digit, shift over by one [1,2,3,4] */
  #define kDisplayDataOffset   (5) /* data directly adjoins it [5,6] */
  #define kDisplayShift        (0) /* shift indicator goes on half digit */
  #define kDisplayDot          (4) /* dot separates addr from data */
#endif


/* ****************************************************************************************** */
/* Keypad */

/* are we using the stock scanning (3x8) or the keypad library (4x6) */
#undef CKeyThreeEight  /* original to KIM-UNO hardware */

#undef CUseKeypadLibrary /* all of the below require this */
#undef CKeyFourSix
#define CKeyNovus /* Novus calculator */


/* ****************************************************************************************** */
/* Serial */

/* Serial interface baud rate */
#define kBaudRate (9600)

/* additional debug config */
#undef DEBUGUNO

