// ARDSIM.H
//
//	Include all the headers for the Arduino simulation stuff

#ifndef __ARDSIM_H__
#define __ARDSIM_H__

// EEPROM simuator
#include "eesim.h"

// Data line IO
#include "iosim.h"

// Misc stuff - Delay/timer, random, etc.
#include "miscsim.h"

// Serial IO
#include "sersim.h"

// Binary definitions
#include "binary.h"


// simulate ATMega 328p type device
#ifndef FLASHEND
#define FLASHEND (0x0fff)
#endif

#ifndef RAMEND
#define RAMEND (0x0fff)
#endif

// boolean datatype
#ifndef boolean
typedef bool boolean;
#endif

// byte datatype
typedef unsigned char byte;
typedef unsigned char prog_uchar;

// to account for the progmem shortcut
#define F(S) (S)

#endif
