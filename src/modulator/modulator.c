#include "../app.h"

int modulator_run(void *userArg)
{
    App *app = (App*)userArg;

    if(app->keepRunning)
    {
    }
    else
    {
        return 0;
    }
    
    return 1;
}