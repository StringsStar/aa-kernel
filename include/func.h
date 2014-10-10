#ifndef _FUNC_H
#define _FUNC_H

//From string.asm
extern void* memcpy(void* pDst, void* pSrc, int nSize);
extern void memset(void* pDst, char ch, int size);
//From io.c
extern void disp_int(int input);

//From io.asm
extern void	disp_str(char* pszInfo);
extern void disp_colorstr(char* pszInfo,u8 color);
void out_byte(u16 port, u8 value);
u8 in_byte(u16 port);

//From kernel.asm
extern void idt_divide_error();
extern void idt_debug();
extern void idt_nmi();
extern void idt_breakpoint();
extern void idt_overflow();
extern void idt_bounds();
extern void idt_inval_opcode();
extern void idt_copr_invalid();
extern void idt_double_fault();
extern void idt_seg_overrun();
extern void idt_inval_tss();
extern void idt_seg_present();
extern void idt_stack();
extern void idt_gp();
extern void idt_page_fault();
extern void idt_copr_err();

extern void hwint0();
extern void hwint1();
extern void hwint2();
extern void hwint3();
extern void hwint4();
extern void hwint5();
extern void hwint6();
extern void hwint7();
extern void hwint8();
extern void hwint9();
extern void hwint10();
extern void hwint11();
extern void hwint12();
extern void hwint13();
extern void hwint14();
extern void hwint15();

//From idt.c
extern void Init_8259();
extern void init_idt();
extern void spurious_irq(int irq);

//From process.c
extern void Delay(int time);
#endif
