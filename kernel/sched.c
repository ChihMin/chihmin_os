#include <kernel/task.h>
#include <kernel/cpu.h>
#include <inc/x86.h>

#define ctx_switch(ts) \
  do { env_pop_tf(&((ts)->tf)); } while(0)

/* TODO: Lab5
* Implement a simple round-robin scheduler (Start with the next one)
*
* 1. You have to remember the task you picked last time.
*
* 2. If the next task is in TASK_RUNNABLE state, choose
*    it.
*
* 3. After your choice set cur_task to the picked task
*    and set its state, remind_ticks, and change page
*    directory to its pgdir.
*
* 4. CONTEXT SWITCH, leverage the macro ctx_switch(ts)
*    Please make sure you understand the mechanism.
*/
extern Task * cur_task;
extern Task tasks[];

//
// TODO: Lab6
// Modify your Round-robin scheduler to fit the multi-core
// You should:
//
// 1. Design your Runqueue structure first (in kernel/task.c)
//
// 2. modify sys_fork() in kernel/task.c ( we dispatch a task
//    to cpu runqueue only when we call fork system call )
//
// 3. modify sys_kill() in kernel/task.c ( we remove a task
//    from cpu runqueue when we call kill_self system call
//
// 4. modify your scheduler so that each cpu will do scheduling
//    with its runqueue
//    
//    (cpu can only schedule tasks which in its runqueue!!) 
//    (do not schedule idle task if there are still another process can run)	
//
void sched_yield(void)
{
    return;
    Task *next_task = NULL;
    Task *last_task = cur_task;
    int i;
 
    int start = (cur_task->task_id + 1) % NR_TASKS;
    //printk("[%s] start task id = %d\n", __func__, cur_task->task_id);
    
    if (cur_task->state == TASK_RUNNING)
        cur_task->state = TASK_RUNNABLE;
    
    while (!next_task) {
        if (tasks[start].state == TASK_SLEEP 
            && tasks[start].remind_ticks <= 0)
                tasks[start].state = TASK_RUNNABLE;

        if (tasks[start].state == TASK_RUNNABLE) { 
            next_task = &tasks[start];
            next_task->remind_ticks = 100;  // set 100 ticks for one task
        }
        start = (start + 1) % NR_TASKS;
    }
    
    next_task->state = TASK_RUNNING;
    if (next_task != cur_task) {
        cur_task = next_task;
        lcr3(PADDR(next_task->pgdir));
        ctx_switch(next_task);
    }

}
