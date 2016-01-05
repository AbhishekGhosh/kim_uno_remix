Kim Uno Remix
=============

KIM Uno Remix is a MOS/Commodore KIM-1 emulator for Arduino and
Desktop platforms.  The core of this code was originally created
by Oscar Vermeulen for his KIM Uno PCB kit, which emulates a
MOS/Commodore KIM-1 single board computer using an ATmega 328p AVR
micro. It provides 24 pushbutton keys and red seven-segment LED
displays.  Kim Uno Remix will run on any modern Arduino (for the
micro build version) as well as Windows/Mac/Linux (for the QT
version).

This "remix" takes Oscar's original KIM-1 emulation codebase, and
changes some things around for expandability, reusability, and
portability.  Some things have been removed to make space for the
additional functionality, including the calculator and floating
point library.  Some things have been added such as support for other
platforms, and ease of expandability.

Best part is, it's a remix. If you don't want any of this, you can
always just go back to Oscar's code if you'd like. ;)

--------
"Remixed" differences from Oscar's version:
- Broken out into multiple files (eg cpu.c, memory.c, keys.cpp, display.cpp)
- Easily configured for different platforms (config.h)
- Easily configurable ROM layouts (see memory.c)
- Easy-loaded programs to RAM (see memory.c)
- PC version uses the same core cpu engine and ROM data
- PC version adds more "RAM" for the emulated target

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
- Utility data 
 - Reads from zero page $FE give a random number ($00..$FF)
 - Writes to $FE will set the random seed ($00..$FF)
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

Desktop notes:

The desktop version uses QT for the OS interface and GUI widgets and 
things. It has a few differences over the AVR version

- Keyboard shortcuts for all main-screen buttons:
 - 0..9, a..f - hex input buttons
 - G, S, R, T - press "GO", "ST", "RS", "SST" buttons respectively
 - A, D, P - press "AD", "DA', "PC" respectively
 - [enter] - press "+"

- Peripherals / Serial Console
 - This behaves like a serial terminal hooked up to the KIM-1.
 - It is currently incomplete.

- Peripherals / Video Display
 - This is a 32x32 pixel display which resides at $4000
 - one byte per pixel, only the lower nibble is used 0x00 - 0x0F
 - each pixel can be one of 16 colors from the Commodore 64 palette
 - Experimental right now, as it is VERY slow

- Emulation / Code Drop
 - Load CC65 .lst files directly into KIM memory space.
 - Other listing file formats are untested
 - Drag the .lst file to the window or browse for the one to use.
 - "Auto ADDR Seek" - hits "RS", then enters the first address in the .lst
 - "Auto GO" - does the above then presses "GO"
 - "Load to RAM" will (re)load in the .lst file.
 - Sending SIGUSR1 to the application will "press" the "Load to RAM" button
 - This makes it scriptable from your build tool or makefile. eg:

    ca65 project.asm -l project.lst
    killall -SIGUSR1 Kim

- Emulation / Memory Browser
 - Scroll through a snapshot of memory to see what's going on 
 - "Update Now" refreshes the display
 - "Automatic Updates" refreshes the display every 1/4 second


--------

License:

Oscar and Mike's code do not contain licenses, so don't sell this
or do anything like that. It would make them grumpy... me too! ;)

Any of my additional effort should be under an MIT license.  If you want
to re-use my code, for commercial or non-commercial use, please do but
give me credit.  If you do, contact Oscar and Mike to check for their
permissions as well.

--------
Credits:

- Oscar Vermeulen - KIM-1 architecture and codebase
- Mike Chambers - 6502 CPU Emulator

