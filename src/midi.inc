; Calculate baud rate generator value
MIDI_BAUD   EQU 31250
MIDI_BRG    EQU ((SYSFREQ + (MIDI_BAUD * 8)) / (MIDI_BAUD * 16))
; Second UART control: MIDI port
M_BRH       EQU U1BRH
M_BRL       EQU U1BRL
M_CTL0      EQU U1CTL0
M_CTL1      EQU U1CTL1
M_STAT0     EQU U1STAT0
M_D         EQU U1D

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Intialize UART1(MIDI)
; Only MIDI-out for now
init_midi:
    ldx M_BRH, #HIGH(MIDI_BRG)
    ldx M_BRL, #LOW(MIDI_BRG)
    ldx M_CTL1, #%00     ; clear for normal non-Multiprocessor operation
    ldx M_CTL0, #%C0     ; Transmit enable, Receive DISABLE, No Parity, 1 Stop
    ret
	
;===================================================================================================
; M I D I   R O U T I N E S
;===================================================================================================

; Routine: midiw
; Output a byte to the MIDI port
; R0: holds the byte to send	
midiout:
mo:
	push R1
$$:
    ldx R1, M_STAT0
    and R1, #%04
    jr Z, $B
    ldx M_D, R0
    pop R1
    ret
	
	
	;===================================================================================================
; M I D I   P L A Y E R
;===================================================================================================
; Notes to self:
; - Timeout values can only be 2 bytes long as of right now.
;   if timeout values of more than 2 bytes occur in the wild: change the code!
; -

midi_bytesleft	equ %0E	;R14
midi_position	equ	%08 ;RR8
midi_PPQ		equ %0C	;RR12
midi_ticklength	equ %06	;RR6
midi_timeout	equ %0A	;RR10
	SCOPE
;Start of program ----------------------------------------------------------------------------------
prg_midi:
	cp argc,#2
	jp ult,err_argument
	
	ld R0,argv+2
	ld R1,argv+3
	call str2int
	ld midi_position,R0
	ld midi_position+1,R1
	
;Verify midi file header ---------------------------------------------------------------------------
	ld R2,#HIGH(midiId)
	ld R3,#LOW(midiId)
	jp $verifyId
$headerIdValid:
	jp $verifyHeaderSize
$headerSizeValid:
	jp $verifyMidiType
$midiTypeValid:
	jp $verifyTrackCount
$trackCountValid:
	ldc R12,@RR8								; Load PPQ (RR12=midi_PPQ)
	incw midi_position
	ldc R13,@RR8
	add midi_position+1,#8						;Skip the track id and track size
	adc midi_position,#0
	incw midi_position
	
	ld16_im midi_ticklength,8333/256			; Standard tick length for 120 BPM
	
	jr $main									; Jump to main loop
	
;Send MIDI messages --------------------------------------------------------------------------------
$loop:
	incw midi_position							; Jump to next byte
	ldc R0,@RR8									; Load byte from flash
$sendMsg:
	call mo 									; Send to midi out
	djnz R14,$loop	;(R14 == midi_bytesleft, but it will not assemble for whatever reason.)
	incw midi_position

;Main loop for decoding midi files -----------------------------------------------------------------
$main:
	ld R10,#0									; Load timeout value
	ldc R11,@RR8
	incw midi_position							; Jump to next byte
	btjz 7,R11,$F								; If bit7 is high load another byte
	ld R10,R11									; |
	and R10,#127								; |Mask out bit 7 (7Fh)
	ldc R11,@RR8								; |Load lower byte of timeout value
	rlc R11										; |
	srl R10										; |Shift R10 one bit to the right
	rrc R11										; | and store bit0 in RR11 bit 7
	incw midi_position							; |Jump to next byte
$$:												;/
	;and R11,#127								; Mask out bit 7 (7Fh)
	ldc R0,@RR8									; Load midi message
	cp R0,#FFh
	jr eq,$meta									; Check if it's a meta message
	ld R1,#F0h
	and R1,R0
	cp R1,#80h
	jr ult,$F
	cp R1,#D0h									; Determine the size of the message
	jr eq,$F
	cp R1,#C0h
	jr eq,$F
	ld midi_bytesleft,#3						; 3 bytes to be sent to the midi device
	call $timeout
	jr $sendMsg
$$:
	ld midi_bytesleft,#2						; 2 bytes to be sent to the midi device
	call $timeout
	jr $sendMsg
	ret
$errorHandler:
	call puth
	ld R0,#'-'
	call putc
	ld R0,R8
	ld R1,R9
	call puti
	ld R0,#13
	call putc
	call $timeout
	jr $main
$meta:
	incw midi_position							; Jump to next byte
	ldc R0,@RR8									; Load meta type
	incw midi_position							; Jump to next byte
	ldc R14,@RR8								; Load meta size (R14 is midi_bytesleft)
	cp R0,#51h
	jr eq,$tempo
	add midi_position+1,midi_bytesleft			; Skip meta message
	adc midi_position,#0
	incw midi_position
	cp R0,#2fh									; Check if this meta marks the end of the track
	jr ne,$main
	
	ld R0,#HIGH(str_midi_done)
	ld R1,#LOW(str_midi_done)
	call puts
	ret

$tempo:
	incw midi_position
	ldc R0,@RR8									; Load 'microseconds per beat' (MSB)
	incw midi_position
	ldc R1,@RR8
	ld R2,midi_PPQ
	ld R3,midi_PPQ+1
	call div16									; Divide MSB/PPQ												
	;=========================================================================
	; R E M A R K:
	; >>div16 returns the value to RR6 which is exactely where the ticklength is stored.
	; >>MSB is officially a 24 bit number, but here a 16 bit number is used.
	; >>To compensate for this the delay will simply be a factor 256 longer.
	;_________________________________________________________________________											
	add midi_position+1,#2						; Skip to the next midi message
	adc midi_position,#0
	jr $main
	
;Routine for verifying header and file ID's --------------------------------------------------------
$verifyId_:
	incw midi_position
	incw RR2
$verifyId:
	ldc R0,@RR8	
	ldc R1,@RR2
	cp R1,#0
	jr eq,$valid
	cp R0,R1
	jp ne,err_file_invalid
	jr $verifyId_
$valid:
	jp $headerIdValid
;Routine for verifying header size (should always be 6 for standard midi files) --------------------
$verifyHeaderSize
	ldc R0,@RR8
	incw midi_position
	ldc R1,@RR8
	incw midi_position
	ldc R2,@RR8
	incw midi_position
	ldc R3,@RR8
	incw midi_position
	cp R0,#00h
	jp ne,err_file_invalid
	cp R1,#00h
	jp ne,err_file_invalid
	cp R2,#00h
	jp ne,err_file_invalid
	cp R3,#06h
	jp ne,err_file_invalid
	jp $headerSizeValid
;Routine for verifying midi type (should be type0 in this case)-------------------------------------
$verifyMidiType
	ldc R0,@RR8
	incw midi_position
	ldc R1,@RR8
	incw midi_position
	cp R0,#00h
	jp ne,err_file_invalid
	cp R1,#00h
	jp ne,err_file_invalid
	jp $midiTypeValid
;Routine for verifying amount of tracks (should be 1 in case of a midi type0 file)------------------
$verifyTrackCount:
	ldc R0,@RR8
	incw midi_position
	ldc R1,@RR8
	incw midi_position
	cp R0,#00h
	jp ne,err_file_invalid
	cp R1,#01h
	jp ne,err_file_invalid
	jp $trackCountValid

$timeout:
	cp midi_timeout+1,#0
	cpc midi_timeout,#0
	jr eq,$F	
	ld R2,R7
	ld R4,R7
	ld R3,midi_timeout
	ld R5,midi_timeout+1
	mult RR2
	mult RR4
	add R3,R4
	ld R2,R3
	ld R3,R5
	call $delay256Microseconds
$$:
	ret

;Routine:Delay256microseconds
;Delay for RR6 * 256 microseconds
	SCOPE
$delay256Microseconds:
	push R8
$outer:
	ld R8,#0
$inner:
	;nop
	;nop
	;nop
	;nop
	;nop
	;nop
	;nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	djnz R8,$inner
	decw RR2
	jr nz,$outer
	pop R8
	ret

;Data area -----------------------------------------------------------------------------------------
str_midi_done:
	asciz "Playback complete!"
midiId:
	asciz "MThd"
trackId:
	asciz "MTrk"
	
	