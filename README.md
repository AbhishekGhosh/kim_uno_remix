Kim Uno Remix
=============

KIM Uno Remix is a MOS/Commodore KIM-1 emulator for Arduino and
Desktop platforms.  This code was originally created by Oscar
Vermeulen for his KIM Uno PCB kit, which emulates a MOS/Commodore
KIM-1 single board computer on an ATmega 328p AVR micro, and provides
24 pushbutton keys and red LED displays.  It will run on any Arduino
(for the micro build version) as well as Windows/Mac/Linux (for the
QT version).

This "remix" takes his original KIM-1 emulation codebase from, and
changes some things around for expandability, reusability, and
portability.  Some things have been removed to make space for the
additional functionality, including the calculator and floating
point library.  Some things have been added such as support for other
platforms, and ease of expandability.

--------
"Remixed" differences from Oscar's oroginal version:
- Broken out into multiple files (eg cpu.c, memory.c, keys.cpp, display.cpp)
- Easily configured for different platforms (config.h)
- Easily configurable ROM layouts (see memory.c)
- Easy-loaded programs to RAM (see memory.c)
- PC version uses the same core engine and ROM data

New changes implemented:
- Serial interface (temporarily removed for a code shuffle in progress)
 - "smart" serial - only "display" if it detects serial traffic
 - Serial baud rate configurable
 - Serial interface usability: new keys to replace ctrl-keys:
  - 'g' - GO - go
  - 's' - ST - stop
  - 'r' - RS - reset
  - 'l' - AD - address *L*ocation
  - 'v' - DA - data *V*alue)
  - 'p' - PC - program counter
- Display
 - Common anode OR cathode LED support
 - display digit arragement configurability
  - address and data start on specified digits
  - decimal point for very narrow displays
  - shift indicator position
- Keypad
 - customizable keypad support
 - 'press-and-hold shift' option for keypads smaller than 24 keys
- Misc
 - Calculator Mode removed
 - Floating Point ROM removed

--------

File notes:

- "source/KimUnoRemix" contains the Arduino source code.
 - Doubleclick "KimUnoRemix.ino" to load it into the Arduino application.
 - check the platform define/undef's in config.h to set it up for your platform.
- "source/Desktop" contains the desktop application version.
 - Doubleclick "KIM.pro" to load it into v3+ QT Creator application.
- "Documentation" contains the three main KIM books for reference
- "source/KIM Uno 6502 ROM sources" contains various ROMs as source code
- "source/KIMUNO" contains Oscar's original code, with some additional comments
- "source/kimpc" contains an abandoned effort for a desktop build

--------

License:

Oscar and Mike's code do not contain licenses, so don't sell this
or do anything like that. It would make them grumpy... me too! ;)

--------
Credits:

- Oscar Vermeulen - KIM-1 architecture and codebase
- Mike Chambers - 6502 CPU Emulator

