/*
 * Arduino.h
 *
 * Pseudo-replacement for some Arduinoish stuff so that we can include
 * some of the core of the emulator without problems.
 *
 * Those files include Arduino.h so we might as well put in platform
 * specific stuff in here.  Mainly replacements for the AVR PROGMEM
 * functionality and Define the DESKTOP_KIM variable
 */

#ifndef ARDUINO_H
#define ARDUINO_H

/* tell the code we're desktop */
#define DESKTOP_KIM (1)

/* null-out the PROGMEM directive */
#define PROGMEM /* */

/* and convert the read-byte-near function to be a direct reference */
#define pgm_read_byte_near( A ) (*(A))

#endif // ARDUINO_H
