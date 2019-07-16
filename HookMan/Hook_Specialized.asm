
;External function used to Find out the name of th function given its address
extrn GetAlarmRegist1:Proc
extrn GetAlarmRegist2:Proc

.data

.code
;--------------------------------------------------------------------
; _dummy_alarmRegist1 procedure
;--------------------------------------------------------------------

_dummy_alarmRegist1 proc

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
	call GetAlarmRegist1
	
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

_dummy_alarmRegist1 endp

;--------------------------------------------------------------------
; _dummy_alarmRegist2 procedure
;--------------------------------------------------------------------

_dummy_alarmRegist2 proc

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
	call GetAlarmRegist2
	
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

_dummy_alarmRegist2 endp

end