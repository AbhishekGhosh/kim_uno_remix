Kim Uno Remix
=============

KIM Uno is a MOS/Commodore KIM-1 emulator for Arduino.
This code is intended for the KIM Uno PCB, but actually runs on most newer Arduinos using ATMega 328p spec or newer micros.

This "remix" takes the original KIM-1 emulation codebase from Oscar, which and changes some things around for expandability and reusability.  One thing removed is the calculator and floating point library.

Planned differences:
- Easily configurable ROM layouts (in progress)
- PC version will updated use the same source files (cpu.c, kimuno.ino) directly
- Remove ROM hooks for display and keyboard, and simulate RIOT hardware
- I2C expansion devices: (future)
  - Framebuffer/TV Out
  - PS2 Keyboard

--------
New features implemented:
	- New keys to replace ctrl-keys over serial: (mirror mode)
		- 'g' - GO
		- 's' - ST
		- 'r' - RS
		- 'l' - AD (*L*ocation)
		- 'v' - DA (*V*alue)
		- 'p' - PC
	- "smart" serial - only "display" if there's serial traffic in
	- config.h set up for setting system configuration, version
	- Common cathode LED support
	- 4x6 Keypad support
	- 'shift' option for keypads smaller than 24 keys (in progress)
	- Serial baud rate configurable
	- Calculator Mode removed
	- Floating Point ROM removed
	- various cleanups

--------

Module Credits:

- Oscar Vermeulen - KIM-1 architecture and codebase
- Mike Chambers - 6502 CPU Emulator

--------

Oscar's notes:

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
