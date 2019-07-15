
;External function used to Find out the name of th function given its address
extrn FindSymbol:Proc
extrn FindSymbol_1:Proc
extrn GetOriginal:Proc
extrn PushVal:Proc
extrn PopVal:Proc

.data
;ep dq 0ffh

.code
;--------------------------------------------------------------------
; _fake_penter procedure
;-------------

_fake_penter proc
	
	mov  rax,_basecamp
	call rax

_fake_penter endp

;--------------------------------------------------------------------
; _basecamp procedure
;-------------

_basecamp proc

	; create stack frame
	push rbp
	lea	 rbp,qword ptr[rsp]
	sub  rsp,20h

	; store
	push rcx

	; save entry point
	mov  rcx,qword ptr[rbp+8h]
	sub  rcx,5h
	sub  rcx,7h
	mov  qword ptr[rbp-8],rcx	; entry pointer
	
	pop  rcx
	
	push rax
	mov  rax,_henter
	xchg qword ptr[rsp],rax
	ret

_basecamp endp

;--------------------------------------------------------------------
; _henter procedure
;--------------------------------------------------------------------

_henter proc

	; Store the volatile registers
	push r11
	push r10
	push r9
	push r8
	push rax
	push rdx
	push rcx
	
	mov  r11,qword ptr[rsp+68h]
	mov  qword ptr[rbp-10h],r11	; return pointer
	
	; set my return addr
	mov  r11,_hexit
	mov  qword ptr[rsp+68h],r11

	; reserve space for 4 registers [ rcx,rdx,r8 and r9 ] 32 bytes
	sub  rsp,20h 
	
	; Get the return address of the function
	mov  rcx,qword ptr[rbp-8]
	
	; call the function to get the name of the callee and caller	
	call FindSymbol

	mov  rcx,qword ptr[rbp-8]
	
	; for get original function
	call GetOriginal

	;Release the space reserved for the registersk by adding 32 bytes
	add  rsp,20h 
	
	; save function ptr for call original
	mov  qword ptr[rbp-18h],rax	; original pointer
	
	; backup to custom stack
	mov  rcx,qword ptr[rbp-8]	; entry pointer
	call PushVal
	mov  rcx,qword ptr[rbp-10h]	; return pointer
	call PushVal
	
	;Restore the registers back by poping out
	pop rcx
	pop rdx
	pop rax
	pop r8
	pop r9
	pop r10
	pop r11
	
	add  rsp,20h
	pop  rbp
	add  rsp,8h
	
	; jmp to original function, not return
	jmp  qword ptr[rsp-28h]

_henter endp

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
	
	; reset my return pointer
	call PopVal
	mov  r9,rax	; return pointer
	mov  qword ptr[rsp+38h],r9
	
	; entry pointer param
	call PopVal
	mov  r10,rax	; entry pointer

	; reserve space for 4 registers [ rcx,rdx,r8 and r9 ] 32 bytes
	sub  rsp,20h 
	
	; inject FindSymbol_1's params 
	mov  rdx,qword ptr[rsp+20h]
	mov  rcx,r10

	; call the function to get the name of the callee and caller	
	call FindSymbol_1
	
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