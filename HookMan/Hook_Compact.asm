
;External function used to Find out the name of th function given its address
;extrn FindSymbol:Proc
;extrn FindSymbol_1:Proc
;extrn CommitSymbol:Proc
extrn ProcEnter:Proc
extrn ProcExit:Proc
extrn GetOriginal:Proc
extrn PushVal:Proc
extrn PopVal:Proc

.data
;ep dq 0ffh

.code
;--------------------------------------------------------------------
; _dummy procedure
;--------------------------------------------------------------------

_dummy proc

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

	; Get the return address of the function
	mov  rdx,rax
	sub  rdx,12
	mov  qword ptr[rsp+18h],rdx

	; call the function to get the name of the callee and caller	
	call ProcEnter
	
	; backup to custom stack
	mov  rcx,qword ptr[rsp+38h+20h]	; address of home
	call PushVal
	
	mov  rcx,qword ptr[rsp+18h]
	
	; for get original function
	call GetOriginal
	
	add rsp,20h
	;;;
	
	; swap return point
	mov  rcx,_hexit
	xchg qword ptr[rsp+38h],rcx

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

_dummy endp

;--------------------------------------------------------------------
; _fake_penter procedure
;--------------------------------------------------------------------

_fake_penter proc
	
	mov  rax,_basecamp
	call rax

_fake_penter endp

;--------------------------------------------------------------------
; _basecamp procedure
;--------------------------------------------------------------------

_basecamp proc

	mov  rax,qword ptr[rsp]
	push rax
	mov  rax,_dummy
	mov  qword ptr[rsp+8],rax
	pop  rax
	ret

_basecamp endp

;--------------------------------------------------------------------
; _hexit procedure
;--------------------------------------------------------------------

_hexit proc
	
	; reserve return pointer and local
	sub  rsp,8h 

	; Store the volatile registers
	push r11
	push r10
	push r9
	push r8
	push rdx
	push rcx
	push rax
	
	; reserve space for 4 registers [ rcx,rdx,r8 and r9 ] 32 bytes
	sub  rsp,20h 
	
	; pop return pointer
	call PopVal
	xchg qword ptr[rsp+38h+20h],rax	; return pointer
	
	; inject ProcExit's params 
	mov  rcx,qword ptr[rsp+20h]

	; call the function to get the name of the callee and caller	
	call ProcExit
	
	;Release the space reserved for the registersk by adding 32 bytes
	add  rsp,20h 
	
	;Restore the registers back by poping out
	pop rax
	pop rcx
	pop rdx
	pop r8
	pop r9
	pop r10
	pop r11
	
	;return
	ret

_hexit endp


end