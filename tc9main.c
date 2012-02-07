/*

    Another Telecar 9 controller firmware
    Copyright (C) 2009-2011 Wulf-Gerd Traving, DL1FAC

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    The author may be contacted by eMail: tc9@dl1fac.de
    
    Feel free to add your extensions, but don't expect any support until
    I am retired :-) You will definitely need the schematics to understand
    the code, but it's quite straightforward then.
    
    To compile the code, you need the free sdcc compiler available at sourceforge.
    I was using sdcc version 2.8.0 from 2008 all the time.
    
    Example command line for the 2m version:
    
    set VERSION=15
    sdcc -DTC2M -DVERSION=%VERSION% tc9main.c -o tc2m.ihx --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000
    srec_cat -Disable_Sequence_Warnings tc2m.ihx -intel -random-fill 0x0 0x4000 -exclude 0x26 0x28 -l-e-checksum-neg 0x26 2 2 -o tc2m-V%VERSION%.bin -binary
    
    A great Thank You to Yves Oesch and Thomas Müser for their continuous support of this project
    (testing, documentation, discussions and deliviering the firmware to several fist time users).
    
    Wulf-Gerd, DL1FAC, DOK Eutin M02, JO54hd

    Version 1.5

    Former ToDo (all done now), version history:
        + display icon support
        + star icon active during scanning
        + memory
        + auto repeat
        + skip startup logo on rx activity
        + display of memory number
        + pll lock check during rx
        + pll lock check before tx
        + outside band tx lock
        + startup with channel 0
        + memory content plausibility check
        + power off
        + rev for repeater input
        + send dtmf tones during transmit
        + mic off during dtmf
        + preserve first display char (memory)
        + memory scan function
        + automatic display of memory number, if any freq already in memory
        + sophisticated menu for squelch/power/volume
        + preserve power/volume/squelch level during power down
        + use eeprom instead of zero power ram (wait for write finish)
        + tx low and high limit configurable for easy vco calibration
        + repeater scan function
        + squelch acts immediately
        + less writes to eeprom (only during power down)
        + timeout for direct frequency entry
        + abort chance during direct frequency entry
        + long press "A": mute / unmute
        + abort start logo with every keypress
        + copyright logo also with hidden key
        + flash integrity check
        + doubled CPU clock possible (BUFU)
        + allow transmission outside band limits during calibration
        ++++ V08 ++++
        + power on beep
        + beep in parallel to 1750Hz transmission
        + allow ptt during frequency enter
        + Error 2, if eeprom defective
        + allow B DTMF again: release 1750Hz, if B pressed before PTT and then released
        ++++ V09 ++++
        + bugfix: hang after ptt/tone simultaneously pressed
        ++++ V10 ++++
        + modified repeater and scan ranges, changed icon to "D"uplex for repeater use
        + channel attribute: simplex, duplex, reverse with long "D" press
        + store channel attribute also in memory
        + reverse receive with hash key now depending on channel attribute
        + 2816 eeprom now also supported (and zero power ram, 2864, 28256 as before)
        ++++ V11 ++++
        + preset channel attribute NO_SHIFT / NORMAL_SHIFT for scan operation 2, 3, 4
        ++++ V12 ++++
        + select keyboard type through UP key (recognized as UP, A or CH(D) key
          on 19 key normal keyboards, "A B C D" trunk radio or 9 key keyboards respectively
        + support small 9 key keyboard
        ++++ V13 ++++
        + automatic switch to tx on aprs 144.800 channel 1 using /NOT input as ptt
        + PTT release during dual tone send now ends tx operation
        + 1750Hz send without ML79 using B key: remove P- in display
        ++++ V14 ++++
        + small keyboard: KONE to allow transmission outside band limits
        ++++ V15 ++++
        + added 4m version
        + source released to the public Jan 2012
*/

#include <8051.h>

// following options are meant to be set by commandline compile options

// #define TC70CM
// #define TC2M
// #define TC4M

enum {
    KEY_LARGE = 0,
    KEY_ABCD,
    KEY_SMALL
} keyboard_types;



// Trunk net (german "Bündelfunk") version with full clock 8.4672 MHz from coproc and 27C256
// otherwise half clock 4.2336 MHz and 27C128/27C64 (two zero ohm resistors under control board)
// #define BUFU

// permanent options

#ifndef TC70CM
#ifndef TC2M
#ifndef TC4M
#error either TC70CM or TC2M or TC4M must be defined
#endif
#endif
#endif

#define VERSION_H VERSION/10
#ifndef VERSION
#error VERSION shall be defined
#else
#define VERSION_L VERSION%10
#endif

#ifdef TC70CM
#define RASTER    12500L
#define ZF     21400000L
#endif
#ifdef TC2M
// max. usable frequency with unsigned int = 163,8375 MHz at 2.5kHz raster
#define RASTER     2500L
#define ZF     21400000L
#endif
#ifdef TC4M
#define RASTER    5000L
// LO VCO is oscillating above RX frequency
#define ZF     (-21400000L)
#endif

#ifdef TC70CM
#define SHIFT   7600000L
#define DEFAULTFREQ     433025000L
#define TXLOWLIMIT     (430012500L / RASTER)
#define TXHIGHLIMIT    (439987500L / RASTER)
#define BANDSTART       430012500L
#define BANDEND         439987500L
#define RELAISSCANLOWLIMIT  438550000L
#define RELAISSCANHIGHLIMIT 439425000L
#define RELAISLOWLIMIT  438300000L
#define RELAISHIGHLIMIT 439587500L
#define SCANSTART       420000000L // only for verifying valid values of scanlow and scanhigh
#endif

#ifdef TC2M
#define SHIFT   600000L
#define DEFAULTFREQ     145275000L
#define TXLOWLIMIT     (144012500L / RASTER)
#define TXHIGHLIMIT    (145987500L / RASTER)
#define BANDSTART       144012500L
#define BANDEND         145987500L
#define RELAISSCANLOWLIMIT  145600000L
#define RELAISSCANHIGHLIMIT 145787500L
#define RELAISLOWLIMIT  145600000L
#define RELAISHIGHLIMIT 145787500L
#define SCANSTART       130000000L // only for verifying valid values of scanlow and scanhigh
#endif

#ifdef TC4M
#define SHIFT   9800000L
#define DEFAULTFREQ     85675000L
// to disable transmission set allow range outside technical possible range
#define TXLOWLIMIT     (144012500L / RASTER)
#define TXHIGHLIMIT    (145987500L / RASTER)
#define BANDSTART       74215000L
#define BANDEND         87255000L
#define RELAISSCANLOWLIMIT  84015000L
#define RELAISSCANHIGHLIMIT 87255000L
#define RELAISLOWLIMIT  84015000L
#define RELAISHIGHLIMIT 87255000L
#define SCANSTART       60000000L // only for verifying valid values of scanlow and scanhigh
#endif

// time before autorepeating of up and down key starts in 5ms steps
#define AUTOREPEAT 40
// time before a number key will store instead of recall a memory in 5ms steps
#define STORAGETIME 250
// time before scanning starts again after activity on one memory channel (in 5ms steps)
#define SCANHOLDTIME 1000

unsigned char display [8];
unsigned char segments [8];
unsigned char icons;
unsigned int vfo_tx, vfo_rx; // times RASTER is the actual VCO operating frequency
unsigned char copro_inp, pwlevel, sqlevel, channel_attribute;
__bit transmitting, txallowed, back_to_rx, monitor, txlimitoff, tone;

// copro_inp bits
#define C_AFSK 0x80 // AFSK signal from modem (if any)
#define C_IGN  0x40 // high = ignition on
#define C_EINT 0x20 // low = EIN button on front pressed or mike switch in position on
#define C_NOT  0x10 // low = emergency alert
#define C_RUTA 0x08 // low = left button "note" pressed
#define C_SPTA 0x04 // low = PTT pressed
#define C_GAKO 0x02 // low = mike switch off, high in position 1,2,3
#define C_LSPT 0x01 // low = right button "speaker" pressed

#define FUNK P1_4   // low to route TAKT/DAT to trx part
#define LATCHE P1_3 // low to enable strobe multiplexer
#define RSP P1_7    // input high, if no carrier
#define RSAZ P1_6   // output low to switch on red LED (Rauschsperrenanzeige)
#define LOCKE P3_4  // low, if rx pll locked
#ifdef TC2M
#define LOCKS P3_5  // low, if tx pll locked
#endif

#define TATU ((__xdata volatile unsigned char *) 0x0000) // keyboard colums read back via xdata bus
// #define AWSP ((__xdata volatile unsigned char *) 0x8000) // eeprom xdata
#define FLASH ((__code volatile int *) 0x0000) // program flash
#define FLASHSIZE 0x4000 // for a 27C128

#define MEMORYCHANNELS 10
//__xdata volatile unsigned int * __data memory = (__xdata volatile unsigned int *) 0x8000; // AWSP
// if an eeprom 2864 is used for permanent memory, 10ms write cycles are needed after writing
// to single a 32 byte page up to 32 bytes. In case of zero power ram no wait is needed
// __xdata space has to be accessed in portions of 32 bytes
__xdata volatile unsigned int memory [MEMORYCHANNELS];            // 20 bytes
__xdata unsigned char power_f, volume_f, squelch_f, haltmode;     // 4 bytes
__xdata signed char scanmode;                                     // 1 byte
__xdata unsigned int scanlow, scanhigh;                           // 4 bytes
__xdata volatile unsigned char attribute [MEMORYCHANNELS];        // 10 bytes
__xdata unsigned char keyboardtype_f;                             // 1 byte
                                                                  // 40 bytes, not fitting into one page
                                                                  // so wait after each write access

// these are retrieved from flash at startup and written back into flash during power down
unsigned char power, volume, squelch;
// retrieved from flash at startup to determine keyboard type
// (detected by pressing UP key during startup DL1FAC display)
unsigned char keyboardtype;

// channel attributes
enum {
    NO_SHIFT = 0,
    NORMAL_SHIFT,
    REVERSE_SHIFT
} channel_attribs;

// extra characters beyond digits 0...9
enum {
	CHARA = 10,
	CHARB,
	CHARC,
	CHARD,
	CHARE,
	CHARF,
	CHARH,
	CHARJ,
	CHARL,
	CHARM,
	CHARN,
	CHARO,
	CHARP,
	CHARQ,
	CHARR,
	CHART,
	CHARU,
	CHARu,
	BLANK,
	MINUS,
	EQUAL,
	LINE,
	CHARCOUNT
} extra_chars;

/* trunck / normal keybard extra keys beyond digits 0...9 */
enum {
	KA = 10, // Audio volume
	KB,      // Power level
	KC,      // squelCh level
	KD,      // Direct frequency input
	KUP,
	KDOWN,
	KSTAR,
	KHASH,
	KTEN,
	KONE,
	KPLUS,
	KNOTES,  // used different on small keyboard
	KSPKR,   // used like KA Audio volume
	KNOTE,   // used like KB Power level on large keyboard
	KNOISE,  // used like KC Squelch level
	KCH      // used like KD Direct frequency entry
} extra_keys;

/*
   segment coding f a b g c d I e       D7...D0, I = icon
   0              1 1 1 0 1 1 0 1
   1              0 0 1 0 1 0 0 0
   2              0 1 1 1 0 1 0 1
   3              0 1 1 1 1 1 0 0
   4              1 0 1 1 1 0 0 0
   5              1 1 0 1 1 1 0 0
   6              1 1 0 1 1 1 0 1
   7              0 1 1 0 1 0 0 0
   8              1 1 1 1 1 1 0 1
   9              1 1 1 1 1 1 0 0
   A              1 1 1 1 1 0 0 1
   b              1 0 0 1 1 1 0 1
   C              1 1 0 0 0 1 0 1
   d              0 0 1 1 1 1 0 1
   E              1 1 0 1 0 1 0 1
   F              1 1 0 1 0 0 0 1
   H              1 0 1 1 1 0 0 1
   J              0 0 1 0 1 1 0 0
   L              1 0 0 0 0 1 0 1
   m              0 0 0 1 1 0 0 1
   N              1 1 1 0 1 0 0 1
   o              0 0 0 1 1 1 0 1
   P              1 1 1 1 0 0 0 1
   q              1 1 1 1 1 0 0 0
   r              0 0 0 1 0 0 0 1
   t              1 0 0 1 0 1 0 1
   U              1 0 1 0 1 1 0 1
   u              0 0 0 0 1 1 0 1
   BLANK          0 0 0 0 0 0 0 0
   -              0 0 0 1 0 0 0 0
   =              0 0 0 1 0 1 0 0
   _              0 0 0 0 0 1 0 0
*/

__code unsigned char dl1fac_tab [] = "-.. .-.. .---- ..-. .- -.-.";

__code unsigned char led_tab [CHARCOUNT] =
	 {0xed, 0x28, 0x75, 0x7c, 0xb8, 0xdc, 0xdd, 0x68, 0xfd, 0xfc,
	  0xf9, 0x9d, 0xc5, 0x3d, 0xd5, 0xd1, 0xb9, 0x2c, 0x85, 0x19,
	  0xe9, 0x1d, 0xf1, 0xf8, 0x11, 0x95, 0xad, 0x0d, 0x00, 0x10, 0x14, 0x04};

__code unsigned char dl1fac_copy [] = "(C) 2009 Wulf-Gerd Traving DL1FAC";

__code unsigned char squelch_tab [5] = {3, 3, 1, 2, 0}; // squelch = 0: monitor, use same level as squelch = 1

__code __at (0x26) unsigned int checksum = 0x0000;

void error_print (unsigned char error);

void clock_out (unsigned char c)
{
    char i;
    for (i=0; i<8; i++)
    {
        P1_1 = c & 1;
        c >>= 1;
        P1_0 = 0;
        P1_0 = 1;
    }
}

void clock_rev (unsigned char c)
{
    char i;
    for (i=0; i<8; i++)
    {
        P1_1 = c & 0x80;
        c <<= 1;
        P1_0 = 0;
        P1_0 = 1;
    }
}

void delay5ms (void)
{
    volatile unsigned int tmp;
#ifdef BUFU
    for (tmp=0; tmp<200; tmp++); // about 5ms delay
#else
    for (tmp=0; tmp<100; tmp++); // about 5ms delay
#endif
}

void delay500us (void)
{
    volatile unsigned int tmp;
#ifdef BUFU
    for (tmp=0; tmp<20; tmp++); // about 500us delay
#else
    for (tmp=0; tmp<10; tmp++); // about 500us delay
#endif
}

// 50ms timeout for eeprom writes
#define EEPROMTIMEOUT 100

void xdata_write_char (__xdata volatile char * ptr, char dat)
{
    unsigned char timeout;
    *ptr = dat;
    // wait until write complete, max. 50ms
    timeout = 0;
    while (*ptr != dat && timeout < EEPROMTIMEOUT)
    {
        timeout++;
        delay500us ();
    }
    if (timeout >= EEPROMTIMEOUT) error_print (2);
}

void xdata_write_int (__xdata volatile int * ptr, int dat)
{
#if 0
    // this code works only with XLS2864 upwards or zero power ram, not with XLS2816
    unsigned char timeout;
    *ptr = dat;
    // wait until write complete, max. 50ms
    timeout = 0;
    while (*ptr != dat && timeout < EEPROMTIMEOUT)
    {
        timeout++;
        delay500us ();
    }
    if (timeout >= EEPROMTIMEOUT) error_print (2);
#endif
    // so we use two single writes instead (slower, but who cares?)
    xdata_write_char ((__xdata volatile char *) ptr, (char) dat);
    xdata_write_char ((__xdata volatile char *) ptr+1, (char) (dat>>8));
}

#define I_STAR    0x01
#define I_CARET   0x02
#define I_NOISE   0x04
#define I_ANSWER  0x08
#define I_SPEAKER 0x10
#define I_DELTA   0x20
#define I_UP      0x40
#define I_DOWN    0x80

void disp_to_seg (void)
{

/*
   first display half, digit 0...3
   segments [0]: digit 1, 0: segments c d icon1caret e , c d icon0star e
   segments [1]: digit 3, 2: segments c d icon3answer e , c d icon2noise e
   segments [2]: digit 2, 3: segments f a b g, f a b g
   segments [3]: digit 0, 1: segments f a b g, f a b g

   second display half, digit 4...7
   segments [4]: digit 5, 4: segments c d icon5Delta e, c d icon4speaker e
   segments [5]: digit 7, 6: segments c d icon7down, c d icon6up e
   segments [6]: digit 6, 7: segments f a b g, f a b g
   segments [7]: digit 4, 5: segments f a b g, f a b g
*/

    // this reordering is necessary, because the layout is really weired
    segments[0] = (led_tab [display [1]]<<4) | (led_tab [display [0]] & 0x0f) | ((icons & I_CARET) << 4) | ((icons & I_STAR) << 1);
    segments[1] = (led_tab [display [3]]<<4) | (led_tab [display [2]] & 0x0f) | ((icons & I_ANSWER) << 2) | ((icons & I_NOISE) >> 1);
    segments[2] = (led_tab [display [2]] & 0xf0) | (led_tab [display [3]] >> 4);
    segments[3] = (led_tab [display [0]] & 0xf0) | (led_tab [display [1]] >> 4);
    segments[4] = (led_tab [display [5]]<<4) | (led_tab [display [4]] & 0x0f) | (icons & I_DELTA) | ((icons & I_SPEAKER) >> 3);
    segments[5] = (led_tab [display [7]]<<4) | (led_tab [display [6]] & 0x0f) | ((icons & I_DOWN) >> 2) | ((icons & I_UP) >> 5);
    segments[6] = (led_tab [display [6]] & 0xf0) | (led_tab [display [7]] >> 4);
    segments[7] = (led_tab [display [4]] & 0xf0) | (led_tab [display [5]] >> 4);
}

void disp_out (void)
{
   char i;
	// set FUNK high to access display shift register
	P1_0 = 1; // start always with clock high
	FUNK = 1;
	LATCHE = 1;
	disp_to_seg ();
   // now the display contents can be clocked out
   for (i=0; i<8; i++)
   {
        clock_out (segments [i]);
   }
	// select AZSTR
	P1_0 = 1;
	P1_1 = 1;
	P1_2 = 0;
	// strobe AZSTR high by enabling 74HC238
	LATCHE = 0; // low enable
	LATCHE = 1; // and disable again
	// display content updated now
}


char key_scan (void)
{
    unsigned char col;
    LATCHE = 1;
    // select ZEIL0
    P1_0 = 1;
    P1_1 = 0;
    P1_2 = 1;
    LATCHE = 0;
    // read inverting key buffer
    col = ~*TATU & 0x3f;
    LATCHE = 1;
    if (col)
    {
        if (keyboardtype == KEY_LARGE)
        {
            if (col & 0x20) return 9;
            if (col & 0x10) return 8;
            if (col & 0x08) return 0;
            if (col & 0x04) return KDOWN;
            if (col & 0x02) return KNOTE;
            return 7;
        }
        else if (keyboardtype == KEY_ABCD)
        {
            if (col & 0x20) return 9;
            if (col & 0x10) return 8;
            if (col & 0x08) return 0;
            if (col & 0x04) return KUP;
            if (col & 0x02) return KDOWN;
            return 7;
        }
        else // keyboardtype == KEY_SMALL
        {
            if (col & 0x10) return KTEN;
            if (col & 0x08) return KONE;
            if (col & 0x02) return KDOWN;
        }
    }
    // select ZEIL1
    P1_0 = 0;
    P1_1 = 1;
    P1_2 = 1;
    LATCHE = 0;
    col = ~*TATU & 0x3f;
    LATCHE = 1;
    if (col)
    {
        if (keyboardtype == KEY_LARGE)
        {
            if (col & 0x20) return 6;
            if (col & 0x10) return 5;
            if (col & 0x08) return KSTAR;
            if (col & 0x04) return KSPKR;
            if (col & 0x02) return KNOISE;
            return 4;
        }
        else if (keyboardtype == KEY_ABCD)
        {
            if (col & 0x20) return 6;
            if (col & 0x10) return 5;
            if (col & 0x08) return KSTAR;
            if (col & 0x04) return KC;
            if (col & 0x02) return KD;
            return 4;
        }
        else // keyboardtype == KEY_SMALL
        {
            if (col & 0x20) return KPLUS;
            if (col & 0x04) return KSPKR;
            return KNOISE;
        }
    }
    // select ZEIL2
    P1_0 = 1;
    P1_1 = 1;
    P1_2 = 1;
    LATCHE = 0;
    col = ~*TATU & 0x3f;
    LATCHE = 1;
    if (col)
    {
        if (keyboardtype == KEY_LARGE)
        {
            if (col & 0x20) return 3;
            if (col & 0x10) return 2;
            if (col & 0x08) return KHASH;
            if (col & 0x04) return KUP;
            if (col & 0x02) return KCH;
            return 1;
        }
        else if (keyboardtype == KEY_ABCD)
        {
            if (col & 0x20) return 3;
            if (col & 0x10) return 2;
            if (col & 0x08) return KHASH;
            if (col & 0x04) return KA;
            if (col & 0x02) return KB;
            return 1;
        }
        else // keyboardtype == KEY_SMALL
        {
            if (col & 0x10) return KNOTES; // handled extra on small keyboard: RX: switch to reverse, TX: emit 1750Hz on small keyboard
            if (col & 0x02) return KUP;
        }
    }
    return -1; // no key found
}

/*
   serial communication with co-proc 8051
   mode 2: 9 bit communication, 1 start and 1 stop bit
*/

void putchar (char c)
{
	while (!TI);
	TI=0;
	SBUF=c;
	TB8 = 1;
}

char getchar (void)
{
	char c;
	while (!RI);
	RI=0;
	c=SBUF;
	return c;
}

void hw_init (void)
{

/*
  Set initial state of P1
  RSP       = 1
  RSAZ      = 0
  -SAS      = 1
  FUNK      = 0
  IS653.-E1 = 1
  IS653.A2  = 1
  IS653.A1  = 1
  IS653.A0  = 1
*/
   P1 = 0xbf;

	TH1 = 0x50;  //	Load timer 1 high byte (52.92 KHz).
	TMOD = 0x22;
	TR1 = 1;     // Start timer 0 & 1 to 8 bits autoreload
#ifdef BUFU
   // CPU clock = CoProc Clock = 8.4672 MHz
	PCON = 0x00; // Setup power control register
					 // Double baud rate bit = 0
#else
   // CPU clock = CoProc Clock / 2 = 4.2336 MHz
	PCON = 0x80; // Setup power control register
					 // Double baud rate bit = 1
#endif
	SCON = 0x98; // Setup serial port control register
					 // SM0-1 UART Mode 2 (9 bits UART, baud rate FCPU/32) FCPU = CLK0/2 = 4.2336 MHz (7.6us per bit)
					 // SM2 = 0 (no mulitprocessor mode)
					 // REN Enable reception
	TB8 = 1;     // default bit 8
}

void clock_synth (unsigned char c)
{
    // set FUNK low to generate CLOCKF / DATF
    FUNK = 0;
    clock_rev (c);
    FUNK = 1;
    // select SYNTE signal
    P1_0 = 0;
    P1_1 = 0;
    P1_2 = 1;
    LATCHE = 0; // now trigger SYNTE high
    LATCHE = 1; // and low again
    LATCHE = 0; // now trigger SYNTE high (two times for unknown reasons)
    LATCHE = 1; // and low again
}

void set_rx (unsigned char c)
{
/*
    rx control via: TAKTF, DATF, EMSTR
    D3 RSPU  low: noise tail suppression ON
    D2 RSP B: high (S+N)/N 18dB squelch level   RSP B and RSP A low: 25dB
    D1 RSP A: high: (S+N)/N 12dB squelch level (sensitive)
    D0 low: rx vco ON
*/
    // set FUNK low to generate CLOCKF / DATF
    FUNK = 0;
    clock_rev (c);
    FUNK = 1;
    // select EMSTR signal
    P1_0 = 1;
    P1_1 = 0;
    P1_2 = 0;
    LATCHE = 0; // now trigger EMSTR high
    LATCHE = 1; // and low again
}

void set_tx (unsigned char c)
{
/*
    tx control via: TAKTF, DATF, SESTR and ENA
    D6 VORST  high: tx preamp on
    D5 SVEIN/ low: tx vco on
    D4 SEIN   high: tx on
    D3 LSTD tx power MSB
    D2 LSTC tx power
    D1 LSTB tx power
    D0 LSTA tx power LSB
*/
    // set FUNK low to generate CLOCKF / DATF
    FUNK = 0;
    clock_rev (c);
    FUNK = 1;
    // select SESTR signal
    P1_0 = 0;
    P1_1 = 0;
    P1_2 = 0;
    LATCHE = 0; // now trigger EMSTR high
    LATCHE = 1; // and low again
}

// set synthesizer to a multiple of RASTER (12.5 kHz or 10kHz)
void set_synth (unsigned int mul)
{
    unsigned int M;
    unsigned char A;

    // reference frequency fr = 8 MHz (TCXO 70cm), 6.4MHz (xtal 2m, 70cm)
    // factor R = 8000 / RASTER = 640  at 12.5 kHz (70cm)
    // factor R = 6400 / RASTER = 512  at 12.5 kHz (2m, 4m)
    // factor R = 6400 / RASTER = 640  at 10   kHz (2m, 4m)
    // factor R = 6400 / RASTER = 2560 at 2.5  kHz (2m, 4m)
	 LATCHE = 1;
    // (P*M + A) * fr is output freq.     TX = 12500 * (128 * 262 + 64) = 420 MHz
    //                                    RX = 12500 * (128 * 249 + 16) = 398.6 + 21.4 ZF = 420 MHz
    // Prescaler P = 128 (70cm)
    // Prescaler P = 64 (2m)
    // Prescaler P = 64 (4m)

#ifdef TC70CM
    M = mul / 128;
    A = mul % 128;
	 // set R to 640 for 12.5kHz raster at fr = 8.0 MHz
    clock_synth (0x71); // Latch 7 X R10 R9 R8 R = 0001 0100 0000 0 (last 0 for additional internal division by 2)
    clock_synth (0x64); // Latch 6 R7 R6 R5 R4 R = 640, fr = 8MHz / 640 = 12.5 kHz
    clock_synth (0x50); // Latch 5 R3 R2 R1 R0
#endif
#ifdef TC4M
    M = mul / 64;
    A = mul % 64;
	 // set R to 1280 for 5.0 kHz raster at fr = 6.4 MHz
    clock_synth (0x72); // Latch 7 X R10 R9 R8 R = 0010 1000 0000 0 (last 0 for additional internal division by 2)
    clock_synth (0x68); // Latch 6 R7 R6 R5 R4 R = 1280, fr = 6.4MHz / 1280 = 5.0 kHz
    clock_synth (0x50); // Latch 5 R3 R2 R1 R0
#endif
#ifdef TC2M
    M = mul / 64;
    A = mul % 64;
#if 0
	 // set R to 512 for 12.5 kHz raster at fr = 6.4 MHz
    clock_synth (0x71); // Latch 7 X R10 R9 R8 R = 0001 0000 0000 0 (last 0 for additional internal division by 2)
    clock_synth (0x60); // Latch 6 R7 R6 R5 R4 R = 512, fr = 6.4MHz / 512 = 12.5 kHz
    clock_synth (0x50); // Latch 5 R3 R2 R1 R0
#endif
	 // set R to 2560 for 2.5 kHz raster at fr = 6.4 MHz
    clock_synth (0x75); // Latch 7 X R10 R9 R8 R = 0101 0000 0000 0 (last 0 for additional internal division by 2)
    clock_synth (0x60); // Latch 6 R7 R6 R5 R4 R = 512, fr = 6.4MHz / 2560 = 2.5 kHz
    clock_synth (0x50); // Latch 5 R3 R2 R1 R0
#endif

    // set A
    clock_synth (0x40 | ((A >> 4) & 0x0f)); // Latch 4 XX A6 A5 A4
    clock_synth (0x30 | (A & 0x0f));        // Latch 3 A3 A2 A1 A0

    // set M
    clock_synth (0x20 | ((M >> 6) & 0x0f)); // Latch 2 M9 M8 M7 M6
    clock_synth (0x10 | ((M & 0x3c) >> 2)); // Latch 1 M5 M4 M3 M2
    clock_synth ((M & 0x03) << 2);          // Latch 0 M1 M0 XX XX
}

void print_hex (unsigned char c)
{
    display [0] = (c>>4) & 0x0f;
    display [1] = c & 0x0f;
    disp_out ();
}

void print_flag (unsigned char pos, unsigned char c)
{
    if (c) display [pos] = 1; else display [pos] = 0;
    disp_out ();
}

void coproc_out (unsigned char port, unsigned char val)
{
    if (port < 1 || port > 3) return; // not allowed
    // need to write direct into SBUF only for the very first char (TI initially low)
    SBUF = 0x08 + port;
    delay500us ();
    TB8 = 0;
    putchar (val);
    // give co proc some time to consume the command (unknown duration, unknown handshake, if any at all
    delay5ms ();
}

// power off the unit
void switch_off (char write)
{
    unsigned char tmp;
    delay5ms ();
    set_tx (0x20); // switch off tx
    set_rx (0xF0 | (sqlevel << 1));
    delay5ms ();
    // switch off AF and red RSAZ LED
    RSAZ = 0;
    coproc_out (2, 0x8B | ((7-volume) << 4)); // MIK off
    delay5ms ();
    display [0] = CHARA;
    display [1] = CHARU;
    display [2] = 5;
    display [3] = BLANK;
    display [4] = BLANK;
    display [5] = 0;
    display [6] = CHARF;
    display [7] = CHARF;
    disp_out ();
    if (write)
    {
        // save current volume, power, squelch levels
        xdata_write_char (&volume_f, volume);
        xdata_write_char (&squelch_f, squelch);
        xdata_write_char (&power_f, power);
    }
    for (tmp=1; tmp!=0; tmp++) delay5ms (); // about 0.5s delay to show sign off message
    delay5ms ();
    coproc_out (1, 0x7f);
    while (1); // wait for power down
}

// fatal error, switch off after 1s
void error_print (unsigned char error)
{
    unsigned char tmp;
    delay5ms ();
    display [0] = CHARE;
    display [1] = CHARR;
    display [2] = CHARR;
    display [3] = CHARO;
    display [4] = CHARR;
    display [5] = BLANK;
    display [6] = error;
    display [7] = BLANK;
    disp_out ();
    for (tmp=1; tmp!=0; tmp++) delay5ms (); // about 0.5s delay to show sign off message
    switch_off (0); // dont try to write into eeprom (may be cause of error)
}

void send_single_tone (unsigned int freq)
{
    delay5ms ();
    // switch single tone (e.g. 1750 Hz)
    putchar (0x12); // switch single tone
    delay500us ();
    TB8 = 0;
    putchar (freq >> 8); // frequency in Hz, MSB first
    delay500us ();
    TB8 = 0;
    putchar (freq & 0xff); // LSB of frequency in HZ
    delay5ms ();
}

void send_dual_tone (signed char number)
{
    if (number == -1)
    {
        // switch off tone
        send_single_tone (0);
        return;
    }
    delay5ms ();
    coproc_out (2, 0xFF); // disable af (NF) and disable mic (MIK), enable STAZ Sendertastanzeige
    // send dtmf tone
    putchar (0x14); // switch dual tone
    delay500us ();
    TB8 = 0;
    putchar (number);
    delay5ms ();
    // wait until key released again or message from coproc (e.g. ptt release)
    RI = 0;
    while ((key_scan () != -1) && !RI);
    send_single_tone (0);
    // normal transmission
    coproc_out (2, 0xFE); // disable af (NF) and enable mic (MIK) and STAZ Sendertastanzeige
}

// does not work...
void switch_off_tone (void)
{
    delay5ms ();
    putchar (0x01);
    delay5ms ();
}

void rx_vfo_to_display (void)
{
    unsigned long int freq;
    signed char i;
    freq = (unsigned long) vfo_rx * (unsigned long) RASTER + ZF;
    freq /= 1000; // no Hz, 10 Hz and 100Hz display
    for (i=7; i>=2; i--)
    {
        display [i] = freq % 10;
        freq /= 10;
    }
#if TC4M
    // suppress leading zero (only in case of 4m)
    if (display[2] == 0) display[2] = BLANK;
#endif
    disp_out ();
}

void tx_vfo_to_display (void)
{
    unsigned long int freq;
    signed char i;
    freq = (unsigned long) vfo_tx * (unsigned long) RASTER;
    freq /= 1000; // no Hz, 10 Hz and 100Hz display
    for (i=7; i>=2; i--)
    {
        display [i] = freq % 10;
        freq /= 10;
    }
#if TC4M
    // suppress leading zero (only in case of 4m)
    if (display[2] == 0) display[2] = BLANK;
#endif
    disp_out ();
}

void set_tx_from_rx (void)
{
    if (channel_attribute == NORMAL_SHIFT)
    {
        // force normal shift (negative)
        vfo_tx = vfo_rx + ZF / RASTER - SHIFT / RASTER;
        icons |= I_DELTA; // D symbol for repeater operation ("D"uplex)
        icons &= ~I_ANSWER;
    }
    else if (channel_attribute == REVERSE_SHIFT)
    {
        // force REVERSE shift (positive)
        vfo_tx = vfo_rx + ZF / RASTER + SHIFT / RASTER;
        icons |= I_DELTA; // D symbol for repeater operation ("D"uplex)
        icons |= I_ANSWER; // Answer Icon indicates reverse Duplex operation
    }
    else // channel_attribute == NO_SHIFT
    {
        // is simplex
        vfo_tx = vfo_rx + ZF / RASTER;
        icons &= ~I_DELTA;
        icons &= ~I_ANSWER;
    }
    txallowed = (vfo_tx >= TXLOWLIMIT && vfo_tx <= TXHIGHLIMIT) || txlimitoff;
}

// return true, if PLL locked
// this is the same PLL as for rx, so the LOCKE signal is valid, too
// only some data version with fast rx/tx switching are equipped with a 2nd PLL (LOCKS)
char wait_for_tx_lock (void)
{
    int i;
    if (!LOCKE) return 1;
#ifdef BUFU
    for (i=0; i<2000; i++)
#else
    for (i=0; i<1000; i++)
#endif
    {
        if (!LOCKE) return 1;
    }
    return 0;
}

// return true, if PLL locked
// wait a little bit unto lock state
// if no lock: set caret icon as a warning
char wait_for_rx_lock (void)
{
    int i;
    icons &= ~I_CARET;
    if (!LOCKE) return 1;
#ifdef BUFU
    for (i=0; i<2000; i++)
#else
    for (i=0; i<1000; i++)
#endif
    {
        if (!LOCKE) return 1;
    }
    icons |= I_CARET;
    return 0;
}

void clear_display (void)
{
    display [0] = BLANK;
    display [1] = BLANK;
    display [2] = BLANK;
    display [3] = BLANK;
    display [4] = BLANK;
    display [5] = BLANK;
    display [6] = BLANK;
    display [7] = BLANK;
    disp_out ();
}

// return BLANK if a vfo freq is not in memory or first memory number with this entry
unsigned char check_if_already_in_memory (unsigned int vfo)
{
    unsigned char channel;
    for (channel=0; channel < MEMORYCHANNELS; channel++)
    {
        if (memory [channel] == vfo) return channel;
    }
    return BLANK;
}

#define BEEPDURATION 15 // time 5ms

void beep (void)
{
    volatile unsigned char tmp;
    if (!transmitting)
    {
        // switch on ATONE
        delay5ms ();
        coproc_out (2, 0x83 | ((7-volume) << 4)); // MIK off, NF off, ATONE on
        for (tmp=0; tmp<BEEPDURATION; tmp++) delay5ms (); // about 0.2s tone
        coproc_out (2, 0x8B | ((7-volume) << 4)); // MIK off, NF off, ATONE off
        for (tmp=0; tmp<BEEPDURATION; tmp++) delay5ms (); // about 0.2s tone
        back_to_rx = 1; // to reset to proper NF state depending on RSP
    }
}

void longbeep (void)
{
    volatile unsigned char tmp;
    if (!transmitting)
    {
        // switch on ATONE
        delay5ms ();
        coproc_out (2, 0x83 | ((7-volume) << 4)); // MIK off, NF off, ATONE on
        for (tmp=0; tmp<3*BEEPDURATION; tmp++) delay5ms (); // about 0.2s tone
        coproc_out (2, 0x8B | ((7-volume) << 4)); // MIK off, NF off, ATONE off
        for (tmp=0; tmp<BEEPDURATION; tmp++) delay5ms (); // about 0.2s tone
        back_to_rx = 1; // to reset to proper NF state depending on RSP
    }
}

void pause (void)
{
    volatile unsigned char tmp;
    delay5ms ();
    for (tmp=0; tmp<2*BEEPDURATION; tmp++) delay5ms (); // about 0.2s tone
}

void dl1fac (void)
{
    longbeep (); beep (); beep (); pause ();
    beep (); longbeep (); beep (); beep (); pause ();
    beep (); longbeep (); longbeep (); longbeep (); longbeep (); pause ();
    beep (); beep (); longbeep (); beep (); pause ();
    beep (); longbeep (); pause ();
    longbeep (); beep (); longbeep (); beep ();
}

char getkey (void)
{
    char key;
    while ((key = key_scan ())== -1);
    // wait until key released again
    while (key_scan () != -1);
    return key;
}

#define ENTERTIMEOUT 1000 // in 5ms steps

void enter_frequency (void)
{
    unsigned char c, i;
    unsigned long freq;
    unsigned int timeout;

    display [0] = CHARF;
    display [1] = EQUAL;
#ifdef TC70CM
    display [2] = 4;
#endif
#ifdef TC2M
    display [2] = 1;
#endif
#ifdef TC4M
    display [2] = BLANK;
#endif
    display [3] = LINE;
    display [4] = LINE;
    display [5] = LINE;
    display [6] = LINE;
    display [7] = LINE;
    disp_out ();

    for (i=3; i<=7; i++)
    {
        timeout = 0;
        do
        {
            c = key_scan ();
            delay5ms ();
            timeout++;
            // abort if ptt (or other mikro key) pressed
            if (RI && !RB8) goto restorelast;
            if (RI) RI = 0;
        }
        while (c > 9 && c != KSTAR && c != KHASH && timeout < ENTERTIMEOUT);
        if (timeout >= ENTERTIMEOUT) goto restorelast;
        // wait until key released again
        while (key_scan () != -1);
        if (c == KSTAR)
        {
            // erase last input
            if (i > 3)
            {
                i--;
                display [i] = LINE;
                i--;
            }
            else
            {
                // discard any changes
                goto restorelast;
            }
        }
        else if (c == KHASH)
        {
            i--;
            dl1fac ();
        }
        else
        {
            display [i] = c;
        }
        disp_out ();
    }

    freq = 0;
#if TC4M
    for (i=3; i<=7; i++)
#else
    for (i=2; i<=7; i++)
#endif
    {
        freq *= 10L;
        freq += (unsigned long) display [i];
    }
    freq *= 1000L;
    if (display [7] == 2 || display [7] == 7) freq += 500L; // 500 Hz offset for (1)2.5kHz raster
    vfo_rx = freq / RASTER - ZF / RASTER;
    // set new channel attribute depending on frequency
    if ((vfo_rx >= (RELAISLOWLIMIT - ZF) / RASTER) && (vfo_rx <= (RELAISHIGHLIMIT - ZF) / RASTER))
    {
       channel_attribute = NORMAL_SHIFT;
    }
    else
    {
       channel_attribute = NO_SHIFT;
    }
restorelast:
    set_tx_from_rx ();
    set_synth (vfo_rx);
    display [0] = check_if_already_in_memory (vfo_rx);
    display [1] = BLANK;
    wait_for_rx_lock ();
    rx_vfo_to_display ();
}

void store (unsigned char mem)
{
    if (mem < MEMORYCHANNELS)
    {
        xdata_write_int (&memory [mem], vfo_rx);
        xdata_write_char (&attribute [mem], channel_attribute);
    }
}

// set receiver and tx vfo according to vfo_rx
void set_rx_from_vfo (void)
{
    set_tx_from_rx ();
    set_synth (vfo_rx);
    wait_for_rx_lock ();
    display [0] = BLANK;
    display [1] = BLANK;
    rx_vfo_to_display ();
}

void recall (unsigned char mem)
{
    if (mem >= MEMORYCHANNELS) return;
    vfo_rx = memory [mem];
    channel_attribute = attribute [mem];
    // integrity check
#ifdef TC70CM
    if (vfo_rx < ((400000000L - ZF) / RASTER) || vfo_rx > ((500000000L - ZF) / RASTER))
    {
        vfo_rx = (DEFAULTFREQ - ZF) / RASTER;
        channel_attribute = NO_SHIFT; // DEFAULTFREQ should be simplex...
    }
#endif
#ifdef TC2M
    if (vfo_rx < ((120000000L - ZF) / RASTER) || vfo_rx > ((163000000L - ZF) / RASTER))
    {
        vfo_rx = (DEFAULTFREQ - ZF) / RASTER;
        channel_attribute = NO_SHIFT; // DEFAULTFREQ should be simplex...
    }
#endif
#ifdef TC4M
    if (vfo_rx < ((60000000L - ZF) / RASTER) || vfo_rx > ((90000000L - ZF) / RASTER))
    {
        vfo_rx = (DEFAULTFREQ - ZF) / RASTER;
        channel_attribute = NORMAL_SHIFT; // DEFAULTFREQ inside 4m is a relais frequency
    }
#endif
    set_tx_from_rx ();
    set_synth (vfo_rx);
    wait_for_rx_lock ();
    display [0] = mem;
    display [1] = BLANK;
    rx_vfo_to_display ();
}

char squelch_check (char checkalways)
{
    static char rspold = 0;
    char rsp;
    if (monitor) rsp = 0; else rsp = RSP;
    if (rspold != rsp || checkalways)
    {
        rspold = rsp;
        if (rsp)
        {
            // switch off AF and red RSAZ LED
            RSAZ = 0;
            coproc_out (2, 0x8B | ((7-volume) << 4)); // MIK off
            return 0;
        }
        else
        {
            // switch on AF and red RSAZ LED
            RSAZ = 1;
            coproc_out (2, 0x89 | ((7-volume) << 4)); // MIK off
            return 1;
        }
    }
    if (rsp) return 0; else return 1;
}

#define SCANINPUTTIME 400 // 2s (units of 5ms)
#ifdef TC2M
#define SCANSTEP 5 // 12.5 KHz raster
#endif
#ifdef TC4M
#define SCANSTEP 5 // 12.5 KHz raster
#endif
#ifdef TC70CM
#define SCANSTEP 1 // 12.5 kHz raster
#endif

// this scan function returns the terminating key code
signed char memory_scan (char simple_scan)
{
    unsigned char channel;
    signed char key;
    unsigned int activity;
    int i;
restart:
    display [0] = 5;
    display [1] = CHARC;
    display [2] = CHARA;
    display [3] = CHARN;
    display [4] = EQUAL;
    display [5] = scanmode;
    display [6] = BLANK;
    display [7] = haltmode;
    disp_out();

    while (key_scan () != -1); // wait for initiating key release
    i = 0;
    // simple scan used with small keyboard: no mode change possible
    if (!simple_scan)
    {
        while ((key = key_scan ()) == -1 && i<SCANINPUTTIME) i++, delay5ms ();
        if (key != -1)
        {
            if (key == 0)
            {
                if (haltmode != CHARH) xdata_write_char (&haltmode, CHARH); // hold mode
                goto restart;
            }
            if (key == 9)
            {
                if (haltmode != CHARP) xdata_write_char (&haltmode, CHARP); // pause mode
                goto restart;
            }
            if (key == KC || key == KNOISE)
            {
                char k;
                // display copyright
                display [0] = BLANK ^ 0x55;
                display [1] = CHARD ^ 0x55;
                display [2] = CHARL ^ 0x55;
                display [3] = 1 ^ 0x55;
                display [4] = CHARF ^ 0x55;
                display [5] = CHARA ^ 0x55;
                display [6] = CHARC ^ 0x55;
                display [7] = BLANK ^ 0x55;
                for (k=0; k<8; k++) display [k] ^= 0x55;
                disp_out ();
                while (key_scan () != -1); // wait for key release
                goto restart;
            }
            if (key > 0 && key < 7 && key != scanmode) xdata_write_char (&scanmode, key); // key star again starts previous mode immediately
            display [5] = scanmode;
            disp_out ();
            while (key_scan () != -1); // wait for key release
        }

        // use up and down key to select and store upper and lower scan limit for mode 5 scanning

        if (key == KDOWN)
        {
            // set lower scan limit
            if (scanlow != vfo_rx) xdata_write_int (&scanlow, vfo_rx);
            display [0] = check_if_already_in_memory (vfo_rx);
            display [1] = BLANK;
            rx_vfo_to_display ();
            return -1;
        }

        if (key == KUP)
        {
            // set higher scan limit
            if (vfo_rx > scanlow && scanhigh != vfo_rx) xdata_write_int (&scanhigh, vfo_rx);
                else xdata_write_int (&scanhigh, scanlow + 1);
            display [0] = check_if_already_in_memory (vfo_rx);
            display [1] = BLANK;
            rx_vfo_to_display ();
            return -1;
        }

        if (key == 6)
        {
            // set scan range to current vfo +/- 500kHz
            if (scanlow != (vfo_rx - 500000L / RASTER)) xdata_write_int (&scanlow, vfo_rx - 500000L / RASTER);
            if (scanhigh != (vfo_rx + 500000L / RASTER)) xdata_write_int (&scanhigh, vfo_rx + 500000L / RASTER);
        }
    }

    // switch off AF and red RSAZ LED as initial scan state always
    RSAZ = 0;
    coproc_out (2, 0x8B | ((7-volume) << 4)); // MIK off

    // now start scanning with scanmode
    // 1: scan all memory channels
    // 2: scan all repeater channels
    // scan mode 3 and 4 only for 2m
    // 3: scan all FM simplex channels
    // 4: scan all repeater and simplex channels
    // 5: scan a certain range previously defined with down/up
    // 6: define range to actual vfo_rx +/-500kHz and start scanning
    // and haltmode
    // 0 = H hold mode until 5s carrier loss
    // 9 = P pause mode hold for 5s, then scan on again

    if (RI) RI = 0; // discard possibly unwanted serial char from coproc
    while ((key = key_scan ()) == -1)
    {
        if (scanmode == 1)
        {
            // scan through all memory channels and stop while carrier
            for (channel=0; channel < MEMORYCHANNELS; channel++)
            {
                recall (channel);
                activity = 0;
stayOnThisChannel1:
                while (squelch_check (0) && activity < SCANHOLDTIME)
                {
                    if (haltmode == CHARH) activity=1; else activity++;
                    // stay on this channel during activity
                    if ((key = key_scan ()) != -1) return key;
                    delay5ms ();
                    if (RI && !RB8) return key;
                    if (RI) RI = 0;
                }
                delay5ms ();
                if (RI && !RB8) return key;
                if (RI) RI = 0;
                // if there was any activity, stay a little while on this channel
                if (activity) activity++;
                if (activity && activity < SCANHOLDTIME) goto stayOnThisChannel1;
                if ((key = key_scan ()) != -1) return key;
            }
        }
        if (scanmode == 2 || scanmode == 4)
        {
            // scan through all voice mode repeater channels
            channel_attribute = NORMAL_SHIFT;
            for (vfo_rx = (RELAISSCANLOWLIMIT - ZF) / RASTER; vfo_rx <= (RELAISSCANHIGHLIMIT - ZF) / RASTER; vfo_rx += SCANSTEP)
            {
                set_rx_from_vfo ();
                activity = 0;
stayOnThisChannel2:
                while (squelch_check (0) && activity < SCANHOLDTIME)
                {
                    if (haltmode == CHARH) activity=1; else activity++;
                    // stay on this channel during activity
                    if ((key = key_scan ()) != -1) return key;
                    delay5ms ();
                    if (RI && !RB8) return key;
                    if (RI) RI = 0;
                }
                delay5ms ();
                if (RI && !RB8) return key;
                if (RI) RI = 0;
                // if there was any activity, stay a little while on this channel
                if (activity) activity++;
                if (activity && activity < SCANHOLDTIME) goto stayOnThisChannel2;
                if ((key = key_scan ()) != -1) return key;
            }
        }
        if (scanmode == 3 || scanmode == 4)
        {
            // scan through all simplex channels
            channel_attribute = NO_SHIFT;
#ifdef TC70CM
            for (vfo_rx = (430012500L - ZF) / RASTER; vfo_rx <= (434000000L - ZF) / RASTER; vfo_rx += SCANSTEP)
#endif
#ifdef TC2M
            for (vfo_rx = (144500000L - ZF) / RASTER; vfo_rx < (RELAISLOWLIMIT - ZF) / RASTER; vfo_rx += SCANSTEP)
#endif
#ifdef TC4M
            for (vfo_rx = (74215000L - ZF) / RASTER; vfo_rx < (77475000L - ZF) / RASTER; vfo_rx += SCANSTEP)
#endif
            {
#ifdef TC70CM
                if (vfo_rx == (430362500L - ZF) / RASTER) vfo_rx = (432500000L - ZF) / RASTER; // skip non fm part
#endif
#ifdef TC2M
                if (vfo_rx == (145000000L - ZF) / RASTER) vfo_rx = (145200000L - ZF) / RASTER; // skip repeater inputs
#endif
#ifdef TC4M
#endif
                set_rx_from_vfo ();
                activity = 0;
stayOnThisChannel3:
                while (squelch_check (0) && activity < SCANHOLDTIME)
                {
                    if (haltmode == CHARH) activity=1; else activity++;
                    // stay on this channel during activity
                    if ((key = key_scan ()) != -1) return key;
                    delay5ms ();
                    if (RI && !RB8) return key;
                    if (RI) RI = 0;
                }
                delay5ms ();
                if (RI && !RB8) return key;
                if (RI) RI = 0;
                // if there was any activity, stay a little while on this channel
                if (activity) activity++;
                if (activity && activity < SCANHOLDTIME) goto stayOnThisChannel3;
                if ((key = key_scan ()) != -1) return key;
            }
        }
        if (scanmode == 5 || scanmode == 6)
        {
            // scan through certain range
            for (vfo_rx = scanlow; vfo_rx <= scanhigh; vfo_rx += SCANSTEP)
            {
                set_rx_from_vfo ();
                activity = 0;
stayOnThisChannel5:
                while (squelch_check (0) && activity < SCANHOLDTIME)
                {
                    if (haltmode == CHARH) activity=1; else activity++;
                    // stay on this channel during activity
                    if ((key = key_scan ()) != -1) return key;
                    delay5ms ();
                    if (RI && !RB8) return key;
                    if (RI) RI = 0;
                }
                delay5ms ();
                if (RI && !RB8) return key;
                if (RI) RI = 0;
                // if there was any activity, stay a little while on this channel
                if (activity) activity++;
                if (activity && activity < SCANHOLDTIME) goto stayOnThisChannel5;
                if ((key = key_scan ()) != -1) return key;
            }
        }
    }
    return key;
}

void set_pwlevel_from_power (void)
{
    // power bit 3 LEISTD high: lowest power (without high temperature control)
    // always without control through bit 0..2
    // power bit 0...2 (LEISTA, LEISTB, LEISTC) only valid with LEISTD = 0
    // 0: lowest power, 7: highest power (roughly 1dB steps).
    // Three power levels are enough (23W max. only on certain units)
    if (power == 2) pwlevel = 6; // high power 20W (pwlevel = 7 would be 23W max. output)
       else if (power == 1) pwlevel = 1; // medium power 10W
       else pwlevel = 0x08; // lowest power 3W
}

// switch power is called during TX if pressing up/down key
// and during RX if using the "B"ower / note key
void switch_power (void)
{
    unsigned char display0;
    unsigned char menutime;
    signed char key;

    display0 = display [0];
restartP:
    // power levels depending on PA module
    // power = 0: low 3W
    // power = 1: middle 10W
    // power = 2: high 20W
    display [0] = CHARP;
    display [1] = EQUAL;
    display [5] = BLANK;
    display [6] = BLANK;
    display [7] = BLANK;
    if (power == 0)
    {
        display [2] = CHARL;
        display [3] = CHARO;
        display [4] = CHARu;
    } else if (power == 1)
    {
        display [2] = CHARM;
        display [3] = 1;
        display [4] = CHARD;
    }
    else
    {
        display [2] = CHARH;
        display [3] = 1;
        display [4] = 9;
        display [5] = CHARH;
    }
    disp_out ();
    // wait until key released again
    // but if PTT pressed, release tone instead
    // to send 1750Hz, if B pressed before PTT
    while (key_scan () != -1)
    {
        if (RI && !RB8)
        {
            tone = 1;
            display [0] = BLANK;
            display [1] = BLANK;
            // now evaluate PTT and emit tone
            return;
        }
    }
    // now get up/down input or new squelch
    menutime = 0;
    while ((key = key_scan ()) == -1 && menutime<200 && !(RI && !RB8))
    {
        if (RI && RB8) RI = 0; // remove unwanted serial char from coproc
        menutime++;
        delay5ms ();
        delay5ms ();
    }
    if (key == KUP || key == KB || key == KNOTE)
    {
        if (power < 2) power++;
            else if (key == KB || key == KNOTE) power = 0; // wrap around only with B key
        set_pwlevel_from_power ();
        if (transmitting) set_tx (0x50 | pwlevel); // enable tx vco LSB = 0: max. power
        goto restartP;
    }
    if (key == KDOWN)
    {
        if (power > 0) power--; // no wrap around
        set_pwlevel_from_power ();
        if (transmitting) set_tx (0x50 | pwlevel); // enable tx vco LSB = 0: max. power
        goto restartP;
    }
    if (key >= 0 && key <= 2)
    {
        power = key;
        set_pwlevel_from_power ();
        if (transmitting) set_tx (0x50 | pwlevel); // enable tx vco LSB = 0: max. power
        goto restartP;
    }
    // wait until key released again
    while (key_scan () != -1);
    display [0] = display0;
    display [1] = BLANK;
    if (transmitting)
    {
        tx_vfo_to_display ();
    }
    else
    {
        rx_vfo_to_display ();
    }
}

void flash_check (void)
{
    unsigned int i;
    unsigned int sum;
    sum = 0;
    for (i=0; i<FLASHSIZE; i++) sum += FLASH[i];
    if (sum != 0) error_print (1); // flash content invalid
}

void main(void)
{
    char key, i, display0;
    __bit rb8, rsp, rsplast, newtrans, wastone;
    __bit txunlock;
    char wasnumber, numberkey, aprs_tx;
    volatile unsigned int tmp;
    unsigned char keydowntime;
    unsigned char menutime;
    static char oldvolume = -1;

    hw_init ();

    // initial settings

    coproc_out (3, 0x37); // initial port 3 value (tx = PM for crisp modulation)
    coproc_out (3, 0x27); // switch on the light via co proc
    set_rx (0xF2); // switch on rx vco, high sensitivity
    set_tx (0x20); // switch off tx

    display [0] = BLANK;
    display [1] = BLANK;
    display [2] = BLANK;
    display [3] = BLANK;
    display [4] = BLANK;
    display [5] = BLANK;
    display [6] = BLANK;
    display [7] = BLANK;

    icons = 0;
    txunlock = 0;
    txlimitoff = 0; // prevent from transmitting outside bands
    // give sqlevel and monitor a valid value
    sqlevel = squelch_tab [0];
    if (squelch_f > 4) xdata_write_char (&squelch_f, 2);
    squelch = squelch_f;
    monitor = (squelch == 0);
    sqlevel = squelch_tab [squelch];
    tone = 0; // no 1750 Hz tone transmission
    wastone = 0;
    if (scanmode < 1 || scanmode > 6) xdata_write_char (&scanmode, 1);
    // give haltmode from xdata a valid value, if not initialized before
    if (haltmode != CHARH && haltmode != CHARP) xdata_write_char (&haltmode, CHARH); // stop scanning until carrier loss for at least 5s
    power = power_f;
    set_pwlevel_from_power (); // initialize powerlevel to a valid value (xdata power may be invalid)
    if (volume_f > 7) xdata_write_char (&volume_f, 2); // make valid value always (xdata volume may be not initialized properly)
    volume = volume_f;
    if (volume == 0) icons &= ~I_SPEAKER; else icons |= I_SPEAKER;
    coproc_out (2, 0x88 | ((7-volume) << 4));
    if (keyboardtype_f > KEY_SMALL) xdata_write_char (&keyboardtype_f, KEY_ABCD); // default: trunk radio keyboard layout
    keyboardtype = keyboardtype_f;
    transmitting = 0;
    back_to_rx = 1; // switch on/off speaker in case of monitor true (from squelch saved in xdata during power down)
    rsplast = 1;
    rsp = 1;
    keydowntime = 0;
    menutime = 0;
    wasnumber = 0;
    numberkey = 0;
    i = 0;
    aprs_tx = 0;

    // initialize scan low and high frequency limits to valid values, if found invalid
    if (scanlow >= scanhigh || scanlow < (SCANSTART - ZF) / RASTER)
    {
        xdata_write_int (&scanlow, (BANDSTART-ZF) / RASTER);
        xdata_write_int (&scanhigh, (BANDEND-ZF) / RASTER);
    }

    set_rx (0xF0 | (sqlevel << 1)); // | 0x08: noise tail suppression off

    recall (0); // set initial rx freq from memory 0

    display [0] = BLANK;
    display [1] = CHARD;
    display [2] = CHARL;
    display [3] = 1;
    display [4] = CHARF;
    display [5] = CHARA;
    display [6] = CHARC;
    display [7] = BLANK;

    disp_out ();
     // about 0.5s delay to show sign on message DL1FAC (interrupted by key press)
    keyboardtype = KEY_LARGE; // switch temporarily to a defined layout
    for (tmp=1; tmp!=20; tmp++) if (!RSP || key_scan () != -1) break; else delay5ms ();
    // hidden option: press UP key during version display
    // to determine keyboard type
    if (key_scan () == KCH) xdata_write_char (&keyboardtype_f, KEY_SMALL);
    if (key_scan () == KUP) xdata_write_char (&keyboardtype_f, KEY_LARGE);
    if (key_scan () == KDOWN) xdata_write_char (&keyboardtype_f, KEY_ABCD);
    keyboardtype = keyboardtype_f;
    if (key_scan () == KTEN) dl1fac (); // only on small keyboards: hidden option to send "dl1fac" in morse

    if (RSP) flash_check (); // this takes 2 seconds on slow, 1s on clock doubled versions (roughly)

    if (RSP && key_scan () == -1)
    {
        display [0] = CHARU;
        display [1] = CHARE;
        display [2] = CHARR;
        display [3] = 5;
        display [4] = BLANK;
        display [5] = VERSION_H;
        display [6] = MINUS;
        display [7] = VERSION_L;
        disp_out ();
         // about 1.5s delay to show version info (interrupted by key press)
        for (tmp=1; tmp!=300; tmp++) if (!RSP || key_scan () != -1) break; else delay5ms ();
        // hidden option: press 0 during version display, tx allowed outside band limits
        // for calibration purposes then
        if (key_scan () == 0) txlimitoff = 1;
        if (key_scan () == KONE) txlimitoff = 1;
    }

    // restore display
    recall (0);

    longbeep (); // signal: now entering main loop

    // main loop
    while(1)
    {
        // process key strokes
        if ((key = key_scan ())!= -1)
        {
            switch (key) {
                case KUP:
                          if (!transmitting) // may be used for test purposes also during transmit, but without lock control
                          {
                              icons |= I_UP;
                              if (keydowntime < AUTOREPEAT) keydowntime++;
                              if (keydowntime == 1 || keydowntime >= AUTOREPEAT)
                              {
                                  vfo_rx += 1; // n-times RASTER (12.5kHz)
                                  // set new channel attribute depending on frequency
                                  if ((vfo_rx >= (RELAISLOWLIMIT - ZF) / RASTER) && (vfo_rx <= (RELAISHIGHLIMIT - ZF) / RASTER))
                                  {
                                      channel_attribute = NORMAL_SHIFT;
                                  }
                                  else
                                  {
                                      channel_attribute = NO_SHIFT;
                                  }
                                  set_tx_from_rx ();
                                  display [0] = BLANK; // remove memory number just in case
                                  display [1] = BLANK;
                                  if (transmitting)
                                  {
                                      set_synth (vfo_tx);
                                      tx_vfo_to_display ();
                                  }
                                  else
                                  {
                                      set_synth (vfo_rx);
                                      display [0] = check_if_already_in_memory (vfo_rx);
                                      wait_for_rx_lock ();
                                      rx_vfo_to_display ();
                                  }
                              }
                              else
                              {
                                  // little delay times AUTOREPEAT until key repeats as fast as possible
                                  delay5ms ();
                              }
                          }
                          // during transmit the down key will toggle output power (in three steps)
                          else
                          {
                              switch_power ();
                          }
                          break;
                case KDOWN:
                          if (!transmitting) // may be used for test purposes also during transmit, but without lock control
                          {
                              icons |= I_DOWN;
                              if (keydowntime < AUTOREPEAT) keydowntime++;
                              if (keydowntime == 1 || keydowntime >= AUTOREPEAT)
                              {
                                  vfo_rx -= 1; // n-times RASTER (12.5kHz)
                                  // set new channel attribute depending on frequency
                                  if ((vfo_rx >= (RELAISLOWLIMIT - ZF) / RASTER) && (vfo_rx <= (RELAISHIGHLIMIT - ZF) / RASTER))
                                  {
                                      channel_attribute = NORMAL_SHIFT;
                                  }
                                  else
                                  {
                                      channel_attribute = NO_SHIFT;
                                  }
                                  set_tx_from_rx ();
                                  display [0] = BLANK; // remove memory number just in case
                                  display [1] = BLANK;
                                  if (transmitting)
                                  {
                                      set_synth (vfo_tx);
                                      tx_vfo_to_display ();
                                  }
                                  else
                                  {
                                      set_synth (vfo_rx);
                                      display [0] = check_if_already_in_memory (vfo_rx);
                                      wait_for_rx_lock ();
                                      rx_vfo_to_display ();
                                  }
                              }
                              else
                              {
                                  // little delay times AUTOREPEAT until key repeats as fast as possible
                                  delay5ms ();
                              }
                          }
                          // during transmit the down key will toggle output power (in three steps)
                          else
                          {
                              switch_power ();
                          }
                          break;
                case KSPKR:
                case KA: if (transmitting)
                         {
                            send_dual_tone (10);
                         }
                         else
                         {
                            display0 = display [0];
restartA:
                            display [0] = CHARU;
                            display [1] = CHARO;
                            display [2] = CHARL;
                            display [3] = EQUAL;
                            display [4] = volume;
                            display [5] = BLANK;
                            display [6] = BLANK;
                            display [7] = BLANK;
                            if (volume == 0) icons &= ~I_SPEAKER; else icons |= I_SPEAKER;
                            disp_out ();
                            // wait until key released again
                            // long press >1s: toggle mute
                            menutime = 0;
                            while ((key = key_scan ()) != -1 && menutime<100)
                            {
                                menutime++;
                                delay5ms ();
                                delay5ms ();
                            }
                            if (menutime >= 100)
                            {
                                // audio mute / unmute
                                if (volume == 0)
                                {
                                    // only if already initialized through mute function:
                                    if (oldvolume >= 0) volume = oldvolume;
                                    if (volume == 0) volume = 1; // minimum level for unmute
                                    icons |= I_SPEAKER;
                                    display [0] = CHARN;
                                    display [1] = CHARU;
                                    display [2] = CHART;
                                    display [3] = CHARE;
                                    display [4] = BLANK;
                                    display [5] = 0;
                                    display [6] = CHARF;
                                    display [7] = CHARF;
                                    disp_out ();
                                }
                                else
                                {
                                    oldvolume = volume;
                                    volume = 0;
                                    icons &= ~I_SPEAKER;
                                    display [0] = CHARN;
                                    display [1] = CHARU;
                                    display [2] = CHART;
                                    display [3] = CHARE;
                                    display [4] = BLANK;
                                    display [5] = CHARO;
                                    display [6] = CHARM;
                                    display [7] = BLANK;
                                    disp_out ();
                                }
                                if (rsp) coproc_out (2, 0x8A | ((7-volume) << 4));
                                    else coproc_out (2, 0x88 | ((7-volume) << 4));
                            }
                            else
                            {
                                // now get up/down input or new volume
                                menutime = 0;
                                while ((key = key_scan ()) == -1 && menutime<200 && !(RI && !RB8))
                                {
                                    if (RI && RB8) RI = 0; // remove unwanted serial char from coproc
                                    menutime++;
                                    delay5ms ();
                                    delay5ms ();
                                }
                                if (key == KUP || key == KA || key == KSPKR)
                                {
                                    if (volume < 7) volume++; // no wrap around
                                    if (rsp) coproc_out (2, 0x8A | ((7-volume) << 4));
                                        else coproc_out (2, 0x88 | ((7-volume) << 4));
                                    goto restartA;
                                }
                                if (key == KDOWN)
                                {
                                    if (volume > 0) volume--; // no wrap around
                                    if (rsp) coproc_out (2, 0x8A | ((7-volume) << 4));
                                        else coproc_out (2, 0x88 | ((7-volume) << 4));
                                    goto restartA;
                                }
                                if (key >= 0 && key <= 7)
                                {
                                    volume = key;
                                    if (rsp) coproc_out (2, 0x8A | ((7-volume) << 4));
                                        else coproc_out (2, 0x88 | ((7-volume) << 4));
                                    goto restartA;
                                }
                            }
                            // wait until key released again
                            while (key_scan () != -1);
                            key = -1;
                            display [0] = display0;
                            display [1] = BLANK;
                            if (volume == 0) icons &= ~I_SPEAKER; else icons |= I_SPEAKER;
                            rx_vfo_to_display ();
                         }
                         break;
                case KNOTE:
                case KB:
                         // if "B" is pressed and hold before PTT, then 1750 will be generated after
                         // releasing "B" (until PTT is released). If "B" is pressed after PTT,
                         // a normal DTMF tone B will be generated. This somewhat weired scheme is
                         // needed only on microphones without extra "note" key
                         if (transmitting)
                         {
                              send_dual_tone (11);
#if 0
                              // "B" cannot be transmitted - need this key for 1750Hz transmission
                              // in case of using a mikro without note key - this is the note key
                              // on layout 1 keypads (normal one) anyway.
                              // in transmitting state use for 1750Hz tone (if mikro has no note button)
                              // send 1750 Hz tone, mic off
                              send_single_tone (1750);
                              coproc_out (2, 0xE7); // switch on ATONE to give user acoustic feedback
                              // coproc_out (2, 0xFF); // no feedback: disable af, disable mic, enable STAZ
                              // wait until key released again
                              while (key_scan () != -1);
                              delay5ms (); // not necessary?
                              // normal transmission, switch off tone
                              send_single_tone (0);
                              coproc_out (2, 0xFE); // disable af (NF) and enable mic (MIK) and STAZ Sendertastanzeige
#endif
                         }
                         else
                         {
                            switch_power ();
                         }
                         break;
                case KNOISE:
                case KC: if (transmitting)
                         {
                            send_dual_tone (12);
                         }
                         else
                         {
                            display0 = display [0];
restartC:
                            monitor = (squelch == 0);
                            sqlevel = squelch_tab [squelch];
                            delay5ms ();
                            set_rx (0xF0 | (sqlevel << 1)); // | 0x08: noise tail suppression off
                            delay5ms ();
                            squelch_check (1); // switch audio always depending on squelch and monitor after each level change
                            // squelch = 0: monitor, NF always open
                            // squelch = 1: sqlevel 2, open at -136dBm
                            // squelch = 2: sqlevel 0, open at -125dBm
                            // squelch = 3: sqlevel 3, open at -120dBm
                            // squelch = 4: sqlevel 1, open at -112dBm
                            display [0] = 5;
                            display [1] = CHARQ;
                            display [2] = CHARL;
                            display [3] = EQUAL;
                            display [4] = squelch;
                            display [5] = BLANK;
                            display [6] = BLANK;
                            display [7] = BLANK;
                            disp_out ();
                            // wait until key released again
                            while (key_scan () != -1);
                            // now get up/down input or new squelch
                            menutime = 0;
                            while ((key = key_scan ()) == -1 && menutime<200 && !(RI && !RB8))
                            {
                                if (RI && RB8) RI = 0; // remove unwanted serial char from coproc
                                menutime++;
                                delay5ms ();
                                delay5ms ();
                                squelch_check (0);
                            }
                            if (key == KUP || key == KC || key == KNOISE)
                            {
                                if (squelch < 4) squelch++;
                                    else if (key == KC || key == KNOISE) squelch = 0; // wrap around only with C button
                                goto restartC;
                            }
                            if (key == KDOWN)
                            {
                                if (squelch > 0) squelch--; // no wrap around
                                goto restartC;
                            }
                            if (key >= 0 && key <= 4)
                            {
                                squelch = key;
                                goto restartC;
                            }
                            // wait until key released again
                            while (key_scan () != -1);
                            key = -1;
                            display [0] = display0;
                            display [1] = BLANK;
                            rx_vfo_to_display ();
                         }
                         break;
                case KCH:
                case KD: if (transmitting)
                         {
                             send_dual_tone (13);
                         }
                         else
                         {
                             // wait until key released again
                             // long press >1s: toggle simplex channel on repeater frequencies
                             menutime = 0;
                             while ((key = key_scan ()) != -1 && menutime<100)
                             {
                                 menutime++;
                                 delay5ms ();
                                 delay5ms ();
                             }
                             if (menutime >= 100)
                             {
                                 // increment channel attribute
                                 if (channel_attribute == REVERSE_SHIFT)
                                 {
                                     // wrap around
                                     channel_attribute = NO_SHIFT;
                                 }
                                 else
                                 {
                                     channel_attribute++;
                                 }
                                 set_tx_from_rx (); // calculate new tx frequency and icons
                                 disp_out ();       // display changed icons
                                 // wait until "D" key released again
                                 while (key_scan () != -1);
                             }
                             else
                             {
                                enter_frequency ();
                             }
                         }
                         break;
                case KHASH: if (!transmitting)
                         {
                             // switch receiver to repeater input (that is vfo_tx - IF) temporarily
                             // but only inside repeater bands
                             unsigned int save_vfo_rx = vfo_rx;
                             vfo_rx = vfo_tx - ZF / RASTER;
                             display0 = display [0];
                             display [0] = CHARR;
                             set_synth (vfo_rx);
                             wait_for_rx_lock ();
                             rx_vfo_to_display ();
               	           RSAZ = 1;
                	           // open audio independent of squelch state
                             coproc_out (2, 0x89 | ((7-volume) << 4)); // MIK off
                             rsplast = 0;
                             // wait until key released again
                             while (key_scan () != -1);
                             // and switch back to previous freq
                             vfo_rx = save_vfo_rx;
                             display [0] = display0;
                             display [1] = BLANK;
                             set_synth (vfo_rx);
                             wait_for_rx_lock ();
                             rx_vfo_to_display ();
                         }
                         else
                         {
                             send_dual_tone (15);
                         }
                         break;
               case KSTAR: if (transmitting)
                         {
                             send_dual_tone (14);
                         }
                         else
                         {
                             icons |= I_STAR;
                             key = memory_scan (0);
                             icons &= ~I_STAR;
                             disp_out (); // remove star sign
                             // prevent star key from triggering a new scan, if star key has been used to terminate scan, too
                             if (key == KSTAR) while ((key = key_scan ()) != -1);
                         }
                         break;
               case KTEN: if (!transmitting) // this key found only on small keyboards
                         {
                             icons |= I_STAR;
                             key = memory_scan (1); // use simple mode on small keyboards
                             icons &= ~I_STAR;
                             disp_out (); // remove star sign
                             // prevent "10" key from triggering a new scan, if "10" key has been used to terminate scan, too
                             if (key == KTEN) while ((key = key_scan ()) != -1);
                         }
                         break;
               case KNOTES: if (!transmitting) // this key with different function on small keyboard
                         {
                             // switch receiver to repeater input (that is vfo_tx - IF) temporarily
                             // but only inside repeater bands
                             unsigned int save_vfo_rx = vfo_rx;
                             vfo_rx = vfo_tx - ZF / RASTER;
                             display0 = display [0];
                             display [0] = CHARR;
                             set_synth (vfo_rx);
                             wait_for_rx_lock ();
                             rx_vfo_to_display ();
               	           RSAZ = 1;
                	           // open audio independent of squelch state
                             coproc_out (2, 0x89 | ((7-volume) << 4)); // MIK off
                             rsplast = 0;
                             // wait until key released again
                             while (key_scan () != -1);
                             // and switch back to previous freq
                             vfo_rx = save_vfo_rx;
                             display [0] = display0;
                             display [1] = BLANK;
                             set_synth (vfo_rx);
                             wait_for_rx_lock ();
                             rx_vfo_to_display ();
                         }
                         else
                         {
                              // in case of using a mikro without note key - this is the note key
                              // on layout 1 keypads (normal one) anyway.
                              // in transmitting state use for 1750Hz tone (if mikro has no note button)
                              // send 1750 Hz tone, mic off
                              send_single_tone (1750);
                              coproc_out (2, 0xE7); // switch on ATONE to give user acoustic feedback
                              // coproc_out (2, 0xFF); // no feedback: disable af, disable mic, enable STAZ
                              // wait until key released again
                              while (key_scan () != -1);
                              delay5ms (); // not necessary?
                              // normal transmission, switch off tone
                              send_single_tone (0);
                              coproc_out (2, 0xFE); // disable af (NF) and enable mic (MIK) and STAZ Sendertastanzeige
                         }
                         break;
               case KONE: // used on small keyboard only, switch to memory 0
                          if (!transmitting)
                          {
                              numberkey = 0; // first storage
                              wasnumber = 1;
                              if (keydowntime < STORAGETIME)
                              {
                                  keydowntime++;
                              }
                              else
                              {
                                  display [0] = numberkey;
                                  disp_out ();
                              }
                              delay5ms ();
                          }
                         break;
               case KPLUS: // used on small keyboard only, increment memory
                          if (!transmitting)
                          {
                              wasnumber = 2;
                              if (keydowntime < STORAGETIME)
                              {
                                  keydowntime++;
                              }
                              else
                              {
                                  display [0] = numberkey;
                                  disp_out ();
                              }
                              delay5ms ();
                          }
                         break;
               default:  if (key >= 0 && key <= 9)
                         {
                              if (transmitting)
                              {
                                  send_dual_tone (key);
                              }
                              else
                              {
                                  numberkey = key;
                                  wasnumber = 1;
                                  if (keydowntime < STORAGETIME)
                                  {
                                      keydowntime++;
                                  }
                                  else
                                  {
                                      display [0] = numberkey;
                                      disp_out ();
                                  }
                                  delay5ms ();
                              }
                         }
                         break;
           }
        }
        else
        {
            // keyup again
            if (wasnumber)
            {
                if (keydowntime >= STORAGETIME)
                {
                    // store actual frequency into memory numberkey
                    store (numberkey);
                }
                else
                {
                    // recall memory numberkey
                    if (wasnumber == 2)
                    {
                        // inc. to next memory, if triggered by KPLUS key on small keyboard
                        numberkey++;
                        if (numberkey>9) numberkey = 0;
                    }
                    recall (numberkey);
                }
                wasnumber = 0;
            }
            icons &= ~(I_UP | I_DOWN);
            keydowntime = 0;
        }

        // message from co proc (i.e. PTT pressed)?
        if (RI)
        {
            RI=0;
	         copro_inp=SBUF;
	         rb8 = RB8;
	         if (!rb8)
	         {
                // print_hex (copro_inp); // TEST
                /*  copro_inp values:
	                 bit 7 AFSK signal
	                 bit 6 IGN  high = ignition on
	                 bit 5 EINT/ high if mike switch in position off (parallel to EIN button on front)
	                 bit 4 NOT/  low = emergency alert
	                 bit 3 RUTA/ low = left button "note" pressed
	                 bit 2 SPTA/ low = PTT pressed
	                 bit 1 GAKO/ low = handset operation
	                 bit 0 LSPT/ low = right button "speaker" pressed
                */

                // if external mike with permanent on switch like ML79 is connected
                // GAKO/ must be pulled low permanently
                // With simple mikes (using on/off button on front panel) GAKO must
                // be left open to prevent from shutting down power with every PTT action
                if (copro_inp & C_EINT && !(copro_inp & C_GAKO))
                {
                    // mike switch in position off
                    switch_off (1); // does not return
                }
                if (!(copro_inp & C_EINT) && (copro_inp & C_GAKO))
                {
                    // normal mike without switch, but front panel button pressed
                    while (!RI); // wait for button release event
                    RI = 0;
                    delay5ms ();
                    switch_off (1); // does not return
                }

                if (!tone && !transmitting) tone = !(copro_inp & C_RUTA);
                aprs_tx = !(copro_inp & C_NOT);
	             newtrans = !(copro_inp & C_SPTA) || tone || aprs_tx;
                if (!transmitting && newtrans && !txallowed)
                {
                    // transmission outside band limits prohibited
                    newtrans = 0;
                    // switch on ATONE
                    delay5ms ();
                    coproc_out (2, 0x83 | ((7-volume) << 4)); // MIK off, NF off, ATONE on
                    for (tmp=1; tmp!=40; tmp++) delay5ms (); // about 0.2s tone
                    back_to_rx = 1;
                }
//	             if (newtrans != transmitting)
//	             {
	                 transmitting = newtrans; // state changed
                    if (transmitting)
                    {
                        if (tone)
                        {
                            // wait until "B" key released again
                            while (key_scan () != -1);
                        }
                        // do everything to switch over to transmitting state
                        // enable tx_vco
                        RSAZ = 0;
                        set_rx (0xF1); // disable rx vco (what happens, if both vco are left on?)
                        set_tx (0x00 | pwlevel); // enable tx vco
                        if (aprs_tx)
                        {
                            // tx on aprs channel, this is an explicitely allowed frequency in memory (channel) 1
                            vfo_tx = memory[1] + ZF / RASTER;
                            txallowed = 1;
                        }
                        set_synth (vfo_tx);
                        tx_vfo_to_display ();
                        if (aprs_tx) set_tx_from_rx (); // restore old tx freq just in case (e.g. next transmission)
                        if (wait_for_tx_lock ())
                        {
                            if (txunlock)
                            {
                                // was previously unlocked, reset yellow LED
                                txunlock = 0;
                                // switch off yellow LED ("pll unlock")
                                coproc_out (3, 0x27); // switch off ANR LED (yellow LED) via co proc
                                delay5ms ();
                            }
                            // pll locked, switch on driver and pa
                            set_tx (0x50 | pwlevel); // enable tx vco, driver, pa
                            if (tone)
                            {
                                // send 1750 Hz tone, mic off
                                send_single_tone (1750);
                                // coproc_out (2, 0xFF); // no acoustic feedback
                                coproc_out (2, 0xE7); // switch on ATONE to give user acoustic feedback
                                wastone = 1;
                                tone = 0;
                            }
                            else
                            {
                                delay5ms (); // not necessary? tbd
                                if (aprs_tx)
                                {
                                    // transmission with FSK input (pin 1, was MIKV)
                                    coproc_out (2, 0xFF); // disable af (NF) and disable mic (MIK) and STAZ Sendertastanzeige
                                }
                                else
                                {
                                    // normal transmission
                                    coproc_out (2, 0xFE); // disable af (NF) and enable mic (MIK) and STAZ Sendertastanzeige
                                }
                            }
                        }
                        else
                        {
                            // switch on yellow LED ("pll unlock") and fall back into receiving state
                            coproc_out (3, 0xA7); // switch on ANR LED (yellow LED) via co proc
                            txunlock = 1;
                            transmitting = 0;
                            goto txfail;
                        }
                    }
                    else
                    {
                        // and back to receiving here
txfail:
                        if (wastone)
                        {
                            wastone = 0;
                            tone = 0;
                            send_single_tone (0);
                        }
                        set_tx (0x20); // switch off tx
                        set_rx (0xF0 | (sqlevel << 1)); // | 0x08: noise tail suppression off
                        set_synth (vfo_rx);
                        back_to_rx = 1;
                        wait_for_rx_lock ();
                        rx_vfo_to_display ();
                    }
//                }
                if (!transmitting)
                {
                    // tbd: delay only if monitor state change
                    delay5ms ();
                    monitor = !(copro_inp & C_LSPT);
                }
            }
	     }

        if (!transmitting)
        {
            /* Squelch handling */
            /* rsp = (RSP && !monitor); did not work this way... */
            if (monitor) rsp = 0; else rsp = RSP;
    	      if ((rsp != rsplast) || back_to_rx)
    	      {
    	         back_to_rx = 0;
    	         // squelch status changed
    	         rsplast = rsp;
    	         if (rsp)
    	         {
    	             // switch off AF and red RSAZ LED
    	             RSAZ = 0;
                   coproc_out (2, 0x8B | ((7-volume) << 4)); // MIK off
    	         }
    	         else
    	         {
    	             // switch on AF and red RSAZ LED
    	             RSAZ = 1;
                   coproc_out (2, 0x89 | ((7-volume) << 4)); // MIK off
    	         }
    	      }
    	  }
    }
}
