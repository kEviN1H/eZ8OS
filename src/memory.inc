;===================================================================================================
; Z E R O   P A G E
;===================================================================================================
	keypress_data   	equ %020						;8 bytes

	keyboard_char   	equ (keypress_data + 8)			;1 byte
	tokenset_size		equ (keypress_data + 9)			;1
	output_device		equ (keypress_data + 10)		;1

	cr_x				equ (keypress_data + 11)		;1
	cr_y				equ (keypress_data + 12)		;1
	cr_scroll			equ	(keypress_data + 13)		;1

	vect_uart_rx		equ (keypress_data + 14)

	rx_buffer			equ (keypress_data + 16)		;2

	prgm_base			equ	(keypress_data + 18)		;2
	
	_block				equ	(keypress_data + 20)		;2
	_blocksize			equ	(keypress_data + 22)		;2
	_blocknext			equ	(keypress_data + 24)		;2
	_blockno			equ (keypress_data + 26)		;2
	
	argc				equ (keypress_data + 28)		;1
	argv		    	equ (keypress_data + 38)		;16 arguments *2
		
	input_string    	equ 0100h;(argv + 32?)			;42+ characters
	stack_base      	equ (input_string + 42 + 256)	;fingers crossed they never meet

	page_buffer			equ (stack_base+1)				;512
	spi_chars			equ (page_buffer+512)			;1
	spi_buffer			equ (spi_chars+1)				;x