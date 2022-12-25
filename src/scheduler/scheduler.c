/*
 * scheduler.c
 *
 *  Created on: Dec 13, 2022
 *      Author: Sefa Unal
 */

#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"
#include "linkedlist.h"

// TODO: Rate Monotonic Scheduling, Rate Monotonic Analysis

typedef struct Scheduler
{
	List 		*taskList;
	uint16_t	taskIDindex;

	uint64_t	(*get_time_ns)(void);
	uint64_t 	get_time_ns_burst_time;

} Scheduler;

char *task_state_str[] = {
	"PAUSED",
	"RUNNING",
	"SCHEDULED",
	"FINISHED",
	"ERROR"
};

char *task_priority_str[] = {
	"REALTIME",
	"HIGH",
	"NORMAL",
	"LOW"
};

char *task_type_str[] = {
	"RUNONCE",
	"PERIODIC"
	//"CONTINUOUS"
};

// static uint64_t scheduler_get_time_ns_start(Scheduler *handler)
// {
// 	return handler->get_time_ns();
// }

static uint64_t scheduler_get_time_ns_end(Scheduler *handler)
{
	return handler->get_time_ns() + handler->get_time_ns_burst_time;
}

Scheduler *scheduler_open(uint64_t (*get_time_ns)(void))
{
	Scheduler *handler = calloc (1, sizeof (Scheduler));
    if (!handler) return 0;
	if (!get_time_ns) return 0;


    handler->taskList = list_open();
    if (handler->taskList == 0)
    {
    	free (handler);
    	return 0;
    }
    handler->taskIDindex = 1;	// taskIDs start from 1

	handler->get_time_ns = get_time_ns;
    uint64_t timeBefore_ns = handler->get_time_ns();
    uint64_t timeNow_ns = handler->get_time_ns();
    handler->get_time_ns_burst_time = timeBefore_ns - timeNow_ns;

    return handler;
}

static int tasklist_run_loop (void *aTask, void *handler)
{
	Scheduler_Task *task = (Scheduler_Task*)aTask;

	uint64_t currentTime_ns = scheduler_get_time_ns_end(handler);
	if (task->status == SCHEDULER_TASK_STATE_SCHEDULED)
	{
		if (task->nextExecTime == 0)
		{
			task->nextExecTime = currentTime_ns;
			// task->metrics.arrivalTime = currentTime_ns;
		}
		// Get current time and check if current task needs to be executed
		int64_t timeDelta = currentTime_ns - task->nextExecTime;
		if (timeDelta >= 0)
		{
			// Execute the current task

			// Set the task as running
			task->status = SCHEDULER_TASK_STATE_RUNNING;

			// Get current time
			task->lastExecTime = scheduler_get_time_ns_end(handler);

			//
			// Call the function
			//
			uint8_t taskResult = task->pFunction(task, task->funcArgument);
			//
			//
			//

			// // Get current time
			// uint64_t timeAfterTaskExecution = scheduler_get_time_ns_end ();
			// // Calculate some metrics after calling the task
			// calculate_metrics (handler, task, timeAfterTaskExecution);

			if (!taskResult)
			{
				// Task returned a 0 value, pause the task
				task->status = SCHEDULER_TASK_STATE_PAUSED;
			}
			else if (task->period)
			{
				// Set the task as scheduled so it can be called again
				task->status = SCHEDULER_TASK_STATE_SCHEDULED;
				task->nextExecTime += task->period;
			}
			else
			{
				// Task has no period so task is called one time only
				task->status = SCHEDULER_TASK_STATE_FINISHED;
			}
		}
	}

	return 0;
}

void scheduler_run (Scheduler *handler)
{
	// TODO: assert
    if (!handler) return;

	while(1)
	{
		list_loop(handler->taskList, tasklist_run_loop, handler);
	}
}

uint16_t scheduler_task_new (Scheduler 	*handler,
                            char 		*name,
                            uint64_t 	period_ns,
                            userTaskFn 	userFn,
                            void 		*userArg)
{
    if (!handler) return 0;

    // allocate space for new task
    Scheduler_Task *newTask = calloc (1, sizeof (Scheduler_Task));
    if (!newTask) return 0;

	newTask->ID = handler->taskIDindex++;
    newTask->pFunction = userFn;
    newTask->funcArgument = userArg;
    newTask->period = period_ns;
	newTask->status = SCHEDULER_TASK_STATE_SCHEDULED;
	newTask->nextExecTime = 0; // 0 means scheduler will execute the task whenever its ready

    // set task name
    char lTaskNumber[8] = {0};
    if (!name)
    {
    	name = "Task";
    	snprintf (lTaskNumber, sizeof (lTaskNumber) - 1, "-%d", newTask->ID);
    }
    snprintf (newTask->name, sizeof (newTask->name), "%s%s", name, lTaskNumber);

	// add new task
	if (list_append (handler->taskList, newTask) == 0)
	{
        // Error adding new task
		free (newTask);
		return 0;
	}

	return newTask->ID;
}

static int _remove_task (void *task, void *searchID)
{
	// remove node on match
	if (((Scheduler_Task*)task)->ID == *(uint16_t*)searchID) return -1;
	return 0;
}

void scheduler_task_remove (Scheduler *handler, uint16_t taskID)
{
	// TODO: assert
    if (!handler) return;

    // list_loop removes the object from list if callback returns 1
    list_loop(handler->taskList, _remove_task, &taskID);
}
