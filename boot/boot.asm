%ifdef _DOS_DEBUG
	org 100h
%else
	org 7c00h
%endif


BaseOfStack		equ	0x100
BaseOfLoader	equ	0x9000
OffsetOfLoader	equ	0x100
; ==========================================================
	jmp L_START
	nop
%include "fat12.inc"

L_START:
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov	ss, ax
	mov sp, BaseOfStack

	call Cls
	mov dh, 2
	call Printf

	xor	ah, ah	; `.
	xor	dl, dl	;  |  软驱复位
	int	13h	; /
L_SEARCH_BEGIN:
	cmp word [wRootSize], 0
	jz L_NOT_FOUND
	mov ax, BaseOfLoader
	mov es, ax
	mov bx, OffsetOfLoader
	mov ax, word [wCurRootNo]
	mov cl, 1
	call ReadSector			;1Sector From es:bx
	mov di, OffsetOfLoader
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
	mov ax, BaseOfLoader
	mov es, ax
	mov bx, OffsetOfLoader
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
	jmp BaseOfLoader:OffsetOfLoader

wRootSize	dw	14
wCurRootNo	dw	19	;从这里开始
LoaderName	db	"LOADER  BIN",0
Line		db	0
Bodd		db	0
MsgLen	equ	9
MsgStart:	
MsgNotFound	db	"NOT FOUND"
MsgFound	db	"!!Loading"
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
	mov bx, 0xc
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


times	510-($-$$)	db	0
dw 0xaa55
