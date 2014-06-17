;===================================================================================================
; U S E R   B A U D R A T E  C O N F I G
;===================================================================================================
UART_BAUD   EQU 9600;115200
;----------------------------------------------------------------
;Baud rate calculation
UART_BRG    EQU ((SYSFREQ + (UART_BAUD * 8)) / (UART_BAUD * 16))

; UART control, status and data register mnemonics
U_BRH       EQU U0BRH
U_BRL       EQU U0BRL
U_CTL0      EQU U0CTL0
U_CTL1      EQU U0CTL1
U_STAT0     EQU U0STAT0
U_D         EQU U0D

EI_U: MACRO
    orx IRQ0ENH, #%10
    orx IRQ0ENL, #%10
ENDMAC

O_GD	equ 1
O_RS232	equ 0

VECTOR UART0_RX = isr_uart_rx

;===================================================================================================
; S E R I A L  I N I T
;===================================================================================================

; Intialize UART0
init_uart:
    ldx U_BRH, #HIGH(UART_BRG)
    ldx U_BRL, #LOW(UART_BRG)
    ldx U_CTL1, #%00     ; clear for normal non-Multiprocessor operation
    ldx U_CTL0, #%C0     ; Transmit enable, Receive Enable, No Parity, 1 Stop
    EI_U
    ret

;===================================================================================================
; S E R I A L  R O U T I N E S
;===================================================================================================

; Routine: putc
; Output a character to UART/video terminal
; R0: holds the character to send
; output_device can either be 0 or 1 for selecting the current output device
putc:
	cp output_device,#O_RS232
	jr eq,$F
	push R1
	call gd_putis
	pop R1
	 ret
$$:
	push R1
$$:
    ldx R1, U_STAT0
    and R1, #%04
    jr Z, $B
    ldx U_D, R0
    pop R1
    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Output a string to UART
; RR0: holds the address of the string
puts:
	push R0
	push R1
    push R2
    push R3
    ld R2, R0
    ld R3, R1
$$:
    ldc R0, @RR2
    cp R0, #0
    jr Z, $F
    call putc
    incw RR2
    jr $B
$$:
	pop R3
	pop R2
    pop R1
    pop R0
    ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Output a string to UART from register file instead of flash
; RR0: holds the address of the string
puts_r:
    push R2
    push R3
    ld R2, R0
    ld R3, R1
$$:
    ldx R0, @RR2
    cp R0, #0
    jr Z, $F
    call putc
    incw RR2
    jr $B
$$:
    pop R3
    pop R2
    ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Output data from Flash of SIZE bytes long
; RR0: holds the address of the data
; R4: The size
putFd:
    push R2
    push R3
	ld R2,R0
	ld R3,R1
$$:
    ldc R0, @RR2
    call putc
    incw RR2
    djnz R4,$B

    pop R3
    pop R2
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Output an integer to UART formatted as a string
; RR0: holds the integer
puti:
	cp	R1,#0
	cpc R0,#0
	jr nz,$F
	
	ld	R0,#30h
	call putc
	ret
$$:
	
	ld  R5,#00h
	
	ld	R2,#high(-10000)
	ld	R3, #low(-10000)
	call	Num1
	ld	R2,#high(-1000)
	ld	R3, #low(-1000)
	call	Num1
	ld	R2,#high(-100)
	ld	R3, #low(-100)
	call	Num1
	ld	R2,#high(-10)
	ld	R3, #low(-10)
	call	Num1
	ld	R3,R2
Num1:	
	ld	R4,#('0'-1)
Num2:
	inc	R4
	
	add R1,R3
	adc R0,R2
	
	jr	c,Num2
	
	sub R1,R3
	sbc R0,R2
	
	cp R5,1
	jr z,Num3
	
	cp R4,#('0')
	jr z,Num4
Num3:	
	ld R5,1
	push R0
	ld	R0,R4
	call	putc
	pop R0
Num4:	
	ret

	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;  Routine: puth
;;;;;  Print byte as hexadecimal
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
puth:
	push R0
	;Upper nibble
	srl R0 ;>>4
	srl R0
	srl R0
	srl R0
	add R0,#30h
	cp R0,#(30h + 9) ;0-9
	jr ule,$F
	add R0,#(10h - 9);A-F
$$:
	call putc
	pop R0
	;Lower nibble
	and R0,#0Fh
	add R0,#30h
	cp R0,#(30h + 9) ;0-9
	jr ule,$F
	add R0,#(10h - 9);A-F
$$:
	call putc
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Interrupt Service Routine for UART
; Will look up which routine to call
isr_uart_rx:
	;srp #1
	call @vect_uart_rx
	;srp #0
	iret
	
dummy_isr:
	;Do nothing and return immediately
	iret
	