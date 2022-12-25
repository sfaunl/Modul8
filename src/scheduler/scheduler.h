/*  @file       scheduler.h
*   @brief      Task scheduler library
*   @details    A simple non-preemptive task scheduler.
*   @author     Sefa Unal
*   @version    0.1
*   @date       13/12/2022
*
*   @since		v0.1 (13/12/2022) : Initial release
*/

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define SCHEDULER_PERIOD_MAX (__UINT32_MAX__)
#define SCHEDULER_PERIOD_HZ(Hz) ((Hz == 0) ? SCHEDULER_PERIOD_MAX : 1000000000 / Hz)
#define SCHEDULER_PERIOD_MS(ms) ((ms) * (uint64_t)1000000)
#define SCHEDULER_PERIOD_US(us) ((us) * (uint64_t)1000)
#define SCHEDULER_PERIOD_NS(ns) (ns)

typedef enum{
	SCHEDULER_TASK_STATE_PAUSED,
	SCHEDULER_TASK_STATE_RUNNING,
	SCHEDULER_TASK_STATE_SCHEDULED,
	SCHEDULER_TASK_STATE_FINISHED,
	SCHEDULER_TASK_STATE_EXITED,
	SCHEDULER_TASK_STATE_ERROR,
	SCHEDULER_N_TASK_STATES
} Scheduler_TaskState;

typedef struct Scheduler Scheduler;
typedef struct Scheduler_Task Scheduler_Task;

typedef int (*userTaskFn)(Scheduler_Task *taskInfo, void *funcArg);

struct Scheduler_Task
{
	char 						name[16];	    // Task name.
	uint16_t 					ID;			    // Task ID
	Scheduler_TaskState 		status;			// Task status (running, scheduled, paused etc...)
	// Task timing metrics
	uint64_t					lastExecTime;	// Tasks last execution time in ns
	uint64_t 					nextExecTime;	// Time at which the task should arrive in the ready queue.
	uint32_t 					period;			// Task period in ns
	// Task function
	userTaskFn                  pFunction;      // Pointer to task function
    void 	                    *funcArgument;  // Pointer to task function argument
};

/** @brief Creates the handle for the scheduler
 *
 * @pre Returns NULL pointer on error.
 *
 * Typical usage example:
 * @code
 * #include <time.h>
 * uint64_t get_time() {
 *  struct timespec ts;
 *  if (timespec_get(&ts, TIME_UTC) != TIME_UTC) return 0;
 *  return 1e9 * ts.tv_sec + ts.tv_nsec;
 * }
 * int printout(Scheduler_Task* task, void* userArg) {
 *  static int counter = 0;
 *  printf("%s[%d]: %d\n", task->name, task->ID, counter++);
 * }
 * Scheduler* scheduler = scheduler_open(get_time);
 * scheduler_task_new(scheduler, "TaskName", SCHEDULER_PERIOD_MS(1000), printout, NULL);
 * scheduler_run(scheduler);
 * @endcode
 *
 * @param[in]   get_time_ns : Pointer to a function that returns current time in nanoseconds as uint64_t 
 *
 * @return                  Pointer for the scheduler handle.
 * @retval      NULL        : Error
 */
Scheduler *scheduler_open(uint64_t (*get_time_ns)(void));

/** @brief Runs the created tasks in an infinite loop
 *
 * @param[in]   handler     : Pointer to the scheduler handle 
 */
void scheduler_run(Scheduler *handler);

/** @brief Creates a new task
 *
 * @param[in]   handler     : Pointer to the scheduler handle 
 * @param[in]   name        : Task name, max 15 characters
 * @param[in]   period_ns   : Period of the current task. Set to 0, if task runs once 
 * @param[in]   userFn      : Pointer to the user function 
 * @param[in]   userArg     : Argument passed to the user function 
 *
 * @return                  Task ID
 * @retval      0           : Error
 */
uint16_t scheduler_task_new (Scheduler 	*handler,
                            char 		*name,
                            uint64_t 	period_ns,
                            userTaskFn 	userFn,
                            void 		*userArg);

/** @brief Removes a task
 *
 * @param[in]   handler     : Pointer to the scheduler handle 
 * @param[in]   taskID      : Task ID to be removed
 */
void scheduler_task_remove (Scheduler *handler, uint16_t taskID);

#endif /* SCHEDULER_H_ */
