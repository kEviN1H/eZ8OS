; Graphics.inc
; Tile based graphics. For use with the Parallax Propeller multicore chip.

; Routine: copy tilemap
; RR2 holds pointer to the tilemap in flash
cp_tilemap:	
	;add R5,#20h		;Calculate address of last tile
	;adc R4,#01h
	ld R1,#0
row:
	ld R0,#0		;Amount of columns
tile:
	ldc R2,@RR4
	call w_tile
	incw RR4
	inc R0
	cp R0,#17
	jr ne,tile
	inc R1
	cp R1,#15
	jr ne,row

	ret

w_tile:
	push R0
	;Send x coordinate
	call putc
	call delayshort
	call delayshort

	
	;Send y coordinate
	ld R0,R1
	call putc
	call delayshort
	call delayshort
	
	;Send tile number
	ld R0,R2
	call putc
	call delayshort
	call delayshort
	call delayshort
	pop R0
	ret
	