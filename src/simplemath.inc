;===================================================================================================
; M A T H  R O U T I N E S
;===================================================================================================

;Warning: Do not use with register R15!
rl16 macro reg0,reg1,amt
	ld R15,#amt
$$:
	rlc reg1
	rlc reg0
	;and reg1,#~1
	djnz R15,$B
endmac rl16

;routine: random
;random:
;	adc R15,@R0
;	rl R15
;	xor R15,#92h
;	ret

;Routine: div8
; R4 = R0/R1
; remainder: R2
	SCOPE
div8:
	ld R2,#0
	ld R3,#8
$loop:
	rlc R0
	rlc R2		; Shift R0 into R2
	cp R1,R2    ; If R2 is less than...
	jr nz,$F	; or equal to R1			(if Z : set C)
	scf
$$:
	rlc R4
	cp R1,R2
	jr ugt,$F
	sub R2,R1  ; Subtract R1 from R2
$$:
	djnz R3,$loop
	ret



;Routine: div16
;RR6 = RR0 / RR2
;remainder: RR4
;uses R0-7 and R15
	SCOPE
div16:
	ld R4,#0
	ld R5,#0
	ld R15,#16
$loop:
	rlc R1
	rlc R0
	rlc R5
	rlc R4
	cp R3,R5
	cpc R2,R4
	jr nz,$F
	scf
$$:
	rlc R7
	rlc R6
	cp R3,R5
	cpc R2,R4
	jr ugt,$F
	sub R5,R3
	sbc R4,R2
$$:
	djnz R15,$loop
	ret
	
	

	
	