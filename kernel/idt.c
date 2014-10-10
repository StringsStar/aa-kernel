#include "global.h"
#include "func.h"
#include "kernel.h"
//=============================================================
//LOCAL DEFINITIONS
//END OF LOCAL DEFINAITION
void Init_8259();
void Init_idt();
void init_idt_vector(u8 vector,u8 dist_type,\
	int_handler handler, u8 pri);
void exception_handler(int vec_no,int err_code,\
	int eip,int cs,int eflags);

void Init_8259()
{
	out_byte(INT_M_CTL, 0x11);	
	out_byte(INT_S_CTL, 0x11);
	out_byte(INT_M_CTL2,INT_VECTOR_IRQ0);
	out_byte(INT_S_CTL2,INT_VECTOR_IRQ8);
	
	//ICW3 设置主8259 IRQ2 对应从8259
	out_byte(INT_M_CTL2, 0x4);
	out_byte(INT_S_CTL2, 0x2);

	//ICW4
	out_byte(INT_M_CTL2, 0x1);
	out_byte(INT_S_CTL2 , 0x1);

	//OCW1
	//out_byte(INT_M_CTL2, 0xFF);
	//打开时钟中断
	out_byte(INT_M_CTL2, 0xFE);
	//out_byte(INT_M_CTL2, 0xFD);
	out_byte(INT_S_CTL2, 0xFF);
}

void init_idt()
{
	disp_str("Init 8259 start......\n");
	Init_8259();
	disp_str("Init 8259 end ......\n");
	disp_str("Init Idt table start.....\n");
	//just set 1 here
	init_idt_vector(INT_VECTOR_DIVIDE, DA_386IGate,\
		idt_divide_error, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_DEBUG, DA_386IGate,\
		idt_debug, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_NMI , DA_386IGate,\
		idt_nmi, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_BREAKPOINT , DA_386IGate,\
		idt_breakpoint , PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_OVERFLOW, DA_386IGate, \
		idt_overflow, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_BOUNDS, DA_386IGate, \
		idt_bounds, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_INVAL_OP, DA_386IGate, \
		idt_inval_opcode, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_COPROC_NOT, DA_386IGate, \
		idt_copr_invalid, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, \
		idt_double_fault, PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_COPROC_SEG, DA_386IGate, \
		idt_seg_overrun, PRIVILEGE_KRNL);
	
	init_idt_vector(INT_VECTOR_SEG_NOT,	DA_386IGate,
		idt_seg_present,	PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_STACK_FAULT,	DA_386IGate,
		idt_stack,		PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_PROTECT,	DA_386IGate,
		idt_gp,	PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_PAGE_FAULT,	DA_386IGate,
		idt_page_fault,		PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_COPROC_ERR,	DA_386IGate,
		idt_copr_err,		PRIVILEGE_KRNL);

	init_idt_vector(INT_VECTOR_IRQ0 + 0,      DA_386IGate,
		hwint0,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 1,      DA_386IGate,
		hwint1,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 2,      DA_386IGate,
		hwint2,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 3,      DA_386IGate,
		hwint3,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 4,      DA_386IGate,
		hwint4,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 5,      DA_386IGate,
		hwint5,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 6,      DA_386IGate,
		hwint6,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ0 + 7,      DA_386IGate,
		hwint7,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 0,      DA_386IGate,
		hwint8,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 1,      DA_386IGate,
		hwint9,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 2,      DA_386IGate,
		hwint10,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 3,      DA_386IGate,
		hwint11,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 4,      DA_386IGate,
		hwint12,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 5,      DA_386IGate,
		hwint13,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 6,      DA_386IGate,
		hwint14,                  PRIVILEGE_KRNL);
	init_idt_vector(INT_VECTOR_IRQ8 + 7,      DA_386IGate,
		hwint15,                  PRIVILEGE_KRNL);

	disp_str("Init Idt table End.....\n");
}

void init_idt_vector(u8 vector,u8 dist_type,\
	int_handler handler, u8 pri)
{
	GATE* p_Gate = &idt[vector];	
	u32 base = (u32)handler;
	p_Gate->offset_low	= base & 0xFFFF;
	p_Gate->selector 	= SELECTOR_FLAT_C;
	p_Gate->dcount		= 0;
	p_Gate->attr		= (dist_type) | (pri << 5);
	p_Gate->offset_high = (base>>16) & 0xFFFF;
}


void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	char * err_msg[] = {"#DE Divide Error",
			    "#DB RESERVED",
			    "--  NMI Interrupt",
			    "#BP Breakpoint",
			    "#OF Overflow",
			    "#BR BOUND Range Exceeded",
			    "#UD Invalid Opcode (Undefined Opcode)",
			    "#NM Device Not Available (No Math Coprocessor)",
			    "#DF Double Fault",
			    "    Coprocessor Segment Overrun (reserved)",
			    "#TS Invalid TSS",
			    "#NP Segment Not Present",
			    "#SS Stack-Segment Fault",
			    "#GP General Protection",
			    "#PF Page Fault",
			    "--  (Intel reserved. Do not use.)",
			    "#MF x87 FPU Floating-Point Error (Math Fault)",
			    "#AC Alignment Check",
			    "#MC Machine Check",
			    "#XF SIMD Floating-Point Exception"
	};

	disp_str(err_msg[vec_no]);
	disp_str("vec_no: ");
	disp_int(vec_no);
	disp_str(" err_code: ");
	disp_int(err_code);
	disp_str(" eip: ");
	disp_int(eip);
	disp_str(" cs: ");
	disp_int(cs);
	disp_str("\n");
}

void spurious_irq(int irq)
{
  disp_str("spurious_irp int:");
  disp_int(irq);
  disp_str("\n");
}
