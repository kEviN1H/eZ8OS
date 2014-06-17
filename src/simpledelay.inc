;===================================================================================================
; S U P E R   L O N G   D E L A Y
;===================================================================================================
delay_l:
	push R0
	push R1
	push R2
	ld R0,#10h			;Load value 1000h in the 16 bit register pair RR0 (consists of R0 and R1)
	ld R1,#00h
$$:
	ld R2,#FFh			;Load value 0FFh in 8 bit register R2

	djnz R2,$			;inner loop
	decw RR0
	jr nz, $B			;While the register pair has not reached 0 (nz) yet jump back to outer
	pop R2
	pop R1
	pop R0
	ret					;Return from routine

;===================================================================================================
; L O N G   D E L A Y
;===================================================================================================
delay:
	push R0
	push R1
	push R2
	ld R0,#00h			;Load value 00FFh in the 16 bit register pair RR0 (consists of R0 and R1)
	ld R1,#FFh
$$:
	ld R2,#FFh			;Load value 0FFh in 8 bit register R2

	djnz R2,$			;Decrease
	decw RR0
	jr nz, $B			;While the register pair has not reached 0 (nz) yet jump back to outer
	pop R2
	pop R1
	pop R0
	ret					;Return from routine
	
;===================================================================================================
; S H O R T   D E L A Y
;===================================================================================================
delayshort:
	push R8
	push R9
	push R10
	ld R8,#00h			;Load value 000Bh in the 16 bit register pair RR0 (consists of R0 and R1)
	ld R9,#0Bh
$$:
	ld R10,#FFh			;Load value 0FFh in 8 bit register R2

	djnz R10,$			;Decrease
	decw RR8
	jr nz, $b			;While the register pair has not reached 0 (nz) yet jump back to outer
	pop R10
	pop R9
	pop R8
	ret					;Return from routine	