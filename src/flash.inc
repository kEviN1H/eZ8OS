;===================================================================================================
; F L A S H   R O U T I N E S
;===================================================================================================
;; Flash size: 128 pages * 512 bytes

;; init Flash
init_flash
	ldx FFREQH,#HIGH(SYSFREQ/1000)			; Set flash frequency
	ldx FFREQL,#LOW(SYSFREQ/1000)
	ldx FCTL,#00h							; Lock flash
	ret
	
;; erase page
;; R0 = flash page
F_erase_page:
	call F_unlock
	ldx FCTL,#95h
	ret

;unlock page
; R0=flash page
F_unlock:
	cp R0,#0
	jp eq,err_critical
	ldx FCTL,#00h
	ldx FPS,R0
	ldx FCTL,#73h
	ldx FCTL,#8Ch
	ldx FPS,R0
	ret

	
	