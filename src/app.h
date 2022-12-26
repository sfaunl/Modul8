#include <time.h>
#include "modulator/modulator.h"

typedef struct App
{
    char    *name;
    int     keepRunning;

    Mod     *mod;
} App;

App *app_init();
int app_run(App *app);