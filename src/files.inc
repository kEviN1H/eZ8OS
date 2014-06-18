;===================================================================================================
; F I L E  S Y S T E M
;===================================================================================================
;files.inc
;Filesystem operations (prefix:fs_)

;===================================================================================================
; I N D E X I N G   A N D   S E A R C H I N G
;===================================================================================================
;routine: fs_next
;Searches for the next file
;uses RR0
;RR4 = file identifier to be found. For example: 0x0000 a scratched file.
;RR2 = current address
;on fail: C = 1
;on success: C = 0
	SCOPE
$retry:
	add R2,#02								; Skip to the next block
	ld R3,#0
	btjz 7,R2,$last_file					; Hardcoded file memory boundary
fs_next:
	ldc R0,@RR2								; Load file identifier
	incw RR2
	ldc R1,@RR2
	cp R1,R5								; Check if file identifier matches with RR4
	cpc R0,R4
	jr ne,$retry
	incw RR2
	rcf
	ret
$last_file:
	scf
	ret
	
;routine: fs_find
;RR0 =  pointer to filename (string)
;C = 1 when file is not found
;C = 0 when file is found

	SCOPE
fs_find:
	ld R2,#HIGH(8000h)						; Base address of filesystem
	ld R3,#LOW(8000h)
$$:
	ld R4,#HIGH(0002h)						; Looking for a file-identifier 0002h (start of file)
	ld R5,#LOW(0002h)
	push R0
	push R1
	call fs_next							; Search for next file
	pop R1
	pop R0
	jr c,$notFound							; If no files are left in the filesystem return to OS
	incw RR2
	push R0
	push R1
	call $fname_comp
	pop R1
	pop R0
	jr eq,$found
	add R2,#02								; Skip to the next block
	ld R3,#0
	jr $B
$found:
	ld R3,#19								; Skip to Flags field
	rcf
	ret
$notFound:
	scf
	ret
;Returns Z = 1 on match (equal!)	
$fname_comp:
	ldc R4,@RR2				; Load character from flash
	cp R4,#TERMINATE_CHAR	; Check if the token in flash terminates
 	ldx R5,@RR0				; Load character from register file
	jr z,$c_comp				; 	if zero flag is set >> jump to c_comp
	incw RR0				; Advance to the next character
	incw RR2
	call $c_comp				; Call character compare routine
	jr z,$fname_comp		; If characters matched start over for next character
$c_comp:	
	cp R4,R5				; Compare the characters
	ret	
	
;routine: fs_open_r
;RR0 = pointer to filename
;returns RR2 = -1 when the file was not found
;returns RR2 = the address where the file was found
	SCOPE
fs_open_r:
	push R0
	push R1
	call fs_find							; Search for file with matching name
	pop R1
	pop R0
	jr nc,$F								; Return 0 when file was not found
	ld R2,#HIGH(0)
	ld R3,#LOW(0)
	ret
$$:
	ld _block+1,R2							; Save current block number
	add R3,#1								; Skip flags
	ldc R4,@RR2								; Load address to block after this one in advance
	ld _blocknext,R4
	incw RR2
	ldc R4,@RR2
	ld _blocknext+1,R4
	incw RR2
	ldc R4,@RR2								; Load current block size
	ld _blocksize,R4
	incw RR2
	ldc R4,@RR2
	ld _blocksize+1,R4
	incw RR2
	ret
	
;routine: fs_open_w
;RR0 = pointer to filename
;returns RR = -1 when there are no free blocks available
;returns RR2 = the address of the new file
;Caution! Will overwrite the old file without warning
	SCOPE
fs_open_w:
	push R0
	push R1
	call fs_find
	jr c,$createNew							; If file was found: overwrite file
$overwriteOld:
	push R0
	ld R0,R2
	srl R0
	call F_unlock							; Unlock flash
	ld R3,#0								; Jump back to the file identifier
	ld R4,#0
	ldc @RR2,R4								; Scratch the old file
	incw RR2
	ldc @RR2,R4
	ldx FCTL,#00h							; Lock flash
	pop R0
$createNew:									; Create new file
	ld R6,#0
$searchFreeBlock:
	ld R2,#HIGH(8000h)						; Jump to the start of the filesystem
	ld R3,#LOW(8000h)
	ld R4,#HIGH(FFFFh)						; Searching for empty block (identifier FFFFh)
	ld R5,#LOW(FFFFh)
	call fs_next
	jr nc,$writeHeader						; If an empty block was found: go write the identifier.
	djnz R6,$noFreeBlocks
	;call fs_garbageCollect					; Otherwise: collect garbage to free up space
	brk
	ld R6,#1
	jr $searchFreeBlock
$writeHeader
	ld R0,R2
	srl R0
	call F_unlock							; Unlock flash
	ld R3,#0								; Jump to start of the block
	ld R4,#HIGH(0002h)
	ldc @RR2,R4								; Write file start identifier (0002h)
	incw RR2
	ld R4,#LOW(0002h)
	ldc @RR2,R4
	incw RR2
	ld R4,#21
	ldc @RR2,R4								; Write header size (always 24)
	incw RR2
	pop R1									; Pop pointer to filename from the stack
	pop R0
	ld R5,#15
$writeFname:
	ldx R4,@RR0								; Read character from ram
	ldc @RR2,R4								; Write character to flash
	incw RR0
	incw RR2
	djnz R5,$writeFname
	ldx R4,#0
	ldc @RR2,R4								; Write termination character
	add R3,#6								; Skip flags, address of next block and size of block
	;adc R2,#0	;might not be needed...
	ld _blockno,#HIGH(0)					; Block no.0
	ld _blockno+1,#LOW(0)
	ld _block+1,R2
	ret

$noFreeBlocks:
	pop R2									; Pop unused bytes from stack
	pop R2
	ld R2,#HIGH(0)							; Return error
	ld R3,#LOW(0)
	ret
	
;routine: fs_close_w
fs_close_w:
	ld _blocknext,#FFh	;last block
	ld _blocknext+1,#FEh
	call fs_clsblock
	
	ld R2,#0								; Set file pointer to 0
	ld R3,#0
	ret
	
	
;routine: fs_getc
;RR2 = pointer in file
;R0 = byte read
fs_getc:
	and R2,#1								; Mask address so it's within the bounds of this block
	or R2,_block+1
	ldc R0,@RR2
	incw RR2								; Jump to next address in file
	push R2
	sub R2,_block+1
	cp R3,_blocksize+1						; Jump to next block if address exceeds blocksize
	cpc R2,_blocksize
	pop R2
	jr eq,$nextBlock
	ret

$nextBlock:
	cp _blocknext+1,#00h					; If the next block's address low byte != 0 it means
	jr ne,$eof								; this is the last block!
	ld R2,_blocknext
	ld R3,_blocknext+1	
	ld _block+1,_blocknext					; Jump to next block
	ldc R1,@RR2
	ld _blocknext,R1						; Load address to block after this one in advance
	incw RR2
	ldc R1,@RR2
	ld _blocknext+1,R1
	incw RR2
	ldc R1,@RR2
	ld _blocksize,R1						; Load current block size
	incw RR2
	ldc R1,@RR2
	ld _blocksize+1,R1
	incw RR2
	ret
	
$eof:
	ld R2,#0
	ld R3,#0
	ret
	
;routine: fs_putc
;RR2 = pointer in file
;R0 = byte to write
;Uses R0-R6
	SCOPE
fs_putc:
	and R2,#1								; Mask address so it's within the bounds of this block
	or R2,_block+1
	;-- maybe add overwrite protection here by checking for values other than FF
	;ldc R0,@RR2
	ldc @RR2,R0
	incw RR2								; Jump to next address in file
	push R2
	sub R2,_block+1
	cp R3,#LOW(0200h)						; Jump to next block if address exceeds blocksize
	cpc R2,#HIGH(0200h)
	pop R2
	jr eq,$nextBlock
	ret
$nextBlock:
	push R2
	push R3
	ld R6,#0
$searchFreeBlock:
	ld R2,#HIGH(8000h)						; Jump to the start of the filesystem
	ld R3,#LOW(8000h)
	ld R4,#HIGH(FFFFh)						; Searching for empty block (identifier FFFFh)
	ld R5,#LOW(FFFFh)
	call fs_next
	jr nc,$freeBlockFound					; If an empty block was found: escape from loop
	djnz R6,$noFreeBlocks
	;call fs_garbageCollect					; Otherwise: collect garbage to free up space
	brk
	ld R6,#1
	jr $searchFreeBlock
$freeBlockFound:
	ld _blocknext,R2						; Store next block's address for later use
	ld _blocknext+1,#0
	pop R3									; Restore the file pointer so it points to the end of the current
	pop R2									; block.
	call fs_clsblock
	ld R2,_blocknext	
	ld R3,#4								; Skip blocksize field
	incw _blockno							; Increment number of blocks
	ld _block+1,R2
	ld R0,_block+1
	srl R0
	call F_unlock							; Unlock flash
	ret

$noFreeBlocks:
	ld _block+1,#0
	ld R2,#HIGH(-1)							; Return error: -1 (Memory)
	ld R3,#LOW(-1)
	ret

;Private routines:
;routine:fs_block_close
;RR2 =  pointer in file (should be in block to close)
	SCOPE
fs_clsblock:
	ld R0,R2								; Save current position in block for later use
	ld R1,R3
	ld R2,_block+1							; Jump to start of current block
	and R2,#~1
	ld R3,#0
	cp _blockno+1,#LOW(0)
	cpc _blockno,#HIGH(0)
	jr ne,$F								; If this is the first block the Next_block and Size
	ld R3,#20								; fields are located differently.
$$:
	ld R4,_blocknext
	ldc @RR2,R4
	incw RR2
	ld R4,_blocknext+1
	ldc @RR2,R4
	incw RR2
	
	sub R0,_block+1							; Calculate blocksize
	ldc @RR2,R0								; Write blocksize to block header
	incw RR2
	ldc @RR2,R1
	
	ldx FCTL,#00h							; Lock flash
	ret
	
	

	