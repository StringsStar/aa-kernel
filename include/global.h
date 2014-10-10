#ifndef _PROTECT_H
#define _PROTECT_H

typedef unsigned int	u32;
typedef unsigned short	u16;
typedef unsigned char 	u8;
typedef void (*int_handler) ();
//Trick define
#define PUBLIC
#define PRIVATE	static

#define EXTERN extern
#ifdef GLOBAL_VAR_HERE
#undef EXTERN
#define EXTERN
#endif

//权限
#define PRIVILEGE_KRNL	0
#define PRIVILEGE_TASK	1
#define PRIVILEGE_USER	3
/* 存储段描述符/系统段描述符 */
typedef struct s_descriptor		/* 共 8 个字节 */
{
	u16	limit_low;		/* Limit */
	u16	base_low;		/* Base */
	u8	base_mid;		/* Base */
	u8	attr1;			/* P(1) DPL(2) DT(1) TYPE(4) */
	u8	limit_high_attr2;	/* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
	u8	base_high;		/* Base */
}DESCRIPTOR;

typedef struct s_gate
{
	u16 offset_low;
	u16 selector;
	u8 dcount;
	u8 attr;
	u16 offset_high;
}GATE;

/* 选择子 */
#define SELECTOR_DUMMY		0
#define SELECTOR_FLAT_C		0x08
#define SELECTOR_FLAT_RW	0x10
#define SELECTOR_VIDEO		(0x18+3)	//RPL=3
#define SELECTOR_TSS		SELECTOR_VIDEO+8
#define SELECTOR_FIRST_LDT	SELECTOR_TSS+8
#define SelToIndex(Sel) (u32)((u32)Sel>>8)

#define SELECTOR_KERNEL_CS	SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS	SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS	SELECTOR_VIDEO
/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/

//选择子属性
#define SA_RPL_MASK		0xFFFC
#define SA_RPL0			0
#define SA_RPL1			1
#define SA_RPL2			2
#define SA_RPL3			3

#define SA_TI_MASK		0xFFFB
#define	SA_TI_G			0		//GDT
#define SA_TI_L			4		//LDT
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

//中断向量 0~0x19都被使用
#define INT_VECTOR_DIVIDE		0x0
#define INT_VECTOR_DEBUG 		0x1
#define INT_VECTOR_NMI			0x2	//不可屏蔽中断
#define INT_VECTOR_BREAKPOINT	0x3
#define INT_VECTOR_OVERFLOW		0x4
#define INT_VECTOR_BOUNDS		0x5
#define INT_VECTOR_INVAL_OP		0x6
#define INT_VECTOR_COPROC_NOT	0x7
#define INT_VECTOR_DOUBLE_FAULT	0x8
#define INT_VECTOR_COPROC_SEG	0x9
#define INT_VECTOR_INVAL_TSS	0xA
#define INT_VECTOR_SEG_NOT		0xB
#define INT_VECTOR_STACK_FAULT	0xC
#define INT_VECTOR_PROTECT		0xD
#define INT_VECTOR_PAGE_FAULT	0xE
#define INT_VECTOR_COPROC_ERR	0x10

//中断向量
#define	INT_VECTOR_IRQ0			0x20
#define	INT_VECTOR_IRQ8			0x28

//8259A
#define INT_M_CTL	0x20
#define INT_M_CTL2	0x21
#define INT_S_CTL	0xA0
#define INT_S_CTL2	0xA1
#define	EOI			0x20
//进程调度相关
#define GDT_SIZE 128
#define IDT_SIZE 256
#define LDT_SIZE 128
typedef struct s_tss {
	u32	backlink;
	u32	esp0;	/* stack pointer to use during interrupt */
	u32	ss0;	/*   "   segment  "  "    "        "     */
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
}TSS;

typedef struct s_stackframe {
	u32	gs;		/* \                                    */
	u32	fs;		/* |                                    */
	u32	es;		/* |                                    */
	u32	ds;		/* |                                    */
	u32	edi;		/* |                                    */
	u32	esi;		/* | pushed by save()                   */
	u32	ebp;		/* |                                    */
	u32	kernel_esp;	/* <- 'popad' will ignore it            */
	u32	ebx;		/* |                                    */
	u32	edx;		/* |                                    */
	u32	ecx;		/* |                                    */
	u32	eax;		/* /                                    */
	u32	retaddr;	/* return addr for kernel.asm::save()   */
	u32	eip;		/* \                                    */
	u32	cs;		/* |                                    */
	u32	eflags;		/* | pushed by CPU during interrupt     */
	u32	esp;		/* |                                    */
	u32	ss;		/* /                                    */
}STACK_FRAME;

typedef struct s_process {
  STACK_FRAME regs;
  u16 ldt_sel;
  DESCRIPTOR ldts[LDT_SIZE];
  u32 pid;
  char pName[16];
}PCB;

#endif

