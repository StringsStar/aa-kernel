#ifndef	_KERNEL_H
#define _KERNEL_H

//variables From global.c
EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR g_gdts[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE];

//variables define global.c, use in process.c

#endif
