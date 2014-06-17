eZ8OS
=====

An operating system for the eZ8 microcontroller

eZ8 OS

A free operating system for the eZ8 microcontrollers licenced under the GNU General Public Licence v2. This operating system is written from scratch in pure eZ8 assembly language.

Features
========

Kernel:

Simple filesystem in internal flash memory. Can be expanded using spi flash. Has built in wear leveling.
Output to rs232 serial or vga using a gameduino.
Input using rs232 serial or using a matrix keyboard using the built in driver software.
Various kernel routines available to the user using jumptables. (display,string,spi,filesystem,flash,gameduino,math)
Flash page0 protection 

OS:

Commandline interface from which you can run programs.
Built in text file viewer.
Built in image viewer.
Built in machine code monitor. Prints a page of flash on the screen as hexadecimal as well as ascii.
Built in file loader program. Loads files/software via rs232 serial.
