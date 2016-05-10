#include <kernel/task.h>
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
void sched_yield(void)
{
    //lcr3(PADDR(kern_pgdir));
    Task *next_task = NULL;
    Task *last_task = cur_task;
    int i;
 
    int start = (cur_task->task_id + 1) % NR_TASKS;
    printk("[%s] start task id = %d\n", __func__, cur_task->task_id);

    cur_task->state = TASK_RUNNABLE;
    while (!next_task) {
        if (tasks[start].state == TASK_RUNNABLE) 
            next_task = &tasks[start];
        start = (start + 1) % NR_TASKS;
    }
    
    next_task->state = TASK_RUNNING;
    if (next_task != cur_task) {
        cur_task = next_task;
        lcr3(PADDR(next_task->pgdir));
        ctx_switch(next_task);
    }

}
