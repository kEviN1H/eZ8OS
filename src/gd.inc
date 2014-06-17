;===================================================================================================
; e Z 8   G A M E D U I N O   L I B R A R Y
;    (for Z8F64xx microcontrollers)
;
; Ported to eZ8 assembly Koen van Vliet
; Last Revision: February 2014
;
;===================================================================================================

;---------------------------------------------------------------------------------------------------

;===================================================================================================
; M A C R O's
;===================================================================================================
gd_wr: MACRO addr,dat
	ldx (spi_buffer),	#HIGH(addr)
	orx (spi_buffer),	#80h
	ldx (spi_buffer+1),	#LOW(addr)
	ldx (spi_buffer+2),	#dat
	ldx spi_chars,#3
	call spi_transfer_buffer
ENDMAC gd_wr
;---------------------------------------------------------------------------------------------------
gd_rd: MACRO addr
	ldx (spi_buffer),	#HIGH(addr)
	orx (spi_buffer),	#80h
	ldx (spi_buffer+1),	#LOW(addr)
	ldx (spi_buffer+2),	#0
	ldx spi_chars,#3
	call spi_transfer_buffer
ENDMAC gd_rd
;---------------------------------------------------------------------------------------------------
gd_wr16: MACRO addr,dat
	ldx (spi_buffer),	#HIGH(addr)
	orx (spi_buffer),	#80h
	ldx (spi_buffer+1),	#LOW(addr)
	ldx (spi_buffer+2),	#LOW(dat)
	ldx (spi_buffer+3),	#HIGH(dat)
	ldx spi_chars,#4
	call spi_transfer_buffer
ENDMAC gd_wr16
;---------------------------------------------------------------------------------------------------	
gd_fill: MACRO addr,val,amt
	ld R1,#HIGH(addr)
	ld R0,#LOW(addr)
	ld R2,#HIGH(amt)
	ld R3,#LOW(amt)
	ld R4,#val
	call gd_fillmeup
ENDMAC gd_fill
;---------------------------------
_gd_move: MACRO GDptrSRC,GDptrDST,size,buff
	ld R2,#HIGH(GDptrDST)
	ld R3,#LOW(GDptrDST)
	ld R1,#HIGH(GDptrSRC)
	ld R0,#LOW(GDptrSRC)
	ld R4,#HIGH(size)
	ld R5,#LOW(size)
	ld R6,#HIGH(buff)
	ld R7,#LOW(buff)
	call gd_move
ENDMAC _gd_move

gd_fillmeup:
	gd_spi_start
	or R1,#80h
	call spi_transfer
	ld R1,R0
	call spi_transfer
$$:
	ld R1,R4
	call spi_transfer
	decw RR2
	jr nz,$B
	gd_spi_end
	ret
;---------------------------------------------------------------------------------------------------
gd_fill16: MACRO addr,val,amt
	ld R1,#HIGH(addr)
	ld R0,#LOW(addr)
	ld R2,#HIGH(amt)
	ld R3,#LOW(amt)
	ld R4,#LOW(val)
	ld R5,#HIGH(val)
	call gd_fillmeup16
ENDMAC gd_fill16

gd_fillmeup16:
	gd_spi_start
	or R1,#80h
	call spi_transfer
	ld R1,R0
	call spi_transfer
$$:
	ld R1,R4
	call spi_transfer
	ld R1,R5
	call spi_transfer
	decw RR2
	jr nz,$B
	gd_spi_end
	ret
;===================================================================================================
; G A M E D U I N O   R O U T I N E S
;===================================================================================================
init_gd:
	call delay_l									; Delay to give the gameduino some time to boot.
	
	gd_wr 	J1_RESET,1								; Halt co-processor
	
	gd_fill RAM_PIC, 0, 4095						; Zero all character RAM
	gd_fill RAM_SPRPAL, 0, 2048						; Sprite palletes black
	gd_fill RAM_SPRIMG, 0, 16384					; Clear all sprite data
	gd_fill VOICES, 0, 256							; Silence
	gd_fill PALETTE16A, 0, 128						; Black 16-, 4-palletes and COMM
	gd_fill16 RAM_SPR,400,512
	;Load defaults
	gd_wr16 PALETTE16A,8000h						; Set palette to transparent
	gd_wr16 SCROLL_X, 0 
	gd_wr16 SCROLL_Y,0 
	gd_wr 	JK_MODE,0 
	gd_wr 	SPR_DISABLE,0 
	gd_wr 	SPR_PAGE,0 
	gd_wr 	IOMODE,0 
	gd_wr16 BG_COLOR,c_black
	gd_wr16 SAMPLE_L,0 
	gd_wr16 SAMPLE_R,0 
	gd_wr16 SCREENSHOT_Y,0 
	gd_wr 	MODULATOR,64 
	;call spi_transfer
	ld output_device,#O_GD
	ld cr_x,#0
	ld cr_y,#0
	ld cr_scroll,#0
	ret
;---------------------------------------------------------------------------------------------------
	SCOPE
gd_putis:
	push R0
	push R1
	push R2
	push R3
	push R4
	push R5
	ld R4,#0
$cmpbackspace:
	cp R0,#08h
	jr ne,$cmpnewline
	dec cr_x
	cp cr_x,#255
	jr ne,$F
	cp cr_y,#0
	jr eq,$F
	ld cr_x,#49
	dec cr_y
$$:
	ld R0,#0
	ld R4,#1
	jr $putis
$cmpnewline:	
	cp R0,#0Dh
	jr ne,$putis
	ld cr_x,#0
	call gd_newline
    jr $end
$putis:	
	call gd_putchar
	
	cp R4,#0
	jr ne,$end
	inc cr_x
	cp cr_x,#50
	jr ne,$F
	ld cr_x,#0
	call gd_newline
$$:
$end:
	pop R5
	pop R4
	pop R3
	pop R2
	pop R1
	pop R0
	ret
;---------------------------------------------------------------------------------------------------
	SCOPE	
gd_newline:
	cp cr_y,#36
	jr uge,$F
	inc cr_y
	ret
$$: 
	_gd_move (RAM_PIC+64),(RAM_PIC),1024,spi_buffer
	_gd_move (RAM_PIC+64+1024),(RAM_PIC+1024),1024,spi_buffer
	_gd_move (RAM_PIC+64+2048),(RAM_PIC+2048),320,spi_buffer
	gd_fill RAM_PIC+2368,0,64
	ret
	
;---------------------------------------------------------------------------------------------------
gd_color: macro c1,c2,c3,c4
	ldx (spi_buffer+2),#LOW(c1)
	ldx (spi_buffer+3),#HIGH(c1)
	
	ldx (spi_buffer+4),#LOW(c2)
	ldx (spi_buffer+5),#HIGH(c2)
	
	ldx (spi_buffer+6),#LOW(c3)
	ldx (spi_buffer+7),#HIGH(c3)
	
	ldx (spi_buffer+8),#LOW(c4)
	ldx (spi_buffer+9),#HIGH(c4)
	call gd_char_pal
endmac gd_color

gd_char_pal:
	ld R0,#0
	rl16 R0,R1,3
	
	add R0,#(20h | 80h)
	
	ldx (spi_buffer+0),R0
	ldx (spi_buffer+1),R1
		
	ldx spi_chars,#10
	call spi_transfer_buffer
	ret

; Routine: gd_cursor
; put a character at (cr_x,cr_y)
; R0 holds the cursor symbol
gd_putchar:
	push R2
	push R3
	ld R2,cr_y
	and cr_y,#63
	ld R3,#64
	mult RR2
	add R3,cr_x
	adc R2,#0
	gd_spi_start
	ld R1,R2
	or R1,#80h
	call spi_transfer
	ld R1,R3
	call spi_transfer
	ld R1,R0
	call spi_transfer
	gd_spi_end	
	pop R3
	pop R2
	ret
;---------------------------------------------------------------------------------------------------
;Routine: gd_copy
;Copy from flash to gameduino's character ram
;RR0 = flash pointer to bitmap (low byte first!)
;RR2 = gameduino pointer
;RR4 = size
_gd_copy: MACRO Fptr,GDptr,size
	ld R2,#HIGH(Fptr)
	ld R3,#LOW(Fptr)
	ld R1,#HIGH(GDptr)
	ld R0,#LOW(GDptr)
	ld R4,#HIGH(size)
	ld R5,#LOW(size)
	call gd_copy
ENDMAC _gd_copy
gd_cpy:
	push R0
	ld R0,R1
	pop R1
gd_copy:
	gd_spi_start			;begin transmission
	or R1,#80h			;mode:write
	call spi_transfer	;send address high byte
	ld R1,R0
	call spi_transfer	;send address low byte
$$:
	ldc R1,@RR2			;load byte from flash
	call spi_transfer	;send byte to gameduino
	incw RR2			;increment flash pointer
	decw RR4			;decrement byte counter
	jr nz,$B			;repeat if there are bytes left to be sent
	gd_spi_end				;otherwise end transmission
	ret
;---------------------------------------------------------------------------------------------------	
;Routine: gd_move
;R1[bit7]=0 : Copy block of gameduino memory data to another part of gameduino memory
;
;RR0 = gameduino source pointer (low byte first!)
;RR2 = gameduino destination pointer
;RR4 = size
;R1[bit7]=1 : Write page buffer content to gameduino memory
;RR0 = gameduino destination pointer
;RR4 = size
	SCOPE
gd_mv:
	push R0
	ld R0,R1
	pop R1
gd_move:
	btjnz 7,R1,$writeBack
	push R4
	push R5
	push R6
	push R7
$writeBack:
	gd_spi_start			; Begin transmission mode:read
	call spi_transfer	; Send address high byte
	ld R1,R0
	call spi_transfer	; Send address low byte
$$:
	ldx R1,@RR6			; Load byte from page buffer
	call spi_transfer	; Send/receive byte
	ldx @RR6,R1			; Save byte to page buffer
	incw RR6			; Increment buffer pointer
	decw RR4			; Decrement byte counter
	jr nz,$B			; Repeat if there are bytes left to be tranceived
	gd_spi_end				; Otherwise end current transmission
	btjnz 7,R2,$end		; Check the mode. End if we already wrote back to ram	
	or R2,#80h			; Change mode to write
	ld R1,R2			; Load destination pointer in RR0
	ld R0,R3			;
	pop R7				; Pop buffer location off the stack
	pop R6
	pop R5				; Pop the block size off the stack
	pop R4				;
	jr $writeBack		; Write to another location
$end:
	ret
	
;===================================================================================================
; G A M E D U I N O   A D D R E S S E S
;===================================================================================================
	RAM_PIC     equ %0000    ; Screen Picture, 64 x 64 = 4096 bytes
	RAM_CHR     equ %1000    ; Screen Characters, 256 x 16 = 4096 bytes
	RAM_PAL     equ %2000    ; Screen Character Palette, 256 x 8 = 2048 bytes

	IDENT         equ %2800
	REV           equ %2801
	FRAME         equ %2802
	VBLANK        equ %2803
	SCROLL_X      equ %2804
	SCROLL_Y      equ %2806
	JK_MODE       equ %2808
	J1_RESET      equ %2809
	SPR_DISABLE   equ %280a
	SPR_PAGE      equ %280b
	IOMODE        equ %280c

	BG_COLOR      equ %280e
	SAMPLE_L      equ %2810
	SAMPLE_R      equ %2812

	MODULATOR     equ %2814

	SCREENSHOT_Y  equ %281e

	PALETTE16A equ %2840   ; 16-color palette RAM A, 32 bytes
	PALETTE16B equ %2860   ; 16-color palette RAM B, 32 bytes
	PALETTE4A  equ %2880   ; 4-color palette RAM A, 8 bytes
	PALETTE4B  equ %2888   ; 4-color palette RAM A, 8 bytes
	COMM       equ %2890   ; Communication buffer
	COLLISION  equ %2900   ; Collision detection RAM, 256 bytes
	VOICES     equ %2a00   ; Voice controls
	J1_CODE    equ %2b00   ; J1 coprocessor microcode RAM
	SCREENSHOT equ %2c00   ; screenshot line RAM

	RAM_SPR     equ %3000    ; Sprite Control, 512 x 4 = 2048 bytes
	RAM_SPRPAL  equ %3800    ; Sprite Palettes, 4 x 256 = 2048 bytes
	RAM_SPRIMG  equ %4000    ; Sprite Image, 64 x 256 = 16384 bytes

;===================================================================================================
; C O L O R   V A L U E S
;===================================================================================================
	c_red		equ	%7C00
	c_green		equ	%03E0
	c_dkgreen	equ	%00E0
	c_blue		equ	%001F
	c_black		equ	%0000
	c_white		equ	%7FFF
	c_alpha		equ %8000
	
	
	