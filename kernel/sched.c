#include <kernel/task.h>
#include <inc/x86.h>

static int index = 0;
void sched_yield(void)
{
	/* Lab4: Implement a simple round-robin scheduling there */
	extern Task tasks[];
	extern Task *cur_task;
	int i;
	int next_i = 0;

	i = (index +1)%NR_TASKS;
	while (1)
	{
		if ((tasks[i].state == TASK_RUNNABLE))
		{
			next_i = i;
			break;
		}
		if (++i >= NR_TASKS) i = 0;

		if (index == i)
		{
			next_i = -1;
			break;
		}
		

	}

	if (next_i == -1 ) //only one task can run
		next_i = index;

	if (next_i >= 0 && next_i < NR_TASKS)
	{
		cur_task = &(tasks[next_i]);
		cur_task->remind_ticks = TIME_QUANT;
		cur_task->state = TASK_RUNNING;
		index = next_i;
    lcr3(PADDR(cur_task->pgdir));
		env_pop_tf(&(cur_task->tf));
	}

}
