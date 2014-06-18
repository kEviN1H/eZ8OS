;===================================================================================================
; S T R I N G   R O U T I N E S
;===================================================================================================

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Options for string functions:
TERMINATE_CHAR EQU 00h
;	Terminate character: Default set to 0 (zero termination)
;------------------------------------------------------------------

SCOPE
str_tok:
	;brk
	push R2
	push R3
	ld R5,#(1<<1)	;Set flag to indicate we are searching for the first token!
	ld argc,#0
	ld R7,#argv
$parseloop:
	ldx R4,@RR2
	cp R4,#0
	jr ne,$F
	pop R3
	pop R2
	ret
$$:
	cp R4,#22h ; ?char = quotation mark?
	jr ne,$F
	btjnz 0,R5,$endQuote
$startQuote
	ld R4,#0
	bset 0,R5
	bset 1,R5
	jr $endswitch
$endQuote:
	bclr 0,R5
	bset 1,R5
	ld R4,#0
	jr $endswitch
$$:	
	btjnz 0,R5,$F
	cp R4,#20h ; ?char = space?
	jr ne,$F
	ld R4,#0
	bset 1,R5
	jr $endswitch
$$:
$default:			; if it got all the way to this point there was no separation character found!
	btjz 1,R5,$F	; if searching for a new token
		ld @R7,R2   ; save pointer to parameter string in argv[argc]
		inc R7
		ld @R7,R3
		inc R7
		inc argc	;increment argc
		bclr 1,R5
$$:
$endswitch:
	ldx @RR2,R4
	incw RR2
	jr $parseloop
	
	
	
	;f_quote 	equ 0
	;f_search 	equ 1
	;f_escape	equ 2


;routine: str_Fcopy
;Copy string from flash to register file
;[15 bytes]
;~27 cycles per character
;~31 cycles for termination character + return
;Input:
;	RR0 Flash pointer to string
;	RR2 Register pointer to output string
;Output:
;	String in register file
Fstr_copy:
	ldc R4,@RR0				;Load character from flash	
	ldx @RR2,R4				;Load the character from R4 to register										
							;The character is now copied	
	incw RR0				;Increase flash pointer
	incw RR2				;Increase register pointer
	
	cp R4,#TERMINATE_CHAR	;Check if it is a termination character
	jr nz,Fstr_copy			;	if it is not: copy another character
	ret						;	otherwise return
	
	

backspace:
	ld R5,#0
$$:
	ldx R4,@RR2		;load character at the string pointer
	incw RR2
	inc R5
	cp R4,#0		;check if the string terminated there
	jr nz,$B
	cp R5,#1		;if the string is empty: return and do not perform any action
	jr z,$F
	ld R1,#0
	decw RR2		;decrement the pointer
	decw RR2		;decrement the pointer
	ldx @RR2,R1		;terminate the string with 0
$$:
	ret

str_append_single:
	ldx R4,@RR2		;load character at the string pointer
	incw RR2
	cp R4,#0		;check if the string terminated there
	jp nz,str_append_single
	ld R1,#0
	ldx @RR2,R1		;terminate the string with 0
	decw RR2		;decrement the pointer
	ldx @RR2,R0		;put the character at the end of the string
	ret

		
;routine: str_compare_set
;Tokenize single string to a token at register pointer
SCOPE
str_compare_set:
	ld R8,R2					;Copy register pointer
	ld R9,R3
compare_token:
	ld R2,R8					;Reset register pointer to the beginning of the string
	ld R3,R9					;
	call str_compare				;Call a string compare routine
	jr z,ret_match				;Return Z=1 if there is a match
	djnz R6,$next				;Counter -1. (eventually we run out of possible tokens)
	jr ret_match				;If we ran out of tokens to compare to: return Z=0
$next:
	ldc R4,@RR0					;Load character at flash pointer
	incw RR0					;Increase the flash pointer
	cp R4,#TERMINATE_CHAR		;and check if the value at the pointer is a termination character	
	jr nz,$next					;if it's not zero: jump to inc
	jr compare_token			;if it IS zero: go back to compare_token
ret_match:
	ret
	
;routine: str_compare
;Compare string from flash with string at register
;[22 bytes]
;~46 cycles per match
;~32 cycles on termination character + return
;~63 cycles on mismatch + return
;Input:
;	RR0 Flash pointer to string
;	RR2 Register pointer to string
;Output:
;	If the string matches: Z = 1
;	If the string does not match: Z = 0
;	also: R4 != R5
;
str_compare:
	ldc R4,@RR0				; Load character from flash
	cp R4,#TERMINATE_CHAR	; Check if the token in flash terminates
 	ldx R5,@RR2				; Load character from register file
	jr z,c_comp				; 	if zero flag is set >> jump to c_comp
	incw RR0				; Advance to the next character
	incw RR2
	call c_comp				; Call character compare routine
	jr z,str_compare			; If characters matched start over for next character
c_comp:	
	cp R4,R5				; Compare the characters
	ret	

;Routine str2dec
;RR0 = register pointer to string
;RR0 = decimal value
SCOPE
str2dec:
	push R2;added
	push R3
	push R4
	
	ld R3,#0		;load 0 in R3
	ldx R4,@RR0		;load the first character of the string in R4
$loop:
	cp R4,#0		;is the character a termination character?
	jr z,$F			;if it is: jump to the end of the routine
	ld R2,#10		;otherwise multiply R2 by 10
	mult RR2		;
	sub R4,#30h		;subtract '0' from R4 to convert from ascii char to decimal digit
	add R3,R4		;add the digit to R3
	incw RR0		;increment string pointer
	ldx R4,@RR0		;load next character
	jr $loop		;jump back to the start
$$:
	ld R0,#0		;load the number in RR0 (only the lower byte contains data)
	ld R1,R3
	
	pop R4
	pop R3
	pop R2;added
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Routine int2dec
;RR0 = register pointer to string
;RR0 = decimal value
SCOPE
str2int:
	push R2
	push R3
	push R4
	push R5
	ld R3,#0		;initialize R3
	ld R5,#0		;initialize R4
$loop:
	ldx R15,@RR0	;Load character from string
	cp R15,#30h		;Check if character represents a decimal digit
	jr ult,$end		;Otherwise terminate the routine
	cp R15,#39h
	jr ugt,$end
	ld R2,#10		;Multiply integer by 10
	ld R4,#10
	mult RR2
	mult RR4
	add R5,R2
	sub R15,#30h
	add R3,R15
	adc R5,#0
	incw RR0
	jr $loop
$end:
	ld R0,R5
	ld R1,R3
	pop R5
	pop R4
	pop R3
	pop R2
	ret
	