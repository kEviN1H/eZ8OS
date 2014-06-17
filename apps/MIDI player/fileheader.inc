; Fileheader.inc
; Author: Koen van Vliet
; Date: 22-3-2014
;
; Add to executable files for the eZ8 operating system written by Koen van Vliet
; -- Usage -------
; 	prg_org _PAGE# (0-63)
; 	include "fileheader.inc"
; 	fileheader "FILENAME"
; 	-- Your program --
; 	_label_end_001:

fileheader MACRO _FileName
	dw 0002h								; File identifier 0002h marks the beginning of a file
	db 21									; Header size excluding first 3 bytes.
	db _FileName							; Filename (15 characters+00h)
	db 00h									; Terminate string with 00h
	db FFh									; Reserved for flags (one byte)
	dw FFFEh								; Address of next block. FFFEh means: last block in file
	dw _label_end_001 - _label_start_001	; Calculate program size
ENDMAC fileheader
	
	