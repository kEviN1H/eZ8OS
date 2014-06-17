prg_org: MACRO p
_label_start_001:
	org(8000h + p * 512)
ENDMAC prg_org

prg_end: MACRO
_label_end_001:
ENDMAC prg_end

cr_x	equ		%2B
cr_y	equ		%2C

table_io					equ		%0800
	putc					equ		table_io + 0
	puts					equ		table_io + 3
	puts_r					equ		table_io + 6
	putFd					equ		table_io + 9
	puti					equ		table_io + 21
	puth					equ		table_io + 24
	;---- reserved space ----
	spi_transfer			equ		table_io + 48
	spi_transfer_buffer 	equ		table_io + 51
	;---- reserved space ----
	fs_open_r	 			equ		table_io + 72
	fs_getc					equ		table_io + 75
	;---- reserved space ----	
table_memory				equ		table_io + 96
	F_unlock				equ		table_memory + 0
	F_erase_page			equ		table_memory + 3
	;---- reserved space ----
	
table_video					equ		table_memory + 48
	gd_cpy					equ		table_video	+ 0
	gd_copy					equ		table_video	+ 3
	gd_fillmeup				equ		table_video	+ 6
	gd_char_pal				equ		table_video	+ 9
	;---- reserved space ----

table_math					equ		table_video + 48
	div8					equ		table_math + 0
	div16					equ		table_math + 3
	str2int					equ		table_math + 12
	;---- reserved space ----

table_errors				equ		table_math + 48
	err_file_notFound		equ		table_errors + 0