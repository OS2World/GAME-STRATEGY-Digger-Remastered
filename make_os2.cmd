@echo off

rem  Creates a binary file for OS/2 and eComStation (with Open Watcom C)
rem  Cleaning: make_os2.cmd clean
rem  
rem  OS/2 version features:
rem    F3                 - switch between full screen and window mode
rem    F12                - exit (instead F10)
rem    ALT + <numpad +>   - increase volume
rem    ALT + <numpad ->   - decrease volume
rem
rem  Andrey Vasilkin, 2014
rem  digi@os2.snc.ru

wmake -h -f makefile.os2 %1
