#include <inc/mmu.h>
#include <inc/types.h>
#include <inc/string.h>
#include <inc/x86.h>
#include <inc/memlayout.h>
#include <kernel/task.h>
#include <kernel/mem.h>
#include <inc/error.h>
#include <kernel/cpu.h>

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
struct Segdesc gdt[NCPU + 5] =
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



static struct tss_struct tss;
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


/* TODO: Lab5
 * 1. Find a free task structure for the new task,
 *    the global task list is in the array "tasks".
 *    You should find task that is in the state "TASK_FREE"
 *    If cannot find one, return -1.
 *
 * 2. Setup the page directory for the new task
 *
 * 3. Setup the user stack for the new task, you can use
 *    page_alloc() and page_insert(), noted that the va
 *    of user stack is started at USTACKTOP and grows down
 *    to USR_STACK_SIZE, remember that the permission of
 *    those pages should include PTE_U
 *
 * 4. Setup the Trapframe for the new task
 *    We've done this for you, please make sure you
 *    understand the code.
 *
 * 5. Setup the task related data structure
 *    You should fill in task_id, state, parent_id,
 *    and its schedule time quantum (remind_ticks).
 *
 * 6. Return the pid of the newly created task.
 
 */

int task_create()
{
    struct PageInfo *pages[PAGES_PER_TASK];
	Task *ts = NULL;
    int i; 
    int task_id;
	/* Find a free task structure */
    bool find_free_task = false;
    int id_start = cur_task ? cur_task->task_id : -1;
    for (i = (id_start + 1) % NR_TASKS; i != id_start; i = (i + 1) % NR_TASKS) {
        ts = &tasks[i];
        task_id = i;
        if (ts->state == TASK_FREE || ts->state == TASK_STOP) {
            find_free_task = true;
            break;
        } 
    }
    
    if (!find_free_task)
        return -E_NO_MEM;

    /* Setup Page Directory and pages for kernel*/
    if (!(ts->pgdir = setupkvm()))
        panic("Not enough memory for per process page directory!\n");

    
    /* Setup User Stack */
    /* Here should handle if there's no enough memory to alloc, */
    /* and prevent from memory leakage */
    struct PageInfo *new_page_array[USR_STACK_SIZE/PGSIZE];
    int current_page = 0;
    int j;
    for (i = USTACKTOP - USR_STACK_SIZE; i < USTACKTOP; i = i + PGSIZE) {
        struct PageInfo *pp = page_alloc(1);
        if (page_insert(ts->pgdir, pp, i, PTE_U | PTE_W)) {
            printk("No memory for setup user stack!\n");
            for (j = 0; j < current_page; ++j)
                page_free(new_page_array[j]);
            pgdir_remove(ts->pgdir);
            return -E_NO_MEM;
        }
        else
            new_page_array[current_page++] = pp;
    }

	/* Setup Trapframe */
	memset( &(ts->tf), 0, sizeof(ts->tf));

	ts->tf.tf_cs = GD_UT | 0x03;
    ts->tf.tf_ds = GD_UD | 0x03;
    ts->tf.tf_es = GD_UD | 0x03;
	ts->tf.tf_ss = GD_UD | 0x03;
	ts->tf.tf_esp = USTACKTOP-PGSIZE;

	/* Setup task structure (task_id and parent_id) */
    ts->task_id = task_id;
    ts->remind_ticks = 100;
    
    if (cur_task != NULL) 
        ts->parent_id = cur_task->task_id;
    else
        ts->parent_id = -1;
    ts->state = TASK_RUNNABLE;
     
    return task_id;
}


/* TODO: Lab5
 * This function free the memory allocated by kernel.
 *
 * 1. Be sure to change the page directory to kernel's page
 *    directory to avoid page fault when removing the page
 *    table entry.
 *    You can change the current directory with lcr3 provided
 *    in inc/x86.h
 *
 * 2. You have to remove pages of USER STACK
 *
 * 3. You have to remove pages of page table
 *
 * 4. You have to remove pages of page directory
 *
 * HINT: You can refer to page_remove, ptable_remove, and pgdir_remove
 */
static void task_free(int pid)
{
    Task *task = &tasks[pid];
    int i;
    
    lcr3(PADDR(kern_pgdir));
    for (i = USTACKTOP - USR_STACK_SIZE; i < USTACKTOP; i += PGSIZE) 
        page_remove(task->pgdir, i);
    ptable_remove(task->pgdir);
    pgdir_remove(task->pgdir); 
}

// Lab6 TODO
//
// Modify it so that the task will be removed form cpu runqueue
// ( we not implement signal yet so do not try to kill process
// running on other cpu )
//
void sys_kill(int pid)
{
    /* TODO: Lab 5
     * Remember to change the state of tasks
     * Free the memory
     * and invoke the scheduler for yield
     */
	if (pid > 0 && pid < NR_TASKS)
	{
        Task *task = &tasks[pid];
        if (task->state == TASK_FREE || task->state == TASK_STOP)
            return;
        task->state = TASK_STOP;
        task_free(pid);
        sched_yield();
    }
}

/* TODO: Lab 5
 * In this function, you have several things todo
 *
 * 1. Use task_create() to create an empty task, return -1
 *    if cannot create a new one.
 *
 * 2. Copy the trap frame of the parent to the child
 *
 * 3. Copy the content of the old stack to the new one,
 *    you can use memcpy to do the job. Remember all the
 *    address you use should be virtual address.
 *
 * 4. Setup virtual memory mapping of the user prgram 
 *    in the new task's page table.
 *    According to linker script, you can determine where
 *    is the user program. We've done this part for you,
 *    but you should understand how it works.
 *
 * 5. The very important step is to let child and 
 *    parent be distinguishable!
 *
 * HINT: You should understand how system call return
 * it's return value.
 */

//
// Lab6 TODO:
//
// Modify it so that the task will disptach to different cpu runqueue
// (please try to load balance, don't put all task into one cpu)
//
int sys_fork()
{
    /* pid for newly created process */
    int pid, i;
    Task *new_task;
    if ((uint32_t)cur_task)
    {
        pid = task_create();
        if (pid < 0)     // create task error
            return -1;
        
        /* Step 2: copy parents' frames to children's */
        new_task = &tasks[pid];
        new_task->tf = cur_task->tf;
        
        /* Step 3: copy the old stack to the new one */
        for (i = USTACKTOP - USR_STACK_SIZE; i < USTACKTOP; i += PGSIZE) {
            struct PageInfo* dst_page;
            uint32_t dst_addr;

            dst_page = page_lookup(new_task->pgdir, i, NULL);
            dst_addr = page2kva(dst_page);
            
            memcpy(dst_addr, i, PGSIZE);
        }
        /* Step 4: All user program use the same code for now */
        setupvm(tasks[pid].pgdir, (uint32_t)UTEXT_start, UTEXT_SZ);
        setupvm(tasks[pid].pgdir, (uint32_t)UDATA_start, UDATA_SZ);
        setupvm(tasks[pid].pgdir, (uint32_t)UBSS_start, UBSS_SZ);
        setupvm(tasks[pid].pgdir, (uint32_t)URODATA_start, URODATA_SZ);       
        /* Step 5: dispatch return value */
        /* Returned value for child process */
        new_task->tf.tf_regs.reg_eax = 0;
        
        /* Return to parent process */
        return pid; // return child pid to parent
    }
    return -1;  // fork error 
}

/* TODO: Lab5
 * We've done the initialization for you,
 * please make sure you understand the code.
 */
void task_init()
{
    extern int user_entry();
    int i;
    UTEXT_SZ = (uint32_t)(UTEXT_end - UTEXT_start);
    UDATA_SZ = (uint32_t)(UDATA_end - UDATA_start);
    UBSS_SZ = (uint32_t)(UBSS_end - UBSS_start);
    URODATA_SZ = (uint32_t)(URODATA_end - URODATA_start);

	/* Initial task sturcture */
	for (i = 0; i < NR_TASKS; i++)
	{
		memset(&(tasks[i]), 0, sizeof(Task));
		tasks[i].state = TASK_FREE;

	}
	task_init_percpu();
}

// Lab6 TODO
//
// Please modify this function to:
//
// 1. init idle task for non-booting AP 
//    (remember to put the task in cpu runqueue) 
//
// 2. init per-CPU Runqueue
//
// 3. init per-CPU system registers
//
// 4. init per-CPU TSS
//
void task_init_percpu()
{
	int i;
	extern int user_entry();
	extern int idle_entry();
	
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
    struct tss_struct *tss = &thiscpu->cpu_tss;

	memset(tss, 0, sizeof(struct tss_struct));
	tss->ts_esp0 = (uint32_t)percpu_kstacks[cpunum()] + KSTKSIZE;
	tss->ts_ss0 = GD_KD;

	// fs and gs stay in user data segment
	tss->ts_fs = GD_UD | 0x03;
	tss->ts_gs = GD_UD | 0x03;

	/* Setup TSS in GDT */
	gdt[(GD_TSS0 >> 3) + cpunum()] = SEG16(STS_T32A, (uint32_t)(tss), sizeof(struct tss_struct), 0);
	gdt[(GD_TSS0 >> 3) + cpunum()].sd_s = 0;

	/* Setup first task */
	i = task_create();
	cur_task = &(tasks[i]);
    
    printk("[CPUID %d] create_task id = %d, parent_id = %d\n", thiscpu->cpu_id, cur_task->task_id, cur_task->parent_id);
    /* For user program */
    setupvm(cur_task->pgdir, (uint32_t)UTEXT_start, UTEXT_SZ);
    setupvm(cur_task->pgdir, (uint32_t)UDATA_start, UDATA_SZ);
    setupvm(cur_task->pgdir, (uint32_t)UBSS_start, UBSS_SZ);
    setupvm(cur_task->pgdir, (uint32_t)URODATA_start, URODATA_SZ);
    
    if (thiscpu->cpu_id == 0) 
        cur_task->tf.tf_eip = (uint32_t)user_entry;
    else
        cur_task->tf.tf_eip = (uint32_t)idle_entry;
    thiscpu->cpu_task = cur_task;	
	/* Load GDT&LDT */
	lgdt(&gdt_pd);


	lldt(0);

	// Load the TSS selector 
    printk("[%s] TSS selector 0x%x user_entry = 0x%x\n", __func__, GD_TSS0 + (cpunum() << 3), &user_entry);
	ltr(GD_TSS0 + (cpunum() << 3));

	cur_task->state = TASK_RUNNING;
}
