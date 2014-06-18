; S h e l l  I n t e r f a c e
; For the eZ8 operating system.
; Written by Koen van Vliet
; Last revision: June 2014


;===================================================================================================
; B O O T   M E S S A G E
;===================================================================================================
; Temporary fix until I implement boot message files.
bootmsg:
    ascii   "     *** eZ8 computer system ***     \r"
	asciz	"64K ROM, 4K RAM, "

bootmsg2:
	ascii	" commands built in.\r"
	ascii	"by Keoni29 -- eZ8tut.sourceforge.net\r\r"
    asciz   "Ready to rock!\r"
;===================================================================================================
; K E Y  M A P
;===================================================================================================
; Temporary fix until I implement keymap files.
keymap:
    db  '1',00h,00h,00h,' ',00h,'q','2';Col A
    db  '!',00h,00h,00h,' ',00h,'Q','"';Col A
    db  '3','w','a',00h,'z','s','e','4';Col B
    db  '#','W','A',00h,'Z','S','E','$';Col B
    db  '5','r','d','x','c','f','t','6';Col C
    db  '%','R','D','X','C','F','T','&';Col C
    db  '7','y','g','v','b','h','u','8';Col D
    db  "'",'Y','G','V','B','H','U','(';Col D
    db  '9','i','j','n','m','k','o','0';Col E
    db  ')','I','J','N','M','K','O','0';Col E
    db  '+','p','l',',','.',':','@','-';Col F
    db  '+','P','L','<','>','[','@','-';Col F
    db  '$','*',';','/',00h,'=',00h,00h;Col G
    db  '$','*',']','?',00h,'=',00h,00h;Col G
    db  08h,0Dh,00h,00h,00h,00h,00h,00h;Backspace,Return(newline)
    db  08h,0Dh,00h,00h,00h,00h,00h,00h;Backspace,Return(newline)    
newline:
    asciz    "\r"

;===================================================================================================
; B U I L T  I N  C O M M A N D S
;===================================================================================================
; Note: Temporary fix until I implement executable files.
;
; To add a new command simply add it to this list.
; Make sure to add both the token and the label.
program_table:
	dw  prg_divide
	dw	prg_help
	dw	prg_help
	dw	prg_info
	dw	prg_register
	dw	prg_cls
	dw	prg_flash
	dw	prg_erase
	dw	prg_open
	dw	prg_forth
	dw 	prg_ascii
	dw  prg_iomon
	dw	prg_load
	dw  prg_execute
	dw	prg_image
	;dw	prg_midi
	dw	prg_gdwrite
	dw	prg_dir
	dw	prg_fprint
	dw	prg_fwrite
	dw	prg_rdid

compare_string:
	asciz "divide"
    asciz "help"
    asciz "commands"
    asciz "info"
    asciz "register"
	asciz "cls"
	asciz "flash"
	asciz "erase"
	asciz "open"
	asciz "forth"
	asciz "ascii"
	asciz "iomon"
	asciz "load"
	asciz "execute"
	asciz "image"
	;asciz "midi"
	asciz "gdwrite"
	asciz "dir"
	asciz "fprint"
	asciz "fwrite"
	asciz "rdid"
	db 0

	
success_message:
    asciz "\rREADY\r"
fail_message:
    asciz "\rSYNTAX ERROR\r"

;===================================================================================================
; S H E L L   M A I N   P R O G R A M
;===================================================================================================
initShell:
	ld16_im vect_uart_rx,process_rx
	gd_wr16 BG_COLOR,c_blue
	
	;Count available tokens:
	ld R0,#HIGH(compare_string)
	ld R1,#LOW(compare_string)
	ld tokenset_size,#0

$$:
	ldc R3,@RR0
	incw RR0
	cp R3,#0
	jr ne,$B
	ldc R3,@RR0
	incw RR0
	inc tokenset_size
	cp R3,#0
	jr ne,$B
$$:	
	ld output_device,#O_RS232
	call term_bootmsg	
	ld R0,#00
	;ld output_device,#O_RS232
	EI
	
	ldx input_string,#0								; First character is 0 (termination character)
	;jp inputUserString

inputUserString:
	;call delay_l
	;xor R0,#FFh
	;call gd_putchar
	;jr inputUserString
	
    call getc                ;Get a column of key triggers
    jr z,inputUserString    ;If no keys are hit: re-scan the keyboard
	dec R1    ;fixes some sloppy code with this instruction
    ;R0 = x[7..0] == 8*keys[1..0]
    ;R1 = column
    ld R8,#16                
    ld R9,R1
    mult RR8
    add R9,#LOW(keymap)        	;Column number * 8 + keymap base ptr
    adc R8,#HIGH(keymap)
    
	; Check if shift is held down
    ld R4,(keypress_data+1)
    and R4,#(1<<3)            	;Check if shift is held down
    jr z,$F                   	;
    add R9,#8                 	;If it is: Select the shifted keys
    adc R8,#0
$$: 
ifkeypress:
    btjz 0,R0,$F     ;Check if key is triggered and if it is: process the hit
	push R0
	call processKeyHit
	pop R0
$$:
    incw RR8
    srl R0                     	;shift right
    jr nz,ifkeypress         	;while there are still keys to process jump back to ifkeypress
    jr inputUserString        	;when all keys are processed jump back to the main loop
    
processKeyHit:    
    ;Check if a character should be printed
    ldc R0,@RR8                	;If so: load character from character map
    ld keyboard_char,R0        	;keyboard_char = R0
    cp keyboard_char,#0        	;if keyboarchar = 0 no character should be printed
    jr z,ifkeypress
    
    ;Append character to the user input string
    ld R2,#HIGH(input_string)
    ld R3,#LOW(input_string)
    cp keyboard_char,#08h
    jr nz,$F
    call backspace
    jr skipChar
$$:
    cp keyboard_char,#0Dh
    jr nz,$F
    call parse_input
	call putc
    ;jr skipChar
	ret
$$:
    ld    R0,keyboard_char
    call str_append_single
skipChar:
    ld    R0,keyboard_char
    call putc
    call delay
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;  END OF MAIN LOOP                                                    ;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;routine: parse_input
SCOPE
parse_input:
	ld R0,#00			;hide cursor icon
	call gd_putchar
	
	push R1
	push R2
	push R3
	
	ld R0,R2				;Set the string pointer to that of the original string
	ld R1,R3				;So the original appears at the same memory address.
	call str_tok
	;brk
	
	ld R2,argv+0
	ld R3,argv+1
	
	ld R0,#HIGH(compare_string)
	ld R1,#LOW(compare_string)
	ld R6,tokenset_size
	call str_compare_set
	jp z,$yes
$no:
	ld R0,#HIGH(fail_message)
	ld R1,#LOW(fail_message)
	call puts
	jr $F
	
$yes:
	cp R6,#0					;If the token was not found R6=0
	jp z,$no
	;dec R6
	ldx R0,tokenset_size
	sub R0,R6
	ld R6,R0
	rl R6						;*2
	ld R3,#LOW(program_table)
	add R3,R6
	ld R2,#0
	adc R2,#HIGH(program_table)
	ldc R0,@RR2					;Load program address
	incw RR2
	ldc R1,@RR2
	
	di
	call @RR0					;Execute program
	ei
	
	ld R0,#HIGH(success_message)
	ld R1,#LOW(success_message)
	call puts
$$:		
	ld R1,#0
	pop R3
	pop R2
	ldx @RR2,R1					;Clear user input string
	pop R1
	ld R0,#FFh
	call gd_putchar
	ret

    
;Routine: getKey
; R0 holds column
; R0 returns row
getCol:
    ldx PGADDR,#DDR                	;Set port C to output
    com R0
    ldx PGCTL,R0
    com R0
    
    ldx PGOUT,R0          			;Put row on port G
		
    call delayshort       			;Give the lines some time to settle
    ldx R0,PFIN           			;Read the row and store in R0
    ret
    
SCOPE    
getc:
    ld R1,#8            			;Set column to H (last row)
    ld R3,#128
scanKbd:
    ld R0,R3            			;Copy R3 to R0
    rr R3
    call getCol            			;Get the column bits
    ld R2,R1            			;Load R2 with the current row index
    add R2,#(keypress_data-1)    	;Add the address of the key register to that number
    push R0                			;Push the column bits onto the stack
    com @R2                			;Invert the key register
    and R0,@R2            			;R0 = Key bits AND NOT register bits
    pop @R2                			;Set register to current key bits
    ;call delay
    jr nz,$return        			;When a key is detected return R1 = column no. and R0 = Column key bits
    djnz R1,scanKbd        			;Decrease column index and scan next row.
$return:
    ret
    
term_bootmsg:
    ;ld R0,#HIGH(setup_term)    ;Load address of configuration data
    ;ld R1,#LOW(setup_term)
    ;ld R4,#4                ;Amount of bytes to send
    ;call putFd                ;Send bytes over serial
    ;call delay                ;Wait while the propeller returns to terminal mode
    
    ld R0,#HIGH(bootmsg)    ;Load string pointer
    ld R1,#LOW(bootmsg)
    call puts                ;Send string over serial
    
	ld R0,#0
	ld R1,tokenset_size
	call puti
	
	ld R0,#HIGH(bootmsg2)    ;Load string pointer
    ld R1,#LOW(bootmsg2)
    call puts                ;Send string over serial
	
    
    ;ld R0,#HIGH(text_color)    ;Load address of register data
    ;ld R1,#LOW(text_color)
    ;ld R4,#2                ;Amount of bytes to send
    ;call putFd                ;Send bytes over serial
    ret
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;                                                                      ;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	SCOPE
process_rx:
	push R0
	push R1
	push R2
	push R3
	
	ld R0,#00
	call gd_putchar
	
	ldx R0,U0RXD
	ld R2,#HIGH(input_string)
    ld R3,#LOW(input_string)
	
	cp R0,#08h
    jr ne,$F
	jr $backspace
$$:
	cp R0,#7Fh
	jr ne,$F
$backspace:
    call backspace
	cp R5,#1
	jr eq,$end
	ld R0,#08h
	call gd_putis ; used to be putc
	jr $end
$$:
	cp R0,#0Dh
    jr ne,$F
	call gd_putis
    call parse_input
	jr $end
$$:

	call gd_putis;used to be call putc
    call str_append_single
$end:
	ld R0,#FFh
	call gd_putchar
	pop R3
	pop R2
	pop R1
	pop R0
	ret
	
	EXTERN prg_add,prg_help,prg_help,prg_info,prg_ping,prg_register
	EXTERN prg_cls,prg_flash,prg_erase,prg_compare,prg_open,prg_forth,prg_putis
	EXTERN prg_ascii,prg_iomon,prg_load,prg_execute,prg_image,prg_midi,prg_gdwrite
	