;Error Messages
SCOPE
$argument:
asciz "\rErr: Argument"
err_argument:
	ld R0,#HIGH($argument)
	ld R1,#LOW($argument)
	call puts
	ret
	
$file_invalid:
asciz "\rErr: File Invalid"
err_file_invalid:
	ld R0,#HIGH($file_invalid)
	ld R1,#LOW($file_invalid)
	call puts
	ret
	
$file_notFound:
asciz "\rErr: File not found"
err_file_notFound:
	ld R0,#HIGH($file_notFound)
	ld R1,#LOW($file_notFound)
	call puts
	ret
	
$memory:
asciz "\rErr: Memory"
err_memory:
	ld R0,#HIGH($memory)
	ld R1,#LOW($memory)
	call puts
	ret
	
err_critical:
	jr init
	