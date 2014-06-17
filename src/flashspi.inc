; SPI flash
; Tested with: 
; Device:	S25FL127SABMFI101
; Farnell:	2328002

; Routine:SPI flash initialize
init_fspi:
	ret

; Routine: SPI flash bulk erase
fspi_b_erase:
	spi_start
	ld R1,#60h
	call spi_transfer			; Send command: BE (Bulk erase)
	spi_end

;Routine: SPI flash write page
fspi_wp:
	spi_start
	ld R1,#06h
	call spi_transfer			; Send command: WREN (write enable)
	spi_end
	
	spi_start
	ld R1,#12h
	call spi_transfer			; Send command: 4PP (Page program)
	spi_end	
	ld R2,#HIGH(512)			; Initialize counter
	ld R3,#LOW(512)
$$:
	ldx R1,@RR4
	call spi_transfer			; Write data byte to flash
	incw RR4
	decw RR2
	jr nz,$B					; Repeat 512 times
	spi_end
	
	spi_start
	ld R1,#04h
	call spi_transfer			; Send command: WRDI (write disable)
	spi_end
	ret
