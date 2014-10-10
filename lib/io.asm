
[SECTION .data]

disp_pos			dd	0

[SECTION .text]

; 导出函数
global	disp_str
global	disp_colorstr
global	out_byte
global	in_byte
global	cls

; ========================================================================
;                  void disp_str(char * pszInfo);
; ========================================================================
disp_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	cmp edi, 3998
	jge .above
	jmp .less
.above:
	call cls
	pop ebp
	ret
.less:
	jmp	.1
.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

disp_colorstr:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov ah, [ebp + 12]
	;mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	cmp edi, 3998
	jge .above
	jmp .less
.above:
	call cls
	pop ebp
	ret
.less:
	jmp	.1
.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

cls:
	push ecx
	push ax
	push edi
	mov ecx, 2000
	mov al, ' '
	xor edi, edi
.loop:
	mov [gs:edi], ax
	add edi, 2
	loop .loop

	mov dword [disp_pos], 0
	pop edi
	pop ax
	pop ecx
	ret
; ======================== 8259A
out_byte:
	mov edx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	nop
	nop
	ret

in_byte:
	mov edx, [esp + 4]
	xor eax, eax
	in al, dx
	nop 
	nop
	ret
