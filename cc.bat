rem echo off
set VERSION=15

sdcc -DTC4M -DVERSION=%VERSION% tc9main.c -o tc4m.ihx --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000
if errorlevel 1 goto exit
srec_cat -Disable_Sequence_Warnings tc4m.ihx -intel -random-fill 0x0 0x4000 -exclude 0x26 0x28 -l-e-checksum-neg 0x26 2 2 -o tc4m-V%VERSION%.bin -binary

sdcc -DTC2M -DVERSION=%VERSION% tc9main.c -o tc2m.ihx --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000
if errorlevel 1 goto exit
srec_cat -Disable_Sequence_Warnings tc2m.ihx -intel -random-fill 0x0 0x4000 -exclude 0x26 0x28 -l-e-checksum-neg 0x26 2 2 -o tc2m-V%VERSION%.bin -binary

sdcc -DTC70CM -DVERSION=%VERSION% tc9main.c -o tc70cm.ihx --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000
if errorlevel 1 goto exit
srec_cat -Disable_Sequence_Warnings tc70cm.ihx -intel -random-fill 0x0 0x4000 -exclude 0x26 0x28 -l-e-checksum-neg 0x26 2 2 -o tc70cm-V%VERSION%.bin -binary

sdcc -DTC2M -DBUFU -DVERSION=%VERSION% tc9main.c -o tc2mB.ihx --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000
if errorlevel 1 goto exit
srec_cat -Disable_Sequence_Warnings tc2mB.ihx -intel -random-fill 0x0 0x4000 -exclude 0x26 0x28 -l-e-checksum-neg 0x26 2 2 -o tc2mB-V%VERSION%.bin -binary

sdcc -DTC70CM -DBUFU -DVERSION=%VERSION% tc9main.c -o tc70cmB.ihx --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000
if errorlevel 1 goto exit
srec_cat -Disable_Sequence_Warnings tc70cmB.ihx -intel -random-fill 0x0 0x4000 -exclude 0x26 0x28 -l-e-checksum-neg 0x26 2 2 -o tc70cmB-V%VERSION%.bin -binary

rem eps1001 cannot cope with logn filenames
copy /y tc4m-V%VERSION%.bin tmp.bin
eps1001.exe tmp.bin T27128 BIN COM1 R+

:exit
