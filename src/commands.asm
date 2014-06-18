;===================================================================================================
; B U I L T  I N  C O M M A N D S
;===================================================================================================

str_help:
asciz "\rValid Instructions: add help commands info ping register cls flash erase compare open forth putis ascii iomon load execute image midi gdwrite\r"
prg_help:
	push R0
	push R1
	ld R0,#HIGH(str_help)
	ld R1,#LOW(str_help)
	call puts
	pop R1
	pop R0
	ret
	
str_info:
asciz "\rCode By Keoni29\rPlease Visit eZ8tut.sourceforge.net"
prg_info:
	push R0
	push R1
	ld R0,#HIGH(str_info)
	ld R1,#LOW(str_info)
	call puts
	pop R1
	pop R0
	ret


SCOPE
str_register:
asciz "\rRegister contains:"

prg_register:
	cp argc,#3
	jp ult,err_argument
	
	ld R0,#HIGH(str_register)
	ld R1,#LOW(str_register)
	call puts
	
	ld R0,argv+4
	ld R1,argv+5
	call str2int
	ld R2,R0
	ld R3,R1
	
	ld R0,argv+6
	ld R1,argv+7
	call str2dec
	ld R4,#0
	ld R5,R1
	
	ld R0,argv+2
	ld R1,argv+3
	ldx R1,@RR0
	cp R1,#'w'
	jr eq,$write
	cp R1,#'r'
	jr eq,$read
	jr $end
$write:
	cp argc,#4
	jp ult,err_argument
	ldx @RR2,R3
	ldx R0,@RR2
	call puth
	jr $end	
$read:
	cp argc,#2
	jp ult,err_argument
	ldx R0,@RR2
	call puth
$end:
	ret
SCOPE
str_decimal:
asciz" A / B ="
	prg_divide:
	ld R0,#HIGH(newline)
	ld R1,#LOW(newline)
	call puts
	
	cp argc,#3
	jr ult,$error
	
	ld R0,#HIGH(str_decimal)
	ld R1,#LOW(str_decimal)
	call puts
	
	ld R0,argv+2
	ld R1,argv+3
	call str2int
	push R0
	push R1
	
	ld R0,argv+4
	ld R1,argv+5
	call str2int
	ld R2,R0
	ld R3,R1
	pop R1
	pop R0
	
	call div16
	push R4
	push R5
	
	ld R0,R6
	ld R1,R7
	call puti
	ld R0,#'R'
	call putc
	ld R0,#0
	pop R1
	pop R0
	call puti

	ret
	$error:
		ret
	
	prg_cls:
		;ld R0,#HIGH(setup_term)    ;Load address of configuration data
		;ld R1,#LOW(setup_term)
		;ld R4,#4                ;Amount of bytes to send
		;call putFd                ;Send bytes over serial
		;call delay                ;Wait while the propeller returns to terminal mode
		
		gd_fill RAM_PIC, 0, 4095;10240		;Zero all character RAM
		gd_wr16 SCROLL_Y,0
		ld cr_scroll,#0
		ld cr_x,#0
		ld cr_y,#0
		
		ret
		
		SCOPE
str_flash:
asciz	"\rWriting to flash...\r"
str_flash2:
asciz	"\rCheck:"
	prg_flash:
		cp argc,#4
		jr ult,$error
		
		ld R0,#HIGH(str_flash)
		ld R1,#LOW(str_flash)
		call puts
		
		ld R0,argv+2
		ld R1,argv+3
		call str2dec
		push R1
		
		ld R0,argv+6
		ld R1,argv+7
		call str2dec
		push R1
		
		ld R0,argv+4
		ld R1,argv+5
		call str2dec
		pop R2
		pop R0
		; R0: flash page
		; R1: flash address
		; R2: data byte
		
		call F_unlock
		
		ld R4,R0
		ld R5,#2 ;*2
		mult RR4 ;page*256
		ld R4,R5
		ld R5,#0
		add R5,R1 ;+rel address
		adc R4,#0 ;= real address in flash	
		;call delay		
		ldc @RR4,R2
		;call delay
		ldx FCTL,#00h;lock flash
		
		ld R0,#HIGH(str_flash2)
		ld R1,#LOW(str_flash2)
		call puts 
		
		;call delay
		
		cpx FSTAT,#8
		jr ult,$F
		nop
	$$:
		
		ldc R1,@RR4
		ld R0,#0
		call puti
		
		$error:
			ret
	SCOPE		
	prg_erase:
		cp argc,#2
		jr ult,$error
		
		ld R0,argv+2
		ld R1,argv+3
		call str2dec
		ld R0,R1
		
		call F_erase_page

		$error:
			ret
			
	SCOPE
	
	SCOPE
str_open:
	asciz	" 0 1 2 3 4 5 6 7 8 9 A B C D E F\r"
prg_open:
	call prg_cls
	
	_gd_move (RAM_CHR+(16*'0')),(RAM_CHR+(128*16)),(10*16),spi_buffer		; 0,1,2,3,4,5,6,7,8,9
	_gd_move (RAM_CHR+(16*'A')),(RAM_CHR+(138*16)),(6*16),spi_buffer 		; A,B,C,D,E,F
	
	ld R0,#HIGH(str_open)
	ld R1,#LOW(str_open)
	call puts
	
	cp argc,#2
	jr ult,$error
	
	ld R0,argv+2
	ld R1,argv+3
	call str2int
	
	ldx FPS,R1
	
	ld R2,R1
	ld R3,#2 ;*2
	mult RR2 ;page*256
	ld R2,R3
	ld R3,#0
	
	ld R0,#HIGH(page_buffer)
	ld R1,#LOW(page_buffer)
	
	ld R5,#32
	ld R7,#0		;Current color value
$outer:
	ld R6,#16
	push R2
	push R3
	$inner:
		ldc R4,@RR2
		ldx @RR0,R4
		push R0
		;cp R4,#FFh
		;jr ne,$notFF
		;ld R0,#(' ')	;if the value is FF do not show anything. The flash memory is most likely empty
		;call putc
		;call putc
		;jr $end
	;$notFF:
		ld R0,R4
		btjnz 0,R7,$F
		call puth
		jr $end
	$$:
		call puth_alt
	$end:
		;ld R0,#(',')
		;call putc
		xor R7,#01
		pop R0
		incw RR0
		incw RR2
		djnz R6,$inner
	pop R3
	pop R2
	ld R6,#16
	$inner2:
		ldc R4,@RR2
		push R0
		cp R4,#32				;If the ascii value is lower than 32 display an empty space instead
		jr ult,$notAlphaNum
		cp R4,#127
		jr ugt,$notAlphaNum
		ld R0,R4
		jr $end2
	$notAlphaNum:
		ld R0,#' '
	$end2:
		call putc
		pop R0
		incw RR0
		incw RR2
		djnz R6,$inner2
		
		ld R0,#0Dh;New line (CR)
		call putc
	djnz R5,$outer
$$:
$error:
	ret
;  Routine: puth_alt
;  Print byte as hexadecimal using the secondary character set for a different color text.
puth_alt:
	push R0
	;Upper nibble
	srl R0 ;/16
	srl R0
	srl R0
	srl R0
	add R0,#128
	call putc
	pop R0
	;Lower nibble
	and R0,#0Fh
	add R0,#128
	call putc
	ret
;---------------------------------------------------------------------------------------------------
;Program: Forth interpreter.
SCOPE
forth_tokens:
	asciz "OK!"
prg_forth:
	cp argc,#2
	jp ult,err_argument
	;ld R3,#LOW(routine_table)
	;add R3,#0					;Offset of 0 for first program: str_tok
	;adc R2,#HIGH(routine_table)
	;ldc R4,@RR2					;Load program address
	;incw RR2
	;ldc R5,@RR2

	ld R0,argv+2
	ld R1,argv+3
	ldwx RR2,RR0				;Set the string pointer to that of the original string
								;So the original appears at the same memory address.
	call str_tok							
	;call @RR4					;call OS routine
	
	ld R0,#0
	ld R1,argc
	call puti
	ret
;---------------------------------------------------------------------------------------------------	
prg_ascii:
	call prg_cls
	
	_gd_move (RAM_CHR+(16*'0')),(RAM_CHR+(128*16)),(10*16),spi_buffer		; 0,1,2,3,4,5,6,7,8,9
	_gd_move (RAM_CHR+(16*'A')),(RAM_CHR+(138*16)),(6*16),spi_buffer 		; A,B,C,D,E,F
	
	ld R0,#0
$$:
	push R0
	call gd_putis
	pop R0
	djnz R0,$B
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		
;Change character X's palette
prg_color:
	ld R0,argv+2
	ld R1,argv+3
	call str2dec
	;           00     01     10      11
	gd_color c_white,c_black,c_green,c_red
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Prg_iomon I/O monitor program
SCOPE
	include "chr_gfx.inc"
str_portb:
	asciz "Port B input register:"
prg_iomon:
	call prg_cls
	push cr_x
	push cr_y
	_gd_copy chr_boolean,(RAM_CHR+(128*16)),16	;Write char#128 bitmap data
	ld R1,#128
	gd_color c_white,c_black,c_green,c_black	;Set char#128 color palette
	_gd_copy chr_boolean,(RAM_CHR+(129*16)),16	;Write char#129 bitmap data
	ld R1,#129
	gd_color c_white,c_red,c_green,c_red		;Set char#129 color palette
	
	ld R0,#HIGH(str_portb)
	ld R1,#LOW(str_portb)
	ld cr_x,#8
	ld cr_y,#11
	call puts
	;gd_fill (RAM_PIC+(12*64)+8),128,8			;Fill a row of characters
	ld16_im vect_uart_rx,$check_rx
	ei
	ldx rx_buffer,#0
$loop:
	ldx R2,PBIN
	ld cr_x,#8
	ld cr_y,#12
	ld R1,#8
$$:	
	ld R0,#1
	and R0,R2
	add R0,#128
	call putc
	srl R2
	djnz R1,$B
	cp rx_buffer,#0
	jr eq,$loop
	ldx rx_buffer,#0
	di
	pop cr_y
	pop cr_x
	ld16_im vect_uart_rx,process_rx
	ret
	
$check_rx:
	ldx rx_buffer,U0RXD
	ret

SCOPE
str_load:
	asciz	"\rEstablishing Connection\r"
str_load2:
	asciz	"Writing to FFile\r"
str_loadfail:
	asciz	"\rCould not Connect\r"
str_load3:
	asciz	"\rConnection Timed Out. Written to file succesfully.\r"
prg_load:
	cp argc,#3
	jr ult,$error
	
	ld R0,#HIGH(str_load)
	ld R1,#LOW(str_load)
	call puts
	
	ld R0,argv+2
	ld R1,argv+3
	
	call fs_open_w
	cp R3,#LOW(0)						; Check if there was an error
	cpc R2,#HIGH(0)
	jr eq,err_memory
	
	
	ld output_device,#O_RS232			; Set output device to RS232 serial port
	ld R0,#1
	call putc							; Set mode to Read File (1)
	ld R0,argv+2
	ld R1,argv+3
	call puts_r
	ld output_device,#O_GD
	
	ld16_im vect_uart_rx,$receive_file	; The rx interrupt will now call the receive_file routine
	ei
$$:
	
	jr ne,$B
	di									; Disable interrupts
	ld16_im vect_uart_rx,process_rx		; Set the rx interrupt vector back to its default routine
	;call delay
	call fs_close_w						; Close file		
$error:
	ld output_device,#1
	ret
	
$receive_file:
	ldx R0,U0RXD
	call fs_putc
	ld R8,#HIGH(0200h)
	ld R9,#LOW(0200h)
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Prg_execute allows executing programs from any page in flash.
SCOPE
	prg_execute:
		cp argc,#2
		jr ult,$error
		ld R0,argv+2
		ld R1,argv+3
		call str2int
		ld prgm_base,R0
		ld prgm_base+1,R1
		call @RR0
		ret
	$error:		
		ret

	SCOPE
prg_image: 
	gd_wr16 PALETTE16A+48,c_alpha
	gd_wr16	PALETTE16B+48,c_alpha
	gd_wr16 PALETTE4A,c_alpha
	gd_wr16 PALETTE4B,c_alpha
	
	call prg_cls			;Clear screen
	
	ld R6,#27				;Image height
	ld R2,#HIGH(_filemem)
	ld R3,#LOW(_filemem)
	ld R1,#HIGH(RAM_PIC+12+(64*5))	;Position image to xtile 11,ytile 5
	ld R0,#LOW(RAM_PIC+12+(64*5))	;
$$:
	push R0
	push R1
	ld R4,#HIGH(25)			;Image width
	ld R5,#LOW(25)
	call gd_copy
	pop R1
	pop R0
	add R0,#64
	adc R1,#0
	djnz R6,$B
	_gd_copy _filemem+675,RAM_CHR,1000h
	_gd_copy _filemem+675+1000h,RAM_PAL,0800h
	ret
		
prg_gdwrite:
	ld R0,argv+2
	ld R1,argv+3
	call str2int
	or R0,#80h
	ldx (spi_buffer+0),R0
	ldx (spi_buffer+1),R1
	
	ld R0,argv+4
	ld R1,argv+5
	call str2int
	ldx (spi_buffer+2),R1
	ldx spi_chars,#3
	call spi_transfer_buffer
	
	ret
	
;Program: dir
;Prints a list of all files in the filesystem in internal flash
	SCOPE
prg_dir:
	ld R0,#13
	call putc
	ld R4,#HIGH(0002h)						; Looking for a file-identifier 0002h (start of file)
	ld R5,#LOW(0002h)
	ld R2,#HIGH(8000h)						; Base address of filesystem
	ld R3,#LOW(8000h)
$$:
	call fs_next							; Search for next file
	jr c,$end								; If no files are left in the filesystem return to OS
	incw RR2
	ld R0,R2
	ld R1,R3
	call puts								; Print the filename
	ld R0,#13								; Newline
	call putc
	add R2,#02h							; Skip to the next block
	ld R3,#00h
	jr $B
$end:
	ret
	
str_eof:
	asciz "\r---End Of File----\r"
prg_fprint
	cp argc,#2
	jr ult,err_argument

	ld R0,argv+2
	ld R1,argv+3
	call fs_open_r
	cp R3,#LOW(0)							; When file was not found:
	cpc R2,#HIGH(0)
	jr eq,err_file_notFound					; Display an error message
$$:
	call fs_getc
	call putc
	cp R3,#LOW(0000h)						; While there are bytes to read: read byte
	cpc R2,#HIGH(0000h)
	jr ne,$B
	ld R0,#HIGH(str_eof)
	ld R1,#LOW(str_eof)
	call puts
	ret
	
prg_fwrite:
	ld R0,argv+2
	ld R1,argv+3
	call fs_open_w
	cp R3,#LOW(0)							; Check if there was an error
	cpc R2,#HIGH(0)
	jr eq,err_memory
	; Write a file 512 bytes long filled with the letter A
	ld R5,#2				; *2
$outer:
	ld R4,#0				;256 repeats
$inner:
	ld R0,#'A'
	add R0,R5
	push R4
	push R5
	call fs_putc
	pop R5
	pop R4
	cp R3,#LOW(-1)
	cpc R2,#HIGH(-1)
	jr eq,err_memory
	djnz R4,$inner
	djnz R5,$outer
	call fs_close_w
	ld R0,_blockno
	ld R1,_blockno+1
	call puti
	ret
	
; Program: SPI flash read identification
prg_rdid:
	spi_start
	ld R1,#9Fh
	call spi_transfer			; Send command: RDID
	
	ld R2,#3					; Initialize counter
$$:
	call spi_transfer			; Read data byte from spi
	ld R0,R1					; 
	call puth					; Print as hex on screen
	ld R0,#' '					; Separate with a space
	call putc
	djnz R2,$B					; Repeat 3 times
	
	ld R2,#78					; Initialize counter
$$:
	call spi_transfer			; Read data byte from spi
	ld R0,R1					; 
	call puth					; Print as hex on screen
	djnz R2,$B					; Repeat 78 times
	spi_end
	ret
	