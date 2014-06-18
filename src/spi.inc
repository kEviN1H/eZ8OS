;===================================================================================================
; S P I   R O U T I N E S
;===================================================================================================


; Warning: Never allow both spi pins to be low at the same time.
gd_spi_start: macro 
	andx SPIMODE,#~1
endmac

gd_spi_end: macro
	orx SPIMODE,#1
endmac

spi_start: macro 
	andx PAOUT,#~(1<<2)
endmac

spi_end: macro
	orx PAOUT,#(1<<2)
endmac

wait_spi_rdy: macro
	push R0
$$:
	ldx R0,SPISTAT
	btjnz 1,R0,$B
	pop R0
endmac wait_spi_rdy

init_spi:
	ldx PCADDR,#DDR									; Set MOSI and SCK to output,
	ldx PCCTL,#1110_0111b							; MISO to input, SCK to output
	ldx PCADDR,#AF									; Set alternate SPI functions
	ldx PCCTL,#0011_1100b							; MOSI,MISO,SCK,/SS
    ;SPI:
	ldx SPICTL,#000__11__011b						; SPI Master,transmit falling edge,SCK idle HIGH
	ldx SPIMODE,#00_0_000_1_1b						; 8 bits per char, SS is output and set to 1
	ldx SPIBRH,#0
	ldx SPIBRL,#2
	ret

spi_transfer_buffer:
	ldx R2,spi_chars
	ld R4,#HIGH(spi_buffer)
	ld R5,#LOW(spi_buffer)
	gd_spi_start
$$:	
	ldx R1,@RR4
	call spi_transfer
	incw RR4
	djnz R2,$B
	gd_spi_end
	ldx spi_chars,#0
	ret
	
spi_transfer:
	ldx SPIDATA,R1
	wait_spi_rdy
	ldx R1,SPIDATA
	ret
	