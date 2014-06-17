;===================================================================================================
;	 ______     ______        ______     ______    
;	/\  ___\   /\___  \      /\  __ \   /\  ___\   
;	\ \  __\   \/_/  /__     \ \ \/\ \  \ \___  \  
;	 \ \_____\   /\_____\     \ \_____\  \/\_____\ 
;	  \/_____/   \/_____/      \/_____/   \/_____/ 
;
;	An operating system for the Z8F64xx microcontroller
;	Written by Koen van Vliet
;	Last revision: June - 2014
;
;===================================================================================================
;   Notes to self:
;   - Filesystem functions do not share the same pointer register.
;	- Filesystem size is hardcoded. Might want to change that.
;	- When opening a file for writing the filename is not trailed with zeroes. Instead some 
;     garbage can appear trailing the termination character. Clean up when I have time.
;   - On error filesystem can return FFFF. This should be 0! otherwise the last page cannot be used
;     entirely!
;	- All output is on both gameduino and rs232!!!
;	- Line 339 In shell.asm is changed so Return characters are sent too!


;Insert system frequency. (crystal frequency)
	SYSFREQ		EQU	20000000

;===================================================================================================
; S Y S T E M   J U M P T A B L E
;===================================================================================================
	org 0800h
;0800h : I/O operations ----------------------------------------------------------------------------
	jp putc								;+0
	jp puts								;+3
	jp puts_r							;+6
	jp putFd							;+9
	jp puti								;+12
	jp puth								;+15
	jp 0								;+18
	jp 0								;+21
	jp 0								;+24
	jp 0								;+27
	jp 0								;+30
	jp 0								;+33
	jp 0								;+36
	jp 0								;+39
	jp 0								;+42
	jp 0								;+45
	jp spi_transfer						;+48
	jp spi_transfer_buffer				;+51
	jp 0								;+54
	jp 0								;+57
	jp 0								;+60
	jp 0								;+63
	jp 0								;+66
	jp 0								;+69
	jp fs_open_r						;+72
	jp fs_getc							;+75
	jp 0								;+78
	jp 0								;+81
	jp 0								;+84
	jp 0								;+87
	jp 0								;+90
	jp 0								;+93
;0860h : Memory operations ---------------------------------------------------------------------------------
	jp F_unlock							;+0
	jp F_erase_page						;+3
	jp 0								;+6
	jp 0								;+9
	jp 0								;+12
	jp 0								;+15
	jp 0								;+18
	jp 0								;+21
	jp 0								;+24
	jp 0								;+27
	jp 0								;+30
	jp 0								;+33
	jp 0								;+36
	jp 0								;+39
	jp 0 	 							;+42
	jp 0								;+48
;0890h : Video operations ----------------------------------------------------------------------------------
	jp gd_cpy							;+0
	jp gd_copy							;+3
	jp gd_fillmeup						;+6
	jp gd_char_pal						;+9
	jp 0								;+12
	jp 0								;+15
	jp 0								;+18
	jp 0								;+21
	jp 0								;+24
	jp 0								;+27
	jp 0								;+30
	jp 0								;+33
	jp 0								;+36
	jp 0								;+39
	jp 0								;+42
	jp 0								;+48
;08C0h : Math routines -------------------------------------------------------------------------------------
	jp div8								;+0
	jp div16							;+3
	jp 0								;+6
	jp 0								;+9
	jp str2int							;+12
	jp 0								;+15
	jp 0								;+18
	jp 0								;+21
	jp 0								;+24
	jp 0								;+27
	jp 0								;+30
	jp 0								;+33
	jp 0								;+36
	jp 0								;+39
	jp 0								;+42
	jp 0								;+48
;08F0h : Error messages ------------------------------------------------------------------------------------
	jp err_file_notFound						;+0


;===================================================================================================
; M A C R O'S
;===================================================================================================
ld16_im: MACRO d1,d2
	ld (d1),#HIGH(d2)
	ld (d1+1),#LOW(d2)
ENDMAC ld16_im

;===================================================================================================
; O S  S O U R C E  F I L E S
;===================================================================================================
	org 1000h
	include "ez8.inc"
	include "kernel.asm"
	include "shell.asm"
	include "errors.inc"
	include "programs.asm"

;===================================================================================================
; F I L E   M E M O R Y
;===================================================================================================
; Mapped from 8000h to FFFFh
; May contains configuration files as well
;---------------------------------------------------------------------------------------------------
	_filemem	equ 8000h
	org _filemem
