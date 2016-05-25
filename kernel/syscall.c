#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/mem.h>
#include <inc/syscall.h>
#include <inc/stdio.h>
#include <inc/trap.h>

extern void sys_settextcolor(unsigned char forecolor, unsigned char backcolor); // kernel/screen.c
extern void sys_cls(); // kernel/screen.c

void do_puts(char *str, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++)
	{
		k_putch(str[i]);
	}
}

int32_t do_getc()
{
	return k_getc();
}

int32_t do_syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    int32_t retVal = -1;
    extern Task *cur_task;
    extern void sched_yield();

    if (syscallno < NSYSCALLS)
    {
        switch (syscallno)
        {
        case SYS_fork:
	        retVal = sys_fork(); //In task.c
	        break;

        case SYS_getc:
	        retVal = do_getc();
	        break;

        case SYS_puts:
	        do_puts((char*)a1, a2);
	        retVal = 0;
	        break;

        case SYS_getpid:
	        /* Lab4: get current task's pid */
	        retVal = cur_task->task_id;
	        break;

        case SYS_sleep:
	        /* Lab4: yield this task */
	        if (cur_task != NULL)
	        {
		        cur_task->remind_ticks = a1;
		        cur_task->state = TASK_SLEEP;
		        sched_yield();
	        }
	        retVal = 0;
	        break;

        case SYS_kill:
	        /* Lab4: kill task */
	        sys_kill(a1);
	        retVal = 0;
	        break;

        case SYS_get_num_free_page:
            retVal = sys_get_num_free_page();
            break;

        case SYS_get_num_used_page:
            retVal = sys_get_num_used_page();
            break;

        case SYS_get_ticks:
            retVal = sys_get_ticks();
            break;

        case SYS_settextcolor:
            sys_settextcolor(a1,a2);
            retVal = 0;
            break;

        case SYS_cls:
            sys_cls(); 
            retVal = 0;
            break;
        
        /* TODO: Lab7 file I/O system call */    
        case SYS_open:
        case SYS_read:    
        case SYS_write:
        case SYS_close:   
        case SYS_lseek:
        case SYS_unlink:
            retVal = -1; //Not yet implemented
            break;

	    }
	}
	    
	return retVal;
}

static void syscall_handler(struct Trapframe *tf)
{
	/* Lab4: call do_syscall */
	int32_t ret = -1;
	ret = do_syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx, tf->tf_regs.reg_ebx, tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);	
	/* Set system return value */
	tf->tf_regs.reg_eax = ret;

}

void syscall_init()
{
	/* Initial syscall trap after trap_init()*/
	/* Register trap handler */
	extern void SYS_ISR();
	register_handler( T_SYSCALL, &syscall_handler, &SYS_ISR, 1, 3);
}

