tc9-V XXXXXXXXXXXXXXXXXXXXX.zip readme file
==================
HB9DTX / April 2012

Introduction:
============
This archive contains the TC9 firmware. It can be used on AEG Telecar 9 / BBC RT-61 professional radios to use them for amateur radio purposes. This amateur-dedicated firmware is much more user friendly and gives access to functions otherwise not accessible, like Setting any frequency, automatic repeater shift, 1750 Hz tone, squelch level setting, scanning, on the fly memory storing,...

From V15 onwards, the software source been opened under GNU GPLv3 by its creator, DL1FAC, Wulf-Gerd.
Sourceforge hosts the project (see link below)

The firmware has to be burnt into an EPROM chip inside the radio (27C128 or 27C258). The use parameters (memory channels, last used powe rsetting,...) are stored in a parallel EEPROM. There is no need to programm the EEPROM as this is done by the firmware. For stability reason it is recommanded to fully eraset the EEPROM prior to first power up of the radio witt the new firmware.

Archive content:
===============
Archive:  tc9-VXXXXXXXXXXXXXXXXXXXXX.zip
    testing: tc2m-VXXXXXXXXXXXXXXXXXXXXX.bin             OK
    testing: tc2mB-VXXXXXXXXXXXXXXXXXXXXX.bin            OK
    testing: tc4m-VXXXXXXXXXXXXXXXXXXXXX.bin             OK
    testing: tc70cm-VXXXXXXXXXXXXXXXXXXXXX.bin           OK
    testing: tc70cmB-VXXXXXXXXXXXXXXXXXXXXX.bin          OK
    testing: tc2m-VXXXXXXXXXXXXXXXXXXXXX_double.bin      OK
    testing: tc2mB-VXXXXXXXXXXXXXXXXXXXXX_double.bin     OK
    testing: tc4m-VXXXXXXXXXXXXXXXXXXXXX_double.bin      OK
    testing: tc70cm-VXXXXXXXXXXXXXXXXXXXXX_double.bin    OK
    testing: tc70cmB-VXXXXXXXXXXXXXXXXXXXXX_double.bin   OK
		testing: readme.txt 							OK
No errors detected in compressed data of tc9-VXXXXXXXXXXXXXXXXXXXXX.zip.

Just for fun an MD5 sum file is provided along with the archive (tc9-VXXXXXXXXXXXXXXXXXXXXX.md5)


Which firmware for which radio type:
===================================
There are several version of the radio, i.e 70 MHz, 144 MHz, 430 MHz, trunking or not trunking, as well as front panel layout. The firmware handles most of them. The firmware is provided in raw binary format.

Frequency band
**************
70 MHz:tc4m*.bin
144 MHz:tc2m*.bin
430 MHz: tc70cm*.bin

Trunking / standard radio
*************************
If the radio is a "Trunking" set (normally the front panel keypad shows the A,B,B,D keys) then the processor speed is different from the classical radios (front panel without Alpha keys).
Standard radio: tc*-V??.bin
Trunking radio: tc*B-V??*.bin

EPROM size
**********
Normally the sets are shiped with 27C128 memories. These chips tends to be less easy to find than 27C256 ones. It's possible to use 27C256 EPROMS for TC9 as well. The firmware is simply doubled in the upper memory space.
27C128: tc9*-V??.bin
27C256: tc9*-V??_double.bin


EEPROM erasing:
==============
You might have a small problem if the original content of the EEPROM is random. It probably contains the settings of the previous firmware such as channels, CTCSS, tones squelch,... Therefore it's good practice to erase the EEPROM chip as well. This is the XLS2816 chips situated juste beside the EPROM. Put FFs in the whole memory.

Chip sizes 2816, 2864 or 28256 can be used, as well as the Zero-RAM chips found in the some "truncking" version of the radio.


Keypad Layout detection
***********************
The AEG Telecar9 / BBC RT-61 radio has been shipped with different front panels. Both versions 9 keys and 19 keys are supported by the firmware.
To change the front panel type:
KEEP THE UP-ARROW KEY PRESSED WHILE POWERING ON THE RADIO SET.

If you miss this step, you can also to manually set the byte at 0x27 in the EEPROM to 0x00 for 19 keys and 0x02 for 9 keys


Hardware modifications on the radio:
===================================
At least for the UHF version the VCO have to be modified to be brought up or down to 435 MHz and the input filters have to be adjusted. The 144 MHz doesn't require re-tuning.
There are other settings like max power, for example that can be adjusted only by hardware.
Description of the hardware modifications is available on internet (see link below)


Main internet ressources about the AEG Telecar 9 / BBC RT-61 radio:
==================================================================
http://sourceforge.net/projects/tc9/
http://www.yvesoesch.ch/radio/trxmod/aeg/telecar9.htm
http://www.deilbachtaler.de/aeg_telecar_9_seite.htm
http://www.dc5ww.de/telecar/telecar.htm
http://home.hccnet.nl/hv.leijden/home.html
http://www.triple1.nl/index.php/page/6.html

