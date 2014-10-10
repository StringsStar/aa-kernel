BaseOfStack			equ		0100h
BaseOfLoader		equ		09000h
OffsetOfLoader		equ		0100h

BaseOfPhyAddr		equ		BaseOfLoader*10h

; Kernel.Bin 加载位置
BaseOfKernel		equ		08000h
OffsetOfKernel		equ		0


BaseOfPhyKernel		equ		BaseOfKernel * 10h
KernelEntry			equ		030400h
