Kim Uno Remix
=============

KIM Uno is a MOS/Commodore KIM-1 emulator for Arduino.
This code is intended for the KIM Uno PCB, but actually runs on most newer Arduinos using ATMega 328p spec or newer micros.

This "remix" takes the original KIM-1 emulation codebase from Oscar, which and changes some things around for expandability and reusability.  Some things have been removed to make space for the additional functionality, including his calculator and floating point library.

--------
"Remixed" differences from Oscar's oroginal version:
- Broken out into multiple files (cpu.c, memory.c, keys.cpp, display.cpp)
- Easily configured for different platforms (config.h)
- Easily configurable ROM layouts (in progress)
- Easy-loaded programs to RAM (see memory.c)
- PC version will updated use the same source files (cpu.c, kimuno.ino) directly
- I2C expansion devices: (future, maybe)
  - Framebuffer/TV Out
  - PS2 Keyboard

New changes implemented:
- Serial interface
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
 - display arragement configurability
  - address and data start on specified digits
  - decimal point for very narrow displays
  - shift indicator position
- Keypad
 - customizable keypad support
 - 'shift' option for keypads smaller than 24 keys
- Misc
 - Calculator Mode removed
 - Floating Point ROM removed

--------

Module Credits:

- Oscar Vermeulen - KIM-1 architecture and codebase
- Mike Chambers - 6502 CPU Emulator

--------

Scott's notes:
- subdirectory source/KimUnoRemix contains the Arduino source code.
 - Doubleclick "KimUnoRemix.ino" to load it into the Arduino application.
 - check the platform define/undef's in config.h to set it up for your platform.
- subdirectory source/KIMUNO contains the original Arduino source code.
 - Doubleclick "KIMUNO.ino" to load it into the Arduino application.


--------

Oscar's original notes:

Please check the website for documentation and news:
http://obsolescence.wix.com/obsolescence#!kim-uno-summary/c1uuh

Notes:
- subdirectory KIMUNO contains the Arduino source code.
- subdirectory KIMPC contains a PC version used during debugging.
  it compiles under any plain C compiler and gives the same functionality as the
  Arduino version, except that the 6502 Programmable Calculator mode is more primitive.
  This is code only used during development, but maybe it's of some use to someone.
  note that the cpu.c is the same as for the Arduino version! Only the emulator wrapper is different.
- This is not elegant programming. I'm a tinkerer, not a developer. :)
