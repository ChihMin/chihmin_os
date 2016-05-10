#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/mem.h>
#include <kernel/syscall.h>
#include <kernel/trap.h>
#include <inc/stdio.h>

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

extern Task * cur_task;
int32_t do_syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t retVal = -1;
	extern Task *cur_task;

	switch (syscallno)
	{
	case SYS_fork:
		/* TODO: Lab 5
         * You can reference kernel/task.c, kernel/task.h
         */
        retVal = sys_fork();
		break;

	case SYS_getc:
		retVal = do_getc();
		break;

	case SYS_puts:
		do_puts((char*)a1, a2);
		retVal = 0;
		break;

	case SYS_getpid:
		/* TODO: Lab 5
         * Get current task's pid
         */
        if (cur_task != NULL)
            retVal = cur_task->task_id;
		else
            retVal = -1;
        break;

	case SYS_sleep:
		/* TODO: Lab 5
         * Yield this task
         * You can reference kernel/sched.c for yielding the task
         */
        //printk("[PID %d] Sleep %d ticks\n", cur_task->task_id, a1);
        cur_task->remind_ticks = sys_get_ticks() + a1;
        //cur_task->state = TASK_RUNNABLE;
        sched_yield();
		break;

	case SYS_kill:
		/* TODO: Lab 5
         * Kill specific task
         * You can reference kernel/task.c, kernel/task.h
         */
        sys_kill(a1);
		break;

    case SYS_get_num_free_page:
        /* TODO: Lab 5
         * You can reference kernel/mem.c
         */
        retVal = sys_get_num_free_page();
        break;

    case SYS_get_num_used_page:
        /* TODO: Lab 5
         * You can reference kernel/mem.c
         */
        retVal = sys_get_num_used_page();
        break;

    case SYS_get_ticks:
        /* TODO: Lab 5
         * You can reference kernel/timer.c
         */
        retVal = sys_get_ticks();
        break;

    case SYS_settextcolor:
        /* TODO: Lab 5
         * You can reference kernel/screen.c
         */
        sys_settextcolor(a1, a2);
        break;

    case SYS_cls:
        /* TODO: Lab 5
         * You can reference kernel/screen.c
         */
        sys_cls();
        break;

    case SYS_test:
        retVal = 0x12345678;
        break;
    }
    return retVal;
}

static void syscall_handler(struct Trapframe *tf)
{
	/* TODO: Lab5
   * call do_syscall
   * Please remember to fill in the return value
   * HINT: You have to know where to put the return value
   */
    //int32_t do_syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
    uint32_t syscallno, a1, a2, a3, a4, a5;
    int32_t ret_value;
    syscallno = tf->tf_regs.reg_eax;
    a1 = tf->tf_regs.reg_edx;
    a2 = tf->tf_regs.reg_ecx;
    a3 = tf->tf_regs.reg_ebx;
    a4 = tf->tf_regs.reg_edi;
    a5 = tf->tf_regs.reg_esi; 
    ret_value = do_syscall(syscallno, a1, a2, a3, a4, a5);
    tf->tf_regs.reg_eax = ret_value;
}

void syscall_init()
{
  /* TODO: Lab5
   * Please set gate of system call into IDT
   * You can leverage the API register_handler in kernel/trap.c
   */
    extern void SYSCALL_ISR();
    register_handler(T_SYSCALL, &syscall_handler, &SYSCALL_ISR, 1, 3);
}

