/* KIM-I emulator for Arduino.
   Most of this is the 6502 emulator from Mike Chambers,
   http://forum.arduino.cc/index.php?topic=193216.0
   http://forum.arduino.cc/index.php?PHPSESSID=po7fh9f1tkuorr32n58jgofc61&action=dlattach;topic=193216.0;attach=56567
   
     KIM-I hacked in by Oscar
     http://obsolescenceguaranteed.blogspot.ch/

  Additional modification by Scott
*/

#include "Arduino.h"
#include "config.h"
#include "memory.h"
//#define USE_TIMING

#ifdef AVRX
  /* AVR/Arduino specific... */
  #ifndef NULL
    #define NULL (void *) 0
  #endif
  #include <avr/pgmspace.h>
  extern uint8_t eepromread(uint16_t eepromaddress);
  extern void eepromwrite(uint16_t eepromaddress, uint8_t bytevalue);
#else
  /* Desktop specific... */
  #include <stdio.h>
  #include <stdint.h>
  //  #pragma warning(disable : 4996) // MS VC2008 does not like unsigned char -> signed char converts.  
#endif

uint8_t KimSerialIn();
void KimSerialClearIn();
void KimSerialOut( uint8_t );


#ifdef DEBUGUNO
//--------------- debug
extern FILE *fpx;
uint16_t debugPC;
// --------------
#endif


uint8_t blitzMode = 1;		// status variable only for microchess 
// microchess status variable. 1 speeds up chess moves (and dumbs down play)

/* externally defined. */
extern void printhex(uint16_t val);
extern void serout(uint8_t value);
extern void serouthex(uint8_t val);

/* mode variables */
uint8_t useKeyboardLed=0x01; 	// set to 0 to use Serial port, to 1 to use onboard keyboard/LED display.
uint8_t iii;  					// counter for various purposes, declared here to avoid in-function delay in 6502 functions.
uint8_t nmiFlag=0; 				// added by OV to aid single-stepping SST mode on KIM-I
uint8_t SSTmode = 0; 			// SST switch in KIM-I: 1 = on.
#ifdef AVRX
extern uint8_t eepromProtect;
#endif

/* from down below */
void reset6502();
void nmi6502();


/*
 * Accuracy notes:
 *  - Interval timer $1704-$170F are not implented (p170 FBOK)
 *  - LED display is hardcoded, so KIM-1 Alphabet is not implemented (p168 FBOK)
 *
*/

/* ************************************************* */
/* Display support */

unsigned char kimHex[6]; // buffer for 3 hex digits

extern void driveLEDs();


/* ************************************************** */
/* Keypad support */

/* currently being pressed key */
static uint8_t pressingKey = kKimScancode_none;

/* weuse this valve to indicate something was pressed */
void KIMKeyPress( uint8_t ch )
{
    /* initialize with an empty scancode */
    pressingKey = kKimScancode_none;

    /* check for a special function */
    switch( ch ) {
    case( kKimScancode_STOP ):   nmi6502(); break;   /* ST - throw NMI to stop execution */
    case( kKimScancode_RESET ):  reset6502(); break; /* RS - hardware reset */
    case( kKimScancode_SSTON ):  SSTmode = 1; break; /* SSTOn - turn on Single STep */
    case( kKimScancode_SSTOFF ): SSTmode = 0; break; /* SSTOff - turn off Single STep */
    case( kKimScancode_SSTTOGGLE ):
      if( SSTmode == 1 ) SSTmode=0;
      else SSTmode = 1;
      break;
#ifdef AVRX
    case( kKimScancode_EEPTOGGLE ):
      if( eepromProtect == 1 ) eepromProtect = 0;
      else                     eepromProtect = 1;
      break;
#endif
    default:
        /* If the code was in the lower range, store it! */
        if( ch < kKimScancode_none ) {
            pressingKey = ch;
        }
    }
}

/* return 1 if there was a press, otherwise 0 */
#define KIMKeyPressing() \
  ( pressingKey != kKimScancode_none)

/* get the currently pressed key */
#define KIMKeyGet() \
  (pressingKey)

/* notifying us that the key has been "used" */
#define KIMKeyUsed() \
    pressingKey = kKimScancode_none;


/* ************************************************** */
/* CPU macros and flags */

#define FLAG_CARRY     0x01
#define FLAG_ZERO      0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL   0x08
#define FLAG_BREAK     0x10
#define FLAG_CONSTANT  0x20
#define FLAG_OVERFLOW  0x40
#define FLAG_SIGN      0x80
#define BASE_STACK     0x100

#define saveaccum(n) (a = (uint8_t)((n) & 0x00FF))

//flag modifier macros
#define setcarry() cpustatus |= FLAG_CARRY
#define clearcarry() cpustatus &= (~FLAG_CARRY)
#define setzero() cpustatus |= FLAG_ZERO
#define clearzero() cpustatus &= (~FLAG_ZERO)
#define setinterrupt() cpustatus |= FLAG_INTERRUPT
#define clearinterrupt() cpustatus &= (~FLAG_INTERRUPT)
#define setdecimal() cpustatus |= FLAG_DECIMAL
#define cleardecimal() cpustatus &= (~FLAG_DECIMAL)
#define setoverflow() cpustatus |= FLAG_OVERFLOW
#define clearoverflow() cpustatus &= (~FLAG_OVERFLOW)
#define setsign() cpustatus |= FLAG_SIGN
#define clearsign() cpustatus &= (~FLAG_SIGN)

//flag calculation macros
#define zerocalc(n) { if ((n) & 0x00FF) clearzero(); else setzero(); }
#define signcalc(n) { if ((n) & 0x0080) setsign(); else clearsign(); }
#define carrycalc(n) { if ((n) & 0xFF00) setcarry(); else clearcarry(); }
#define overflowcalc(n, m, o) { if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x0080) setoverflow(); else clearoverflow(); }


/* ************************************************** */
//6502 CPU registers
uint16_t pc;
uint8_t sp, a, x, y, cpustatus;
// BCD fix OV 20140915 - helper variables for adc and sbc
uint16_t lxx,hxx;
// end of BCD fix part 1

//helper variables
uint32_t instructions = 0; //keep track of total instructions executed
int32_t clockticks6502 = 0, clockgoal6502 = 0;
uint16_t oldpc, ea, reladdr, value, result;
uint8_t opcode, oldcpustatus, useaccum;


/* ************************************************* */
uint8_t read6502(uint16_t address) {
  uint8_t tempval = 0;

  int idx = 0;
  uint16_t addr = 0;
  uint16_t len = 0;
  uint8_t * data = NULL;
  uint8_t flags = 0;

  /* hack to fix ROM NMI Vector issue (look up table FAIL.) */
  /* due to int16 unsigned, regions filling to 0xffff will fail the loop! */
  if( address >=0xFFFA ) {
    return( pgm_read_byte_near( &rom002Vectors[ address - 0xFFFA] ));
  }

  
  do {
    addr = pgm_read_word_near( &MemoryReadSegments[idx].addr );
    len = pgm_read_word_near( &MemoryReadSegments[idx].len );
    flags = pgm_read_byte_near( &MemoryReadSegments[idx].flags );
    data = (uint8_t *)pgm_read_word_near( &MemoryReadSegments[idx].data );

    if( address >= addr && address < (addr + len) ) {
      /* this is the right block */
      if( flags & kMMAP_PROGMEM ) {
        /* PROGMEM */
        tempval = pgm_read_byte_near( data + ( address - addr ) );
      } else if( flags & kMMAP_RAM ) {
        /* RAM */
        return tempval = data[ address - addr ];
      }
#ifdef AVRX
      else if( flags & kMMAP_EEPROM ) {
        /* EEPROM.. never patched */
        return eepromread(address-addr);
      }
#endif
      else {
        return 0xFF;
      }
    }

    idx++;
  } while( !(flags & kMMAP_END) );

    // RIOT 002 patch handlers
	if (address == 0x1747) return (0xFF); // CLKRDI  =$1747,READ TIME OUT BIT,count is always complete...
	if (address == 0x1740) return (useKeyboardLed);	// returns 1 for Keyboard/LED or 0 for Serial terminal

  
    ////////////////////////////////////////
    // OUTCH -serial output byte
    if (address == 0x1EA0) // intercept OUTCH (send char to serial)
    {
      KimSerialOut(a);		// print A to serial
      pc = 0x1ED3;   // skip subroutine
      return (0xEA); // and return from subroutine with a fake NOP instruction
    }

    ////////////////////////////////////////
    // GETCH - get key from serial
    if (address == 0x1E65)	//intercept GETCH (get char from serial). used to be 0x1E5A, but intercept *within* routine just before get1 test
    {
        a = KimSerialIn();
        if (a==0) {
            pc=0x1E60;	// cycle through GET1 loop for character start, let the 6502 runs through this loop in a fake way
            return (0xEA);
        }
        KimSerialClearIn();
        x = read6502( 0x00FD );
//SDL        x = RAM[0x00FD];	// x is saved in TMPX by getch routine, we need to get it back in x;
        pc = 0x1E87;   // skip subroutine
        return (0xEA); // and return from subroutine with a fake NOP instruction
    }

    ////////////////////////////////////////
    // DETCPS 
    if (address == 0x1C2A) // intercept DETCPS
    {
        RAM002[0x17F3-0x17C0] = 1;  // just store some random bps delay on TTY in CNTH30
        RAM002[0x17F2-0x17C0] = 1;	// just store some random bps delay on TTY in CNTL30
        pc = 0x1C4F;    // skip subroutine
        return (0xEA); // and return from subroutine with a fake NOP instruction
    }

    ////////////////////////////////////////
    // SCANDS - display F9 FA FB to LEDs
    if (address == 0x1F1F) // intercept SCANDS (display F9,FA,FB)
    {
        // light LEDs ---------------------------------------------------------
        kimHex[0]= (RAM[0x00FB] & 0xF0) >> 4;
        kimHex[1]= RAM[0x00FB] & 0x0F;
        kimHex[2]= (RAM[0x00FA] & 0xF0) >> 4;
        kimHex[3]= RAM[0x00FA] & 0x0F;
        kimHex[4]= (RAM[0x00F9] & 0xF0) >> 4;
        kimHex[5]=  RAM[0x00F9] & 0x0F;
#ifdef NEVER_SEROUT
        //#ifndef AVRX      // remove this line to get led digits on serial for AVR too
        serout(13); serout('>');
        serouthex( kimHex[0] );
        serouthex( kimHex[1] );
        serouthex( kimHex[2] );
        serouthex( kimHex[3] );
        serout( ' ' );
        serouthex( kimHex[4] );
        serouthex( kimHex[5] );
        serout('<'); serout( 13 );
#endif
        //#endif          // remove this line to get led digits on serial for AVR too
        driveLEDs();

        pc = 0x1F45;    // skip subroutine part that deals with LEDs
        return (0xEA); // and return a fake NOP instruction for this first read in the subroutine, it'll now go to AK
    }
    
    ////////////////////////////////////////
    // AK  - key pressed?
    if (address == 0x1EFE) // intercept AK (check for any key pressed)
    {
        a=KIMKeyPressing();	 // 0 means no key pressed - the important bit - but if a key is pressed is curkey the right value to send back?
        if (a==0) a=0xFF; // that's how AK wants to see 'no key'
        pc = 0x1F14;    // skip subroutine
        return (0xEA); // and return a fake NOP instruction for this first read in the subroutine, it'll now RTS at its end
    }

    ////////////////////////////////////////
    // GETKEY - get key from keypad
    if (address == 0x1F6A) // intercept GETKEY (get key from keyboard)
    {
          a = KIMKeyGet();  // get the key
          KIMKeyUsed();     // notify that we've used it
          pc = 0x1F90;    // skip subroutine part that deals with LEDs
          return (0xEA);  // and return a fake NOP instruction for this first read in the subroutine, it'll now RTS at its end
     }
  
    // I/O functions just for Microchess: ---------------------------------------------------
    // $F003: 0 = no key pressed, 1 key pressed
    // $F004: input from user
    // (also, in write6502: $F001: output character to display)
    if (address == 0xCFF4) 					//simulated keyboard input
    {
        tempval = KIMKeyGet();
        KIMKeyUsed();
        // translate KIM-1 button codes into ASCII code expected by this version of Microchess
        switch (tempval)
        {
            case 16:  tempval = 'P';  break;    // PC translated to P
            case 'F':  tempval = 13;  break;    // F translated to Return
            case '+': tempval = 'W'; break;    // + translated to W meaning Blitz mode toggle
        }
#ifdef NEVER_SEROUT
        if (tempval==0x57) // 'W'. If user presses 'W', he wants to enable Blitz mode.
        {
            if (blitzMode==1) (blitzMode=0);
            else              (blitzMode=1);
            serout('>'); serout( (blitzMode==1)?'B':'N' );	serout('<');
        }
#endif
        return(tempval);
    }
  
    if (address == 0xCFF3) 					//simulated keyboard input 0=no key press, 1 = key press
    {
        // light LEDs ---------------------------------------------------------
        kimHex[0]= (RAM[0x00FB] & 0xF0) >> 4;
        kimHex[1]= RAM[0x00FB] & 0xF;
        kimHex[2]= (RAM[0x00FA] & 0xF0) >> 4;
        kimHex[3]= RAM[0x00FA] & 0xF;
        kimHex[4]= (RAM[0x00F9] & 0xF0) >> 4;
        kimHex[5]= RAM[0x00F9] & 0xF;
        #ifdef AVRX
        driveLEDs();
        #endif

        return(KIMKeyPressing()==0?(uint8_t)0:(uint8_t)1);
    }

#ifdef EXPAPI
    // handlers for the KUR API "Chip"
    if( address == 0xF000 ) return( kVersionMajor );
    if( address == 0xF001 ) return( (kVersionMinorA <<4) + kVersionMinorB );
#ifndef AVRX
    if( address == 0xF002 ) return( 0x00 );
#else
    if( address == 0xF002 ) return( 0x01 );
    if( address == 0xF00F ) return( 0x01 /* rnd */ | 0x04 /* ms clock */ );
    if( address == 0xF0F0 ) return( millis() & 0x000F );
    if( address == 0xF0F1 ) return( (millis() & 0x00F0) >>4  );
    if( address == 0xF0F2 ) return( (millis() & 0x0F00) >>8  );
    if( address == 0xF0F3 ) return( (millis() & 0xF000) >>12 );
#endif
    if( address == 0xF010 ) return( random() & 0x00FF );
#endif
    return( tempval );
}


void write6502(uint16_t address, uint8_t value) 
{
  int idx = 0;
  uint16_t addr = 0;
  uint16_t len = 0;
  uint8_t * data = NULL;
  uint8_t flags = 0;

  /* patch first */
  if (address == 0xCFF1) {
      // Character out for microchess only
      KimSerialOut( value );
      return;
  }

  do {
    addr = pgm_read_word_near( &MemoryWriteSegments[idx].addr );
    len = pgm_read_word_near( &MemoryWriteSegments[idx].len );
    flags = pgm_read_byte_near( &MemoryWriteSegments[idx].flags );
    data = (uint8_t *)pgm_read_word_near( &MemoryWriteSegments[idx].data );

    if( address >= addr && address < addr + len ) {
      /* this is the right block */
      if( flags & kMMAP_RAM ) {
        /* RAM */
        data[ address - addr ] = value;
      }
#ifdef AVRX
      else if( flags & kMMAP_EEPROM ) {
        /* EEPROM */
          eepromwrite( address-addr, value );
      }
#endif
    }

    idx++;
  } while( !(flags & kMMAP_END) );

#ifdef EXPAPI
  if( address == 0xF010 ) randomSeed( value );
#endif
}



/* ***************************************************************** */


//a few general functions used by various other functions
void push16(uint16_t pushval) {
    write6502(BASE_STACK + sp, (pushval >> 8) & 0xFF);
    write6502(BASE_STACK + ((sp - 1) & 0xFF), pushval & 0xFF);
    sp -= 2;
}

void push8(uint8_t pushval) {
    write6502(BASE_STACK + sp--, pushval);
}

uint16_t pull16() {
    uint16_t temp16;
    temp16 = read6502(BASE_STACK + ((sp + 1) & 0xFF)) | ((uint16_t)read6502(BASE_STACK + ((sp + 2) & 0xFF)) << 8);
    sp += 2;
    return(temp16);
}

uint8_t pull8() {
    return (read6502(BASE_STACK + ++sp));
}

void reset6502() {
    pc = (uint16_t)read6502(0xFFFC) | ((uint16_t)read6502(0xFFFD) << 8);

    a = 0;
    x = 0;
    y = 0;
    sp = 0xFD;
    cpustatus |= FLAG_CONSTANT;
}


//addressing mode functions, calculates effective addresses
void imp() { //implied
}

void acc() { //accumulator
  useaccum = 1;
}

void imm() { //immediate
    ea = pc++;
}

void zp() { //zero-page
    ea = (uint16_t)read6502((uint16_t)pc++);
}

void zpx() { //zero-page,X
    ea = ((uint16_t)read6502((uint16_t)pc++) + (uint16_t)x) & 0xFF; //zero-page wraparound
}

void zpy() { //zero-page,Y
    ea = ((uint16_t)read6502((uint16_t)pc++) + (uint16_t)y) & 0xFF; //zero-page wraparound
}

void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
    reladdr = (uint16_t)read6502(pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
}

void abso() { //absolute
    ea = (uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8);
    pc += 2;
}

void absx() { //absolute,X
    /* COMPILER WARNING FIX
    uint16_t startpage;
    */
    ea = ((uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8));
    /*
    startpage = ea & 0xFF00;
    */
    ea += (uint16_t)x;

    pc += 2;
}

void absy() { //absolute,Y
    /* COMPILER WARNING FIX
    uint16_t startpage;
    */
    ea = ((uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8));
    /*
    startpage = ea & 0xFF00;
    */
    ea += (uint16_t)y;

    pc += 2;
}

void ind() { //indirect
    uint16_t eahelp, eahelp2;
    eahelp = (uint16_t)read6502(pc) | (uint16_t)((uint16_t)read6502(pc+1) << 8);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
    ea = (uint16_t)read6502(eahelp) | ((uint16_t)read6502(eahelp2) << 8);
    pc += 2;
}

void indx() { // (indirect,X)
    uint16_t eahelp;
    eahelp = (uint16_t)(((uint16_t)read6502(pc++) + (uint16_t)x) & 0xFF); //zero-page wraparound for table pointer
    ea = (uint16_t)read6502(eahelp & 0x00FF) | ((uint16_t)read6502((eahelp+1) & 0x00FF) << 8);
}

void indy() { // (indirect),Y
    uint16_t eahelp, eahelp2;
    /* COMPILER WARNING FIX
    uint16_t startpage;
    */
    eahelp = (uint16_t)read6502(pc++);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
    ea = (uint16_t)read6502(eahelp) | ((uint16_t)read6502(eahelp2) << 8);
    /*
    startpage = ea & 0xFF00;
    */
    ea += (uint16_t)y;

}

static uint16_t getvalue() {
    if (useaccum) return((uint16_t)a);
        else return((uint16_t)read6502(ea));
}

/*static uint16_t getvalue16() {
    return((uint16_t)read6502(ea) | ((uint16_t)read6502(ea+1) << 8));
}*/

void putvalue(uint16_t saveval) {
    if (useaccum) a = (uint8_t)(saveval & 0x00FF);
        else write6502(ea, (saveval & 0x00FF));
}


//instruction handler functions
void adc() {
    value = getvalue();

// BCD fix OV 20140915 - adc 
	if ((cpustatus & FLAG_DECIMAL)==0) {
    	result = (uint16_t)a + value + (uint16_t)(cpustatus & FLAG_CARRY);
   
	    carrycalc(result);
	    zerocalc(result);
 	 	overflowcalc(result, a, value);
  	 	signcalc(result);
	}    
    else // #ifndef NES_CPU
    {	 // Decimal mode
        lxx = (a & 0x0f) + (value & 0x0f) + (uint16_t)(cpustatus & FLAG_CARRY);
		if ((lxx & 0xFF) > 0x09)
            lxx += 0x06;
        hxx = (a >> 4) + (value >> 4) + (lxx > 15 ? 1 : 0);
        if ((hxx & 0xff) > 9) 
			hxx += 6;
        result = (lxx & 0x0f);
		result += (hxx << 4); 
        result &= 0xff;
		// deal with carry flag:
		if (hxx>15)
			setcarry();
		else
			clearcarry();
	    zerocalc(result);
    	clearsign();	// negative flag never set for decimal mode.
	    clearoverflow();	// overflow never set for decimal mode.
// end of BCD fix PART 2

        clockticks6502++;
    }
//    #endif // of NES_CPU

    saveaccum(result);
}

void op_and() {
    value = getvalue();
    result = (uint16_t)a & value;

    zerocalc(result);
    signcalc(result);

    saveaccum(result);
}

void asl() {
    value = getvalue();
    result = value << 1;

    carrycalc(result);
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void bcc() {
    if ((cpustatus & FLAG_CARRY) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void bcs() {
    if ((cpustatus & FLAG_CARRY) == FLAG_CARRY) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void beq() {
    if ((cpustatus & FLAG_ZERO) == FLAG_ZERO) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void op_bit() {
    value = getvalue();
    result = (uint16_t)a & value;

    zerocalc(result);
    cpustatus = (cpustatus & 0x3F) | (uint8_t)(value & 0xC0);
}

void bmi() {
    if ((cpustatus & FLAG_SIGN) == FLAG_SIGN) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void bne() {
    if ((cpustatus & FLAG_ZERO) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void bpl() {
    if ((cpustatus & FLAG_SIGN) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void brk() {
    pc++;
    push16(pc); //push next instruction address onto stack
    push8(cpustatus | FLAG_BREAK); //push CPU cpustatus to stack
    setinterrupt(); //set interrupt flag
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
}

void bvc() {
    if ((cpustatus & FLAG_OVERFLOW) == 0) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void bvs() {
    if ((cpustatus & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
        oldpc = pc;
        pc += reladdr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
            else clockticks6502++;
    }
}

void clc() {
    clearcarry();
}

void cld() {
    cleardecimal();
}

void cli6502() {
    clearinterrupt();
}

void clv() {
    clearoverflow();
}

void cmp() {
    value = getvalue();
    result = (uint16_t)a - value;

    if (a >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (a == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

void cpx() {
    value = getvalue();
    result = (uint16_t)x - value;

    if (x >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (x == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

void cpy() {
    value = getvalue();
    result = (uint16_t)y - value;

    if (y >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (y == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

void dec() {
    value = getvalue();
    result = value - 1;

    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void dex() {
    x--;

    zerocalc(x);
    signcalc(x);
}

void dey() {
    y--;

    zerocalc(y);
    signcalc(y);
}

void eor() {
    value = getvalue();
    result = (uint16_t)a ^ value;

    zerocalc(result);
    signcalc(result);

    saveaccum(result);
}

void inc() {
    value = getvalue();
    result = value + 1;

    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void inx() {
    x++;

    zerocalc(x);
    signcalc(x);
}

void iny() {
    y++;

    zerocalc(y);
    signcalc(y);
}

void jmp() {
    pc = ea;
}

void jsr() {
    push16(pc - 1);
    pc = ea;
}

void lda() {
    value = getvalue();
    a = (uint8_t)(value & 0x00FF);

    zerocalc(a);
    signcalc(a);
}

void ldx() {
    value = getvalue();
    x = (uint8_t)(value & 0x00FF);

    zerocalc(x);
    signcalc(x);
}

void ldy() {
    value = getvalue();
    y = (uint8_t)(value & 0x00FF);

    zerocalc(y);
    signcalc(y);
}

void lsr() {
    value = getvalue();
    result = value >> 1;

    if (value & 1) setcarry();
        else clearcarry();
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void nop() {
}

void ora() {
    value = getvalue();
    result = (uint16_t)a | value;

    zerocalc(result);
    signcalc(result);

    saveaccum(result);
}

void pha() {
    push8(a);
}

void php() {
    push8(cpustatus | FLAG_BREAK);
}

void pla() {
    a = pull8();

    zerocalc(a);
    signcalc(a);
}

void plp() {
    cpustatus = pull8() | FLAG_CONSTANT;
}

void rol() {
    value = getvalue();
    result = (value << 1) | (cpustatus & FLAG_CARRY);

    carrycalc(result);
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void ror() {
    value = getvalue();
    result = (value >> 1) | ((cpustatus & FLAG_CARRY) << 7);

    if (value & 1) setcarry();
        else clearcarry();
    zerocalc(result);
    signcalc(result);

    putvalue(result);
}

void rti() {
    cpustatus = pull8();
    value = pull16();
    pc = value;
}

void rts() {
    value = pull16();
    pc = value + 1;
}

void sbc() {
// BCD fix OV 20140915 - adc 
    if ((cpustatus & FLAG_DECIMAL)==0) {
	    value = getvalue() ^ 0x00FF;
	    result = (uint16_t)a + value + (uint16_t)(cpustatus & FLAG_CARRY);
   
	 	carrycalc(result);
	    zerocalc(result);
	    overflowcalc(result, a, value);
	    signcalc(result);
	}
 	else //   #ifndef NES_CPU
	{ 		// decimal mode
	    value = getvalue();
        lxx = (a & 0x0f) - (value & 0x0f) - (uint16_t)((cpustatus & FLAG_CARRY)?0:1);  
        if ((lxx & 0x10) != 0) 
			lxx -= 6;
        hxx = (a >> 4) - (value >> 4) - ((lxx & 0x10) != 0 ? 1 : 0);
        if ((hxx & 0x10) != 0) 
			hxx -= 6;
		result = (lxx & 0x0f);
		result += (hxx << 4);
		result = (lxx & 0x0f) | (hxx << 4);
		// deal with carry
		if ((hxx & 0xff) < 15)
			setcarry();			// right? I think so. Intended is   setCarryFlag((hxx & 0xff) < 15);
		else 
			clearcarry();
	    zerocalc(result); // zero dec is zero hex, no problem?
    	clearsign();	// negative flag never set for decimal mode. That's a simplification, see http://www.6502.org/tutorials/decimal_mode.html
	    clearoverflow();	// overflow never set for decimal mode.
		result = result & 0xff;
// end of BCD fix PART 3 (final part)

        clockticks6502++;
    }
//    #endif // of NES_CPU

    saveaccum(result);
}

void sec() {
    setcarry();
}

void sed() {
    setdecimal();
}

void sei6502() {
    setinterrupt();
}

void sta() {
    putvalue(a);
}

void stx() {
    putvalue(x);
}

void sty() {
    putvalue(y);
}

void tax() {
    x = a;

    zerocalc(x);
    signcalc(x);
}

void tay() {
    y = a;

    zerocalc(y);
    signcalc(y);
}

void tsx() {
    x = sp;

    zerocalc(x);
    signcalc(x);
}

void txa() {
    a = x;

    zerocalc(a);
    signcalc(a);
}

void txs() {
    sp = x;
}

void tya() {
    a = y;

    zerocalc(a);
    signcalc(a);
}

//undocumented instructions
#ifdef UNDOCUMENTED
    void lax() {
        lda();
        ldx();
    }

    void sax() {
        sta();
        stx();
        putvalue(a & x);
    }

    void dcp() {
        dec();
        cmp();
    }

    void isb() {
        inc();
        sbc();
    }

    void slo() {
        asl();
        ora();
    }

    void rla() {
        rol();
        op_and();
    }

    void sre() {
        lsr();
        eor();
    }

    void rra() {
        ror();
        adc();
    }
#else
    #define lax nop
    #define sax nop
    #define dcp nop
    #define isb nop
    #define slo nop
    #define rla nop
    #define sre nop
    #define rra nop
#endif


void nmi6502() {
    push16(pc);
    push8(cpustatus);
    cpustatus |= FLAG_INTERRUPT;
pc = (uint16_t)read6502(0xFFFA) | ((uint16_t)read6502(0xFFFB) << 8);
//pc = 0x1C1C;
}

void irq6502() {
    push16(pc);
    push8(cpustatus);
    cpustatus |= FLAG_INTERRUPT;
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
//	pc = 0x1C1F;
}

#ifdef USE_TIMING
const unsigned char ticktable[256] PROGMEM = {
/*        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |     */
/* 0 */      7,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    4,    4,    6,    6,  /* 0 */
/* 1 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 1 */
/* 2 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    4,    4,    6,    6,  /* 2 */
/* 3 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 3 */
/* 4 */      6,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    3,    4,    6,    6,  /* 4 */
/* 5 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 5 */
/* 6 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    5,    4,    6,    6,  /* 6 */
/* 7 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 7 */
/* 8 */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* 8 */
/* 9 */      2,    6,    2,    6,    4,    4,    4,    4,    2,    5,    2,    5,    5,    5,    5,    5,  /* 9 */
/* A */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* A */
/* B */      2,    5,    2,    5,    4,    4,    4,    4,    2,    4,    2,    4,    4,    4,    4,    4,  /* B */
/* C */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* C */
/* D */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* D */
/* E */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* E */
/* F */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7   /* F */
};
#endif

void exec6502(int32_t tickcount) {
#ifdef USE_TIMING
  clockgoal6502 += tickcount;

  while (clockgoal6502 > 0) {
#else
  while (tickcount--) {
#endif

#ifdef DEBUGUNO
	debugPC = pc;
	int ii=0;

#endif

// part 1 of single stepping using NMI
	if ((SSTmode==1) & (pc<0x1C00)) // no mni if running ROM code (K7), that would also single-step the monitor code!
			nmiFlag=1; // handled after this instruction has completed.
// -------------
    opcode = read6502(pc++);
    cpustatus |= FLAG_CONSTANT;

    useaccum = 0;

		switch (opcode) {
		case 0x0:
			imp();
			brk();
			break;
		case 0x1:
			indx();
			ora();
			break;
		case 0x5:
			zp();
			ora();
			break;
		case 0x6:
			zp();
			asl();
			break;
		case 0x8:
			imp();
			php();
			break;
		case 0x9:
			imm();
			ora();
			break;
		case 0xA:
			acc();
			asl();
			break;
		case 0xD:
			abso();
			ora();
			break;
		case 0xE:
			abso();
			asl();
			break;
		case 0x10:
			rel();
			bpl();
			break;
		case 0x11:
			indy();
			ora();
			break;
		case 0x15:
			zpx();
			ora();
			break;
		case 0x16:
			zpx();
			asl();
			break;
		case 0x18:
			imp();
			clc();
			break;
		case 0x19:
			absy();
			ora();
			break;
		case 0x1D:
			absx();
			ora();
			break;
		case 0x1E:
			absx();
			asl();
			break;
		case 0x20:
			abso();
			jsr();
			break;
		case 0x21:
			indx();
			op_and();
			break;
		case 0x24:
			zp();
			op_bit();
			break;
		case 0x25:
			zp();
			op_and();
			break;
		case 0x26:
			zp();
			rol();
			break;
		case 0x28:
			imp();
			plp();
			break;
		case 0x29:
			imm();
			op_and();
			break;
		case 0x2A:
			acc();
			rol();
			break;
		case 0x2C:
			abso();
			op_bit();
			break;
		case 0x2D:
			abso();
			op_and();
			break;
		case 0x2E:
			abso();
			rol();
			break;
		case 0x30:
			rel();
			bmi();
			break;
		case 0x31:
			indy();
			op_and();
			break;
		case 0x35:
			zpx();
			op_and();
			break;
		case 0x36:
			zpx();
			rol();
			break;
		case 0x38:
			imp();
			sec();
			break;
		case 0x39:
			absy();
			op_and();
			break;
		case 0x3D:
			absx();
			op_and();
			break;
		case 0x3E:
			absx();
			rol();
			break;
		case 0x40:
			imp();
			rti();
			break;
		case 0x41:
			indx();
			eor();
			break;
		case 0x45:
			zp();
			eor();
			break;
		case 0x46:
			zp();
			lsr();
			break;
		case 0x48:
			imp();
			pha();
			break;
		case 0x49:
			imm();
			eor();
			break;
		case 0x4A:
			acc();
			lsr();
			break;
		case 0x4C:
			abso();
			jmp();
			break;
		case 0x4D:
			abso();
			eor();
			break;
		case 0x4E:
			abso();
			lsr();
			break;
		case 0x50:
			rel();
			bvc();
			break;
		case 0x51:
			indy();
			eor();
			break;
		case 0x55:
			zpx();
			eor();
			break;
		case 0x56:
			zpx();
			lsr();
			break;
		case 0x58:
			imp();
			cli6502();
			break;
		case 0x59:
			absy();
			eor();
			break;
		case 0x5D:
			absx();
			eor();
			break;
		case 0x5E:
			absx();
			lsr();
			break;
		case 0x60:
			imp();
			rts();
			break;
		case 0x61:
			indx();
			adc();
			break;
		case 0x65:
			zp();
			adc();
			break;
		case 0x66:
			zp();
			ror();
			break;
		case 0x68:
			imp();
			pla();
			break;
		case 0x69:
			imm();
			adc();
			break;
		case 0x6A:
			acc();
			ror();
			break;
		case 0x6C:
			ind();
			jmp();
			break;
		case 0x6D:
			abso();
			adc();
			break;
		case 0x6E:
			abso();
			ror();
			break;
		case 0x70:
			rel();
			bvs();
			break;
		case 0x71:
			indy();
			adc();
			break;
		case 0x75:
			zpx();
			adc();
			break;
		case 0x76:
			zpx();
			ror();
			break;
		case 0x78:
			imp();
			sei6502();
			break;
		case 0x79:
			absy();
			adc();
			break;
		case 0x7D:
			absx();
			adc();
			break;
		case 0x7E:
			absx();
			ror();
			break;
		case 0x81:
			indx();
			sta();
			break;
		case 0x84:
			zp();
			sty();
			break;
		case 0x85:
			zp();
			sta();
			break;
		case 0x86:
			zp();
			stx();
			break;
		case 0x88:
			imp();
			dey();
			break;
		case 0x8A:
			imp();
			txa();
			break;
		case 0x8C:
			abso();
			sty();
			break;
		case 0x8D:
			abso();
			sta();
			break;
		case 0x8E:
			abso();
			stx();
			break;
		case 0x90:
			rel();
			bcc();
			break;
		case 0x91:
			indy();
			sta();
			break;
		case 0x94:
			zpx();
			sty();
			break;
		case 0x95:
			zpx();
			sta();
			break;
		case 0x96:
			zpy();
			stx();
			break;
		case 0x98:
			imp();
			tya();
			break;
		case 0x99:
			absy();
			sta();
			break;
		case 0x9A:
			imp();
			txs();
			break;
		case 0x9D:
			absx();
			sta();
			break;
		case 0xA0:
			imm();
			ldy();
			break;
		case 0xA1:
			indx();
			lda();
			break;
		case 0xA2:
			imm();
			ldx();
			break;
		case 0xA4:
			zp();
			ldy();
			break;
		case 0xA5:
			zp();
			lda();
			break;
		case 0xA6:
			zp();
			ldx();
			break;
		case 0xA8:
			imp();
			tay();
			break;
		case 0xA9:
			imm();
			lda();
			break;
		case 0xAA:
			imp();
			tax();
			break;
		case 0xAC:
			abso();
			ldy();
			break;
		case 0xAD:
			abso();
			lda();
			break;
		case 0xAE:
			abso();
			ldx();
			break;
		case 0xB0:
			rel();
			bcs();
			break;
		case 0xB1:
			indy();
			lda();
			break;
		case 0xB4:
			zpx();
			ldy();
			break;
		case 0xB5:
			zpx();
			lda();
			break;
		case 0xB6:
			zpy();
			ldx();
			break;
		case 0xB8:
			imp();
			clv();
			break;
		case 0xB9:
			absy();
			lda();
			break;
		case 0xBA:
			imp();
			tsx();
			break;
		case 0xBC:
			absx();
			ldy();
			break;
		case 0xBD:
			absx();
			lda();
			break;
		case 0xBE:
			absy();
			ldx();
			break;
		case 0xC0:
			imm();
			cpy();
			break;
		case 0xC1:
			indx();
			cmp();
			break;
		case 0xC4:
			zp();
			cpy();
			break;
		case 0xC5:
			zp();
			cmp();
			break;
		case 0xC6:
			zp();
			dec();
			break;
		case 0xC8:
			imp();
			iny();
			break;
		case 0xC9:
			imm();
			cmp();
			break;
		case 0xCA:
			imp();
			dex();
			break;
		case 0xCC:
			abso();
			cpy();
			break;
		case 0xCD:
			abso();
			cmp();
			break;
		case 0xCE:
			abso();
			dec();
			break;
		case 0xD0:
			rel();
			bne();
			break;
		case 0xD1:
			indy();
			cmp();
			break;
		case 0xD5:
			zpx();
			cmp();
			break;
		case 0xD6:
			zpx();
			dec();
			break;
		case 0xD8:
			imp();
			cld();
			break;
		case 0xD9:
			absy();
			cmp();
			break;
		case 0xDD:
			absx();
			cmp();
			break;
		case 0xDE:
			absx();
			dec();
			break;
		case 0xE0:
			imm();
			cpx();
			break;
		case 0xE1:
			indx();
			sbc();
			break;
		case 0xE4:
			zp();
			cpx();
			break;
		case 0xE5:
			zp();
			sbc();
			break;
		case 0xE6:
			zp();
			inc();
			break;
		case 0xE8:
			imp();
			inx();
			break;
		case 0xE9:
			imm();
			sbc();
			break;
		case 0xEB:
			imm();
			sbc();
			break;
		case 0xEC:
			abso();
			cpx();
			break;
		case 0xED:
			abso();
			sbc();
			break;
		case 0xEE:
			abso();
			inc();
			break;
		case 0xF0:
			rel();
			beq();
			break;
		case 0xF1:
			indy();
			sbc();
			break;
		case 0xF5:
			zpx();
			sbc();
			break;
		case 0xF6:
			zpx();
			inc();
			break;
		case 0xF8:
			imp();
			sed();
			break;
		case 0xF9:
			absy();
			sbc();
			break;
		case 0xFD:
			absx();
			sbc();
			break;
		case 0xFE:
			absx();
			inc();
			break;
		}

#ifdef USE_TIMING
      clockgoal6502 -= (int32_t)pgm_read_byte_near(ticktable + opcode);
#endif
      instructions++;

	#ifdef DEBUGUNO
	// ----------------------------- debug trace file
	if (fpx!=NULL)
	{
		char flagName[]="CZIDB-VN";
				
		fprintf(fpx, "%4x  ", debugPC);
		for (ii=0;ii<3;ii++)
			if ((int)(pc-debugPC-ii)<0)
				fprintf(fpx, "   ");
			else
				fprintf(fpx, "%2x ", read6502(debugPC+ii));
		fprintf(fpx, "               ");
		fprintf(fpx, "A:%2x X:%2x Y:%2x F:%2x S:1%2x [", a, x, y, cpustatus, sp);
		for (ii=7;ii>=6;ii--)
			if (cpustatus & 1<<ii)
				fprintf(fpx, "%c", flagName[ii]);
			else
				fprintf(fpx, ".");
		fprintf(fpx, "-");
		for (ii=4;ii>=0;ii--)
			if (cpustatus & 1<<ii)
				fprintf(fpx, "%c", flagName[ii]);
			else
				fprintf(fpx, ".");
		fprintf(fpx, "]\n");
	}
	// ----------------------------- debug
	#endif


// part 2 of NMI single-step handling for KIM-I
	if (nmiFlag==1) //SST switch on and not in K7 area (ie, ROM002), so single step
	{	nmi6502(); // set up for NMI
		nmiFlag=0;
	}
// ----------------------------------
  }
}

uint16_t getpc() {
  return(pc);
}

uint8_t getop() {
  return(opcode);
}
