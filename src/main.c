
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include "ui/gui.h"
#include "app.h"
#include "scheduler/scheduler.h"
#include "modulator/modulator.h"

#include <time.h>
uint64_t get_time(){
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) return 0;
    return 1e9 * ts.tv_sec + ts.tv_nsec;
}

App *app = NULL;
void intHandler(int dummy) {
    (void)dummy;
    printf("\nInterrupt signal: Ctrl+C\n");
    app->keepRunning = 0;
}

int task_EXIT(Scheduler_Task* task, void *userArg)
{
    (void)task;
    App *app = (App*)userArg;
    if(!app->keepRunning) 
    {
        fputs("exit 0\n", stderr);
        exit(0);
    }
    return 1;
}

int task_MODULATOR(Scheduler_Task* task, void* userArg)
{
    (void)task;
    return modulator_run(userArg);
}

int task_GUI(Scheduler_Task* task, void* userArg)
{
    (void)task;
    return gui_run(userArg);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    signal(SIGINT, intHandler);

    app = app_init();
    if (!app) return 1;

    init_gui();

    Scheduler *scheduler = scheduler_open(get_time);

    scheduler_task_new(scheduler, "EXIT", SCHEDULER_PERIOD_HZ(1), task_EXIT, app);
    scheduler_task_new(scheduler, "GUI", SCHEDULER_PERIOD_HZ(30), task_GUI, app);
    scheduler_task_new(scheduler, "MOD", 1, task_MODULATOR, app);

    scheduler_run(scheduler);

    printf("Exit");

    return 0;
}
