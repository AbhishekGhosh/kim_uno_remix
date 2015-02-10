
/* Config.h
 *
 * lets you pick features to use and such
 */

#define kVersionString "v23.6 Feb 9" // be sure to change kDt_Uno also
/* 
 * Version history
 *
 v v23.6 - popping +, popping fix
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

/* are we using the stock Common Anode LED or the 11 digit Common Cathode display? */
/* NOTE: not updated for calculator mode */
#undef CLedCommonAnode  /* original to KIM-UNO hardware */
#undef CLedCommonCathode
#define CLedNovus        /* novus 750 6.5 digit driver */

/* are we using the stock scanning (3x8) or the keypad library (4x6) */
#undef CKeyThreeEight  /* original to KIM-UNO hardware */

#undef CUseKeypadLibrary /* all of the below require this */
#undef CKeyFourSix
#define CKeyNovus /* Novus calculator */

/* Serial interface baud rate */
#define kBaudRate (9600)

/* additional debug config */
#undef DEBUGUNO

