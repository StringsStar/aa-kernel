#include "global.h"
#include "kernel.h"
#include "func.h"

//Functions Define here
void Delay(int time);
void InitProcess();
void TestA();
void TestB();
void init_descriptor(DESCRIPTOR* pDst,u32 base,u32 limit,u16 attr);
u32 SelToRealAddr(u16 Sel, void* offset);


typedef struct s_task {
  u32 func_addr;
  u32 esp;
}TASK;

#define NM_TASKS	2
#define STACK_SIZE_A 1024
#define STACK_SIZE_B 1024
#define STACK_SIZE_ALL STACK_SIZE_A+STACK_SIZE_B

TASK g_tasks[NM_TASKS];
PCB	g_pcbs[NM_TASKS];
char g_userStacks[STACK_SIZE_ALL];
TSS g_tss;
PCB* P_PROCESS_READY;
int m_CurProc=0;
void InitTasks()
{
  g_tasks[0].func_addr = (u32)TestA;
  g_tasks[0].esp = (u32)&g_userStacks[STACK_SIZE_ALL];

  g_tasks[1].func_addr = (u32)TestB;
  g_tasks[1].esp = (u32)&g_userStacks[STACK_SIZE_ALL-STACK_SIZE_A];
}

void clock_handler()
{
  	P_PROCESS_READY++;
	if((u32)P_PROCESS_READY >= (u32)(g_pcbs + NM_TASKS))
	  P_PROCESS_READY = g_pcbs;
}
//初始化进程相关结构
void InitProcess()
{
  	InitTasks();
  	//选定第一个进程为初始化对象
 	memset(&g_tss, 0 , sizeof(TSS));
	g_tss.ss0 = SELECTOR_KERNEL_DS;
	init_descriptor(&g_gdts[4],
		SelToRealAddr(SELECTOR_KERNEL_DS, &g_tss),
		sizeof(TSS)-1,
		DA_386TSS);
	g_tss.iobase = sizeof(g_tss);

	P_PROCESS_READY = (void*)&g_pcbs[0];

	int i=0;
	for(i=0;i<NM_TASKS;i++)
	{
	  PCB* ppcb = &g_pcbs[i];
	  //加载LDT
	  init_descriptor(&g_gdts[5+i],
		  SelToRealAddr(SELECTOR_KERNEL_DS, ppcb->ldts),
		  sizeof(DESCRIPTOR)*LDT_SIZE-1,
		  DA_LDT
		  );

	  //处理LDT表
	  ppcb->ldt_sel = SELECTOR_FIRST_LDT;
	  memcpy(&ppcb->ldts[0], &g_gdts[SELECTOR_KERNEL_CS>>3],\
		  sizeof(DESCRIPTOR));
	  ppcb->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;	
	  memcpy(&ppcb->ldts[1], &g_gdts[SELECTOR_KERNEL_DS>>3],\
		  sizeof(DESCRIPTOR));
	  ppcb->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

	  //初始化寄存器
	  ppcb->regs.cs =(0 & SA_RPL_MASK & SA_TI_MASK)| SA_TI_L|SA_RPL1;
	  ppcb->regs.ds =(8 & SA_RPL_MASK & SA_TI_MASK)| SA_TI_L|SA_RPL1;
	  ppcb->regs.es = ppcb->regs.ds;
	  ppcb->regs.fs = ppcb->regs.es;
	  ppcb->regs.ss = ppcb->regs.ds;
	  ppcb->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK ) | SA_RPL1;
	  ppcb->regs.eflags = 0x1202;	//IF=1 IOPL=1
	  //调用restart 开启进程
	  //初始化进程
	  ppcb->regs.eip = g_tasks[i].func_addr;
	  ppcb->regs.esp = g_tasks[i].esp;
	 }
  }

  void TestA()
  {
	while(1) {
	  disp_colorstr("A",0xC);
	  Delay(1);
	}	
  }

  void TestB()
  {
	while(1) {
	  disp_colorstr("B",0x9);
	  Delay(2);
	}	
  }
  void Delay(int time)
  {
	int i,j,k;
	for(k=0;k<time;k++)
		for(i=0;i<100;i++)	
		  for(j=0;j<100;j++)
			;
  }

  void init_descriptor(DESCRIPTOR* pDst,u32 base,u32 limit,u16 attr)
  {
	pDst->limit_low = limit & 0xFFFF;
	pDst->base_low = base & 0xFFFF;
	pDst->base_mid = (base>>16) & 0xFFFF;
	pDst->attr1 = attr & 0xFF;
	pDst->limit_high_attr2 = ((limit>>16) & 0xF)|((attr>>8) & 0xF0);
	pDst->base_high = (base>>24) & 0xFF;
  }

  u32 SelToRealAddr(u16 Sel, void* offset)
  {
	DESCRIPTOR* p_Cur = &g_gdts[Sel>>3];
	u32 base = p_Cur->base_high<<24 | p_Cur->base_mid<<16|\
			   p_Cur->base_low;
	return (base + (u32)offset);
  }
