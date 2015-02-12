
/* Config.h
 *
 * lets you pick features to use and such
 */

#define kVersionString "v23.7 Feb 12" // be sure to change kDt_Uno also
/* 
 * Version history
 *
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
/* Display */

/* #define this if it's a common anode display (base KIM Uno board) */
#define kDisplayIsCommonAnode
/* OR */
/* #define this if it's a common cathode display (Novus) */
#undef kDisplayIsCommonCathode 

/*   Novus uses 1  5  0  4 */
/* KIM UNO uses 0  5 -1 -1 */
#define kDisplayAddrOffset  (0)  /* starting segment for address (0 indexed) */
#define kDisplayDataOffset  (5)  /* starting segment for data 0 indeced */

/* these are for optional display, eg shift indicator, separator dot */
#define kDisplayShift       (-1)  /* where to display the shift */
#define kDisplayDot         (-1)  /* where to display the dot */


/* ****************************************************************************************** */
/* Keypad */

/* are we using the stock scanning (3x8) or the keypad library (4x6) */
#define CKeyThreeEight  /* original to KIM-UNO hardware */

#undef CUseKeypadLibrary /* all of the below require this */
#undef CKeyFourSix
#undef CKeyNovus /* Novus calculator */


/* ****************************************************************************************** */
/* Serial */

/* Serial interface baud rate */
#define kBaudRate (9600)

/* additional debug config */
#undef DEBUGUNO

