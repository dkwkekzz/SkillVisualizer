
;External function used to Find out the name of th function given its address
extrn GetAlarmRegist:Proc

.data

.code
;--------------------------------------------------------------------
; _dummy_alarmRegist procedure
;--------------------------------------------------------------------

_dummy_alarmRegist proc

	; Store the volatile registers
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdx
	push rcx
	
	;;;
	sub rsp,20h

	; call the function to get the name of the callee and caller	
	call GetAlarmRegist
	
	add rsp,20h
	;;;
	
	;Restore the registers back by poping out
	pop rcx
	pop rdx
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	
	;Call Original!
	jmp rax

_dummy_alarmRegist endp


end