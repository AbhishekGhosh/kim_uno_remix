#ifndef ARDUINO_H
#define ARDUINO_H

#define DESKTOP_KIM (1)

#define PROGMEM /* - */

//uint8_t pgm_read_byte_near( uint8_t * addr );

#define pgm_read_byte_near( A ) (*(A))

#endif // ARDUINO_H
