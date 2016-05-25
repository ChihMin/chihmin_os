#include <inc/mmu.h>
#include <inc/types.h>
#include <inc/string.h>
#include <inc/x86.h>
#include <inc/memlayout.h>
#include <kernel/task.h>
#include <kernel/mem.h>

// Global descriptor table.
//
// Set up global descriptor table (GDT) with separate segments for
// kernel mode and user mode.  Segments serve many purposes on the x86.
// We don't use any of their memory-mapping capabilities, but we need
// them to switch privilege levels. 
//
// The kernel and user segments are identical except for the DPL.
// To load the SS register, the CPL must equal the DPL.  Thus,
// we must duplicate the segments for the user and the kernel.
//
// In particular, the last argument to the SEG macro used in the
// definition of gdt specifies the Descriptor Privilege Level (DPL)
// of that descriptor: 0 for kernel and 3 for user.
//
struct Segdesc gdt[6] =
{
	// 0x0 - unused (always faults -- for trapping NULL far pointers)
	SEG_NULL,

	// 0x8 - kernel code segment
	[GD_KT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 0),

	// 0x10 - kernel data segment
	[GD_KD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 0),

	// 0x18 - user code segment
	[GD_UT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 3),

	// 0x20 - user data segment
	[GD_UD >> 3] = SEG(STA_W , 0x0, 0xffffffff, 3),

	// First TSS descriptors (starting from GD_TSS0) are initialized
	// in task_init()
	[GD_TSS0 >> 3] = SEG_NULL
	
};

struct Pseudodesc gdt_pd = {
	sizeof(gdt) - 1, (unsigned long) gdt
};



static struct Taskstate tss;
Task tasks[NR_TASKS];

extern char bootstack[];

extern char UTEXT_start[], UTEXT_end[];
extern char UDATA_start[], UDATA_end[];
extern char UBSS_start[], UBSS_end[];
extern char URODATA_start[], URODATA_end[];
/* Initialized by task_init */
uint32_t UTEXT_SZ;
uint32_t UDATA_SZ;
uint32_t UBSS_SZ;
uint32_t URODATA_SZ;

Task *cur_task = NULL; //Current running task

extern void sched_yield(void);


//Find avaiable task slot and setup 
int task_create()
{
  struct PageInfo *p;

	/* Find a free task structure */
	int i = 0, j = 0;
	Task *ts = NULL;
	for (i = 0; i < NR_TASKS; i++)
	{
		if (tasks[i].state == TASK_FREE || tasks[i].state == TASK_STOP)
		{
			ts = &(tasks[i]);
			break;
		}
	}
	if (i >= NR_TASKS)
		return -1;

  /* Setup Page Directory */

  if (!(ts->pgdir = setupkvm()))
    panic("Not enough memory for per process page directory!\n");

	/* Setup Trapframe */
	memset( &(ts->tf), 0, sizeof(ts->tf));

	ts->tf.tf_cs = GD_UT | 0x03;
	ts->tf.tf_ds = GD_UD | 0x03;
	ts->tf.tf_es = GD_UD | 0x03;
	ts->tf.tf_ss = GD_UD | 0x03;
	ts->tf.tf_esp = USTACKTOP-PGSIZE;

  /* Setup User Stack */
  for (j = 0; j < USR_STACK_SIZE; j += PGSIZE)
  {
    p = page_alloc(ALLOC_ZERO);
    if (!p || page_insert(ts->pgdir, p, (void*)(USTACKTOP-USR_STACK_SIZE+j), PTE_W|PTE_U))
      panic("Not enough memory for user stack!\n");
  }
	
	/* Setup task structure (task_id and parent_id) */
	ts->task_id = i;
	ts->state = TASK_RUNNABLE;
	ts->parent_id = ((uint32_t)cur_task)?cur_task->task_id:0;
	ts->remind_ticks = TIME_QUANT;
	return i;
}

/* Free task of pid */
static void task_free(int pid)
{
  lcr3(PADDR(kern_pgdir));
  unsigned i;
  /* Free pages of user stack */
  for (i = 0; i < USR_STACK_SIZE; i += PGSIZE)
    page_remove(tasks[pid].pgdir, (void*)(USTACKTOP-USR_STACK_SIZE+i));
  /* Free page tables */
  ptable_remove(tasks[pid].pgdir);
  /* Free page directory */
  pgdir_remove(tasks[pid].pgdir);
}

void task_switch(int id)
{
}

void sys_kill(int pid)
{
	/* Lab4: Died task recycele, just set task state as TASK_STOP and yield */
  
	if (pid > 0 && pid < NR_TASKS)
	{
		tasks[pid].state = TASK_STOP;
    task_free(pid);
		sched_yield();
	}
}
void sys_kill_self()
{
	/*Lab4: Died task recycele */
	if ((uint32_t)cur_task)
	{
		cur_task->state = TASK_STOP;
    task_free(cur_task->task_id);
		sched_yield();
	}
}

int sys_fork()
{
	int pid = -1, i;
  uint32_t pa_src, pa_dst, *pte_src, *pte_dst;
	/* Setup task space */
	pid = task_create();
	if (pid < 0 )
		return -1;
	
	if ((uint32_t)cur_task)
	{
		/* Lab4: Copy parent tf to new task */
		tasks[pid].tf = cur_task->tf;
	
		/* Lab4: Copy old usr_stack to new task */
    for(i = 0; i < USR_STACK_SIZE; i += PGSIZE)
    {
      if((pte_src = pgdir_walk(cur_task->pgdir, (void *)(USTACKTOP-USR_STACK_SIZE+i), 0)) == 0)
        panic("copy user stack: pte_src should exist");
      if(!(*pte_src & PTE_P))
        panic("copy user stack: page not present");
      pa_src = PTE_ADDR(*pte_src);

      if((pte_dst = pgdir_walk(tasks[pid].pgdir, (void *)(USTACKTOP-USR_STACK_SIZE+i), 0)) == 0)
        panic("copy user stack: pte_dst should exist");
      if(!(*pte_dst & PTE_P))
        panic("copy user stack: page not present");
      pa_dst = PTE_ADDR(*pte_dst);

      memmove((char*)KADDR(pa_dst), (char*)KADDR(pa_src), PGSIZE);
    }

    /* All user program use the same code for now */
    setupuvm(tasks[pid].pgdir, (uint32_t)UTEXT_start, UTEXT_SZ);
    setupuvm(tasks[pid].pgdir, (uint32_t)UDATA_start, UDATA_SZ);
    setupuvm(tasks[pid].pgdir, (uint32_t)UBSS_start, UBSS_SZ);
    setupuvm(tasks[pid].pgdir, (uint32_t)URODATA_start, URODATA_SZ);

		/* Lab4: Set system call's return value */
		cur_task->tf.tf_regs.reg_eax = pid;
		tasks[pid].tf.tf_regs.reg_eax = 0;
	}

	return pid;
}
void task_init()
{
  extern int user_entry();
	int i;
  UTEXT_SZ = (uint32_t)(UTEXT_end - UTEXT_start);
  UDATA_SZ = (uint32_t)(UDATA_end - UDATA_start);
  UBSS_SZ = (uint32_t)(UBSS_end - UBSS_start);
  URODATA_SZ = (uint32_t)(URODATA_end - URODATA_start);
  /*printk("UText start:[%p], UText end:[%p]\n", UTEXT_start, UTEXT_end);*/
  /*printk("UData start:[%p], UData end:[%p]\n", UDATA_start, UDATA_end);*/
  /*printk("UBss start:[%p], UBss end:[%p]\n", UBSS_start, UBSS_end);*/
  /*printk("URODATA start:[%p], URODATA end:[%p]\n", URODATA_start, URODATA_end);*/

	/* Initial task sturcture */
	for (i = 0; i < NR_TASKS; i++)
	{
		memset(&(tasks[i]), 0, sizeof(Task));
		tasks[i].state = TASK_FREE;

	}
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	memset(&(tss), 0, sizeof(tss));
	tss.ts_esp0 = (uint32_t)bootstack + KSTKSIZE;
	tss.ts_ss0 = GD_KD;

	// fs and gs stay in user data segment
	tss.ts_fs = GD_UD | 0x03;
	tss.ts_gs = GD_UD | 0x03;

	/* Setup TSS in GDT */
	gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t)(&tss), sizeof(struct Taskstate), 0);
	gdt[GD_TSS0 >> 3].sd_s = 0;

	/* Setup first task */
	i = task_create();
	cur_task = &(tasks[i]);

  /* For user program */
  setupuvm(cur_task->pgdir, (uint32_t)UTEXT_start, UTEXT_SZ);
  setupuvm(cur_task->pgdir, (uint32_t)UDATA_start, UDATA_SZ);
  setupuvm(cur_task->pgdir, (uint32_t)UBSS_start, UBSS_SZ);
  setupuvm(cur_task->pgdir, (uint32_t)URODATA_start, URODATA_SZ);
  cur_task->tf.tf_eip = (uint32_t)user_entry;
	
	/* Load GDT&LDT */
	lgdt(&gdt_pd);


	lldt(0);

	// Load the TSS selector 
	ltr(GD_TSS0);

	cur_task->state = TASK_RUNNING;
	
}



