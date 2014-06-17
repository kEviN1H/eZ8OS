;Kernel.inc
	include "simplemath.inc"
	include "simpledelay.inc"
	include "string.inc"
	include "simpleserial.inc"
	include "flash.inc"
	include "spi.inc"
	include "flashspi.inc"
	include "gd.inc"
	include "memory.inc"
	include "files.inc"

;===================================================================================================
; I N I T I A L I Z E   C O M P U T E R
;===================================================================================================
	vector RESET = init
init:
	di												; Disable interrupts
    srp #00h										; Set the working register base pointer
    ldx SPH,#HIGH(stack_base)						; Set stack base pointer
    ldx SPL,#LOW(stack_base)
    call init_gpio									; Initialize IO pins
    call init_uart									; Initialize Serial
	call init_flash									; Initialize flash controller
	ld output_device,#O_RS232						; Set the terminal output to serial
    call init_spi
	call init_gd									; Initialize the video display controller
	ldx input_string,#0								; First character is 0 (termination character)
	jp initShell									; Jump to the main shell program
	
;===================================================================================================
; I N I T   G P I O
;===================================================================================================
	DDR equ 01h
	AF  equ 02h
init_gpio:
	ldx PAOUT,#(1<<2)								; Set pin2 HIGH (CS on flash is active low)
    ldx PAADDR,#DDR									; Set TX and pin 2 on port A to output.
    ldx PACTL,#~((1<<5)|(1<<2))						; and all other to input pins.
    ldx PAADDR,#AF	
    ldx PACTL,#((1<<4)|(1<<5))
    
    ldx PFADDR,#DDR									; Set port F to input
    ldx PFCTL,#255
    
	;===Use this for the keyboard instead! PORTC is for the keyboard!===
    ldx PGADDR,#DDR
    ldx PGCTL,#0
	;===================================================================
	
    ldx PDADDR,#DDR									; Set port D to output
    ldx PDCTL,#0
	ldx PDADDR,#AF
	ldx PDCTL,#(1<<5)								; TX alternate function on pin 5
    ret
	