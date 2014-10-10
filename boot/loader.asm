org 0100h	;very important!!!!
;load kernel.bin to BaseOfLoader:OffsetOfLoader
; ==========================================================
	jmp L_START
;	nop 这里不需要了就
%include "fat12.inc"
%include "cpu.inc"
%include "load.inc"
; =========================================================
; GDT
L_GDT:		Descriptor	0,		0,			0
L_DST_EXE:	Descriptor	0,		0fffffh,	DA_CR|DA_32|DA_LIMIT_4K
L_DST_RW:	Descriptor	0,		0fffffh,	DA_DRW|DA_32|DA_LIMIT_4K
L_VIDEO:	Descriptor	0B8000h,0ffffh,		DA_DRW|DA_DPL3

GdtLen		equ	$-L_GDT
GdtPtr		dw GdtLen-1
			dd BaseOfPhyAddr + L_GDT

; Sel选择子由 TI RPL 属性加持, RPL 0~3 表示特权级 TI表示GDT orLDT
SelFlatExe	equ			L_DST_EXE	-	L_GDT
SelFlatRW	equ			L_DST_RW	-	L_GDT
SelVideo	equ			L_VIDEO		-	L_GDT	+ SA_RPL3
L_START:	
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov	ss, ax
	mov sp, BaseOfStack

	;获取内存信息
	mov ebx, 0
	mov di, _MemBuf
.MemChkLoop:
	mov eax, 0E820h
	mov ecx, 20
	mov edx, 0534D4150h
	int 15h
	jc .MemChkFail
	add di, 20
	inc dword [_dwMCRNumber]
	cmp ebx, 0
	jne .MemChkLoop
	jmp .MemChkOk
.MemChkFail:
	mov dword [_dwMCRNumber], 0
.MemChkOk:

	;call Cls
	mov dh, 2
	call Printf

	mov dh, 3
	call Printf
	xor	ah, ah	; `.
	xor	dl, dl	;  |  软驱复位
	int	13h	; /
L_SEARCH_BEGIN:
	cmp word [wRootSize], 0
	jz L_NOT_FOUND
	mov ax,	BaseOfKernel 
	mov es, ax
	mov bx,	OffsetOfKernel 
	mov ax, word [wCurRootNo]
	mov cl, 1
	call ReadSector			;1Sector From es:bx
	mov di,	OffsetOfKernel 
	mov si, LoaderName 		;ds:si
	cld
	mov dx, 10h
L_BLOCK_SEARCH:
	cmp dx, 0
	jz L_SEARCH_GOON
	mov cx, 0Bh
L_CMP_FILE:
	cmp cx, 0
	jz L_FILE_FOUND
	dec cx
	lodsb
	cmp al, byte [es:di]
	jnz L_BLOCK_GOON
	inc di
	jmp L_CMP_FILE

L_BLOCK_GOON:
	dec dx
	and di, 0FFE0h
	add di, 20h
	mov si, LoaderName
	jmp L_BLOCK_SEARCH
		
L_SEARCH_GOON:
	inc word [wCurRootNo]
	dec word [wRootSize]
	jmp L_SEARCH_BEGIN

L_FILE_FOUND:
	mov dh, 1
	call Printf
	and di, 0FFE0h
	add di, 01Ah
	mov cx, word [es:di]
	push cx		;Save Fat Current Sector
	add cx, SectorOffset
	mov ax,	BaseOfKernel 
	mov es, ax
	mov bx,	OffsetOfKernel 
	mov ax, cx
L_LOADING_FILE:
	;let's Digging Dot
	push ax
	push bx
	mov bl, 0ch
	mov ah, 0Eh
	mov al, '.'
	int 10h
	pop bx
	pop ax

	mov cl, 1
	call ReadSector
	pop ax		;取出Sector
	call GetNextEntry
	cmp ax, 0FFFh
	jz L_FILE_LOADED
	push ax
	add ax, SectorOffset
	add bx, [BPB_BytsPerSec]
	jmp L_LOADING_FILE

L_NOT_FOUND:
	mov dh, 0
	call Printf
	jmp $

L_FILE_LOADED:
	mov dh, 3
	call Printf

	;加载 GDTR
	lgdt [GdtPtr]

	;关中断
	cli

	;打开地址线A20
	in al, 92h
	or al, 010b
	out 92h, al

	;准备切换到保护模式
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp dword SelFlatExe:(BaseOfPhyAddr + L_PM_START)
	;这一句太冒失了,需要手动载入文件到地址中才可以解决
	;jmp BaseOfKernel:OffsetOfLoader

wRootSize	dw	14
wCurRootNo	dw	19	;从这里开始
LoaderName	db	"KERNEL  BIN",0
Line		db	4
Bodd		db	0
MsgLen	equ	9
MsgStart:	
MsgNotFound	db	"NO KERNEL"
MsgFound	db	"RedKERNEL"
MsgSBoot	db	"Booting.."
MsgLoad		db 	"Ready.   "

Printf:	;会使用到es
	push es
	push bp
	push bx
	push cx
	push dx
	mov	ax, MsgLen
	mul dh
	add ax, MsgStart
	mov bp, ax
	mov ax, ds
	mov es, ax		;es: bp 
	mov cx, MsgLen	;cx: len
	mov ax, 0x1301
	mov bx, 0x9
	mov dh, byte [Line]
	mov dl, 0		;dh:dl 行列
	int 10h
	inc byte [Line]
	pop dx
	pop cx
	pop bx
	pop bp
	pop es
	ret

Cls:
	mov ax, 0600h	;AH为向上滚动，AL=0窗口信息全部清空
	mov bx, 0700h	;bh为底色 前景色
	mov cx, 0000h	;窗口左上
	mov dx, 184fh	;窗口右下
	int 10h
	ret

; ReadSector to es:bx 
; ax cl 
ReadSector:
	push bp
	push dx
	;mov sp, bp
	mov bp, sp
	sub sp, 2

	mov byte [ebp-2], cl
	push bx
	mov bl, [BPB_SecPerTrk]
	div	bl
	inc ah
	mov cl, ah
	mov dh, al
	shr al, 1		; dh:dl 柱面号 磁盘号
	mov ch, al		; ch:cl 磁道 : 起始扇区
	and dh, 1
	pop bx
	mov dl, [BS_DrvNum]
Reading:
	mov ah, 2
	mov al, byte [bp-2]
	int 13h
	jc Reading

	add sp, 2
	pop dx
	pop bp
	ret

;
GetNextEntry:
	push es	
	push bx
	mov byte [Bodd], 0
	mov bx, 3
	mul bx
	mov bx, 2
	div bx
	cmp dx, 0
	jz	L_EVEN	
	mov byte [Bodd], 1
L_EVEN:
	mov bx, [BPB_BytsPerSec]
	xor dx, dx	;Fucking Badly div指令跟dx有关
	div bx		;dx=offset ax=sector
	mov bx, 0
	add ax, SectorOfFAT1
	mov cl, 2

	push ax
	mov ax, BaseOfLoader
	sub ax, 0100h
	mov es, ax
	pop ax

	call ReadSector 
	add bx, dx
	mov ax, [es:bx]
	cmp byte [Bodd], 1
	jnz L_GNE_EVEN
	shr ax, 4
	jmp L_GNE_RETRUN
L_GNE_EVEN:
	and ax, 0FFFh

L_GNE_RETRUN:
	pop bx
	pop es
	ret

[SECTION .s32]
ALIGN	32
[BITS 32]
L_PM_START:
	mov ax, SelVideo
	mov gs, ax
	mov ax, SelFlatRW
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov	esp, TopOfStack

	;显示内存信息
	call DispMemInfo
	call SetupPaging

	call DispReturn
	push szInitKernel
	call DispStr
	add esp, 4

	;跳入新的内核
	call InitKernel
	jmp SelFlatExe:KernelEntry
	jmp $

%include "lib.inc"

DispMemInfo:
	push esi
	push edi
	push ecx
	
	mov esi, MemBuf
	mov ecx, [dwMCRNumber]
.loop:
	mov edx, 5
	mov edi, ARDStruct
.1:
	push dword [esi]
	call DispInt
	pop eax
	stosd
	add esi, 4
	dec edx
	cmp edx, 0
	jnz .1
	call DispReturn
	cmp dword [dwType], 1
	jne .2
	mov eax, [dwBaseAddrLow]
	add eax, [dwLengthLow]
	cmp eax, [dwMemSize]
	jb .2
	mov [dwMemSize], eax
.2:
	loop .loop
	
	call DispReturn
	push szRAMSize
	call DispStr
	add esp,4

	push dword [dwMemSize]
	call DispInt
	add esp, 4

	pop ecx
	pop edi
	pop esi
	ret

; 启动分页机制
SetupPaging:
	xor edx, edx
	mov eax, [dwMemSize]
	mov ebx, 400000h
	div ebx				;eax=页目录条数 
	mov ecx, eax
	push ecx

	mov ax, SelFlatRW
	mov es, ax
	mov edi, PageDirBase
	mov eax, PageTblBase | PG_P | PG_USU | PG_RWW
.1:
	stosd
	add eax, 4096
	loop .1

	; 初始化页表
	pop eax
	mov ebx, 1024
	mul ebx
	mov ecx, eax
	mov edi, PageTblBase
	mov eax, PG_P | PG_USU | PG_RWW
.2:
	stosd
	add eax, 4096
	loop .2

	mov eax, PageDirBase
	mov cr3, eax
	mov eax, cr0
	or eax, 80000000h
	mov cr0, eax
	jmp .3
.3:
	nop
	ret

; InitKernel
InitKernel:
	mov cx, word [BaseOfPhyKernel + 0x2C]
	movzx ecx, cx
	mov esi, [BaseOfPhyKernel + 0x1C]
	add esi, BaseOfPhyKernel
.Copy
	mov eax, [esi + 0]
	cmp eax, 0
	jz .Out
	push dword [esi + 0x10]	
	mov eax, [esi + 0x04]
	add eax, BaseOfPhyKernel
	push eax
	push dword [esi + 0x08]
	call MemCpy
	add esp, 12
.Out
	add esi, 0x20
	dec ecx
	jnz .Copy
	ret
;; 变量
_dwMCRNumber	dd	0
_MemBuf: times	256	db	0
_dwMemSize		dd	0
_ARDStruct:
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:			dd	0
_szRAMSize:	db	"RAM size:", 0
_szInitKer: db 	"Init Kernel...",0
_dwDispPos:	dd	(80 * 6 + 0) * 2
_szReturn:	db	0Ah, 0
; 跳入Protect模式后,段地址发生变化,变为0,因此访问dwMCRNumber需要新的地址
dwMCRNumber		equ	BaseOfPhyAddr + _dwMCRNumber
MemBuf			equ	BaseOfPhyAddr + _MemBuf
dwMemSize		equ	BaseOfPhyAddr + _dwMemSize
ARDStruct		equ	BaseOfPhyAddr + _ARDStruct
	dwBaseAddrLow	equ BaseOfPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh	equ BaseOfPhyAddr + _dwBaseAddrHigh
	dwLengthLow		equ	BaseOfPhyAddr + _dwLengthLow
	dwLengthHigh	equ	BaseOfPhyAddr + _dwLengthHigh
	dwType			equ	BaseOfPhyAddr + _dwType

szRAMSize		equ	BaseOfPhyAddr + _szRAMSize
dwDispPos		equ BaseOfPhyAddr + _dwDispPos
szReturn		equ BaseOfPhyAddr + _szReturn
szInitKernel	equ BaseOfPhyAddr + _szInitKer
; Page Table 地址
PageDirBase		equ	100000h
PageTblBase		equ	101000h
;
StackSpace: times 1024	db	0
TopOfStack	equ		BaseOfPhyAddr + $

